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
#include "timers.h"
#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pan_hal_pwm.h"

#define PWM_LOW_POWER_TEST        0

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

void app_init(void)
{
    printf("app started\n");
    
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
	PWM_InitOpt init_opt;
	
	init_opt.frequency = 1000;
	init_opt.dutyCycle = 30;
	init_opt.inverter = 1;
	init_opt.lowPowerEn = PWM_LOW_POWER_TEST;
	init_opt.mode = PWM_MODE_INDEPENDENT;
	init_opt.operateType = OPERATION_EDGE_ALIGNED;
	
	HAL_PWM_PinConfiguration(P0, 3, PWM_CH2);

	int handle = HAL_PWM_Init(PWM0_CH2, &init_opt);
	
	if (handle >= 0) {
		HAL_PWM_Start(handle);
	}
	
}
