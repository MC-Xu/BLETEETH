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
#include "semphr.h"

#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "nimble/ble.h"
#include "nimble/pan107x/nimble_glue_spark.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "gatts_sens.h"

#define NOTIFY_THROUGHPUT_PAYLOAD 247
#define MIN_REQUIRED_MBUF         1 /* Assuming payload of 500Bytes and each mbuf can take 292Bytes.  */
#define PREFERRED_MTU_VALUE       247
#define LL_PACKET_TIME            2120
#define LL_PACKET_LENGTH          251
#define MTU_DEF                   247

#define NOTIFY_DELAY_MS     15
static const char *tag = "bleprph_throughput";
static const char *device_name = "bleprph_throughput";
static SemaphoreHandle_t notify_sem;
static SemaphoreHandle_t subscribe_sem;
static bool notify_state;
static int notify_test_time = 20;
static uint16_t conn_handle;
/* Dummy variable */
static uint8_t dummy;
static uint8_t gatts_addr_type;

static int gatts_gap_event(struct ble_gap_event *event, void *arg);

#define LPTMR_CURR_CNT_VAL_REG              (0x50020014)


#define CONFIG_EXAMPLE_CONN_ITVL_MIN       12
#define CONFIG_EXAMPLE_CONN_ITVL_MAX       12
#define CONFIG_EXAMPLE_CONN_LATENCY        0
#define CONFIG_EXAMPLE_CONN_TIMEOUT        200
#define CONFIG_EXAMPLE_CONN_CE_LEN_MIN     0
#define CONFIG_EXAMPLE_CONN_CE_LEN_MAX     0

static struct ble_gap_upd_params conn_params = {
    /** Minimum value for connection interval in 1.25ms units */
    .itvl_min = CONFIG_EXAMPLE_CONN_ITVL_MIN,
    /** Maximum value for connection interval in 1.25ms units */
    .itvl_max = CONFIG_EXAMPLE_CONN_ITVL_MAX,
    /** Connection latency */
    .latency = CONFIG_EXAMPLE_CONN_LATENCY,
    /** Supervision timeout in 10ms units */
    .supervision_timeout = CONFIG_EXAMPLE_CONN_TIMEOUT,
    /** Minimum length of connection event in 0.625ms units */
    .min_ce_len = CONFIG_EXAMPLE_CONN_CE_LEN_MIN,
    /** Maximum length of connection event in 0.625ms units */
    .max_ce_len = CONFIG_EXAMPLE_CONN_CE_LEN_MAX,
};

/**
 * Utility function to log an array of bytes.
 */

void
print_bytes(const uint8_t *bytes, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

void
print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
             u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}


