/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include "signal.h"
 
#ifndef SIGNAL_SLOT_MANAGER__H_
#define SIGNAL_SLOT_MANAGER__H_
 
typedef void (*slot_handler_t)(void);
typedef bool (*signal_handler_t)(void);
 
 typedef struct {
    signal_handler_t sig;
    slot_handler_t slot;
} ss_manager_t;
 

void ss_connect(uint8_t priority, signal_handler_t sig, slot_handler_t slot);
void ss_events_handle(void);
#endif

