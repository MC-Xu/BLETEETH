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
    /* Set pinmux of P10 to GPIO (default) */
    SYS_SET_MFP(P1, 0, GPIO);
    /* Init GPIO P10 to open-drain mode */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_OUTPUT_OPENDRAIN,
        .pull   = HAL_GPIO_PULL_UP,
        .level  = HAL_GPIO_LEVEL_LOW,
    };
    HAL_GPIO_Init(P1_0, &GPIO_InitStruct);
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
    /* Init GPIO P10 as Open-Drain Output */
    hal_bsp_init();

    /* Blink EVB LED3 (Connected to P10) */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500ms
        HAL_GPIO_TogglePin(P1_0);
    }
}
