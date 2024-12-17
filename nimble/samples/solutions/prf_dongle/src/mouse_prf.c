/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "comm_prf.h"
#include "ring_buffer.h"
#include "common.h"
#include "mouse_prf.h"
#include "pan_pri_rf.h"

#define PRF_DBG_PIN                                     0
#define PAIR_ONLY_ONE                                   1

#define DBG_PRF_LOG_ENABLE                              0
#if DBG_PRF_LOG_ENABLE
#define DBG_PRF_LOG printk
#else
#define DBG_PRF_LOG(...)
#endif

#define HOP_ENABLE                                      1
#define __nop()     __asm("nop")
__ramfunc void delay_nop_app(uint32_t times)
{
	while (times--) {
		__nop();
	}
}

volatile uint32_t usr_pid = 0;
struct prf_state_t prf_state;
bool hoping = false;

uint16_t channel_list_private[9] = { 2404, 2412, 2418, 2430, 2438, 2444, 2456, 2474 };
uint16_t channel_public = 2412;
unsigned char read_addr0[6];
unsigned char read_addr1[6];

pan_prf_config_t rx_config =
{
	.work_mode = (prf_mode_t)PRF_WORK_MODE,                                 // PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode = (prf_chip_mode_sel_t)PRF_CHIP_MODE,                        // PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode = (prf_trx_mode_t)PRF_TRX_SELECT,
	.phy = (prf_phy_t)PRF_PHY_SELECT,                                       // PRF_PHY_250K,//PRF_PHY_1M,
	.crc = (prf_crc_sel_t)PRF_CRC,
	.src = (prf_scramble_sel_t)PRF_SCR,
	.mode_conf = (prf_mode_conf_sel_t)PRF_MODE_CONFIG,
	.rx_timeout = 10000,                                                     // us
	.rf_channel = 2412,
	.tx_no_ack = DISABLE,
	.trf_type = (prf_trf_t)PRF_TRANS_TYPE,
	.rx_length = 30,
	.sync_length = 4,
	.crc_include_sync = ENABLE,
	.src_include_sync = ENABLE,
	.sync = { 0x7b, 0x41, 0x29, 0x71 },
	.pid_manual_flag = ENABLE,
	.tx_power = 9,
	.pipe = PRF_PIPE0,
};

panchip_prf_payload_t tx_payload = {
	.data_length = 0,
	.data = { 0 },
};

/* set addr will set the tx addr, if want to ack data from dynamic rx pipe ,rx isr must set tx addr */
__ramfunc void app_prf_set_tx_addr(uint8_t *addr)
{
	#if SET_TX_ADDR_DIRECT
	/* 1.25us */
	uint32_t tx_addr;

	tx_addr = (addr[0]) | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24);

	PRI_RF_WRITE_REG_VALUE(PRI_RF, R05_TX_ADDR_L, L32B, tx_addr);
	#else
	/* 6us no used third param */
	panchip_prf_set_addr(addr, 4, 0, PRI_RF_MODE_SEL_TX);
	#endif

}

void calculate_combo_freq(uint8_t *addr)
{
	/* channel = 8x + yi(x ∈ (0，4), yi from y0 to (y0 + 7) %d)*/
	uint8_t x = addr[0] % 5;
	uint8_t y = addr[2] % 8;

	printf("calculate_combo_freq use ");
	data_printf(addr, 4);
	printf("x = %d y = %d\n", x, y);

	for (uint8_t i = 0; i < 8; i++) {
		channel_list_private[i] = 2402 + 2 * (8 * x + y);
		x = (x + 1) % 5;
		y = (y + 1) % 8;
		if ((channel_list_private[i] == 2432) || (channel_list_private[i] == 2464)) {
			printf("%d abandon\n", channel_list_private[i]);
			channel_list_private[i] = 2402 + 2 * (8 * x + y);
			x = (x + 1) % 5;
		}
	}

	printf("calculate private freq: ");
	for (uint8_t i = 0; i < 8; i++) {
		printf("%d ", channel_list_private[i]);
	}
	channel_list_private[8] = 2412;
	printf("\n");
}

__ramfunc uint8_t panchip_prf_data_rec_fast(panchip_prf_payload_t *p_payload)
{
	uint32_t index, tmp_data;
	uint8_t i;

	p_payload->data_length = ((LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) >> 16) & 0xff);
	/* printk("%d",p_payload->data_length); */
	if ((p_payload->data_length % 4) == 0) {
		index = p_payload->data_length / 4;
	} else {
		index = p_payload->data_length / 4 + 1;
	}

	for (i = 0; i < index; i++) {
		tmp_data = LLHWC_READ32_REG(LIST_RAM_OFST, (data_addr_rx + 4 + i * 4));
		p_payload->data[i * 4] = (uint8_t)tmp_data;
		p_payload->data[i * 4 + 1] = (uint8_t) (tmp_data >> 8);
		p_payload->data[i * 4 + 2] = (uint8_t) (tmp_data >> 16);
		p_payload->data[i * 4 + 3] = (uint8_t) (tmp_data >> 24);
	}
	return p_payload->data_length;
}

