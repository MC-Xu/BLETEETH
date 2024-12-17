/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "FreeRTOS.h"
#include "task.h"
#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mouse_common.h"
#include "mouse_ble.h"

void app_init(void)
{
	mouse_common_init();
	
	mouse_ble_mode_init();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("task overflow:%s\r\n", pcTaskName);
	while (1);
}
