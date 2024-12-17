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

/* Configure App Tasks */
#define APP_TASK_1_STACK_SIZE   256
#define APP_TASK_1_PRIORITY     1
#define APP_TASK_2_STACK_SIZE   256
#define APP_TASK_2_PRIORITY     2

/* Declare App Tasks */
static void app_task1(void *arg);
static void app_task2(void *arg);

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
    BaseType_t r;

#if CONFIG_BOOT_ENABLE
    print_version_info();
#endif

    /* Create an App Task 1 */
    r = xTaskCreate(app_task1,              // Task Function
                    "App Task 1",           // Task Name
                    APP_TASK_1_STACK_SIZE,  // Task Stack Size
                    NULL,                   // Task Parameter
                    APP_TASK_1_PRIORITY,    // Task Priority
                    NULL                    // Task Handle
    );

    /* Check if task has been successfully created */
    if (r != pdPASS) {
        printf("Error, App Task 1 created failed!\n");
        while (1);
    }

    /* Create an App Task 2 */
    r = xTaskCreate(app_task2,              // Task Function
                    "App Task 2",           // Task Name
                    APP_TASK_2_STACK_SIZE,  // Task Stack Size
                    NULL,                   // Task Parameter
                    APP_TASK_2_PRIORITY,    // Task Priority
                    NULL                    // Task Handle
    );

    /* Check if task has been successfully created */
    if (r != pdPASS) {
        printf("Error, App Task 2 created failed!\n");
        while (1);
    }
}

void app_task1(void *arg)
{
    while (1) {
        printf("Hello from %s!\n", __func__);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500ms
    }
}

void app_task2(void *arg)
{
    while (1) {
        printf("Hello from %s!\n", __func__);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1s
    }
}
