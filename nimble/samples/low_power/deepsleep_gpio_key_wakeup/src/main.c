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

/* Configure App Task */
#define APP_TASK_STACK_SIZE     256
#define APP_TASK_PRIORITY       1

/* Declare App Task */
static void app_task(void *arg);

/* Stores the handle of the task that will be notified when the transmission is complete. */
static TaskHandle_t xTaskToNotify = NULL;

#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"
	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

#if CONFIG_UART_LOG_ENABLE
CONFIG_RAM_CODE static void reset_uart_io(void)
{
    /* Wait until all UART0 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }

    /*
     * Reset UART PINs to GPIO function and disable digital input path of UART Rx PIN
     * to avoid possible current leakage.
     */
    SYS_SET_MFP(P1, 6, GPIO);
    SYS_SET_MFP(P1, 7, GPIO);
    GPIO_DisableDigitalPath(P1, BIT7);
}

CONFIG_RAM_CODE static void set_uart_io(void)
{
    /* Resume UART PIN Configurations to reenable UART function */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
}
#endif /* CONFIG_UART_LOG_ENABLE */


CONFIG_RAM_CODE void vSocDeepSleepEnterHook(void)
{
#if CONFIG_UART_LOG_ENABLE
    reset_uart_io();
#endif
}

CONFIG_RAM_CODE void vSocDeepSleepExitHook(void)
{
#if CONFIG_UART_LOG_ENABLE
    set_uart_io();
#endif
}

void GPIO0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P0, BIT(i))) {
            GPIO_ClrIntFlag(P0, BIT(i));
            printf("P0_%d INT occurred.\r\n", i);
            /* Notify the task that gpio key is pressed. */
            vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        }
    }
}

void GPIO1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P1, BIT(i))) {
            GPIO_ClrIntFlag(P1, BIT(i));
            printf("P1_%d INT occurred.\r\n", i);
            /* Notify the task that gpio key is pressed. */
            vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        }
    }
}

static void wakeup_gpio_keys_init(void)
{
    /* Configure GPIO P06 (KEY1) / P12 (KEY2) / P02 (WKUP) as Falling Edge Interrupt/Wakeup */

    /* Set pinmux func as GPIO */
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);
    SYS_SET_MFP(P0, 2, GPIO);

    /* Configure debounce clock */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);
    /* Enable input debounce function of specified GPIOs */
    GPIO_EnableDebounce(P0, BIT6);
    GPIO_EnableDebounce(P1, BIT2);
    GPIO_EnableDebounce(P0, BIT2);

    /* Set GPIOs to input mode */
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_SetMode(P0, BIT2, GPIO_MODE_INPUT);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Enable internal pull-up resistor path */
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_EnablePullupPath(P1, BIT2);
    GPIO_EnablePullupPath(P0, BIT2);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Wait for a while to ensure the internal pullup is stable before entering low power mode */
    SYS_delay_10nop(0x10000);

    /* Enable GPIO interrupts and set trigger type to Falling Edge */
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

void app_init(void)
{
    BaseType_t r;

#if CONFIG_BOOT_ENABLE
    print_version_info();
#endif

    /* Create an App Task */
    r = xTaskCreate(app_task,                // Task Function
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

    wakeup_gpio_keys_init();

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