__ramfunc void panchip_prf_set_ack_data_fast(panchip_prf_payload_t *p_payload)
{
	uint8_t i;
	uint32_t tmp_data;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, R00_CTL, TX_PAYLOAD_LEN, p_payload->data_length);

	for (i = 0; i < (p_payload->data_length + 3) / 4; i++) {
		tmp_data = (((uint32_t) p_payload->data[i * 4]) | (((uint32_t) p_payload->data[i * 4 + 1]) << 8) |
			    (((uint32_t) p_payload->data[i * 4 + 2]) << 16) | (((uint32_t) p_payload->data[i * 4 + 3]) << 24));
		LLHWC_WRITE32_REG(REG_FILE_OFST, LIST_RAM_OFST + (data_addr_tx + i) * 4, tmp_data);
	}
}

__ramfunc void event_tx_fun(void)
{
//	printf("t");
	if (prf_state.paird_addr_active[0]) {
		static bool addr_first0 = false;
		if (!addr_first0) {
			addr_first0 = true;
			if (memcmp(read_addr0, &prf_state.pair_private_addr[0][0], 6)) {
				printf("addr0!=\n");
				FMC_WriteStream(FLCTL, 0x32000, &prf_state.pair_private_addr[0][0], 6);
			} else {
				printf("addr0==\n");
			}
			#if PAIR_ONLY_ONE
			prf_state.pairing = false;
			/* disable PIPE0 pair scan */
			PRI_RF->R10_RX_ADDR_EN &= ~(1 << 24);
			PRI_RF->R10_RX_ADDR_EN &= ~(1 << 26);
			#endif
		}
	}
	if (prf_state.paird_addr_active[1]) {
		static bool addr_first1 = false;
		if (!addr_first1) {
			addr_first1 = true;
			if (memcmp(read_addr1, &prf_state.pair_private_addr[1][0], 6)) {
				printf("addr1!=\n");
				FMC_WriteStream(FLCTL, 0x32006, &prf_state.pair_private_addr[1][0], 6);
			} else {
				printf("addr1==\n");
			}
		}
	}
	if (hoping) {
		hoping = false;
		printf("hop end @ %d\n", channel_list_private[prf_state.channel_index]);
	}
	panchip_prf_trx_start();
}

