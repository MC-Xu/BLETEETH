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
#include "ble_acc_sens.h"

#include "pan_hal.h"
#include "sc7a20_driver.h"


/* Connection handle */
static uint16_t conn_handle;
static bool notify_state;

static const char *device_name = "b+acc sensor";
static uint8_t mu_data[] = {0xD1, 0x07, 0xc9, 0x7a, 0xbb, 0x8f, 0xdd, 0x4b, 0x00, 0x11};

#define ADV_UUID    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x20, 0x40, 0x6e
static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;
static struct ble_npl_callout sensor_notify_timer;

/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void blehr_advertise(void)
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

	fields.mfg_data = mu_data;
	fields.mfg_data_len = sizeof(mu_data);
	fields.mfg_data_len = sizeof(mu_data);

	rc = ble_gap_adv_set_fields(&fields);

    if (rc != 0) {

        printf("error setting advertisement data; rc=%d\n", rc);
        return;
    }

	memset(&fields, 0, sizeof(fields));

	fields.uuids128 = (ble_uuid128_t[]) {
		BLE_UUID128_INIT(ADV_UUID)
	};
	fields.num_uuids128 = 1;
	fields.name_is_complete = 1;

	rc = ble_gap_adv_rsp_set_fields(&fields);

	if (rc != 0) {

        printf("error sd setting advertisement data; rc=%d\n", rc);
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




/* This functions simulates heart beat and notifies it to the client */
void notify_data(uint8_t *data, uint16_t len)
{
    int rc;
    struct os_mbuf *om;

	if (notify_state) {
		om = ble_hs_mbuf_from_flat(data, len);

		rc = ble_gatts_notify_custom(conn_handle, c2003_handle, om);
		assert(rc == 0);
	}
}

static int blehr_gap_event(struct ble_gap_event *event, void *arg)
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
        }

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
        conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

		ble_npl_callout_stop(&sensor_notify_timer);
        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("adv complete\n");
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
		notify_state = event->subscribe.cur_notify;
		ble_npl_callout_reset(&sensor_notify_timer, pdMS_TO_TICKS(current_period_time));
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


void period_notify_cb(struct ble_npl_event *e)
{
	coordinate_notify();
	ble_npl_callout_reset(&sensor_notify_timer, pdMS_TO_TICKS(current_period_time));
}

void period_notify_init(void)
{
	ble_npl_callout_init(&sensor_notify_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(), period_notify_cb, NULL);
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
	
	#if CONFIG_BOOT_ENABLE
	print_version_info();
	#endif
	
    printf("app started\n");

    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif

    //get_mac_addr_from_info(pub_mac); /* get mac addr from flash or efuse */
    db_set_bd_address(pub_mac);

    /* Initialize the NimBLE host configuration */
    ble_hs_cfg.sync_cb = blehr_on_sync;

	rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);

	period_notify_init();

	sc7a20_sensor_init();
	hs_thread_init();
}