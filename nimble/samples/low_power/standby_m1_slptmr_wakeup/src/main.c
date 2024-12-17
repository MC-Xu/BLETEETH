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

/* This function overrides the reserved weak function with same name in os_lp.c */
void sleep_timer1_handler(void)
{
    printf("[Uptime: %d ms] SleepTimer 1 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
}

static void slptmr_init(void)
{
    /* Configure timeout of SleepTimer 1 with interrupt and wakeup enabled */

    /*
     * Configure timeout:
     * timeout1 = SLPTMR1_TIMEOUT_CNT / FREQ_32K_HZ
     *         = (32000 / 1) / 32000 (s)
     *         = 1 s
     */
    const uint32_t SLPTMR1_TIMEOUT_CNT = soc_32k_clock_freq_get() / 1;

    /* Set timeout of SleepTimer 1 */
    LP_SetSleepTime(ANA, SLPTMR1_TIMEOUT_CNT, 1);
}

#if CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
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
    /* Re-enable Sleep Timer interrupt */
    NVIC_EnableIRQ(SLPTMR_IRQn);
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
    case SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP:
        printf("Standby Mode 1 SleepTimer 1 Wakeup (cnt = %d).\n", ++wkup_cnt);
        break;
    default:
        printf("Unhandled Reset Reason (%d), refer to more reason define in os_lp.h!\n", rst_reason);
    }

    /* Init Sleep Timer for wake up use */
    slptmr_init();

    printf("\nBusy wait 5s to keep SoC in active mode..\n");
    soc_busy_wait(5500000);

#if !CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
    printf("\nNow try to enter SoC standby mode 1 (wakeup reset)..\n\n");
#if CONFIG_UART_LOG_ENABLE
    /* Waiting for UART Tx done and re-set UART IO before entering standby mode 1 to avoid current leakage */
    reset_uart_io();
#endif
    /* Enter standby mode1 with all sram power off */
    soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_SLPTMR, STBM1_RETENTION_SRAM_NONE);

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
        soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_SLPTMR, STBM1_RETENTION_SRAM_BLOCK0 | STBM1_RETENTION_SRAM_BLOCK1);
        /* Restore hardware peripherals manually */
        restore_hw_peripherals();
        printf("Waked up from SoC standby mode 1 and continue run (cnt = %d)..\n", ++wkup_cnt);
        printf("\nBusy wait 5s to keep SoC in active mode..\n");
        soc_busy_wait(5000000);
    }
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */
}
