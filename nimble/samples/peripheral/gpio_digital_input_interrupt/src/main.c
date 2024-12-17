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

/* Stores the handle of the task that will be notified when the transmission is complete. */
static TaskHandle_t xTaskToNotify = NULL;

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)0xA0000;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif /* CONFIG_BOOT_ENABLE */

static void gpio_p06_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    /* Notify the task that KEY1 is pressed (P06 falling edge detected). */
    vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);

    if (intMode == HAL_GPIO_INT_FALLING) {
        printf("==== KEY1 Pressed! ====\n");
    } else if (intMode == HAL_GPIO_INT_RISING) {
        printf("==== KEY1 Released! ====\n");
    } else if (intMode == HAL_GPIO_INT_BOTH_EDGE) {
        if (HAL_GPIO_ReadPin(P0_6) == HAL_GPIO_LEVEL_LOW) {
            printf("==== KEY1 Pressed! ====\n");
        } else {
            printf("==== KEY1 Released! ====\n");
        }
    } else if (intMode == HAL_GPIO_INT_LOW) {
#if 0 // Here we can disable interrupt of P06 to avoid GPIO IRQ repeated triggering
        HAL_GPIO_SetIntMode(P0_6, HAL_GPIO_INT_DISABLE);
#endif
        printf("==== KEY1 Pressed! ====\n");
    } else if (intMode == HAL_GPIO_INT_HIGH) {
#if 0 // Here we can disable interrupt of P06 to avoid GPIO IRQ repeated triggering
        HAL_GPIO_SetIntMode(P0_6, HAL_GPIO_INT_DISABLE);
#endif
        printf("==== KEY1 Released! ====\n");
    }
}

static void gpio_p06_init(void)
{
    /* Set pinmux of P06 to GPIO (default) */
    SYS_SET_MFP(P0, 6, GPIO);

    /* Init GPIO P06 to digital input mode */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
        .pull   = HAL_GPIO_PULL_UP,
    };
    HAL_GPIO_Init(P0_6, &GPIO_InitStruct);

    /* Wait a while for internal pull-up resistor stable to avoid later interrupt false triggering */
    soc_busy_wait(10000);

    /* Enable input interrupt of GPIO P06 */
    HAL_GPIO_IntInitTypeDef GPIO_IntInitStruct = {
        .intMode           = HAL_GPIO_INT_FALLING,  // Change this option to see different interrupt behaviors
        .callbackFunc   = gpio_p06_input_callback,
        .debounce       = ENABLE,
    };
    HAL_GPIO_InterruptInit(P0_6, &GPIO_IntInitStruct);

    /* Enable NVIC GPIO0 IRQn */
    NVIC_EnableIRQ(GPIO0_IRQn);
}

void hal_bsp_init(void)
{
    /*
     * Configure clock of GPIO interrupt debouce 
     * NOTE:
     * - This configuration affects all GPIOs with debounce enabled
     * - This API should only be called once at app init flow for safe
     */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_32);    // Set debounce time to 32 cycles of 32K Clock (about 1ms)

    /* Init P06 to digital input mode with interrupt and debounce enabled */
    gpio_p06_init();
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
    uint32_t ulNotificationValue;

    /* Store the handle of current task. */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    if(xTaskToNotify == NULL) {
        printf("Error, get current task handle failed!\n");
        while (1);
    }

    /* Init GPIO P06 as Digital Input with interrupt and debounce enabled */
    hal_bsp_init();

    /* Polling EVB KEY1 (Connected to P06), and print key status when status changed */
    while (1) {
        printf("Wait for Task Notifications..\n");

        /*
         * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
         * is pdTRUE, which has the effect of clearing the task's notification value back to 0, making
         * the notification value act like a binary (rather than a counting) semaphore.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("A notification received, value: %d.\n\n", ulNotificationValue);
    }
}
