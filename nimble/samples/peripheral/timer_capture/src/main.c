/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    GPIO P15(timer ext capture ) connect to GPIO P22(PWM0 channel0)
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


#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


static void timer0_cap_event_handler(TIMER_Cb_Flag_Opt evt_flag)
{				
	static bool cap_flag = false;
	
	if(cap_flag) {
		printf("high level %lldus\n",(uint64_t)TIMER_GetCaptureData(TIMER0_OBJ.TIMERx) * 1000000 / TIMER0_OBJ.initObj.freq);
	} else {
		printf("low level %lldus\n",(uint64_t)TIMER_GetCaptureData(TIMER0_OBJ.TIMERx) * 1000000 / TIMER0_OBJ.initObj.freq);
	}
	cap_flag = !cap_flag;
}

static void timer0_capture_init(void)
{
	SYS_SET_MFP(P1, 5, TIMER0_EXT);
    GPIO_EnableDigitalPath(P1, BIT5);
	
	TIMER0_OBJ.initObj.mode = TIMER_MODE_INCAP;
	TIMER0_OBJ.initObj.cntMode = TIMER_CONTINUOUS_MODE;
    TIMER0_OBJ.initObj.freq = FREQ_1MHZ;
	TIMER0_OBJ.initObj.tmr0CmpSel = TMR0_COMPARATOR_SEL_CMP;
	TIMER0_OBJ.initObj.capSrc = TIMER_CAPTURE_SOURCE_EXT_PIN;
    TIMER0_OBJ.initObj.capMode =  TIMER_CAPTURE_COUNTER_RESET_MODE;
    TIMER0_OBJ.initObj.capEdge =  TIMER_CAPTURE_BOTH_EDGE;
    TIMER0_OBJ.callback = timer0_cap_event_handler;
	
	HAL_TIMER_Init(&TIMER0_OBJ);

	HAL_TIMER_Start(&TIMER0_OBJ);
}

static void pwm_init(void)
{
	HAL_PWM_PinConfiguration(P2, 2, PWM_CH0);
	
	PWM_InitOpt pwm_init_obj  = {
		.frequency = 20,
		.dutyCycle = 30,
		.operateType = OPERATION_EDGE_ALIGNED,
		.lowPowerEn = DISABLE,
	};

	int handle = HAL_PWM_Init(PWM0_CH0, &pwm_init_obj);
	
	if (handle >= 0) {
		HAL_PWM_Start(handle);
	}
}

void app_init(void)
{
    printf("app started\n");
    
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
	pwm_init();

	timer0_capture_init();
}
