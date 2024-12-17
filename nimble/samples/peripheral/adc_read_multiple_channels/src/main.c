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
#include "pan_hal_adc.h"

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)0xA000;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

ADC_HandleTypeDef adc_battery = {
	.id = ADC_CH_BATTERY,
};

ADC_HandleTypeDef adc_temperature = {
	.id = ADC_CH_TEMP,
};

ADC_HandleTypeDef adc_ch1 = {
	.id = ADC_CH1,
};

TimerHandle_t xtimers[2];


void TaskBatteryGet(void * pvParameters)
{
	uint16_t v;
	while(1) {
		vTaskDelay(2000);
		printf("battery start get\n");
		
		HAL_ADC_GetValue(&adc_battery, &v);
		
		printf("battery value->>%d mv\n", v);
	}
}

void TimerCallbacks(TimerHandle_t xTimer)
{
	float value;
	if (xTimer == xtimers[0]) {
		printf("CH1 get\n");
		HAL_ADC_GetValue(&adc_ch1, &value);
		printf("CH1 value->>%f \n", value);
	} else {
		
		printf("temperature get\n");
		HAL_ADC_GetValue(&adc_temperature, &value);
		printf("temperature value->>%f \n", value);
	}
}


void app_init(void)
{
    printf("app started\n");
	
    #if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
	HAL_ADC_Init(&adc_battery);
	HAL_ADC_Init(&adc_temperature);
	HAL_ADC_Init(&adc_ch1);
	
	/* Create the task. */
	xTaskCreate(TaskBatteryGet,    /* Pointer to the function that implements the task. */
				"Demo task", 		/* Text name given to the task. */
				 256,               /* The size of the stack that should be created for the task. This is defined in words, not bytes. */
				 NULL,              /* A reference to xParameters is used as the task parameter. This is cast to a void * to prevent compiler warnings. */
				 7,                 /* The priority to assign to the newly created task. */
				 NULL               /* The handle to the task being created will be placed in xHandle. */
				);
	
	/* Create software timer */
	for (uint8_t x = 0; x < 2; x++) {
		xtimers[x] = xTimerCreate("Timer",               // Just a text name, not used by the kernel.
                                   (1000 * (x + 1)), 	   // The timer period in ticks.
                                    pdTRUE,                // The timers will auto-reload themselves when they expire.
                                    NULL,              // Assign each timer a unique id equal to its array index.
                                    TimerCallbacks          // Each timer calls the same callback when it expires.
                                   );
	}
	
	xTimerStart(xtimers[0], 0);
	xTimerStart(xtimers[1], 2000);

}
