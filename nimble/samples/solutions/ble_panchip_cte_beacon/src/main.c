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

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "nimble/pan107x/nimble_glue_spark.h"


#define MODLOG_DFLT printf

/**API set public address */
extern uint32_t db_set_bd_address(uint8_t *bd_addr);

static const char *adv_name = "PANCHIP-CTE Beacon";

#define COPMANY_ID	0xD1, 0x07 /* Panchip */
#define PACKET_ID	0x01
#define DEVICE_TYPE	0x20 /* Device type */
#define DF_FIELD	0x67, 0xF7, 0xDB, 0x34, 0xC4, 0x03, 0x8E, 0x5C, 0x0B, 0xAA, 0x97, 0x30, 0x56, 0xE6

/* Advertising data */
static uint8_t manuf_data[] = {
	COPMANY_ID,
	PACKET_ID,
	DEVICE_TYPE,
	/* Carries information of Tag's TX rate, TX power and ID type */
	0x15,

	/* Tag ID */
	0xD2, 0x12, 0x03, 0x00, 0x02, 0x23,
	/* Checksum */
	0xC9,

	DF_FIELD
};

static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
#if CONFIG_EXAMPLE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_OWN_ADDR_RPA_RANDOM_DEFAULT;
#else
static uint8_t own_addr_type;
#endif

#if MYNEWT_VAL(BLE_POWER_CONTROL)
static struct ble_gap_event_listener power_control_event_listener;
#endif

static void print_addr(uint8_t* addr)
{
    for(uint8_t i=0; i<6; i++)
    {
        printf("%02x ", addr[i]);
    }
    printf("\n");
}


/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void ble_cte_beacon_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    fields.mfg_data = manuf_data;
    fields.mfg_data_len = sizeof(manuf_data);

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    memset(&fields, 0, sizeof fields);
    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT("error setting advertisement rsp data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT("error enabling advertisement; rc=%d\n", rc);
        return;
    }

    MODLOG_DFLT("adv start successfully\n");
}


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static void bleprph_power_control(uint16_t conn_handle)
{
    int rc;

    rc = ble_gap_read_remote_transmit_power_level(conn_handle, 0x01 );  // Attempting on LE 1M phy
    assert (rc == 0);

    rc = ble_gap_set_transmit_power_reporting_enable(conn_handle, 0x1, 0x1);
    assert (rc == 0);
}
#endif


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static int
bleprph_gap_power_event(struct ble_gap_event *event, void *arg)
{

    switch(event->type) {
    case BLE_GAP_EVENT_TRANSMIT_POWER:
	MODLOG_DFLT("Transmit power event : status=%d conn_handle=%d reason=%d "
                          "phy=%d power_level=%x power_level_flag=%d delta=%d",
		    event->transmit_power.status,
		    event->transmit_power.conn_handle,
		    event->transmit_power.reason,
		    event->transmit_power.phy,
		    event->transmit_power.transmit_power_level,
		    event->transmit_power.transmit_power_level_flag,
		    event->transmit_power.delta);
	return 0;

    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
	MODLOG_DFLT("Pathloss threshold event : conn_handle=%d current path loss=%d "
                          "zone_entered =%d",
		    event->pathloss_threshold.conn_handle,
		    event->pathloss_threshold.current_path_loss,
		    event->pathloss_threshold.zone_entered);
	return 0;

    default:
	return 0;
    }
}
#endif


/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT("advertise complete; reason=%d",
                    event->adv_complete.reason);
        ble_cte_beacon_advertise();
        return 0;
    default:
        return 0;
    }
}

static void bleprph_on_reset(int reason)
{
    MODLOG_DFLT("Resetting state; reason=%d\n", reason);
}

#if CONFIG_EXAMPLE_RANDOM_ADDR
static void
ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);

    assert(rc == 0);
}
#endif

static void bleprph_on_sync(void)
{
    int rc;

#if CONFIG_EXAMPLE_RANDOM_ADDR
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();
#endif

    /* Make sure we have proper identity address set (public preferred) */
#if CONFIG_EXAMPLE_RANDOM_ADDR
    rc = ble_hs_util_ensure_addr(1);
#else
    rc = ble_hs_util_ensure_addr(0);
#endif
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT("error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    MODLOG_DFLT("Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT("\n");
    /* Begin advertising. */
    ble_cte_beacon_advertise();
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
	
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif


    db_set_bd_address(pub_mac);

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(adv_name);
    assert(rc == 0);

    hs_thread_init();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("task overflow:%s\r\n", pcTaskName);
    while(1);
}