static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    printf("handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    printf(" our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    printf(" peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    printf(" peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    printf(" conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}


/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void
gatts_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));

    /*
     * Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /*
     * Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        printf("Error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(gatts_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, gatts_gap_event, NULL);
    if (rc != 0) {
        printf("Error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/* This function sends notifications to the client */
static void notify_task(void *arg)
{
    static uint8_t payload[NOTIFY_THROUGHPUT_PAYLOAD] = {0};/* Data payload */
    int rc, notify_count = 0;
    int64_t start_time, end_time, notify_time_ms = 0;
    struct os_mbuf *om;

    payload[0] = dummy; /* storing dummy data */
    payload[1] = rand();
    payload[99] = rand();

    while (1) {
		
		xSemaphoreTake(subscribe_sem, portMAX_DELAY);
		xSemaphoreGive(notify_sem);
		printf("\n Tx packets...\n");
		start_time = lp_get_curr_tmr_cnt();
		while (notify_time_ms < (notify_test_time * 1000) && notify_state) {
			
			printf("#");
			/* We are anyway using counting semaphore for sending
			* notifications. So hopefully not much waiting period will be
			* introduced before sending a new notification. Revisit this
			* counter if need to do away with semaphore waiting. XXX */
			xSemaphoreTake(notify_sem, portMAX_DELAY);
			if (dummy == 200) {
				dummy = 0;
			}
			dummy++;
		
			om = ble_hs_mbuf_from_flat(payload, sizeof(payload));
		
			if (om) {
				rc = ble_gatts_notify_custom(conn_handle, notify_handle, om);
				if (rc != 0) {
					printf("@");
					notify_count -= 1;
					xSemaphoreGive(notify_sem);
					 /* Most probably error is because we ran out of mbufs (rc = 6),
					  * increase the mbuf count/size from menuconfig. Though
					  * inserting delay is not good solution let us keep it
					  * simple for time being so that the mbufs get freed up
					  * (?), of course assumption is we ran out of mbufs */
					vTaskDelay(NOTIFY_DELAY_MS * configTICK_RATE_HZ / 1000);
				}
			} else {
				notify_count -= 1;
				printf("&");
				vTaskDelay(NOTIFY_DELAY_MS * configTICK_RATE_HZ / 1000);
				xSemaphoreGive(notify_sem);
			}

		 end_time = lp_get_curr_tmr_cnt();
		 notify_time_ms = (end_time - start_time) * 1000 / 32768 ;
		 notify_count += 1;
	}
	
	if (notify_state) {
		
		vTaskDelay(200 * configTICK_RATE_HZ / 1000);
		
		printf("\n*********************************\n");
		printf("Notify throughput = %d bps, count = %d",
				  (notify_count * NOTIFY_THROUGHPUT_PAYLOAD * 8) / notify_test_time, notify_count);
		printf("\n*********************************\n");
		printf(" Notification test complete for stipulated time of %d sec", notify_test_time);
		notify_count = 0;
		notify_time_ms = 0;
		
		vTaskDelay(5000 * configTICK_RATE_HZ / 1000);
		}
	}
     
}

static int
gatts_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        printf("connection %s; status = %d ",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);
        rc = ble_att_set_preferred_mtu(PREFERRED_MTU_VALUE);
        if (rc != 0) {
            printf("Failed to set preferred MTU; rc = %d", rc);
        }

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            gatts_advertise();
        }

		rc = ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);
        
         if (rc != 0) {
                printf("Failed to update params; rc = %d", rc);
        }
            
        conn_handle = event->connect.conn_handle;
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        printf("disconnect; reason = %d", event->disconnect.reason);

        /* Connection terminated; resume advertising */
        gatts_advertise();
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        printf("connection updated; status=%d ",
                 event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
		
		assert(rc == 0);
		if (desc.conn_itvl != CONFIG_EXAMPLE_CONN_ITVL_MIN) {
			ble_gap_update_params(event->connect.conn_handle, &conn_params);
		}
        
        bleprph_print_conn_desc(&desc);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("adv complete ");
        gatts_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        printf("subscribe event; cur_notify=%d; value handle; "
                 "val_handle = %d",
                 event->subscribe.cur_notify, event->subscribe.attr_handle);
        if (event->subscribe.attr_handle == notify_handle) {
            notify_state = event->subscribe.cur_notify;
            if (arg != NULL) {
                printf("notify test time = %d", *(int *)arg);
                notify_test_time = *((int *)arg);
            }
            xSemaphoreGive(subscribe_sem);
        } else if (event->subscribe.attr_handle != notify_handle) {
            notify_state = event->subscribe.cur_notify;
        }
        break;

    case BLE_GAP_EVENT_NOTIFY_TX:
//        printf("NTY\n");
        if ((event->notify_tx.status == 0) ||
                (event->notify_tx.status == BLE_HS_EDONE)) {
            /* Send new notification i.e. give Semaphore. By definition,
             * sending new notifications should not be based on successful
             * notifications sent, but let us adopt this method to avoid too
             * many `BLE_HS_ENOMEM` errors because of continuous transfer of
             * notifications.XXX */
            xSemaphoreGive(notify_sem);
        } else {
//            printf("NTYE=%d\n", event->notify_tx.status);
        }
        break;

    case BLE_GAP_EVENT_MTU:
        printf("mtu update event; conn_handle = %d mtu = %d ",
                 event->mtu.conn_handle,
                 event->mtu.value);
	
        break;
    }
    return 0;
}

static void
gatts_on_sync(void)
{
    int rc;
    uint8_t addr_val[6] = {0};

    printf("gatts_on_sync\n");
    
    rc = ble_hs_id_infer_auto(0, &gatts_addr_type);
    assert(rc == 0);
    rc = ble_hs_id_copy_addr(gatts_addr_type, addr_val, NULL);
    assert(rc == 0);
    printf("Device Address: ");
    print_addr(addr_val);
    
    printf("adv start\n");
    /* Begin advertising */
    gatts_advertise();
}

static void
gatts_on_reset(int reason)
{
    printf("Resetting state; reason=%d\n", reason);
}


#if (CONFIG_OTA_IN_APP)
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


void app_init(void)
{
    int rc;

    printf("app started\n");
	
	#if (CONFIG_OTA_IN_APP)
	print_version_info();
    #endif
	
    /** set public address*/
    uint8_t pub_mac[6]={8,9,9,9,9,6};
	
	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif
	
    db_set_bd_address(pub_mac);
    
    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = gatts_on_sync;
    ble_hs_cfg.reset_cb = gatts_on_reset;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb,
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Initialize Notify Task */
    xTaskCreate(notify_task, "notify_task", 512, NULL, configMAX_PRIORITIES - os_priority_low - 1, NULL);

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);

    notify_sem = xSemaphoreCreateCounting(1, 0);
	subscribe_sem = xSemaphoreCreateCounting(1, 0);
	hs_thread_init();
}
