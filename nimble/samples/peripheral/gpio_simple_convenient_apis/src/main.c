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

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif /* CONFIG_BOOT_ENABLE */


void gpio_init(void)
{
    /* Configure GPIO P10 for EVB LED3 blinky */
    SYS_SET_MFP(P1, 0, GPIO);                   // Set P10 pinmux to GPIO (can be omitted as the default function of P10 is GPIO)
    GPIO_SetMode(P1, BIT0, GPIO_MODE_OUTPUT);   // Push-pull output mode
    GPIO_EnableDigitalPath(P1, BIT0);           // Enable digital input path

    /* Configure GPIO P06 for EVB Key1 */
    SYS_SET_MFP(P0, 6, GPIO);                   // Set P06 pinmux to GPIO (can be omitted as the default function of P06 is GPIO)
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);    // Digital input mode
    GPIO_EnablePullupPath(P0, BIT6);            // Enable internal pull-up resistor path
    /* Wait a while for internal pull-up resistor stable */
    soc_busy_wait(10000);
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

// Try to detect falling edge on GPIO P06
bool is_gpio_p06_falling_edge_detected(void)
{
    static uint8_t last_level = 1;
    uint8_t curr_level = P06;   // Here we directly read the input level of P06 using the so-called PDIO feature
    bool ret = false;

    if ((last_level == 1) && (curr_level == 0)) {
        ret = true;
    }

    last_level = curr_level;

    return ret;
}

void app_task(void *arg)
{
    uint32_t key_press_cnt = 0;
    uint32_t dly_cnt = 0;
    uint32_t blinky_period[] = {500, 250, 50};  // Unit is ms

    /* Init GPIO P10 and P06 using low-level APIs */
    gpio_init();

    /* Change blinky period of EVB LED3 (connected to P10) by pushing EVB KEY1 (connected to P06) */
    while (1) {
        if (is_gpio_p06_falling_edge_detected()) {
            key_press_cnt++;
            if (key_press_cnt >= sizeof(blinky_period) / sizeof(blinky_period[0])) {
                key_press_cnt = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); // 1ms polling time unit
        dly_cnt++;
        if (!(dly_cnt % blinky_period[key_press_cnt])) {
            // Toggle GPIO P10 using PDIO feature by first reading the level and then do
            // XOR to toggle it, and at last write it back to output toggled level.
            // Note that both gpio output and digital input should be enabled in this case.
            P10 ^= 1;   // P10 = P10 ^ 1
            printf("Toggle LED, dly_cnt=%d\n", dly_cnt);
        }
    }
}
