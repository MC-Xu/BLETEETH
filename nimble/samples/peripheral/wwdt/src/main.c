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


#define WWDT_CLK_SRC        CLK_APB1_WWDTSEL_MILLI_PULSE

static void WWDT_INT_Callback()
{
    HAL_WWDT_Feed(NULL);
    printf("Feed Dog\n");
}

void hal_bsp_init(void)
{    
    WWDT_Init_Opt WWDT_Init_Obj;
    CLK_SetWwdtClkSrc(CLK_APB1_WWDTSEL_MILLI_PULSE);

    WWDT_Init_Obj.ClockSource = WWDT_CLK_SRC;
    WWDT_Init_Obj.Prescaler = WWDT_PRESCALER_32;
    WWDT_Init_Obj.CmpValue = 25;
    HAL_WWDT_Init(&WWDT_Init_Obj);
    
    WWDT_Interrupt_Opt WWDT_Interrupt_Obj;

    WWDT_Interrupt_Obj.CallbackFunc = WWDT_INT_Callback;
    HAL_WWDT_Init_INT(&WWDT_Interrupt_Obj);
}

void app_init(void)
{
	
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
    hal_bsp_init();
}
