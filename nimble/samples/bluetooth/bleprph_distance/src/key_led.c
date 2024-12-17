#include "key_led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "PANSeries.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_gatt.h"
#include "blehr_sens.h"

#define task_delay_ms(PAR_MS) vTaskDelay(pdMS_TO_TICKS(PAR_MS))

__IO uint8_t phy_mode = 0;  /* 0 ==> 1M, 1 ==> 2M, 2 ==>Coded S2,  3 ==>Coded S8*/

#define LED_R  P24
#define LED_G  P23
#define LED_B  P22

static TaskHandle_t task_report_hd;
static SemaphoreHandle_t gpio_sem = NULL;
extern uint16_t hid_consumer_input_handle;
extern uint16_t peri_conn_handle;

__IO bool f_key_1 = false;
__IO bool f_key_2 = false;

void GPIO0_IRQHandler(void)
{
    printf("GPIO ISR in..\n");

    if (GPIO_GetIntFlag(P0, BIT6)) {
        GPIO_ClrIntFlag(P0, BIT6);
        f_key_1 = true;
        printf("P06 occurred\n");
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

void GPIO1_IRQHandler(void)
{
    printf("GPIO ISR in..\n");

    if (GPIO_GetIntFlag(P1, BIT2)) {
        GPIO_ClrIntFlag(P1, BIT2);
        f_key_2 = true;
        printf("P12 occurred\n");
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

void led_process(void)
{
    switch(phy_mode)
    {
        case 0:
            LED_R = 0;
            LED_G = 0;
            LED_B = 0;
            ble_gap_set_prefered_default_le_phy(BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_1M_MASK);
        break;
        case 1:
            LED_R = 1;
            LED_G = 0;
            LED_B = 0;
            ble_gap_set_prefered_default_le_phy(BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK);
        break;
        case 2:
            LED_R = 0;
            LED_G = 1;
            LED_B = 0;
            ble_gap_set_prefered_default_le_phy(BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK);
        break;
        case 3:
            LED_R = 0;
            LED_G = 0;
            LED_B = 1;
            ble_gap_set_prefered_default_le_phy(BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK);
        break;
    }
}
void key_1_pro(void)
{
    phy_mode ++;
    phy_mode %= 4;
    led_process();
}

void phy_update_on_connection(void)
{
    switch(phy_mode)
    {
//        case 0:
//        ble_gap_set_prefered_le_phy(peri_conn_handle, BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_1M_MASK,BLE_GAP_LE_PHY_CODED_ANY);
//        break;
//        case 1:
//        ble_gap_set_prefered_le_phy(peri_conn_handle, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK,BLE_GAP_LE_PHY_CODED_ANY);
//        break;
//        case 2:
//        ble_gap_set_prefered_le_phy(peri_conn_handle, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK,BLE_GAP_LE_PHY_CODED_S2);
//        break;
        case 3:
        ble_gap_set_prefered_le_phy(peri_conn_handle, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK,BLE_GAP_LE_PHY_CODED_S8);
        printf("set s8\n");
        break;
    }
}

void key_process(void)
{
    if(f_key_1)
    {
        f_key_1 = false;
        key_1_pro();
    }
    
    if(f_key_2)
    {
        f_key_2 = false;
        /* phy_update_on_connection();*/
    }
}


void ble_report_thread_entry(void *parameter)
{
    while(1) {
        xSemaphoreTake(gpio_sem, portMAX_DELAY);
        key_process();
    }
}

void key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_128);
    
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);

    GPIO_EnableDebounce(P0, BIT6);
    GPIO_EnableDebounce(P1, BIT2);

    GPIO_EnableDigitalPath(P0, BIT6);
    GPIO_EnableDigitalPath(P1, BIT2);

    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT6);
    
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P1, BIT2);
    
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

void init_led(void)
{
    P22 = 0;
    P23 = 0;
    P24 = 0;
    SYS_SET_MFP(P2, 2, GPIO);
    GPIO_SetMode(P2, BIT2, GPIO_MODE_OUTPUT);
    SYS_SET_MFP(P2, 3, GPIO);
    GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT);
    SYS_SET_MFP(P2, 4, GPIO);
    GPIO_SetMode(P2, BIT4, GPIO_MODE_OUTPUT);
}

void key_led_init(void)
{
    init_led();
    gpio_sem = xSemaphoreCreateBinary();
    key_init();
    xTaskCreate(ble_report_thread_entry, "hid_report", 200, NULL, 2, &task_report_hd);
}

