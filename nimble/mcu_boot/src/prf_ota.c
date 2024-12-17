#include "prf_ota.h"
#include "PANSeries.h"
#include "comm_prf.h"
#include "flash_manager.h"
#include "info.h"

#define printf(...)

#define PKT_MASK_TBL_LEN		(128 * 4)
#define PRF_OTA_PKT_LEN			250
#define PRF_BIT_MASK_WIDTH	    32

pan_prf_config_t prf_config = 
{
	.work_mode			= PRF_MODE_ENHANCE,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= PRF_CHIP_MODE_SEL_NRF,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= PRF_RX_MODE,
	.phy				= PRF_PHY_2M,				//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= PRF_CRC_SEL_CRC16,
	.src				= PRF_SRC_SEL_NOSRC,
	.mode_conf			= PRF_BLE_CONF,
	.rx_timeout			= 50000,						//50ms
	.rf_channel			= 2510,
	.tx_no_ack			= DISABLE,
	.trf_type			= PRF_TRF_NRF52,
	.rx_length			= 5,
	.sync_length		= 4,
	.crc_include_sync	= DISABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x71, 0x76, 0x41, 0x29 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};

enum prf_ota_status_t
{
	prf_ota_verify = 0x01,
	prf_ota_start,
	prf_ota_ing,
	prf_ota_check,
	prf_ota_check_ack,
	prf_ota_retrans,
	prf_ota_get_checksum,
	prf_ota_end,
};

bool rx_data_exist = false;
bool prf_verify_timeout = true;
bool prf_check_status = false;
enum prf_ota_status_t prf_ota_stat = prf_ota_verify;
panchip_prf_payload_t rx_payload;
uint16_t chunk_pk_index;
uint16_t pkt_sector_num;
uint16_t pkt_tbl_num;
uint16_t pkt_total_num;
uint8_t chunk_pk_tbl[PKT_MASK_TBL_LEN];
uint32_t calculate_chunk_len;
volatile uint8_t calculate_sum;
uint8_t firmware_data[PRF_OTA_PKT_LEN * PRF_BIT_MASK_WIDTH];
uint32_t firmware_len;
uint8_t prf_id;
uint8_t prf_pkt_len;
uint8_t prf_mask_width;
const uint8_t prf_verify_data[3] = {0x4F, 0x54, 0x41};

void data_printf(uint8_t const *data, uint32_t len)
{
	uint32_t i = 0;

	if (len == 0) 
	{
		return;
	}

	for (; i < len; i++) 
	{
		printf("0x%02X ", data[i]);
	}
	printf("\n");
}

void event_tx_fun(void)
{
	panchip_prf_trx_start();
}

__ramfunc void event_rx_fun(void)
{
	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);

	panchip_prf_payload_t ack_payload;
	static uint8_t ack_count = 0;
	
	if((rx_payload.data[0] == prf_ota_verify) || (rx_payload.data[0] == prf_ota_check))
	{	
		if(ack_count != (prf_id%10))
		{			
			panchip_prf_reset();
			panchip_prf_trx_start();
		}
		ack_count++;
		if(ack_count >= 10)
		{
			ack_count = 0;
		}
	}
	else
	{
		ack_count = 0;
	}
	
	if(rx_payload.data[0] == prf_ota_verify)
	{
		memcpy(&ack_payload.data, &rx_payload.data, rx_payload.data_length);
		ack_payload.data[4] = prf_id;
		ack_payload.data_length = rx_payload.data_length + 1;
	}
	else if(rx_payload.data[0] == prf_ota_check)
	{
		ack_payload.data[0] = prf_ota_check_ack;
		memcpy(&ack_payload.data[1],chunk_pk_tbl, pkt_tbl_num);
		ack_payload.data_length = (pkt_tbl_num + 1);
	}
	else if(rx_payload.data[0] == prf_ota_get_checksum)
	{
		ack_payload.data[0] = prf_ota_get_checksum;
		ack_payload.data[1] = prf_id;
		ack_payload.data[2] = calculate_sum;
		ack_payload.data_length = 3;
	}
	else
	{
		ack_payload.data_length = 0;
	}

	panchip_prf_set_ack_data(&ack_payload);
	
	rx_data_exist = true;
}

