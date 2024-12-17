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

#if !CONFIG_UART_LOG_ENABLE
#error "Log UART should be enabled in this project!"
#endif

/* Configure UART1 data buffer size */
#define UART_DATA_BUFF_SIZE     64

static uint8_t uart_data_buf[UART_DATA_BUFF_SIZE];

/* Configure time interval (in seconds) for switch from deepsleep mode to standby mode */
#define INTERVAL_FOR_SWITCH_LOWPOWER_MODE   30

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

/* This function overrides the reserved weak function with same name in os_lp.c */
void sleep_timer1_handler(void)
{
    printf("\nSoc has stayed in deepsleep mode more than %ds, now try to enter standby mode 1..\n\n", INTERVAL_FOR_SWITCH_LOWPOWER_MODE);

    /* Wait until all UART0 data sending done before entering standby mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }

    /* Enable P02 interrupt before entering standby mode for wakeup */
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);

    /* Enter standby mode1 with all sram power off */
    soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_NONE);

    printf("WARNING: Failed to enter SoC standby mode 1 due to unexpected interrupt detected.\n");
    printf("         Please check if there is an unhandled interrupt during the standby mode 1\n");
    printf("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
}

/*
 * This is the application hook function running in OS IDLE task. Before use this, the
 * macro configUSE_IDLE_HOOK should be enabled in FreeRTOSConfig.h.
 *
 * Note that the function definition is a bit different with FreeRTOS original one, that
 * is, here we add a return value for flexibility.
 *
 * Return Value:
 * - false:   Continue run the following code after vApplicationIdleHook() in IDLE task,
 *            which is equivalent to the original FreeRTOS implementation.
 * - true:    Avoid run following code after vApplicationIdleHook() in IDLE task, and this
 *            could prevent SoC entering DeepSleep flow when CONFIG_PM enabled.
 */
bool vApplicationIdleHook(void)
{
    /*
     * Enter sleep mode (instead of deepsleep) if UART Rx FIFO is not empty
     * (Which means there is already data receiving from remote)
     */
    if (!UART_IsRxFifoEmpty(UART1)) {
        __disable_irq();
        __WFI();
        __enable_irq();
        /* Avoid entering DeepSleep flow here */
        return true;
    }

    /* Allow entering DeepSleep flow */
    return false;
}

/*
 * This is hook function runs right after entering SoC DeepSleep Flow.
 * Note that either the os scheduler or systick are stopped before running this
 * function, so do not run any os related objects (such as OS Timer) here.
 */
CONFIG_RAM_CODE void vSocDeepSleepEnterHook(void)
{
    /* Enable and Set timeout of SleepTimer1 */
    uint32_t timeout = soc_32k_clock_freq_get() * INTERVAL_FOR_SWITCH_LOWPOWER_MODE;
    LP_SetSleepTime(ANA, timeout, 1);
    printf("SleepTimer1 started, timeout=%ds\n", INTERVAL_FOR_SWITCH_LOWPOWER_MODE);

    /* Handle UART0 (Log UART) */

    /* Wait until all UART0 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
    /* Reset UART0 PINs to GPIO function and disable digital input path of UART0 Rx PIN to avoid possible current leakage. */
    SYS_SET_MFP(P1, 6, GPIO);
    SYS_SET_MFP(P1, 7, GPIO);
    GPIO_DisableDigitalPath(P1, BIT7);

    /* Handle UART1 (Data UART) */

    /* Wait until all UART1 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART1) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
    /* Reset UART1 PINs to GPIO function and enable gpio interrupt for uart rx pin. */
    SYS_SET_MFP(P1, 0, GPIO);
    SYS_SET_MFP(P0, 7, GPIO);
    GPIO_EnableInt(P0, 7, GPIO_INT_FALLING);

    /* Enable P02 interrupt before entering deepsleep mode for wakeup */
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);
}

CONFIG_RAM_CODE void vSocDeepSleepExitHook(void)
{
    /* Resume UART0 PIN Configurations to reenable UART0 function */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);

    /* Resume UART1 PIN Configurations here to reenable UART1 function if is not waked up by P07 */
    if (!GPIO_GetIntFlag(P0, BIT7)) {
        SYS_SET_MFP(P1, 0, UART1_TX);
        SYS_SET_MFP(P0, 7, UART1_RX);
        GPIO_DisableInt(P0, 7);
    }

    /* Re-disable P02 interrupt after deepsleep wakeup */
    GPIO_DisableInt(P0, 2);

    /* Trigger App Task reschedule after wakeup from deepsleep mode */
    xTaskNotifyGive(xTaskToNotify);
}

CONFIG_RAM_CODE void GPIO0_IRQHandler(void)
{
    /* Check if WKUP (P02) button pressed */
    if (GPIO_GetIntFlag(P0, BIT2)) {
        GPIO_ClrIntFlag(P0, BIT2);
        printf("\nWKUP key (P02) pressed..\n");
    }

    /* Check if UART1 Rx (P07) GPIO wakeup triggered */
    if (GPIO_GetIntFlag(P0, BIT7)) {
        /* Clear gpio pin irq flag */
        GPIO_ClrIntFlag(P0, BIT7);
        while (P07 == 0) {
            /* Busy wait until P07 pulled high, which means the 1st wakup
             * trigger character (0x00) send done.
             */
        }
        /* Resume UART1 PIN configs after P07 pulled high */
        SYS_SET_MFP(P1, 0, UART1_TX);
        SYS_SET_MFP(P0, 7, UART1_RX);
        GPIO_DisableInt(P0, 7);
        printf("\nUART1 Rx (P07) GPIO wakeup triggered..\n");
    }

    /* Try to do context switch right after current isr return */
    taskYIELD();
}

