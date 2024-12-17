/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#include "signal_slot_manager.h"
#include <stdlib.h>
#define SS_MAX_EVENTS_PAIRED 6

ss_manager_t m_ss_manager[SS_MAX_EVENTS_PAIRED];

void ss_connect(uint8_t priority, signal_handler_t sig, slot_handler_t slot)
{
	m_ss_manager[priority].sig = sig;
	m_ss_manager[priority].slot = slot;
}

void ss_events_handle(void)
{
	for (uint8_t i = 0; i < SS_MAX_EVENTS_PAIRED; i++) {	
		if (m_ss_manager[i].sig == NULL) { /*skip this loop, cause the function point equals to zero */
			continue;
		}
		
		if (m_ss_manager[i].sig()) {
			m_ss_manager[i].slot();
		}
	}
}
