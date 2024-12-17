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
#include "timers.h"
#include "semphr.h"
#include "os_lp.h"

/* Configure App Task */
#define APP_TASK_STACK_SIZE     256
#define APP_TASK_PRIORITY       1
/* Declare App Task */
static void app_task(void *arg);

#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"
	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

#ifdef CONFIG_UART_LOG_ENABLE
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
#ifdef CONFIG_UART_LOG_ENABLE
    reset_uart_io();
#endif
}

CONFIG_RAM_CODE void vSocDeepSleepExitHook(void)
{
#ifdef CONFIG_UART_LOG_ENABLE
    set_uart_io();
#endif
}

static void indication_gpio_init(void)
{
    /* Configure GPIO P13/P14/P15 to push-pull output mode */
    GPIO_SetMode(P1, BIT3 | BIT4 | BIT5, GPIO_MODE_OUTPUT);
}

/* This function overrides the reserved weak function with same name in os_lp.c */
void sleep_timer1_handler(void)
{
    P14 = 0;
    P14 = 1;
    printf("[Uptime: %d ms] SleepTimer 1 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
}

/* This function overrides the reserved weak function with same name in os_lp.c */
void sleep_timer2_handler(void)
{
    P15 = 0;
    P15 = 1;
    printf("[Uptime: %d ms] SleepTimer 2 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
}

static void slptmr_init(void)
{
    /* Configure timeout of SleepTimer 1&2 with interrupt and wakeup enabled */

    /*
     * Configure timeout:
     * timeout1 = SLPTMR1_TIMEOUT_CNT / PAN_RCC_CLOCK_FREQUENCY_SLOW
     *         = (32000 / 2) / 32000 (s)
     *         = 0.5 s = 500 ms
     * timeout2 = SLPTMR2_TIMEOUT_CNT / PAN_RCC_CLOCK_FREQUENCY_SLOW
     *         = 32000 / 32000 (s)
     *         = 1 s
     */
    const uint32_t SLPTMR1_TIMEOUT_CNT = soc_32k_clock_freq_get() / 2;
    const uint32_t SLPTMR2_TIMEOUT_CNT = soc_32k_clock_freq_get();

    /* Set timeout of SleepTimer 1 */
    LP_SetSleepTime(ANA, SLPTMR1_TIMEOUT_CNT, 1);
    /* Set timeout of SleepTimer 2 */
    LP_SetSleepTime(ANA, SLPTMR2_TIMEOUT_CNT, 2);
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
    /* Init specific GPIO as output for ISR indication use */
    indication_gpio_init();
    /* Init Sleep Timers for wake up use */
    slptmr_init();

    while (1) {
        printf("[Uptime: %d ms] Main Thread sleep..\n", soc_lptmr_uptime_get_ms());
        vTaskDelay(pdMS_TO_TICKS(5000));
        P13 = 0;
        P13 = 1;
        printf("[Uptime: %d ms] Main Thread waked up.\n", soc_lptmr_uptime_get_ms());
    }
}
