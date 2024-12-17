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
#include "key_led.h"

/* Connection handle */
uint16_t peri_conn_handle;

static bool notify_state;

static const char *device_name = "ble_distance";

static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;

/* Sending notify data timer */
static struct ble_npl_callout blehr_tx_timer;
static struct ble_npl_callout phy_update_timer;

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

static void
phy_update_timer_reset(void)
{
    int rc;

    rc = ble_npl_callout_reset(&phy_update_timer, configTICK_RATE_HZ);
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

    rc = ble_gatts_notify_custom(peri_conn_handle, hrs_hrm_handle, om);

    assert(rc == 0);
    blehr_tx_hrate_reset();
}

static void
phy_update_func(struct ble_npl_event *ev)
{
    ble_npl_callout_stop(&phy_update_timer);
    phy_update_on_connection();
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
            peri_conn_handle = 0;
        }
        else {
          peri_conn_handle = event->connect.conn_handle;
          phy_update_timer_reset();
        }
        
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
        peri_conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset peri_conn_handle */

        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("adv complete\n");
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
	#if (LOW_POWER_TESET_LATENCY_100MS == 0 && LOW_POWER_TESET_LATENCY_1000MS == 0)
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
	#endif
        break;

    case BLE_GAP_EVENT_MTU:
        printf("mtu update event; peri_conn_handle=%d mtu=%d\n",
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

#if 1
    /* Use privacy */
    rc = ble_hs_id_infer_auto(0, &blehr_addr_type);
    assert(rc == 0);
#else //random address
	ble_addr_t rnd_addr;
	rc = ble_hs_id_gen_rnd(1, &rnd_addr);
	assert(rc == 0);
	
	rc = ble_hs_id_set_rnd(rnd_addr.val);
	assert(rc == 0);
	
	blehr_addr_type = 1;//use rnd addr
#endif

    /* Begin advertising */
    blehr_advertise();
}

#if CONFIG_BOOT_ENABLE
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
	
	#if CONFIG_BOOT_ENABLE
	print_version_info();
    #endif
	
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};
	
	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif
	
    db_set_bd_address(pub_mac);
    
    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = blehr_on_sync;

    ble_npl_callout_init(&blehr_tx_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(),
                    blehr_tx_hrate, NULL);

    ble_npl_callout_init(&phy_update_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(),
                    phy_update_func, NULL);

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);

    key_led_init();

	hs_thread_init();
}