void event_rx_timeout_fun(void)
{
	prf_verify_timeout = false;
}

void event_crc_err_fun(void)
{
	panchip_prf_trx_start();
}

void panchip_prf_isr_init(void)
{
	isr_cb.tx_cb = event_tx_fun;
	isr_cb.rx_cb = event_rx_fun;
	isr_cb.rx_timeout_cb = event_rx_timeout_fun;
	isr_cb.rx_crc_err_cb = event_crc_err_fun;
}

void panchip_prf_irq_enable(void)
{
	NVIC_SetPriority (LL_IRQn, 0);
	/* Enable RF interrupt */
	NVIC_EnableIRQ(LL_IRQn);
}

void uart_printf_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1, ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
	
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);

    UART_InitTypeDef Init_Struct;

    Init_Struct.UART_BaudRate = 921600;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;
	
    /* Init UART0 for printf */
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
}

uint8_t prf_ota_check_sum(uint8_t *data, uint32_t len)
{
	uint8_t check_sum = 0;

	for(int i = 0;i < len;i++)
	{
		check_sum += data[i];
	}
	
	return check_sum;
}

uint8_t prf_pkt_len_get(void)
{
	if(prf_pkt_len == 0) {
		return 250;
	} else {
		return prf_pkt_len;
	}
}

uint8_t prf_mask_width_get(void)
{
	if(prf_mask_width == 0) {
		return 32;
	} else {
		return prf_mask_width;
	}
}

