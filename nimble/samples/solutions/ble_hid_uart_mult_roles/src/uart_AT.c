/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "uart_AT.h"
#include "PanSeries.h"

fmc_data g_fmc_config_data;

static TaskHandle_t app_ble_task_h;
uint32_t conn_index;
uint32_t disconn_index;
uint8_t default_mac_addr[6] = DEFAULT_MAC_ADDR;
uint8_t default_bond_mac[6] = DEFAULT_BOND_ADDR;
uint8_t RX_Sendata[RX_BUFFER_SIZE] = { 0 };
uint16_t USART_RX_CNT;
extern bool Connect_Status;
uint8_t simulate_vnd;
SemaphoreHandle_t xSemaphore_app;
at_sem_flag_t at_sem_flag;
uint16_t send_data_len;
uint8_t send_data[RX_BUFFER_SIZE] = { 0 };
bool uart_running = false;

void sys_reboot(int type)
{
//	/* Ensure RCH ON before reboot */
//	uint32_t tmp_reg;

//	tmp_reg = CLK->CLK_TOP_CTRL;
//	tmp_reg |= CLK_TOPCTL_RCH_EN_Msk;
//	CLK->CLK_TOP_CTRL = tmp_reg;

	__disable_irq();
	SYS_UnlockReg();
	CLK_ResetChip();
}

