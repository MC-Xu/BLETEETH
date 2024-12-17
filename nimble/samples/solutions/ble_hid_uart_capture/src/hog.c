#include "hog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "PANSeries.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_gatt.h"
#include "blehog_sens.h"

#define task_delay_ms(PAR_MS) vTaskDelay(pdMS_TO_TICKS(PAR_MS))

static TaskHandle_t task_report_hd;
static SemaphoreHandle_t gpio_sem = NULL;
extern uint16_t hid_consumer_input_handle;
extern uint16_t peri_conn_handle;

__IO bool f_key_up = false;
__IO bool f_key_down = false;

#ifdef IP_101x
static void button_event(void)
{	
	extern void ble_app_user_evt_post(void);
	ble_app_user_evt_post();
}
#endif

void GPIO0_IRQHandler(void)
{
    if (GPIO_GetIntFlag(P0, BIT6)) {
        GPIO_ClrIntFlag(P0, BIT6);
        f_key_up = true;
        printf("P06 occurred\n");
		#ifdef IP_101x
		button_event();
		#else
        xSemaphoreGiveFromISR(gpio_sem, NULL);
		#endif
    }
}

void GPIO1_IRQHandler(void)
{
    if (GPIO_GetIntFlag(P1, BIT2)) {
        GPIO_ClrIntFlag(P1, BIT2);
        f_key_down = true;
        printf("P12 occurred\n");
        #ifdef IP_101x
		button_event();
		#else
        xSemaphoreGiveFromISR(gpio_sem, NULL);
		#endif
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
	hog_send_consumer_key(CONSUMER_KEY_VAL_VCR_PLUS);
//    hog_send_consumer_key(CONSUMER_KEY_VAL_AC_ZOOM_IN);

    hog_send_consumer_key(CONSUMER_KEY_VAL_REALEASED);

    printf("key_vol_up pressed\n");
}

void key_vol_down(void)
{
	hog_send_consumer_key(CONSUMER_KEY_VAL_VOLUME_DECREMENT);
//	hog_send_consumer_key(CONSUMER_KEY_VAL_AC_ZOOM_OUT);

    hog_send_consumer_key(CONSUMER_KEY_VAL_REALEASED);
	
	printf("key_vol_down pressed\n");
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



#ifdef IP_101x
void ble_app_user_evt(struct ble_npl_event *ev)
{
	key_process();
}
#else
void ble_report_thread_entry(void *parameter)
{
    while(1) {
        xSemaphoreTake(gpio_sem, portMAX_DELAY);
        key_process();
    }
}
#endif

void key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);
    GPIO_EnableDigitalPath(P0, BIT6);
    GPIO_EnableDigitalPath(P1, BIT2);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_EnablePullupPath(P1, BIT2);
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

#ifdef IP_101x
void hog_init(void)
{
	return;
}
#else
void hog_init(void)
{
    gpio_sem = xSemaphoreCreateBinary();
    key_init();
    xTaskCreate(ble_report_thread_entry, "hid_report", 200, NULL, 2, &task_report_hd);
}
#endif


