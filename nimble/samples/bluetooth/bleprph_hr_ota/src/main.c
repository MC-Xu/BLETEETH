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
#include "blehr_sens.h"
#include "smp_bt.h"
#include "smp/smp.h"
#include "img_mgmt/img_mgmt.h"
extern void img_mgmt_module_init(void);
extern void smp_ble_pkg_init(void);


#define LOW_POWER_TESET_CI_100MS         0
#define LOW_POWER_TESET_CI_1000MS        0
#define LOW_POWER_TESET_LATENCY_100MS    0
#define LOW_POWER_TESET_LATENCY_1000MS   0

#if ((LOW_POWER_TESET_CI_100MS+LOW_POWER_TESET_CI_1000MS+LOW_POWER_TESET_LATENCY_100MS+LOW_POWER_TESET_LATENCY_1000MS)>1)
#error "Only 1 test mode exist at a time!"
#endif

/* Connection handle */
static uint16_t conn_handle;

#if LOW_POWER_TESET_CI_100MS || LOW_POWER_TESET_CI_1000MS || LOW_POWER_TESET_LATENCY_100MS || LOW_POWER_TESET_LATENCY_1000MS
void testTimerCb(TimerHandle_t xTimer) {
    #if LOW_POWER_TESET_CI_100MS
    struct ble_gap_upd_params upd_params = {
        .itvl_min = 80,
        .itvl_max = 80,
        .latency = 0,
        .supervision_timeout = 500,
    };
    #endif

    #if LOW_POWER_TESET_CI_1000MS
    struct ble_gap_upd_params upd_params = {
        .itvl_min = 800,
        .itvl_max = 800,
        .latency = 0,
        .supervision_timeout = 500,
    };
    #endif

    #if LOW_POWER_TESET_LATENCY_100MS
    struct ble_gap_upd_params upd_params = {
        .itvl_min = 8,
        .itvl_max = 8,
        .latency = 9,
        .supervision_timeout = 500,
    };
    #endif

    #if LOW_POWER_TESET_LATENCY_1000MS
    struct ble_gap_upd_params upd_params = {
        .itvl_min = 8,
        .itvl_max = 8,
        .latency = 99,
        .supervision_timeout = 500,
    };
    #endif

    ble_gap_update_params(conn_handle, &upd_params);
}

/*LowPower_Deamon_Timer for 32K ticks overflow in 32 bits reg in a longtime unwakable scene*/
void LowPower_Test_Timer()
{
    TimerHandle_t timer = xTimerCreate("LPTest", pdMS_TO_TICKS(2000), pdFALSE, 0, testTimerCb);

    xTimerStart(timer, 0);
}

#endif

static bool notify_state;

static const char *device_name = "ble_hr_ota";

static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;

/* Sending notify data timer */
static struct ble_npl_callout blehr_tx_timer;

/* Variable to simulate heart beats */
static uint8_t heartrate = 90;

/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void
blehr_advertise(void)
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


    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {

        printf("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    #if LOW_POWER_TESET_CI_100MS || LOW_POWER_TESET_LATENCY_100MS
    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(100);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(100);
    #endif

    #if LOW_POWER_TESET_CI_1000MS || LOW_POWER_TESET_LATENCY_1000MS
    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(1000);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(1000);
    #endif

    rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, blehr_gap_event, NULL);
    if (rc != 0) {
        printf("error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

static void
blehr_tx_hrate_stop(void)
{
    ble_npl_callout_stop(&blehr_tx_timer);
}

/* Reset heartrate measurment */
static void
blehr_tx_hrate_reset(void)
{
    int rc;

    rc = ble_npl_callout_reset(&blehr_tx_timer, configTICK_RATE_HZ);
    assert(rc == 0);
}

/* This functions simulates heart beat and notifies it to the client */
static void
blehr_tx_hrate(struct ble_npl_event *ev)
{
    static uint8_t hrm[2];
    int rc;
    struct os_mbuf *om;

    if (!notify_state) {
        blehr_tx_hrate_stop();
        heartrate = 90;
        return;
    }

    hrm[0] = 0x06; /* contact of a sensor */
    hrm[1] = heartrate; /* storing dummy data */

    /* Simulation of heart beats */
    heartrate++;
    if (heartrate == 160) {
        heartrate = 90;
    }

    om = ble_hs_mbuf_from_flat(hrm, sizeof(hrm));

    rc = ble_gatts_notify_custom(conn_handle, hrs_hrm_handle, om);

    assert(rc == 0);
    blehr_tx_hrate_reset();
}

static int
blehr_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        printf("connection %s; status=%d\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            blehr_advertise();
            conn_handle = 0;
        }
        else {
          conn_handle = event->connect.conn_handle;
          #if LOW_POWER_TESET_CI_100MS || LOW_POWER_TESET_CI_1000MS || LOW_POWER_TESET_LATENCY_100MS || LOW_POWER_TESET_LATENCY_1000MS
            LowPower_Test_Timer();
          #endif
        }

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
        conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("adv complete\n");
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        printf("subscribe event; cur_notify=%d\n value handle; "
                          "val_handle=%d\n",
                    event->subscribe.cur_notify, hrs_hrm_handle);
        if (event->subscribe.attr_handle == hrs_hrm_handle) {
            notify_state = event->subscribe.cur_notify;
            blehr_tx_hrate_reset();
        } else if (event->subscribe.attr_handle != hrs_hrm_handle) {
            notify_state = event->subscribe.cur_notify;
            blehr_tx_hrate_stop();
        }
        break;

    case BLE_GAP_EVENT_MTU:
        printf("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);

        break;

    }

    return 0;
}

static void
blehr_on_sync(void)
{
    int rc;

    /* Use privacy */
    rc = ble_hs_id_infer_auto(0, &blehr_addr_type);
    assert(rc == 0);

    /* Begin advertising */
    blehr_advertise();
}

static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)0xA000;

	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}


void app_init(void)
{
    int rc;

    printf("app started\n");

	print_version_info();

    img_mgmt_module_init();

    /** set public address*/
    uint8_t pub_mac[6]={0,2,3,4,5,6};

	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif

    db_set_bd_address(pub_mac);

    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = blehr_on_sync;

    ble_npl_callout_init(&blehr_tx_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(),
                    blehr_tx_hrate, NULL);

    rc = gatt_svr_init();
    assert(rc == 0);

    smp_ble_pkg_init();

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);

	hs_thread_init();
}