void data_printf(uint8_t *data, uint32_t len)
{
	uint32_t i = 0;

	if (len == 0) {
		return;
	}

	for (; i < len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\n");
}

/* Send multiple data to target test Uart */
void TGT_SendMultiData(const uint8_t *data, uint16_t size)
{
	while (size--) {
		TGT_UART->RBR_THR_DLL = *(data++);
		/* Wait until THR is empty to avoid data lost */
		while (!(TGT_UART->LSR & UART_LSR_TEMT_Msk)) {
		}
	}
}

uint8_t num_to_srting(uint32_t num, uint8_t *string_out)
{
	uint8_t i;
	uint8_t length = 0;
	uint32_t temp = num;

	if (temp == 0) {
		string_out[0] = '0';
		return 1;
	}
	for (i = 0; temp > 0; i++) {
		string_out[i] = temp % 10 + '0';
		temp /= 10;
	}
	length = i;
	for (i = 0; i < length / 2; i++) {
		temp = string_out[i];
		string_out[i] = string_out[length - 1 - i];
		string_out[length - 1 - i] = temp;
	}
	return length;
}

uint8_t string_to_num(uint8_t *string_in, uint32_t *num, uint8_t length)
{
	uint8_t i;
	uint32_t temp = 0;

	for (i = 0; i < length; i++) {
		temp *= 10;
		if ((string_in[i] < '0') || (string_in[i] > '9')) {
			return 0;
		}
		temp += (string_in[i] - '0');
	}
	*num = temp;
	return 1;
}

void hex_to_string(uint8_t *buff_num, uint8_t length, uint8_t *buff_str_out)
{
	uint8_t temp;

	for (uint8_t i = 0; i < length; i++) {
		temp = buff_num[i] >> 4;
		if (temp > 9) {
			buff_str_out[(i << 1)] = temp + 'A' - 10;
		} else {
			buff_str_out[(i << 1)] = temp + '0' - 0;
		}
		temp = buff_num[i] & 0x0f;
		if (temp > 9) {
			buff_str_out[(i << 1) + 1] = temp + 'A' - 10;
		} else {
			buff_str_out[(i << 1) + 1] = temp + '0' - 0;
		}
	}
}

uint8_t mac_addr_str_to_hex(unsigned char *str_addr, unsigned char *hex_addr, unsigned int out_hex_length)
{
	unsigned char i, j;

	for (i = 0, j = 0; i < out_hex_length; i++) {
		if ((str_addr[j]  <= '9') && (str_addr[j]  >= '0')) {
			hex_addr[i] = (str_addr[j] - '0') << 4;
		} else if ((str_addr[j]  <= 'f') && (str_addr[j]  >= 'a')) {
			hex_addr[i] = (str_addr[j] - 'a' + 10) << 4;
		} else if ((str_addr[j]  <= 'F') && (str_addr[j]  >= 'A')) {
			hex_addr[i] = (str_addr[j] - 'A' + 10) << 4;
		} else {
			/* hex_addr[i] = 0; */
			return 0;
		}
		j++;
		if ((str_addr[j]  <= '9') && (str_addr[j]  >= '0')) {
			hex_addr[i] += ((str_addr[j] - '0') << 0);
		} else if ((str_addr[j]  <= 'f') && (str_addr[j]  >= 'a')) {
			hex_addr[i] += ((str_addr[j] - 'a' + 10) << 0);
		} else if ((str_addr[j]  <= 'F') && (str_addr[j]  >= 'A')) {
			hex_addr[i] += ((str_addr[j] - 'A' + 10) << 0);
		} else {
			/* hex_addr[i] += 0; */
			return 0;
		}
		j++;
	}
	return 1;
}

void proj_uart_at_handle(uint8_t *data, uint16_t pack_size)
{
	unsigned char buff[128];
	uint8_t num_buff[14];
	uint8_t length;
	uint32_t num = 0;
	bool reset_flag = false;

	memcpy(buff, data, pack_size);

	// struct uart_config uart_cfg_check;

	if (!strncmp((const char *)buff, "AT+BAUD+", sizeof("AT+BAUD+") - 1)) {
		string_to_num(&buff[sizeof("AT+BAUD+") - 1], &num, (pack_size - (sizeof("AT+BAUD+") - 1)));
		// uart_config_get(uart_dev, &uart_cfg_check);
		if (num <= 115200) {
			if (num == g_fmc_config_data.baudrate) {
				TGT_SendMultiData("NO CHANGE BAUD\r\n", sizeof("NO CHANGE BAUD\r\n") - 1);
			} else {
				g_fmc_config_data.baudrate = num;
				FMC_EraseSector(FLCTL, DEFAULT_CONFIG_DATA_ADD);
				SYS_delay_10nop(1000);
				FMC_WriteStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));
				SYS_delay_10nop(1000);
				TGT_SendMultiData("OK+BAUD\r\n", sizeof("OK+BAUD\r\n") - 1);
				reset_flag = true;
			}
		} else {
			TGT_SendMultiData("OVER 115200 REJECT\r\n", sizeof("OVER 115200 REJECT\r\n") - 1);
		}

	} else if (!strncmp((const char *)buff, "AT+BAUD?", sizeof("AT+BAUD?") - 1)) {
		// uart_config_get(uart_dev, &uart_cfg_check);
		length = num_to_srting(g_fmc_config_data.baudrate, num_buff);
		num_buff[length] = '\n';
		TGT_SendMultiData("BAUD+", sizeof("BAUD+") - 1);
		TGT_SendMultiData(num_buff, length + 1);
	} else if (!strncmp((const char *)buff, "AT+PIN+", sizeof("AT+PIN+") - 1)) {
		uint32_t passkey_at;

		string_to_num(&buff[sizeof("AT+PIN+") - 1], &passkey_at, (pack_size - (sizeof("AT+PIN+") - 1)));
		printf("passkey input %d\n", passkey_at);
		g_fmc_config_data.passkey = passkey_at;
		/* bt_passkey_set(g_fmc_config_data.passkey); */
		TGT_SendMultiData("OK+PIN\n", sizeof("OK+PIN\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+PIN?", sizeof("AT+PIN+") - 1)) {
		length = num_to_srting(g_fmc_config_data.passkey, num_buff);
		num_buff[length] = '\n';
		TGT_SendMultiData("PIN+", sizeof("PIN+") - 1);
		TGT_SendMultiData(num_buff, length + 1);
	} else if (!strncmp((const char *)buff, "AT+RESET", sizeof("AT+RESET") - 1)) {
		TGT_SendMultiData("OK+RESET\n", sizeof("OK+RESET\n") - 1);
		reset_flag = true;
	} else if (!strncmp((const char *)buff, "AT+BONDMAC+", sizeof("AT+BONDMAC+") - 1)) {
		if (mac_addr_str_to_hex(&buff[sizeof("AT+BONDMAC+") - 1], g_fmc_config_data.bond_mac, 6)) {
			FMC_EraseSector(FLCTL, DEFAULT_CONFIG_DATA_ADD);
			SYS_delay_10nop(1000);
			FMC_WriteStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));
			SYS_delay_10nop(1000);
			TGT_SendMultiData("OK+BONDMAC\r\n", sizeof("OK+BONDMAC\r\n") - 1);
		} else {
			TGT_SendMultiData("BONDMAC_ERROR\r\n", sizeof("BONDMAC_ERROR\r\n") - 1);
		}
	} else if (!strncmp((const char *)buff, "AT+BONDMAC?", sizeof("AT+BONDMAC?") - 1)) {
		hex_to_string(g_fmc_config_data.bond_mac, 6, num_buff);
		num_buff[12] = '\n';
		TGT_SendMultiData("BONDMAC+", sizeof("BONDMAC+") - 1);
		TGT_SendMultiData(num_buff, 13);
	} else if (!strncmp((const char *)buff, "AT+SETMAC+", sizeof("AT+SETMAC+") - 1)) {
		if (mac_addr_str_to_hex(&buff[sizeof("AT+SETMAC+") - 1], g_fmc_config_data.own_mac, 6)) {
			FMC_EraseSector(FLCTL, DEFAULT_CONFIG_DATA_ADD);
			SYS_delay_10nop(1000);
			FMC_WriteStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));
			SYS_delay_10nop(1000);
			TGT_SendMultiData("OK+SETMAC\r\n", sizeof("OK+SETMAC\r\n") - 1);
			/* Wait until THR is empty to avoid data lost */
			reset_flag = true;
		} else {
			TGT_SendMultiData("MAC_ERROR\r\n", sizeof("MAC_ERROR\r\n") - 1);
		}
	} else if (!strncmp((const char *)buff, "AT+MAC?", sizeof("AT+MAC?") - 1)) {
		hex_to_string(g_fmc_config_data.own_mac, 6, num_buff);
		num_buff[12] = '\n';
		TGT_SendMultiData("MAC+", sizeof("MAC+") - 1);
		TGT_SendMultiData(num_buff, 13);
	} else if (!strncmp((const char *)buff, "AT+NAME?", sizeof("AT+NAME?") - 1)) {
		TGT_SendMultiData(g_fmc_config_data.device_name, g_fmc_config_data.name_length);
	} else if (!strncmp((const char *)buff, "AT+SETNAME+", sizeof("AT+SETNAME+") - 1)) {
		g_fmc_config_data.name_length = pack_size - (sizeof("AT+SETNAME+") - 1);
		if (g_fmc_config_data.name_length > 28) {
			g_fmc_config_data.name_length = 28;
		}
		memset(g_fmc_config_data.device_name, 0, sizeof(g_fmc_config_data.device_name));
		memcpy(g_fmc_config_data.device_name, &buff[(sizeof("AT+SETNAME+") - 1)], g_fmc_config_data.name_length);
		FMC_EraseSector(FLCTL, DEFAULT_CONFIG_DATA_ADD);
		SYS_delay_10nop(1000);
		FMC_WriteStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));
		SYS_delay_10nop(1000);
		TGT_SendMultiData("OK+SETNAME\n", sizeof("OK+SETNAME\n") - 1);
		reset_flag = true;
	} else if (!strncmp((const char *)buff, "AT+DEFAULT", sizeof("AT+DEFAULT") - 1)) {
		default_data_init();
		FMC_EraseSector(FLCTL, DEFAULT_CONFIG_DATA_ADD);
		SYS_delay_10nop(1000);
		FMC_WriteStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));
		SYS_delay_10nop(1000);
		TGT_SendMultiData("OK+DEFAULT\n", sizeof("AT+DEFAULT\n") - 1);
		reset_flag = true;
	} else if ((!strncmp((const char *)buff, "AT", sizeof("AT") - 1))
		   && (pack_size == 2)) { /* LAST ONE */
		TGT_SendMultiData("AT+OK\n", sizeof("AT+OK\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+ADV START", sizeof("AT+ADV START") - 1)) {
		// k_work_init(&adv_start, adv_start_work);
		// k_work_submit(&adv_start);
		TGT_SendMultiData("OK+ADV START\n", sizeof("OK+ADV START\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+ADV STOP", sizeof("AT+ADV STOP") - 1)) {
		// k_work_init(&adv_stop, adv_stop_work);
		// k_work_submit(&adv_stop);
		TGT_SendMultiData("OK+ADV STOP\n", sizeof("OK+ADV STOP\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+SCAN START", sizeof("AT+SCAN START") - 1)) {
		printf("AT+SCAN START\n");
		at_sem_flag = AT_SCAN;
		xSemaphoreGive(xSemaphore_app);
		TGT_SendMultiData("OK+SCAN START\n", sizeof("OK+SCAN START\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+SCAN STOP", sizeof("AT+SCAN STOP") - 1)) {
		// k_work_init(&scan_stop, scan_stop_work);
		// k_work_submit(&scan_stop);
		TGT_SendMultiData("OK+SCAN STOP\n", sizeof("OK+SCAN STOP\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+CONN ", sizeof("AT+CONN ") - 1)) {
		string_to_num(&buff[sizeof("AT+CONN ") - 1], &conn_index, (pack_size - (sizeof("AT+CONN ") - 1)));
		printf("conn_index %d\n", conn_index);
		// k_work_init(&conn_start, conn_start_work);
		// k_work_submit(&conn_start);
		TGT_SendMultiData("OK+CONN\n", sizeof("OK+CONN\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+DISCONN ", sizeof("AT+DISCONN ") - 1)) {
		string_to_num(&buff[sizeof("AT+DISCONN ") - 1], &disconn_index, (pack_size - (sizeof("AT+DISCONN ") - 1)));
		printf("disconn_index %d\n", disconn_index);
		// k_work_init(&disconn_start, disconn_start_work);
		// k_work_submit(&disconn_start);
		TGT_SendMultiData("OK+DISCONN\n", sizeof("OK+DISCONN\n") - 1);
	} else if (!strncmp((const char *)buff, "AT+DEV SHOW", sizeof("AT+DEV SHOW") - 1)) {
		string_to_num(&buff[sizeof("AT+CONN ") - 1], &conn_index, (pack_size - (sizeof("AT+CONN ") - 1)));
		// uart_devs_show();
		TGT_SendMultiData("OK+DEV SHOW\n", sizeof("OK+DEV SHOW\n") - 1);
	} else {
		TGT_SendMultiData("AT+ERROR\n", sizeof("AT+ERROR\n") - 1);
	}

	if (reset_flag) {
		TGT_SendMultiData("RESETTING...\n", sizeof("RESETTING...\n") - 1);
		while (!(TGT_UART->LSR & UART_LSR_TEMT_Msk)) {
		}
		sys_reboot(0);
	}
}

// void ble_hid_uart_irq_rx_ready(const struct device *dev)
// {
//	UART_T *uart_inst = (UART_T *)(*(uint32_t *)dev->config);
//	uint8_t rec_num;

//	if (uart_panchip_evt_cache_get(uart_inst) == UART_EVENT_TIMEOUT) {
//		rec_num = TGT_UART->RFL;
//		for (uint8_t i = 0; i < rec_num; i++) {
//			RX_Sendata[USART_RX_CNT++] = UART_ReceiveData(TGT_UART);
//		}

//		if (RX_Sendata[0] == 0x5a) {
//			printf("count_connected %d\n", count_connected);
//			for (uint8_t i = 0; i < 8; i++) {
//				if (RX_Sendata[1] & (0x1 << i)) {
//					struct bt_conn_info info;

//					bt_conn_get_info(list_connected[i], &info);
//					printf("connect index %d send\n", i);
//					if (info.role == BT_CONN_ROLE_PERIPHERAL) {
//						if (peri_connected) {
//							bt_ven_notify(&RX_Sendata[2], USART_RX_CNT - 2);
//							printf("central sent\n");
//						} else {
//							printf("central no connect\n");
//						}
//					} else {
//						if (central_connected[i]) {
//							write_to_peripheral_from_addr(list_connected[i], &RX_Sendata[2], USART_RX_CNT - 2);
//							printf("peri %d sent\n", i);
//						} else {
//							printf("peri %d no connect\n", i);
//						}
//					}
//				}
//			}
//		} else if ((RX_Sendata[USART_RX_CNT - 2] == '\r') && (RX_Sendata[USART_RX_CNT - 1] == '\n')) {
//			proj_uart_at_handle(RX_Sendata, (USART_RX_CNT - 2));
//		}
//		USART_RX_CNT = 0;
//	}

//	if (uart_panchip_evt_cache_get(uart_inst) == UART_EVENT_DATA) {
//		rec_num = TGT_UART->RFL;
//		for (uint8_t i = 0; i < (rec_num - 1); i++) {
//			RX_Sendata[USART_RX_CNT++] = UART_ReceiveData(TGT_UART);
//		}
//	}
// }

// static void uart_fifo_callback(const struct device *dev, void *user_data)
// {
//      ARG_UNUSED(user_data);
//      uart_running = true;

//      /* Verify uart_irq_update() */
//      if (!uart_irq_update(dev)) {
//              printf("retval should always be 1\n");
//              return;
//      }

//      ble_hid_uart_irq_rx_ready(dev);

//      uart_running = false;
// }

void UART_HandleProc(UART_T *UARTx)
{
	uart_running = true;
	uint8_t rec_num;

	UART_EventDef event = UART_GetActiveEvent(UARTx);

	switch (event) {
	// case UART_EVENT_LINE:
	//      UART_HandleLineStatus(UARTx);
	//      break;
	case UART_EVENT_DATA:
//		printf("Rx thres trig!\n");
		// UART_HandleReceivedData(UARTx);
		// uartRxIntFlag = true;
		rec_num = TGT_UART->RFL;
		for (uint8_t i = 0; i < (rec_num - 1); i++) {
			RX_Sendata[USART_RX_CNT++] = UART_ReceiveData(TGT_UART);
		}
		break;
	case UART_EVENT_TIMEOUT:
//		printf("Rx tout trig!\n");
		// UART_HandleReceivedData(UARTx);
		// uartRxIntFlag = true;

		rec_num = TGT_UART->RFL;
		for (uint8_t i = 0; i < rec_num; i++) {
			RX_Sendata[USART_RX_CNT++] = UART_ReceiveData(TGT_UART);
		}

		if (RX_Sendata[0] == 0x5a) {
//			printf("count_connected %d\n", count_connected);
			printf("data send len %d\n", USART_RX_CNT);

			memcpy(send_data, RX_Sendata, USART_RX_CNT);
			send_data_len = USART_RX_CNT;

			at_sem_flag = AT_SEND_DATA;
			xSemaphoreGive(xSemaphore_app);

		} else if ((RX_Sendata[USART_RX_CNT - 2] == '\r') && (RX_Sendata[USART_RX_CNT - 1] == '\n')) {
			proj_uart_at_handle(RX_Sendata, (USART_RX_CNT - 2));
		}
		USART_RX_CNT = 0;

		break;
	case UART_EVENT_NONE:
		/* Just ignore this event. */
		break;
	default:
		printf("UART Handler Error, is not expected running to here! IID: 0x%x\n", event);
		break;
	}
	uart_running = false;
}

void UART1_IRQHandler(void)
{
	UART_HandleProc(UART1);
}

void default_data_init(void)
{
	memcpy(g_fmc_config_data.own_mac, default_mac_addr, 6);

	g_fmc_config_data.baudrate = 115200;/* Baudrate */
	memset(g_fmc_config_data.device_name, 0, sizeof(g_fmc_config_data.device_name));
	memcpy(&g_fmc_config_data.device_name[0], CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME));
	g_fmc_config_data.name_length = sizeof(CONFIG_BT_DEVICE_NAME) - 1;
	memcpy(g_fmc_config_data.bond_mac, default_bond_mac, 6);
	g_fmc_config_data.passkey = 123456;
	g_fmc_config_data.rst_flag = 0x55555555;
}

void load_config(void)
{
	FMC_ReadStream(FLCTL, DEFAULT_CONFIG_DATA_ADD, CMD_DREAD, (unsigned char *)&g_fmc_config_data, sizeof(g_fmc_config_data));

	printf("\n");
	if (g_fmc_config_data.rst_flag == 0x55555555) {/* flash */
		printf("store_data_init\n");
	} else {
		printf("default_data_init\n");
		default_data_init();
	}
	printf("Baudrate : %d\n", g_fmc_config_data.baudrate);
	printf("Own_mac : ");
	data_printf(g_fmc_config_data.own_mac, 6);
	printf("Bond_mac : ");
	data_printf(g_fmc_config_data.bond_mac, 6);
	printf("Name_length : %d\n", g_fmc_config_data.name_length);
	printf("Device_name : ");
	for (uint8_t i = 0; i < g_fmc_config_data.name_length; i++) {
		printf("%c", g_fmc_config_data.device_name[i]);
	}
	printf("\n");
	printf("Passkey : %d\n", g_fmc_config_data.passkey);
	printf("\n");
}

void uart_at_init(void)
{
	load_config();

	/* Unlock protected registers */
	SYS_UnlockReg();

	CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

	GPIO_EnableDigitalPath(P2, BIT4);
	SYS_SET_MFP(P1, 0, UART1_TX);
	SYS_SET_MFP(P2, 4, UART1_RX);

	/* Relock protected registers */
//	SYS_LockReg();

	UART_InitTypeDef Init_Struct;

	Init_Struct.UART_BaudRate = g_fmc_config_data.baudrate;
	Init_Struct.UART_LineCtrl = Uart_Line_8n1;

	/* Init UART0 for printf */
	UART_Init(UART1, &Init_Struct);
	UART_EnableFifo(UART1);

	UART_SetRxTrigger(UART1, UART_RX_FIFO_HALF_FULL);
	UART_EnableIrq(UART1, UART_IRQ_RECV_DATA_AVL);          // Enable RDA Interrupt
//	UART_EnableIrq(UART1, UART_IRQ_LINE_STATUS);         // Enable RLS Interrupt
	NVIC_EnableIRQ(UART1_IRQn);                             // Enable target UART INT in NVIC
}

// void app_ble_npl_event_run(struct ble_npl_event *ev)
// {
//    ev->fn(ev);
// }

// static struct ble_npl_eventq g_eventq_app_ble;

void app_ble_thread_entry(void *parameter)
{
//    struct ble_npl_event *ev;
//    printf("a\n");
//    ble_npl_eventq_init(&g_eventq_app_ble);
//    printf("b\n");
//    ble_npl_event_init(&app_scan_start, app_scan_start_cb, NULL);
//    printf("c\n");
//    ble_npl_eventq_put(ble_app_evq, &app_scan_start);
//    printf("d\n");
	while (1) {
		xSemaphoreTake(xSemaphore_app, portMAX_DELAY);
		switch (at_sem_flag) {
		case AT_SCAN:
			printf("AT_SCAN\n");
			/* Begin scanning for a peripheral to connect to. */
			blecent_scan();
			break;
		case AT_SEND_DATA:
			printf("AT_SEND_DATA\n");
//			data_printf(&send_data[2], send_data_len - 2);
			if (send_data[1] & (0x1 << 0)) {
				printf("AT_WRITE_PERI\n");
				central_write(&send_data[2], send_data_len - 2);
			}
			if (RX_Sendata[1] & (0x1 << 1)) {
				printf("AT_NOTIFY_CENTRAL\n");
				peri_notify(&send_data[2], send_data_len - 2);
			}
			break;
		default:
			break;
		}
	}
}

void app_ble_thread_init(void)
{
	xTaskCreate(app_ble_thread_entry, "app", MYNEWT_VAL(APP_BLE_THREAD_STACK_SIZE),
		    NULL, configMAX_PRIORITIES - 2, &app_ble_task_h);

	xSemaphore_app = xSemaphoreCreateBinary();
}
