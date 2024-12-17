/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __MOUSE_BLE_H
#define __MOUSE_BLE_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define CONSUMER_KEY_VAL_AC_HOME                0x0001  /* AC Home */
#define CONSUMER_KEY_VAL_BRIGHTNESS_DOWN        0x0002  /* Brightness Down */
#define CONSUMER_KEY_VAL_BRIGHTNESS_UP          0x0004  /* Brightness Up */
#define CONSUMER_KEY_VAL_KEYBOARD_LAYOUT        0x0008  /* Keyboard Layout */
#define CONSUMER_KEY_VAL_AC_SEARCH              0x0010  /* AC Search */
#define CONSUMER_KEY_VAL_SCAN_PREVIOUS_TRACK    0x0100  /* Scan Previous Track */
#define CONSUMER_KEY_VAL_PLAY_PAUSE             0x0200  /* Play/Pause */
#define CONSUMER_KEY_VAL_SCAN_NEXT_TRACK        0x0400  /* Scan Next Track */
#define CONSUMER_KEY_VAL_MUTE                   0x0800  /* Mute */
#define CONSUMER_KEY_VAL_VOLUME_DECREMENT       0x1000  /* Volume Decrement */
#define CONSUMER_KEY_VAL_VOLUME_INCREMENT       0x2000  /* Volume Increment */
#define CONSUMER_KEY_VAL_POWER                  0x4000  /* Power */
#define CONSUMER_KEY_VAL_REALEASED              0x0000  /* Realeased */

#define CONFIG_EXAMPLE_IO_TYPE                  3       // "Just works"
#define CONFIG_EXAMPLE_RANDOM_ADDR              0
#define CONFIG_EXAMPLE_USE_SC                   1
#define CONFIG_EXAMPLE_RESOLVE_PEER_ADDR        1
#define CONFIG_EXAMPLE_BONDING                  1
#define CONFIG_EXAMPLE_MITM                     1

#ifdef __cplusplus
extern "C" {
#endif

void mouse_ble_mode_init(void);
int hog_send_key(uint16_t key_val);
int hog_send_consumer_key(uint16_t key_val);

#ifdef __cplusplus
}
#endif

#endif
