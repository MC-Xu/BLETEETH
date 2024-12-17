#include "hog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "PANSeries.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_gatt.h"
#include "blehog_sens.h"

#define task_delay_ms(PAR_MS) vTaskDelay(pdMS_TO_TICKS(PAR_MS))

#define LED_IND             P22

#define RSSI_LOG_DEBUG      0
#define RSSI_INTERVAL_MS    50
#define RSSI_DEPTH          10

#define RSSI_DEFAULT_CLOSE_THR_VALUE   -65
#define RSSI_DEFAULT_OPEN_THR_VALUE   -55

#define RSSI_REMOVE_BORDER_NUM         2

static TaskHandle_t task_report_hd;
static SemaphoreHandle_t gpio_sem = NULL;
extern uint16_t hid_consumer_input_handle;
extern uint16_t peri_conn_handle;
extern bool ble_connected;

__IO bool f_key_up = false;
__IO bool f_key_down = false;
bool paired = false;

int8_t rssi_open_thr;
int8_t rssi_close_thr;

#include "sendwave.h"
char wave_buffer[128]; /* maximum use of 128 bytes */

void rssiBubbleSort(int8_t arr[], uint8_t n) {
    for (uint8_t i = 0; i < n - 1; i++) {
        for (uint8_t j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int8_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}


void GPIO0_IRQHandler(void)
{
    printf("GPIO ISR in..\n");

    if (GPIO_GetIntFlag(P0, BIT4)) {
        GPIO_ClrIntFlag(P0, BIT4);
        f_key_up = true;
        printf("P04 occurred\n");
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
    
    if (GPIO_GetIntFlag(P0, BIT5)) {
        GPIO_ClrIntFlag(P0, BIT5);
        f_key_down = true;
        printf("P05 occurred\n");
        xSemaphoreGiveFromISR(gpio_sem, NULL);
    }
}

int hog_send_consumer_key(uint16_t key_val)
{
    static uint16_t ntf;
    struct os_mbuf *om;
    int rc;
    
    ntf = key_val;
    
    om = ble_hs_mbuf_from_flat(&ntf, sizeof(ntf));
    
    rc = ble_gatts_notify_custom(peri_conn_handle, hid_consumer_input_handle, om);
    
    return rc;
}


void key_vol_up(void)
{
    hog_send_consumer_key(CONSUMER_KEY_VAL_VOLUME_INCREMENT);

    printf("key_vol_up pressed\n");

    hog_send_consumer_key(CONSUMER_KEY_VAL_REALEASED);
}

void key_vol_down(void)
{
    hog_send_consumer_key(CONSUMER_KEY_VAL_VOLUME_DECREMENT);

    printf("key_vol_down pressed\n");

    hog_send_consumer_key(CONSUMER_KEY_VAL_REALEASED);
}

void key_process(void)
{
    if(f_key_up)
    {
        f_key_up = false;
        key_vol_up();
    }
    
    if(f_key_down)
    {
        f_key_down = false;
        key_vol_down();
    }
}

struct rssi_buf_st {
    uint8_t item_num;
    int8_t item_buf[RSSI_DEPTH];
    uint8_t index;
};

int8_t rssi_sort_buf[RSSI_DEPTH];

struct rssi_buf_st rssi_buf;

void rssi_buf_init(void)
{
    rssi_buf.item_num = 0;
    rssi_buf.index = 0;
}

bool rssi_buf_is_full(void)
{
    return rssi_buf.item_num == RSSI_DEPTH;
}

uint8_t rssi_get_item_num(void)
{
    return rssi_buf.item_num;
}

int8_t filter_out_max_min_avg_value(void)
{
    rssiBubbleSort(rssi_sort_buf, rssi_buf.item_num);
    
    int rssi_total = 0;
    int rssi_border = 0;

    for(uint8_t i=0; i<rssi_buf.item_num; i++)
    {
        rssi_total += rssi_sort_buf[i];
    }

    if(rssi_buf.item_num > (RSSI_REMOVE_BORDER_NUM*2))
    {
        for(uint8_t i=0; i<RSSI_REMOVE_BORDER_NUM; i++)
        {
            rssi_border += rssi_sort_buf[i];
            rssi_border += rssi_sort_buf[rssi_buf.item_num-1-i];
        }

        return (rssi_total - rssi_border)/(rssi_buf.item_num - RSSI_REMOVE_BORDER_NUM*2);
    }
    
    return rssi_total/rssi_buf.item_num;
}

void rssi_add_into_buf(int8_t rssi)
{
    rssi_buf.item_buf[rssi_buf.index] = rssi;

    #if RSSI_LOG_DEBUG
    printf(" add index: %d value:%d ", rssi_buf.index, rssi);
    #endif

    rssi_buf.index++;
    rssi_buf.index %= RSSI_DEPTH;
    if (rssi_buf.item_num < RSSI_DEPTH)
    {
        rssi_buf.item_num++;
    }
}

void open_led(void)
{
    LED_IND = 1;
}

void close_led(void)
{
    LED_IND = 0;
}

void init_led(void)
{
    SYS_SET_MFP(P2, 2, GPIO);
    GPIO_SetMode(P2, BIT2, GPIO_MODE_OUTPUT);
    LED_IND = 0;
}

void disconnected_process(void)
{
    close_led();
    paired = false;
}

void ble_report_thread_entry(void *parameter)
{
    int8_t rssi = 0;
    while(1) {
        task_delay_ms(RSSI_INTERVAL_MS);
        if (ble_connected && paired)
        {
            ble_gap_conn_rssi(peri_conn_handle, &rssi);
            rssi_add_into_buf(rssi);

            memset(wave_buffer, 0, sizeof(wave_buffer));
            ws_frame_init(wave_buffer);
            ws_add_int8(wave_buffer, CH1, (int8_t)(rssi));
            sendBuffer(wave_buffer, ws_frame_length(wave_buffer));
            
            memcpy(rssi_sort_buf, rssi_buf.item_buf, RSSI_DEPTH);
            int8_t filter_rssi = filter_out_max_min_avg_value();
            
            memset(wave_buffer, 0, sizeof(wave_buffer));
            ws_frame_init(wave_buffer);
            ws_add_int8(wave_buffer, CH2, (int8_t)(filter_rssi));
            sendBuffer(wave_buffer, ws_frame_length(wave_buffer));
            
            if( filter_rssi > rssi_open_thr)
            {
                open_led();
            } else if(filter_rssi < rssi_close_thr)
            {
                close_led();
            }

            #if RSSI_LOG_DEBUG
            printf("rssi items:%d |", rssi_get_item_num());
            for(uint8_t i=0; i<rssi_get_item_num(); i++)
            {
                printf(" item[%d]:%d ", i, rssi_buf.item_buf[i]);
            }
            printf("\n");
            #endif
        }
    }
}

void key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    SYS_SET_MFP(P0, 4, GPIO);
    SYS_SET_MFP(P0, 5, GPIO);
    GPIO_EnableDigitalPath(P0, BIT4|BIT5);
    GPIO_SetMode(P0, BIT4|BIT5, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT4|BIT5);
    GPIO_EnableInt(P0, 4, GPIO_INT_FALLING);
    GPIO_EnableInt(P0, 5, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO0_IRQn);
}

void hog_init(void)
{
    gpio_sem = xSemaphoreCreateBinary();
    rssi_buf_init();
    init_led();
    rssi_open_thr = RSSI_DEFAULT_OPEN_THR_VALUE;
    rssi_close_thr = RSSI_DEFAULT_CLOSE_THR_VALUE;

    key_init();
    xTaskCreate(ble_report_thread_entry, "hid_report", 200, NULL, 2, &task_report_hd);
}

