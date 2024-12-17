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

static void wakeup_p02_key_init(uint8_t wakeup_edge)
{
    /* Configure GPIO P02 (WKUP Key) as Lowlevel Wakeup */

    /* Configure P02 wakeup level due to wakeup_edge */
    LP_SetExternalWake(ANA, wakeup_edge);

    /* Set pinmux func of P56 as GPIO */
    SYS_SET_MFP(P0, 2, GPIO);
    /* Set P02 to input mode */
    GPIO_SetMode(P0, BIT2, GPIO_MODE_INPUT);

    if (wakeup_edge == 0) {
        /* Enable internal pull-up resistor if P02 is low level wakeup */
        GPIO_EnablePullupPath(P0, BIT2);
    } else {
        /* Enable internal pull-down resistor if P02 is high level wakeup */
        GPIO_EnablePulldownPath(P0, BIT2);
    }

    /* Necessary for P02 to do manual 3v aon sync */
    CLK_Wait3vSyncReady();

    /* Wait for a while to ensure the internal pullup is stable before entering low power mode */
    soc_busy_wait(10000);
}

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
    uint8_t rst_reason;

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
    case SOC_RST_REASON_STBM0_EXTIO_WAKEUP:
        printf("Standby Mode 0 EXTIO Wakeup (cnt = %d).\n", ++wkup_cnt);
        break;
    default:
        printf("Unhandled Reset Reason (%d), refer to more reason define in os_lp.h!\n", rst_reason);
    }

    /* Enable P02 low level wakeup for standby mode 0 */
    wakeup_p02_key_init(0);

    printf("\nBusy wait 100ms to keep SoC in active mode..\n");
    soc_busy_wait(100000);

    printf("Try to enter SoC sleep/deepsleep mode for 1000ms..\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("Waked up from SoC sleep/deepsleep mode.\n");

    printf("Try to enter SoC standby mode 0..\n\n");
    soc_busy_wait(1000); /* Wait for all log print done */
    soc_enter_standby_mode_0();

    printf("WARNING: Failed to enter SoC standby mode 0 due to unexpected interrupt detected.\n");
    printf("         Please check if there is an unhandled interrupt during the standby mode 0\n");
    printf("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
}
