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

/* Configure Timer0 bias of compare data */
#define TIMER0_CMPDAT_DEVIATION     (-13)

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

CONFIG_RAM_CODE void TMR0_IRQHandler(void)
{
    static uint32_t cnt;
    uint32_t pwm_pulse = 0;
    uint32_t pwm_period = 0;

    /* Handle timer interrupt event */
    if (TIMER_GetIntFlag(TIMER0)) {
        /* Clear timer int flags */
        TIMER_ClearTFFlag(TIMER0, TIMER_GetTFFlag(TIMER0));
        TIMER_ClearIntFlag(TIMER0);
        /* Randomize PWM CH3 duty cycle after 10 times this handler execute */
        if (++cnt > 9) {
            pwm_period = rand() % 319 + 1;
            pwm_pulse = rand() % pwm_period;
            /*
             * PWM Channel 3:
             * - Period = (<0~319> + 1) cycles = <1~320> * 1 / 32000 s = (0~10) ms
             * - Pulse = (<0~319> + 1) cycles = (0~10) ms
             */
            PWM_SetPeriodAndDuty(PWM, PWM_CH3, pwm_period, pwm_pulse);
        }
        /* Toggle GPIO to indicate timer is timeout */
        GPIO_Toggle(P1, BIT5);
        /* Print system uptime to UART */
        printf("[Uptime: %d ms] Timer0 ISR in, pwm_period = %d, pwm_pulse = %d.\n",
            soc_lptmr_uptime_get_ms(), pwm_period, pwm_pulse);
    }

    /* Clear wakeup flag if there is. */
    TIMER_ClearWakeupFlag(TIMER0, TIMER_GetWakeupFlag(TIMER0));
}

static void wakeup_timer_init(void)
{
    /* Configure HW APB Timer0 with interrupt and wakeup enabled */

    /*
     * Configure timeout
     * timeout = TIMER_CMP_VAL * (TIMER_PRESCALE + 1) / SYS_CLOCK_32K
     *         = 32000 / 10 * (0 + 1) / 32000 (s)
     *         = 0.1 s = 100 ms
     */
    const uint32_t TIMER_PRESCALE = 0;
    const uint32_t TIMER_CMP_VAL = soc_32k_clock_freq_get() / 10;

    /* Enable HW Timer0 Module clock */
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);
    /* Select Timer counting clock source to low-speed 32K clock */
    CLK_SetTmrClkSrc(TIMER0, CLK_APB1_TMR0SEL_RCL32K);
    /* Set Timer to periodic mode */
    TIMER_SetCountingMode(TIMER0, TIMER_PERIODIC_MODE);
    /* Enable Timer interrupt */
    TIMER_EnableInt(TIMER0);
    /* Enable Timer wakeup */
    TIMER_EnableWakeup(TIMER0);
    /* Enable Timer0 interrupt flag0 and wakeup flag0 signal */
    TIMER0->CTL |= BIT8 | BIT11;
    /* Enable NVIC IRQ for Timer */
    NVIC_EnableIRQ(TMR0_IRQn);
    /* Set timeout value */
    TIMER_SetPrescaleValue(TIMER0, TIMER_PRESCALE);
    TIMER_SetCmpValue(TIMER0, TMR0_COMPARATOR_SEL_CMP, TIMER_CMP_VAL + TIMER0_CMPDAT_DEVIATION);
    /* Start Timer */
    TIMER_Start(TIMER0);
}

