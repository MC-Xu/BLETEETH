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
#include "os_lp.h"

/* Configure App Task */
#define APP_TASK_STACK_SIZE     256
#define APP_TASK_PRIORITY       1

/* Declare App Task */
static void app_task(void *arg);

/* Stores the handle of the task that will be notified when the transmission is complete. */
static TaskHandle_t xTaskToNotify = NULL;

static uint32_t wkup_cnt = 0;

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

static void parse_stbm1_gpio_wakeup_source(void)
{
    uint32_t src;

    src = soc_stbm1_gpio_wakeup_src_get();

    printf("gpio wakeup src flag = 0x%08x\n", src);

    for (size_t i = 0; i < 32; i++) {
        if (src & (1u << i)) {
            printf("SoC is waked up by GPIO P%d_%d.\n", i / 8, i % 8);
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
    soc_busy_wait(10000);

    /* Enable GPIO interrupts and set trigger type to Falling Edge */
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

#ifdef CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
void restore_hw_peripherals(void)
{
#if CONFIG_UART_LOG_ENABLE
	UART_InitTypeDef Init_Struct = {
		.UART_BaudRate = 921600,
		.UART_LineCtrl = Uart_Line_8n1,
	};
	/* Re-init UART */
	UART_Init(UART0, &Init_Struct);
	UART_EnableFifo(UART0);
	/* Set IO pinmux to UART again */
	set_uart_io();
#endif
	/* Re-enable NVIC GPIO IRQs */
	NVIC_EnableIRQ(GPIO0_IRQn);
	NVIC_EnableIRQ(GPIO1_IRQn);
	/* Re-init GPIO keys */
	wakeup_gpio_keys_init();
}
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */

void app_init(void)
{
    BaseType_t r;

#if CONFIG_BOOT_ENABLE
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
    uint8_t rst_reason;

    /* Store the handle of current task. */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    if(xTaskToNotify == NULL) {
        printf("Error, get current task handle failed!\n");
        while (1);
    }

    /* Get the last reset reason */
    printf("\nReset Reason: ");
    rst_reason = soc_reset_reason_get();
    switch (rst_reason) {
    case SOC_RST_REASON_POR_RESET:
        printf("Power On Reset.\n");
        break;
    case SOC_RST_REASON_PIN_RESET:
        printf("nRESET Pin Reset.\n");
        break;
    case SOC_RST_REASON_SYS_RESET:
        printf("NVIC System Reset.\n");
        break;
    case SOC_RST_REASON_STBM1_GPIO_WAKEUP:
        printf("Standby Mode 1 GPIO Wakeup (cnt = %d).\n", ++wkup_cnt);
        parse_stbm1_gpio_wakeup_source();
        break;
    default:
        printf("Unhandled Reset Reason (%d), refer to more reason define in os_lp.h!\n", rst_reason);
    }

    /* Enable 3 GPIO keys low-level wakeup for standby mode 1 */
    wakeup_gpio_keys_init();

    printf("\nTry to enter SoC sleep/deepsleep mode and wait for 3-times key pressing..\n");
    /*
     * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
     * is pdFALSE, which has the effect of decrease the task's notification value by 1, making
     * the notification value act like a counting semaphore.
     */
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    printf("First key pressed, notify value = %d.\n", ulNotificationValue);
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    printf("Second key pressed, notify value = %d.\n", ulNotificationValue);
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    printf("Third key pressed, notify value = %d.\n", ulNotificationValue);

    printf("\nBusy wait 500ms to keep SoC in active mode..\n");
    soc_busy_wait(500000);

#if !CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
    printf("\nNow try to enter SoC standby mode 1 (wakeup reset)..\n\n");
#if CONFIG_UART_LOG_ENABLE
    /* Waiting for UART Tx done and re-set UART IO before entering standby mode 1 to avoid current leakage */
    reset_uart_io();
#endif
    /* Enter standby mode1 with all sram power off */
    soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_NONE);

    printf("WARNING: Failed to enter SoC standby mode 1 due to unexpected interrupt detected.\n");
    printf("         Please check if there is an unhandled interrupt during the standby mode 1\n");
    printf("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
#else
    while (1) {
        printf("\nNow try to enter SoC standby mode 1 (wakeup continuous run)..\n\n");
#if CONFIG_UART_LOG_ENABLE
        /* Waiting for UART Tx done and re-set UART IO before entering standby mode 1 to avoid current leakage */
        reset_uart_io();
#endif
        /* Enter standby mode1 with all common 48KB sram retention */
        soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_BLOCK0 | STBM1_RETENTION_SRAM_BLOCK1);
        /* Restore hardware peripherals manually */
        restore_hw_peripherals();
        printf("Waked up from SoC standby mode 1 and continue run (cnt = %d)..\n", ++wkup_cnt);
    }
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */
}
