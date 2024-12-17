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

#define TOGGLE_P20_ENABLE                               0
#if TOGGLE_P20_ENABLE
#define TOGGLE_P20 { P20 = 1; P20  = 0; }
#else
#define TOGGLE_P20
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

static uint16_t ack_cnt_test_printk[5];
static bool ack_cnt_test_printk_flag;

uint16_t channel_list_public[8] = { 2404, 2412, 2418, 2430, 2438, 2444, 2456, 2474 };
uint16_t channel_list_private[8] = { 2404, 2412, 2418, 2430, 2438, 2444, 2456, 2474 };

pan_prf_config_t tx_config = {
	.work_mode = (prf_mode_t)PRF_WORK_MODE,                                 // PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode = (prf_chip_mode_sel_t)PRF_CHIP_MODE,                        // PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode = (prf_trx_mode_t)PRF_TRX_SELECT,
	.phy = (prf_phy_t)PRF_PHY_SELECT,                                       // PRF_PHY_250K,//PRF_PHY_1M,
	.crc = (prf_crc_sel_t)PRF_CRC,
	.src = (prf_scramble_sel_t)PRF_SCR,
	.mode_conf = (prf_mode_conf_sel_t)PRF_MODE_CONFIG,
	.rx_timeout = 150,                                                      // us
	.rf_channel = 2404,
	.tx_no_ack = DISABLE,
	.trf_type = (prf_trf_t)PRF_TRANS_TYPE,
	.rx_length = 0,
	.sync_length = 4,
	.crc_include_sync = DISABLE,
	.src_include_sync = DISABLE,
	.sync = { 0x12, 0x34, 0xab, 0xcd },
	.pid_manual_flag = ENABLE,
	.tx_power = 9,
	.pipe = PRF_PIPE0,
};

panchip_prf_payload_t tx_payload = {
	.data_length = 5,
	.data = { 0x01, 0x02, 0x03, 0x04, 0x05 },
};

__ramfunc void event_tx_fun(void)
{
//	printf("t ");

	prf_state.tx_test_cnt++;

//	if (usr_pid >= 256) {
//		usr_pid = 0;
//	}
//	PRI_RF_SetPidManual(PRI_RF, PRF_TX_MODE, usr_pid);
//	usr_pid++;

	prf_state.ack_done = false;
}

__ramfunc void panchip_prf_set_data_fast(panchip_prf_payload_t *p_payload)
{
	uint32_t tmp_data;
	uint16_t data_len;

	data_len = p_payload->data_length;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, R00_CTL, TX_PAYLOAD_LEN, data_len);
	PRI_RF_WRITE_REG_VALUE(PRI_RF, R00_CTL, RX_PAYLOAD_LEN, data_len);

	for (uint8_t i = 0; i < (data_len + 3) / 4; i++) {
		tmp_data = (((uint32_t) p_payload->data[i * 4]) | (((uint32_t) p_payload->data[i * 4 + 1]) << 8) |
			    (((uint32_t) p_payload->data[i * 4 + 2]) << 16) | (((uint32_t) p_payload->data[i * 4 + 3]) << 24));
		LLHWC_WRITE32_REG(REG_FILE_OFST, LIST_RAM_OFST + (data_addr_tx + i) * 4, tmp_data);
	}
}

__ramfunc void rx_fifo_pkt_prf_transfer(void)
{
	if (ring_buf_is_empty(&ringbuf_raw)) {
		prf_state.ack_done = true;
	} else {
		struct pkt_detect_t report_tmp;

		ring_buf_get(&ringbuf_raw, (uint8_t *)&report_tmp, MOUSE_REPORT_SIZE);
		/* check data use pattern */
		if (report_tmp.header != PRF_PKT_TYPE_MOUSE_DATA) {
			prf_state.ack_done = true;
			ring_buf_reset(&ringbuf_raw);
			return;
		}
		tx_payload.data[0] = PRF_PKT_TYPE_MOUSE_DATA;
		tx_payload.data[1] = report_tmp.sequence;
		tx_payload.data[2] = 0;
		tx_payload.data[3] = report_tmp.key_value & 0x7f;
		tx_payload.data[4] = report_tmp.x_value & 0xff;
		tx_payload.data[5] = report_tmp.x_value >> 8;
		tx_payload.data[6] = report_tmp.y_value & 0xff;
		tx_payload.data[7] = report_tmp.y_value >> 8;
		tx_payload.data[8] = report_tmp.roll_value & 0xff;
		tx_payload.data[9] = report_tmp.roll_value >> 8;
		tx_payload.data_length = 10;

		/* delay 30us */
		SYS_delay_10nop(200);
		panchip_prf_set_data(&tx_payload);
		panchip_prf_trx_start();
	}
}