__ramfunc void event_rx_fun(void)
{
	uint8_t addr_format[4];
	panchip_prf_payload_t rx_payload;
	/* get rx reveive pipe addr */
	uint8_t pipe_num = PRI_RF->R01_INT & R01_INT_MULTI_RX_ACC_ADR_Msk;

	prf_state.rx_test_cnt++;
	rx_payload.data_length = panchip_prf_data_rec_fast(&rx_payload);

	switch (pipe_num) {
	case 0:
		if ((rx_payload.data[0] == (PAIR_MAGIC & 0xff)) && (rx_payload.data[1] == (PAIR_MAGIC >> 8))) {
			if ((rx_payload.data[2] == PAIR_MOUSE_8K_FLAG) && (!prf_state.paird_addr_active[0])) {
				prf_state.pair_private_addr[0][0] = rx_payload.data[3];
				prf_state.pair_private_addr[0][1] = rx_payload.data[4];
				prf_state.pair_private_addr[0][2] = rx_payload.data[5];
				prf_state.pair_private_addr[0][3] = rx_payload.data[6];
				prf_state.pair_private_addr[0][4] = rx_payload.data[7];
				prf_state.pair_private_addr[0][5] = rx_payload.data[8];

				addr_format[0] = rx_payload.data[8];
				addr_format[1] = rx_payload.data[7];
				addr_format[2] = rx_payload.data[6];
				addr_format[3] = (rx_payload.data[5] & 0xf) | 0x50;

				panchip_prf_set_addr(addr_format, 4, PRF_PIPE1, PRI_RF_MODE_SEL_RX);

				P11 = 1; P11 = 0;
				app_prf_set_tx_addr(rx_config.sync);
				P11 = 1; P11 = 0;

				panchip_prf_payload_t ack_payload;

				ack_payload.data[0] = PAIR_MAGIC & 0xff;
				ack_payload.data[1] = PAIR_MAGIC >> 8;
				ack_payload.data[2] = PAIR_DONGLE_1K_FLAG;
				ack_payload.data[3] = prf_state.own_addr[0];
				ack_payload.data[4] = prf_state.own_addr[1];
				ack_payload.data[5] = prf_state.own_addr[2];
				ack_payload.data[6] = prf_state.own_addr[3];
				ack_payload.data[7] = prf_state.own_addr[4];
				ack_payload.data[8] = prf_state.own_addr[5];
				ack_payload.data_length = 9;
				panchip_prf_set_ack_data_fast(&ack_payload);

			} else if ((rx_payload.data[2] == 0x20) && (!prf_state.paird_addr_active[1])) {
				prf_state.pair_private_addr[1][0] = rx_payload.data[3];
				prf_state.pair_private_addr[1][1] = rx_payload.data[4];
				prf_state.pair_private_addr[1][2] = rx_payload.data[5];
				prf_state.pair_private_addr[1][3] = rx_payload.data[6];
				prf_state.pair_private_addr[1][4] = rx_payload.data[7];
				prf_state.pair_private_addr[1][5] = rx_payload.data[8];

				addr_format[0] = rx_payload.data[8];
				addr_format[1] = rx_payload.data[7];
				addr_format[2] = rx_payload.data[6];
				addr_format[3] = (rx_payload.data[5] & 0xf) | 0x50;

				panchip_prf_set_addr(addr_format, 4, PRF_PIPE2, PRI_RF_MODE_SEL_RX);

				app_prf_set_tx_addr(rx_config.sync);

				panchip_prf_payload_t ack_payload;

				ack_payload.data[0] = PAIR_MAGIC & 0xff;
				ack_payload.data[1] = PAIR_MAGIC >> 8;
				ack_payload.data[2] = PAIR_DONGLE_1K_FLAG;
				ack_payload.data[3] = prf_state.own_addr[0];
				ack_payload.data[4] = prf_state.own_addr[1];
				ack_payload.data[5] = prf_state.own_addr[2];
				ack_payload.data[6] = prf_state.own_addr[3];
				ack_payload.data[7] = prf_state.own_addr[4];
				ack_payload.data[8] = prf_state.own_addr[5];
				ack_payload.data_length = 9;
				panchip_prf_set_ack_data_fast(&ack_payload);
			}
		}
		break;
	case 1:
		addr_format[0] = prf_state.pair_private_addr[0][5];
		addr_format[1] = prf_state.pair_private_addr[0][4];
		addr_format[2] = prf_state.pair_private_addr[0][3];
		addr_format[3] = (prf_state.pair_private_addr[0][2] & 0xf) | 0x50;

		app_prf_set_tx_addr(addr_format);
		panchip_prf_set_ack_data_fast(&tx_payload);
		prf_state.paird_addr_active[0] = true;
		static uint8_t seq_num;

		if (seq_num != rx_payload.data[1]) {
			seq_num = rx_payload.data[1];
			if (rx_payload.data[0] == 0x04) { /* MS data check */
				prf_state.valid_rx_test_cnt++;
				ring_buf_put(&ringbuf_raw, (uint8_t *)rx_payload.data, MOUSE_REPORT_SIZE);
			}
		}


		break;
	case 2:
		addr_format[0] = prf_state.pair_private_addr[1][5];
		addr_format[1] = prf_state.pair_private_addr[1][4];
		addr_format[2] = prf_state.pair_private_addr[1][3];
		addr_format[3] = (prf_state.pair_private_addr[1][2] & 0xf) | 0x50;

		app_prf_set_tx_addr(addr_format);
		panchip_prf_set_ack_data_fast(&tx_payload);
		prf_state.paird_addr_active[1] = true;
		static uint8_t seq_num2;

		if (seq_num2 != rx_payload.data[1]) { /* MS data check */
			seq_num2 = rx_payload.data[1];
			if (rx_payload.data[0] == 0x04) {
				prf_state.valid_rx_test_cnt++;

				ring_buf_put(&ringbuf_raw2, (uint8_t *)rx_payload.data, MOUSE_REPORT_SIZE);
			}
		}


		break;

	default:
		break;
	}
}

__ramfunc void event_rx_timeout_fun(void)
{
	/* lldbg to isr 9us */
	P11 = 1; P11 = 0;
	prf_state.timout_test_cnt++;

	prf_state.channel_index++;
	#if PAIR_ONLY_ONE
	if (prf_state.pairing) {
		prf_state.channel_index = prf_state.channel_index % 9;
	} else {
		prf_state.channel_index = prf_state.channel_index % 8;
	}
	#else
	prf_state.channel_index = prf_state.channel_index % 9;
	#endif

	rx_config.rf_channel = channel_list_private[prf_state.channel_index];

	P11 = 1; P11 = 0;
	/* 50us */
	panchip_prf_set_chn(rx_config.rf_channel);
	P11 = 1; P11 = 0;
	/* 3us */
	panchip_prf_trx_start();
	P11 = 1; P11 = 0;
	hoping = true;
}

__ramfunc void event_crc_err_fun(void)
{
//	printf("c ");

	prf_state.crc_test_cnt++;

	panchip_prf_trx_start();
}

