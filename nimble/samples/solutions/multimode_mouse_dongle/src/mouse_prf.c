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

#define PRF_DBG_PIN                                     1

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

uint16_t channel_list_public[8] = { 2404, 2412, 2418, 2430, 2438, 2444, 2456, 2474 };
uint16_t channel_list_private[8] = { 2404, 2412, 2418, 2430, 2438, 2444, 2456, 2474 };

pan_prf_config_t rx_config =
{
	.work_mode = (prf_mode_t)PRF_WORK_MODE,                                 // PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode = (prf_chip_mode_sel_t)PRF_CHIP_MODE,                        // PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode = (prf_trx_mode_t)PRF_TRX_SELECT,
	.phy = (prf_phy_t)PRF_PHY_SELECT,                                       // PRF_PHY_250K,//PRF_PHY_1M,
	.crc = (prf_crc_sel_t)PRF_CRC,
	.src = (prf_scramble_sel_t)PRF_SCR,
	.mode_conf = (prf_mode_conf_sel_t)PRF_MODE_CONFIG,
	.rx_timeout = 5000,                                                     // us
	.rf_channel = 2404,
	.tx_no_ack = DISABLE,
	.trf_type = (prf_trf_t)PRF_TRANS_TYPE,
	.rx_length = 30,
	.sync_length = 4,
	.crc_include_sync = DISABLE,
	.src_include_sync = DISABLE,
	.sync = { 0x12, 0x34, 0xab, 0xcd },
	.pid_manual_flag = ENABLE,
	.tx_power = 9,
	.pipe = PRF_PIPE0,
};

panchip_prf_payload_t tx_payload = {
	.data_length = 0,
	.data = { 0 },
};

__ramfunc void event_tx_fun(void)
{
	panchip_prf_trx_start();
}

__ramfunc void event_rx_fun(void)
{
	prf_state.rx_test_cnt++;

	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);

	static uint8_t seq_num;

	if (seq_num != rx_payload.data[1]) {
		prf_state.valid_rx_test_cnt++;
		seq_num = rx_payload.data[1];
		ring_buf_put(&ringbuf_raw, (uint8_t *)rx_payload.data, MOUSE_REPORT_SIZE);
	}

	/* default ack[0] = 0*/
	panchip_prf_set_ack_data(&tx_payload);
}

__ramfunc void event_rx_timeout_fun(void)
{
//	printf("o ");

	prf_state.timout_test_cnt++;

	prf_state.channel_index++;
	prf_state.channel_index = prf_state.channel_index % 8;
	rx_config.rf_channel = channel_list_private[prf_state.channel_index];

	/* 320us contain: reset 62; set chn 109; init 178us */
	panchip_prf_set_chn(rx_config.rf_channel);

	panchip_prf_trx_start();
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

void mouse_prf_init(void)
{
	panchip_prf_init(&rx_config);

	/* adr match bit */
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

	panchip_prf_trx_start();
}