__ramfunc void event_rx_fun(void)
{
//	printf("r ");

	prf_state.ack_test_cnt++;

	prf_state.ack_lost_cnt = 0;

	if (prf_state.hoping_flag) {
		prf_state.hoping_flag = false;
		printf("hop end @ %d\n", tx_config.rf_channel);
	}
	rx_fifo_pkt_prf_transfer();

//	panchip_prf_payload_t rx_payload;
//	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
//	printf("rx data:");
//	data_printf(rx_payload.data, rx_payload.data_length);
}

__ramfunc void judge_prf_hop_channnel(void)
{
	#if HOP_ENABLE
	prf_state.ack_lost_cnt++;
	if (prf_state.ack_lost_cnt > ACK_HOP_CNT) {
		prf_state.hoping_flag = true;
		prf_state.channel_index++;
		prf_state.channel_index = prf_state.channel_index % 8;
		tx_config.rf_channel = channel_list_private[prf_state.channel_index];
		panchip_prf_set_chn(tx_config.rf_channel);
//		printf("%d\n", tx_config.rf_channel);
	}
	#endif
}

__ramfunc void event_rx_timeout_fun(void)
{
//	printf("o ");
	TOGGLE_P20;

	prf_state.timout_test_cnt++;

	judge_prf_hop_channnel();

	/* 60us + 150us = 210 us open tx */
	delay_nop_app(400);

	panchip_prf_trx_start();
}

__ramfunc void event_crc_err_fun(void)
{
//	printf("c ");
	TOGGLE_P20; TOGGLE_P20;

	prf_state.crc_test_cnt++;

	judge_prf_hop_channnel();

	/* 88 us +90us = 178 us open tx */
	delay_nop_app(600);

	panchip_prf_trx_start();
}

__ramfunc void event_rx_len_err_fun(void)
{
//	printf("L ");
	TOGGLE_P20; TOGGLE_P20; TOGGLE_P20;

	prf_state.rx_len_err_test_cnt++;

	judge_prf_hop_channnel();

	/* 48 us + 120us = 168 us open tx */
	delay_nop_app(800);

	panchip_prf_trx_start();
}

__ramfunc void event_pid_err_fun(void)
{
	printf("rx data pid err\n");
}

__ramfunc void ack_cnt_test_printk_f(void)
{
	if (ack_cnt_test_printk_flag) {
		ack_cnt_test_printk_flag = false;
		printf("T ack cnt %d tx cnt %d ", ack_cnt_test_printk[0], ack_cnt_test_printk[1]);
		printf("timout cnt %d crc cnt %d ", ack_cnt_test_printk[2], ack_cnt_test_printk[3]);
		printf("rx len err %d\n", ack_cnt_test_printk[4]);
	}
}

__ramfunc void ack_printk_func(void)
{
	static uint32_t timer_printk_cnt;

	timer_printk_cnt++;
	if (timer_printk_cnt == report_rate) {
		timer_printk_cnt = 0;
		ack_cnt_test_printk[0] = prf_state.ack_test_cnt;
		ack_cnt_test_printk[1] = prf_state.tx_test_cnt;
		ack_cnt_test_printk[2] = prf_state.timout_test_cnt;
		ack_cnt_test_printk[3] = prf_state.crc_test_cnt;
		ack_cnt_test_printk[4] = prf_state.rx_len_err_test_cnt;
		ack_cnt_test_printk_flag = true;
		prf_state.ack_test_cnt = 0;
		prf_state.tx_test_cnt = 0;
		prf_state.timout_test_cnt = 0;
		prf_state.crc_test_cnt = 0;
		prf_state.rx_len_err_test_cnt = 0;
		ack_cnt_test_printk_f();
	}
}

