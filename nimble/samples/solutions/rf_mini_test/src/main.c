/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    main.c
 *
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "PANSeries.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "host/ble_hs.h"
#include "rf_test.h"
#include "pan_hal_uart.h"

#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


void app_init(void)
{
    int rc;

    printf("app started\n");
	
	#if CONFIG_BOOT_ENABLE
	print_version_info();
    #endif
	
	hs_thread_init();
	
	rf_test_start_constant_tone(0);
	
	__WFI();

}
