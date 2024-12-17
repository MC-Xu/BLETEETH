/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void light_init(void);
void light_set_rgb(uint8_t r, uint8_t g, uint8_t b);
void rgb_notify(uint8_t *data, uint8_t len);
uint32_t write_rgb(uint8_t *buf, uint16_t len);
#ifdef __cplusplus
}
#endif
