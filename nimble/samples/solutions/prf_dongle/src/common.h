/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _MOUSE_COMMON_H_
#define _MOUSE_COMMON_H_

#include "PanSeries.h"
#include "ring_buffer.h"
#include "mouse_prf.h"
#include "info.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define __ramfunc __attribute__((section(".ram.text")))

#define MOUSE_REPORT_SIZE                          11
#define PRF_PKT_SIZE                               14
#define RINGBUFFER_SIZE                            440

#define STAND_KB_ENDPOINT               1
#define COMPOSITE_ENDPOINT              2       /* contain mouse report id 5*/
#define VENDOR_ENDPOINT                 3       /* dfu & emi */

#define MOUSE_REPORT_ID                 5
#define USB_MOUSE_REPORT_SIZE           8
#define USB_STAND_KB_REPORT_SIZE        8

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
extern uint8_t m_chip_mac[6];

extern void data_printf(uint8_t const *data, uint32_t len);
extern struct ring_buf ringbuf_raw;
extern struct ring_buf ringbuf_raw2;
extern uint8_t ringbuf_data[RINGBUFFER_SIZE];
extern uint8_t ringbuf_data2[RINGBUFFER_SIZE];
extern bool test_mode_auto_circle;
extern void auto_circle(int16_t *x_value, int16_t *y_value);

#ifdef __cplusplus
}
#endif

#endif