__ramfunc void prf_data_process(void)
{
	ack_printk_func();
//	panchip_prf_trx_start();

//	printf("tx restart\n");

//	/* avoid tx without result */
//	force_restart_tx();

//	/* clear when ringbuf full */
//	if (clearing_ringbuf) {
//		if (ring_buf_is_empty(&ringbuf_raw)) {
//			DBG_PRF_LOG("ringbuf empty\n");
//			clearing_ringbuf = false;
//		} else {
//			DBG_PRF_LOG("-");
//			return;
//		}
//	} else if (ring_buf_space_get(&ringbuf_raw) == 0) {
//		DBG_PRF_LOG("ringbuf full\n");
//		clearing_ringbuf = true;
//		return;
//	}

	/* no pkt put when hoping */
	if (prf_state.hoping_flag) {
		DBG_PRF_LOG("*");
		return;
	}

	struct pkt_detect_t report_tmp;
	static uint8_t pkt_seq;
	uint8_t key_value = 0;
	int16_t x_value, y_value, roll_value = 0;

	pkt_seq++;

//	key_get_value_debounced(&key_value);
//	qdec_get_value(&roll_value);

	if (test_mode_auto_circle) {
		auto_circle(&x_value, &y_value);
	} else {
//		sensor_get_value(&x_value, &y_value);
	}

	report_tmp.header = PRF_PKT_TYPE_MOUSE_DATA;
	report_tmp.key_value = key_value;
	report_tmp.x_value = x_value;
	report_tmp.y_value = y_value;
	report_tmp.roll_value = roll_value;
	report_tmp.sequence = pkt_seq;

//	report_data_idle_detect(&report_tmp);
//	if (low_power_stat != active_stat) {
//		return;
//	}

	if (prf_state.ack_done) {
		tx_payload.data[0] = PRF_PKT_TYPE_MOUSE_DATA;
		tx_payload.data[1] = report_tmp.sequence;
		tx_payload.data[2] = 0;
		tx_payload.data[3] = report_tmp.key_value & 0x7f;
		tx_payload.data[4] = report_tmp.x_value & 0xff;
		tx_payload.data[5] = report_tmp.x_value >> 8;
		tx_payload.data[6] = report_tmp.y_value & 0xff;
		tx_payload.data[7] = report_tmp.y_value >> 8;
		tx_payload.data[8] = report_tmp.roll_value & 0xff;
		tx_payload.data[9] = report_tmp.roll_value >> 8;
		tx_payload.data_length = 10;

		panchip_prf_set_data(&tx_payload);
		panchip_prf_trx_start();
	} else {
		ring_buf_put(&ringbuf_raw, (uint8_t *)&report_tmp, MOUSE_REPORT_SIZE);
	}
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

void mouse_prf_init(void)
{
	panchip_prf_init(&tx_config);

	/* addr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	/* disable pid err isr */
	panchip_prf_pid_cfg(&tx_config, 0);

	/* enable len err isr */
	PRI_RF->R06_TX_CTL |= R06_TX_CTL_MAX_LENGTH_EN_Msk;

	prf_state.ack_done =  true;

	#if PRF_DBG_PIN
	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_11, 0x14); // tx on
	SYS->P1_MFP |= SYS_MFP_P10_LL_DBG_11;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_09, 0x15); // rx on
	SYS->P0_MFP |= SYS_MFP_P04_LL_DBG_9;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_10, 0x17); // rx data
	SYS->P0_MFP |= SYS_MFP_P07_LL_DBG_10;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX03, TST_MUX_SELECT_12, 0x13); // acc match
	SYS->P0_MFP |= SYS_MFP_P05_LL_DBG_12;
	#endif
}
