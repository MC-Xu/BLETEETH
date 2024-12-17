/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    main.c
 *
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "pan_hal.h"
#include "os_lp.h"

/* Configure App Task */
#define APP_TASK_STACK_SIZE     256
#define APP_TASK_PRIORITY       1

/* Declare App Task */
static void app_task(void *arg);

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif /* CONFIG_BOOT_ENABLE */

void hal_bsp_init(void)
{
    /* Set pinmux of P06 to GPIO (default) */
    SYS_SET_MFP(P0, 6, GPIO);
    /* Init GPIO P06 to digital input mode */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
        .pull   = HAL_GPIO_PULL_UP,
    };
    HAL_GPIO_Init(P0_6, &GPIO_InitStruct);
}

void app_init(void)
{
    BaseType_t r;

#if (CONFIG_BOOT_ENABLE)
    print_version_info();
#endif

    /* Create an App Task */
    r = xTaskCreate(app_task,               // Task Function
                    "App Task",             // Task Name
                    APP_TASK_STACK_SIZE,    // Task Stack Size
                    NULL,                   // Task Parameter
                    APP_TASK_PRIORITY,      // Task Priority
                    NULL                    // Task Handle
    );

    /* Check if task has been successfully created */
    if (r != pdPASS) {
        printf("Error, App Task created failed!\n");
        while (1);
    }
}

void app_task(void *arg)
{
    HAL_GPIO_Level curr_level;
    HAL_GPIO_Level last_level = HAL_GPIO_LEVEL_HIGH;

    /* Init GPIO P06 as Digital Input */
    hal_bsp_init();

    /* Polling EVB KEY1 (Connected to P06), and print key status when status changed */
    while (1) {
        curr_level = HAL_GPIO_ReadPin(P0_6);
        if (last_level != curr_level) {
            soc_busy_wait(1000); // Delay 1ms and read IO again for Input Debounce
            curr_level = HAL_GPIO_ReadPin(P0_6);
            if (last_level != curr_level) {
                last_level = curr_level;
                if (curr_level == HAL_GPIO_LEVEL_LOW) {
                    printf("KEY1 Pressed!\n");
                } else {
                    printf("KEY1 Released!\n");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay 10ms for other task scheduling
    }
}
