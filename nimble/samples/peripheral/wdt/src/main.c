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
#include "timers.h"
#include "semphr.h"

#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pan_hal.h"


#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


#define WDT_CLK_SRC        CLK_APB1_WDTSEL_MILLI_PULSE

static void WDT_INT_Callback()
{
    HAL_WDT_Feed(NULL);
    printf("Feed Dog\n");
}

void hal_bsp_init(void)
{
    WDT_Init_Opt WDT_Init_Obj;

    WDT_Init_Obj.ClockSource = WDT_CLK_SRC;
    WDT_Init_Obj.Timeout = WDT_TIMEOUT_2POW12;
    WDT_Init_Obj.ResetDelay = WDT_RESET_DELAY_1025CLK;
    WDT_Init_Obj.ResetSwitch = true;
    WDT_Init_Obj.WakeupSwitch = false;
    HAL_WDT_Init(&WDT_Init_Obj);
    
    WDT_Interrupt_Opt WDT_Interrupt_Obj;

    WDT_Interrupt_Obj.CallbackFunc = WDT_INT_Callback;
    HAL_WDT_Init_INT(&WDT_Interrupt_Obj);
}

void app_init(void)
{
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
    hal_bsp_init();
}