void panchip_prf_ota_init(void)
{
	OTP_STRUCT_T otp;
	
	SystemHwParamLoader(&otp);
	
    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}
	
	memset(firmware_data, 0xff, PRF_OTA_PKT_LEN * PRF_BIT_MASK_WIDTH);
	
	memset(chunk_pk_tbl, 0, sizeof(chunk_pk_tbl));
	
	panchip_prf_init(&prf_config);
	
	if(otp.m.ft_version >= 2)
	{
		srand(otp.m_v2.mac_addr[5]);
		prf_id = rand();
	}
	
	panchip_prf_set_chn(prf_config.rf_channel);

	/*adr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	
	panchip_prf_trx_start();
}

void panchip_prf_recv_data_prase(uint8_t *data, uint8_t len)
{
	switch(data[0])
	{
		case prf_ota_verify:
			if(!memcmp(&data[1], prf_verify_data, 3))
			{
				firmware_len = 0;
				calculate_sum = 0;
				fm_status_refresh(); /* clear flash manager for a new ota process */
				prf_ota_stat = prf_ota_start;
				panchip_prf_rx_timeout(0);
				panchip_prf_trx_start();
			}
			break;
		case prf_ota_start:
			if(prf_ota_stat == prf_ota_start)
			{
				prf_ota_stat = prf_ota_ing;
				pkt_sector_num = data[1];
				prf_pkt_len = data[2];
				prf_mask_width = data[3];
				FMC_EraseCodeArea(FLCTL, FLASH_AREA_IMAGE_START, pkt_sector_num * SECTOR_SIZE);
				
				pkt_total_num = (pkt_sector_num * SECTOR_SIZE) / PRF_OTA_PKT_LEN;
				if((pkt_sector_num * SECTOR_SIZE) % PRF_OTA_PKT_LEN) {
					pkt_total_num = pkt_total_num + 1;
				}

				pkt_tbl_num = (pkt_sector_num * SECTOR_SIZE) / (PRF_OTA_PKT_LEN * 8);
				if((pkt_sector_num * SECTOR_SIZE) % (PRF_OTA_PKT_LEN * 8)) {
					pkt_tbl_num = pkt_tbl_num + 1;
				}
				
				printf("pkt_tbl_num %d,%d,%d\n",pkt_sector_num, pkt_tbl_num, pkt_total_num);
			}
			break;
		case prf_ota_ing:
			if(prf_ota_stat == prf_ota_ing)
			{
				chunk_pk_index = (data[1] << 8) | data[2];
				chunk_pk_tbl[(chunk_pk_index/8)] |= 1 << (chunk_pk_index - 8 * (chunk_pk_index/8));

				memcpy(&firmware_data[(len - 3) * (chunk_pk_index%PRF_BIT_MASK_WIDTH)], &data[3], (len - 3));
				calculate_chunk_len += (len - 3);
				printf("chunk_pk_index %d,%x,%d\n",chunk_pk_index,chunk_pk_tbl[(chunk_pk_index/8)],calculate_chunk_len);
				if((!((chunk_pk_index + 1) % PRF_BIT_MASK_WIDTH)) ||
					(((chunk_pk_index + 1) % PRF_BIT_MASK_WIDTH) && ((chunk_pk_index + 1) == pkt_total_num)))
				{
					FMC_WriteStream(FLCTL, FLASH_AREA_IMAGE_START + firmware_len, firmware_data, PRF_OTA_PKT_LEN * PRF_BIT_MASK_WIDTH);
					memset(firmware_data, 0xff, PRF_OTA_PKT_LEN * PRF_BIT_MASK_WIDTH);
					firmware_len += PRF_OTA_PKT_LEN * PRF_BIT_MASK_WIDTH;
					printf("len %d,%d\n",firmware_len, calculate_chunk_len);
					calculate_chunk_len = 0;
				}
			}
			break;
		case prf_ota_check:
			for(int i = 0;i < pkt_tbl_num;i++)
			{
				if(((chunk_pk_tbl[i] & 0xff) != 0xff))
				{
					prf_check_status = false;
					break;
				}
				else
				{
					prf_check_status = true;
				}
			}
			
			if(prf_check_status == false)
			{
				prf_ota_stat = prf_ota_retrans;
			}
			break;
		case prf_ota_retrans:
			if(prf_ota_stat == prf_ota_retrans)
			{
				chunk_pk_index = (data[1] << 8) | data[2];
				chunk_pk_tbl[(chunk_pk_index/8)] |= 1 << (chunk_pk_index - 8 * (chunk_pk_index/8));
				printf("trans %d,%d\n",chunk_pk_index,(len - 3));
				FMC_WriteStream(FLCTL, FLASH_AREA_IMAGE_START + chunk_pk_index * PRF_OTA_PKT_LEN, &data[3], (len - 3));
			}
			break;
		case prf_ota_get_checksum:
			if(calculate_sum == 0)
			{				
				uint16_t read_index = 0;
				uint8_t m_checksum = 0;
				while(read_index < pkt_sector_num)
				{
					FMC_ReadStream(FLCTL,(FLASH_AREA_IMAGE_START + (read_index * SECTOR_SIZE)), CMD_FAST_READ, firmware_data, SECTOR_SIZE);
					m_checksum += prf_ota_check_sum(firmware_data, SECTOR_SIZE);
					read_index++;
				}
				calculate_sum = m_checksum;
				printf("calculate_sum %x\n",calculate_sum);
			}
			break;
		case prf_ota_end:
			NVIC_SystemReset();
			break;
		default:
			break;
	}
}

bool panchip_prf_ota_start(void)
{
	panchip_prf_ota_init();
	
	while(prf_ota_stat == prf_ota_verify)
	{
		if(rx_data_exist == true)
		{
			rx_data_exist = false;
			panchip_prf_recv_data_prase(rx_payload.data, rx_payload.data_length);
		}
		
		if(prf_verify_timeout == false)
		{
			return false;
		}
	}

	return true;
}

void on_prf_ota_enter(void)
{	
	uart_printf_init();
	
	printf("prf ota server init\n");
	
	while(prf_ota_stat != prf_ota_end)
	{
		if(rx_data_exist == true)
		{
			rx_data_exist = false;
			panchip_prf_recv_data_prase(rx_payload.data, rx_payload.data_length);
		}
	}
}