__ramfunc void event_pid_err_fun(void)
{
	printf("rx data pid err\n");
}

__ramfunc void event_rx_len_err_fun(void)
{
//	printf("L ");

	prf_state.rx_len_err_test_cnt++;
	panchip_prf_trx_start();
}

void panchip_prf_isr_init(void)
{
	isr_cb.tx_cb = event_tx_fun;
	isr_cb.rx_cb = event_rx_fun;
	isr_cb.rx_timeout_cb = event_rx_timeout_fun;
	isr_cb.rx_crc_err_cb = event_crc_err_fun;
	isr_cb.rx_pid_err_cb = event_pid_err_fun;
	isr_cb.rx_len_err_cb = event_rx_len_err_fun;
}

void panchip_prf_irq_enable(void)
{
	NVIC_SetPriority(LL_IRQn, 0);
	/* Enable RF interrupt */
	NVIC_EnableIRQ(LL_IRQn);
}

void mouse_prf_cnt_printf(void)
{
	printf("1s rx %d valid %d timeout %d crc %d len err %d\n",
	       prf_state.rx_test_cnt, prf_state.valid_rx_test_cnt, prf_state.timout_test_cnt, prf_state.crc_test_cnt, prf_state.rx_len_err_test_cnt);
	prf_state.rx_test_cnt = 0;
	prf_state.timout_test_cnt = 0;
	prf_state.crc_test_cnt = 0;
	prf_state.rx_len_err_test_cnt = 0;
	prf_state.valid_rx_test_cnt = 0;
}

void get_mac_addr(void)
{
	for (uint8_t i = 0; i < 6; i++) {
		prf_state.own_addr[i] = 0xa0 + i;
	}

	memcpy(prf_state.own_addr, m_chip_mac, 6);

	printf("get_mac_addr: ");
	data_printf(prf_state.own_addr, 6);

	calculate_combo_freq(&prf_state.own_addr[2]);

}

void load_pair_addr(void)
{
	uint8_t addr_format[4];

	for (uint8_t i = 0; i < PAIR_MAX_NUM; i++) {
		prf_state.paird_addr_active[i] = false;
	}
	prf_state.pairing = true;

	/* 200k start save pair addr two */
	FMC_ReadStream(FLCTL, 0x32000, CMD_DREAD, read_addr0, 6);
	FMC_ReadStream(FLCTL, 0x32006, CMD_DREAD, read_addr1, 6);

	data_printf(read_addr0, 6);
	data_printf(read_addr1, 6);
	memcpy(&prf_state.pair_private_addr[0][0], read_addr0, 6);
	memcpy(&prf_state.pair_private_addr[1][0], read_addr1, 6);

	/* set PRI_RF_MODE_SEL_RX addr will set the rx addr & pipe */
	/* set PRI_RF_MODE_SEL_TX addr will set the single tx addr, no set tx will use init tx addr */
	/* set PRI_RF_MODE_SEL_TRX addr will set the trx addr both, no set tx will use init tx addr */

	addr_format[0] = read_addr0[5];
	addr_format[1] = read_addr0[4];
	addr_format[2] = read_addr0[3];
	addr_format[3] = (read_addr0[2] & 0xf) | 0x50;

	panchip_prf_set_addr(addr_format, 4, PRF_PIPE1, PRI_RF_MODE_SEL_RX);

	addr_format[0] = read_addr1[5];
	addr_format[1] = read_addr1[4];
	addr_format[2] = read_addr1[3];
	addr_format[3] = (read_addr1[2] & 0xf) | 0x50;

	panchip_prf_set_addr(addr_format, 4, PRF_PIPE2, PRI_RF_MODE_SEL_RX);
}

void mouse_prf_pair_init(void)
{
	get_mac_addr();
	load_pair_addr();
}

void mouse_prf_init(void)
{
	panchip_prf_init(&rx_config);

	/* init not init channel, must init channel after init */
	panchip_prf_set_chn(rx_config.rf_channel);

	/* adr match bit match whole absolutely */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	/* disable pid err isr */
	panchip_prf_pid_cfg(&rx_config, 0);
	/* enable len err isr */
	panchip_prf_rx_length_irq_cfg(ENABLE);

	#if PRF_DBG_PIN
	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_11, 0x14); // tx on
	SYS->P1_MFP |= SYS_MFP_P10_LL_DBG_11;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_09, 0x15); // rx on
	SYS->P0_MFP |= SYS_MFP_P04_LL_DBG_9;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_10, 0x17); // data
	SYS->P0_MFP |= SYS_MFP_P07_LL_DBG_10;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX03, TST_MUX_SELECT_12, 0x13); // acc match
	SYS->P0_MFP |= SYS_MFP_P05_LL_DBG_12;
	#endif

	mouse_prf_pair_init();

	panchip_prf_trx_start();
}
