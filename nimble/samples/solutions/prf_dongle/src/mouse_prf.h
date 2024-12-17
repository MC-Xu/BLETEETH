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

#ifdef __cplusplus
extern "C" {
#endif

#define PAIR_MAX_NUM                             2
#define PAIR_MAGIC                               0x3141
#define PAIR_DONGLE_1K_FLAG                      0x01
#define PAIR_DONGLE_4K_FLAG                      0x02
#define PAIR_MOUSE_1K_FLAG                       0x11
#define PAIR_MOUSE_4K_FLAG                       0x12
#define PAIR_MOUSE_8K_FLAG                       0x13

struct prf_state_t {
	uint8_t own_addr[6];
	uint8_t channel_index;
	uint16_t ack_lost_cnt;
	uint16_t rx_test_cnt;
	uint16_t valid_rx_test_cnt;
	uint16_t tx_test_cnt;
	uint16_t timout_test_cnt;
	uint16_t crc_test_cnt;
	uint16_t rx_len_err_test_cnt;
	bool pairing;
	bool paird_addr_active[7];
	uint8_t pair_private_addr[7][6];
};

void mouse_prf_init(void);
void mouse_prf_cnt_printf(void);

extern struct prf_state_t prf_state;

#ifdef __cplusplus
}
#endif

#endif /* MOUSE_PRF_H_ */
