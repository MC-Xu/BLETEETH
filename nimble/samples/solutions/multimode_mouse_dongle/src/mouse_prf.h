/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file */

#ifndef MOUSE_PRF_H_
#define MOUSE_PRF_H_

#include "PanSeries.h"
#include "common.h"

#define ACK_HOP_CNT                15

#ifdef __cplusplus
extern "C" {
#endif

struct prf_state_t {
	bool hoping_flag;
	bool ack_done;
	uint8_t channel_index;
	uint16_t ack_lost_cnt;
	uint16_t rx_test_cnt;
	uint16_t valid_rx_test_cnt;
	uint16_t tx_test_cnt;
	uint16_t timout_test_cnt;
	uint16_t crc_test_cnt;
	uint16_t rx_len_err_test_cnt;
};

void mouse_prf_init(void);
void mouse_prf_cnt_printf(void);

#ifdef __cplusplus
}
#endif

#endif /* MOUSE_PRF_H_ */