static void UART_HandleReceivedData(UART_T* UARTx)
{
    static uint8_t rx_index = 0;

    printf("Data received from UART: ");
    while (!UART_IsRxFifoEmpty(UARTx)) {
        uart_data_buf[rx_index] = UART_ReceiveData(UARTx);
        printf("0x%02x ", uart_data_buf[rx_index]);
        rx_index++;
        // Handle Rx buffer full
        if (rx_index >= UART_DATA_BUFF_SIZE) {
            printf("\nWARNING: Too much data received, UART Rx buffer full!\n");
            rx_index = 0;   // Reset Rx buf index
            break;
        }
    }
    printf("\n");
}

static void UART_HandleProc(UART_T *UARTx)
{
    UART_EventDef event = UART_GetActiveEvent(UARTx);

    switch (event) {
    case UART_EVENT_DATA:
    case UART_EVENT_TIMEOUT:
        /* Handle received data */
        UART_HandleReceivedData(UARTx);
        break;
    case UART_EVENT_NONE:
        /* Just ignore this event. */
        break;
    default:
        SYS_TEST("WARNING: Unhandled event, IID: 0x%x\n", event);
        break;
    }
}

void UART1_IRQHandler(void)
{
    UART_HandleProc(UART1);
}

static void uart1_comm_init(void)
{
    /* Enable UART1 Clock */
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

    /* Configure UART Pinmux */
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P0, 7, UART1_RX);
    /* Enable digital input path of UART Rx Pin */
    GPIO_EnableDigitalPath(P0, BIT7);

    /* Init UART1 */
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = 9600,
        .UART_LineCtrl = Uart_Line_8n1,
    };
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);

    /* Configure interrupt of UART1 */
    UART_SetRxTrigger(UART1, UART_RX_FIFO_HALF_FULL);
    UART_EnableIrq(UART1, UART_IRQ_RECV_DATA_AVL);      // Enable RDA Interrupt
    NVIC_EnableIRQ(UART1_IRQn);                         // Enable target UART INT in NVIC
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

static void wakeup_gpio_key_init(void)
{
    /* Set pinmux func as GPIO */
    SYS_SET_MFP(P0, 2, GPIO);

    /* Configure debounce clock */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);
    /* Enable input debounce function of specified GPIO */
    GPIO_EnableDebounce(P0, BIT2);

    /* Set GPIO to input mode */
    GPIO_SetMode(P0, BIT2, GPIO_MODE_INPUT);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Enable internal pull-up resistor path */
    GPIO_EnablePullupPath(P0, BIT2);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Wait for a while to ensure the internal pullup is stable before entering low power mode */
    soc_busy_wait(10000);

    /* Disable P02 interrupt at this time */
    GPIO_DisableInt(P0, 2);
}

static void wakeup_gpio_uart1_rx_pin_init(void)
{
    /* Set GPIO to input mode */
    GPIO_SetMode(P0, BIT7, GPIO_MODE_INPUT);

    /* Enable internal pull-up resistor path */
    GPIO_EnablePullupPath(P0, BIT7);

    /* Disable P07 interrupt before entering deepsleep */
    GPIO_DisableInt(P0, 7);
}

static void wakeup_gpio_init(void)
{
    /* Configure gpio wakeup key (P02) */
    wakeup_gpio_key_init();

    /* Configure gpio wakeup pin for uart rx */
    wakeup_gpio_uart1_rx_pin_init();

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
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
    uint32_t ulNotificationValue;
    uint8_t rst_reason;

    /* Store the handle of current task. */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    if (xTaskToNotify == NULL) {
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
        printf("Standby Mode 1 GPIO Wakeup.\n");
        parse_stbm1_gpio_wakeup_source();
        break;
    default:
        printf("Unhandled Reset Reason (%d), refer to more reason define in os_lp.h!\n", rst_reason);
    }

    /* Enable and init UART1 for data transmission */
    uart1_comm_init();

    /* Enable specific gpios wakeup for wakeup use */
    wakeup_gpio_init();

    while (1) {
        /* Busy wait a while to simulate cpu busy status */
        printf("Busy wait a while..\n");
        soc_busy_wait(1000000);

        /* Try to take wakeup_sem */
        printf("Wait for Task Notifications..\n");
        /*
         * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
         * is pdTRUE, which has the effect of clearing the task's notification value back to 0, making
         * the notification value act like a binary (rather than a counting) semaphore.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("A notification received, value: %d.\n\n", ulNotificationValue);

        /* Stop the slptmr1 timer (by resetting the counter) */
        LP_SetSleepTime(ANA, 0, 1);
        printf("SleepTimer1 stopped.\n");
    }
}
