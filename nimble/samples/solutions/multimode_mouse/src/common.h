/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _MOUSE_COMMON_H_
#define _MOUSE_COMMON_H_

#include "PanSeries.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOUSE_REPORT_SIZE                             11
#define PRF_PKT_SIZE                                  14
#define RINGBUFFER_SIZE                               440

#define PRF_PKT_TYPE_MOUSE_DATA                       0xff

struct pkt_detect_t {
	uint8_t sequence;
	uint8_t header;
	uint8_t key_value;
	int16_t x_value;
	int16_t y_value;
	int16_t roll_value;
	uint8_t rate_pkt_index;
	int8_t reserved;
} __PACKED;

extern uint16_t report_rate;

extern void data_printf(uint8_t const *data, uint32_t len);
extern struct ring_buf ringbuf_raw;
extern uint8_t ringbuf_data[RINGBUFFER_SIZE];
extern bool test_mode_auto_circle;
extern void auto_circle(int16_t *x_value, int16_t *y_value);

#ifdef __cplusplus
}
#endif

#endif