static void pwm_init(void)
{
    /* Configure HW PWM module which can output waveform even in SoC DeepSleep mode */

    /*
     * Configure PWM waveform period and duty:
     * PWM CH2 (PWM clock source is divided by PWM CLKDIV and CLKPSC):
     * - pwm_cnt_freq = PAN_RCC_CLOCK_FREQUENCY_SLOW / (PWM_CH23_CLKPSC + 1) / PWM_CH2_CLKDIV
     *                = 32000 / (3 + 1) / 2 (Hz)
     *                = 4000 Hz
     * - period = 1 / pwm_cnt_freq * (PWM_CH2_PERIOD_CNT + 1)
     *          = 1 / 4000 * (399 + 1) (s)
     *          = 100 ms
     * - pulse = 1 / pwm_cnt_freq * (PWM_CH2_PULSE_CNT + 1)
     *         = 1 / 4000 * (199 + 1) (s)
     *         = 50 ms
     * PWM CH3 (PWM clock source is configured as directly from 32K Clock):
     * - pwm_cnt_freq = PAN_RCC_CLOCK_FREQUENCY_SLOW
     *                = 32000 Hz
     * - period = 1 / pwm_cnt_freq * (PWM_CH3_PERIOD_CNT + 1)
     *          = 1 / 32000 * (15999 + 1) (s)
     *          = 500 ms
     * - pulse = 1 / pwm_cnt_freq * (PWM_CH3_PULSE_CNT + 1)
     *         = 1 / 32000 * (3199 + 1) (s)
     *         = 100 ms
     */
    const uint32_t PWM_CH23_CLKPSC = 3;

    const PWM_ClkDivDef PWM_CH2_CLKDIV = PWM_CLK_DIV_2;
    const uint32_t PWM_CH2_PERIOD_CNT = 399;
    const uint32_t PWM_CH2_PULSE_CNT = 199;

    const PWM_ClkDivDef PWM_CH3_CLKDIV = PWM_CLK_DIRECT;
    const uint32_t PWM_CH3_PERIOD_CNT = 15999;
    const uint32_t PWM_CH3_PULSE_CNT = 3199;

    /* Set PWM channel 2/3 counting clock source to 32K clock */
    CLK_SetPwmClkSrc(PWM_CH2, CLK_APB1_PWM_CH23_SEL_CLK32K);

    /* Enable clock of PWM and related channel */
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_EN | CLK_APB1Periph_PWM0_CH23, ENABLE);

    /* Enable Pinmux of target PWM channel */
    SYS_SET_MFP(P0, 3, PWM_CH2);
    SYS_SET_MFP(P0, 4, PWM_CH3);

    /*
     * Set clock prescaler (clk_psc) of PWM Channel 2/3.
     * NOTE that the channel 2 and 3 share the same prescaler.
     */
    PWM_SetPrescaler(PWM, PWM_CH2, PWM_CH23_CLKPSC);

    /* Set clock divider (clk_div) of PWM channel 2/3 */
    PWM_SetDivider(PWM, PWM_CH2, PWM_CH2_CLKDIV);
    PWM_SetDivider(PWM, PWM_CH3, PWM_CH3_CLKDIV);

    /* Init PWM channel 2/3 with different period and pulse cycles */
    PWM_SetPeriodAndDuty(PWM, PWM_CH2, PWM_CH2_PERIOD_CNT, PWM_CH2_PULSE_CNT);
    PWM_SetPeriodAndDuty(PWM, PWM_CH3, PWM_CH3_PERIOD_CNT, PWM_CH3_PULSE_CNT);

    /* Invert output level of PWM channel 3 */
    PWM_EnableOutputInverter(PWM, BIT(PWM_CH3));

    /* Enable Output of PWM channel 2/3 */
    PWM_EnableOutput(PWM, BIT(PWM_CH2));
    PWM_EnableOutput(PWM, BIT(PWM_CH3));

    /* Start PWM internal counter */
    PWM_StartChannel(PWM, PWM_CH2);
    PWM_StartChannel(PWM, PWM_CH3);
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

    /* Initialize random seed for later pwm random duty use */
    srand(2024);

    /* Init GPIO P15 as indication IO */
    GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT);
    /* Init specific Timers for wake up use */
    wakeup_timer_init();
    /* Init specific PWM channels */
    pwm_init();

    while (1) {
        printf("Wait for Task Notifications..\n");

        /*
         * Here we try to take the task notify to let OS fall into idle task and
         * enter SoC DeepSleep mode. We'll never wakeup this task by giving notify.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("A notification received, value: %d.\n\n", ulNotificationValue);
    }
}
