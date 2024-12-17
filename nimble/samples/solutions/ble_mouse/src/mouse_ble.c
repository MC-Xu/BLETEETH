/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PANSeries.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "console/console.h"
#include "task.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_hs_mbuf.h"
#include "host/ble_gatt.h"

#include "services/gap/ble_svc_gap.h"
#include "services/bas/ble_svc_bas.h"

#include "mouse_common.h"
#include "mouse_ble_gatt_svr.h"
#include "mouse_ble.h"

#include "nimble/pan107x/nimble_glue_spark.h"


#define MODLOG_DFLT printf

uint16_t peri_conn_handle;

static TaskHandle_t task_report_hd;
static const char *ble_dev_name = "pan_mouse";
#if CONFIG_EXAMPLE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_OWN_ADDR_RPA_RANDOM_DEFAULT;
#else
static uint8_t own_addr_type;
#endif
static bool mouse_ble_connected = false;

#if MYNEWT_VAL(BLE_POWER_CONTROL)
static struct ble_gap_event_listener power_control_event_listener;
#endif

static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
void ble_store_config_init(void);

/**
 * Logs information about a connection to the console.
 */
static void bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
	MODLOG_DFLT("handle=%d our_ota_addr_type=%d our_ota_addr=",
		    desc->conn_handle, desc->our_ota_addr.type);
	data_printf(desc->our_ota_addr.val, 6);
	MODLOG_DFLT(" our_id_addr_type=%d our_id_addr=",
		    desc->our_id_addr.type);
	data_printf(desc->our_id_addr.val, 6);
	MODLOG_DFLT(" peer_ota_addr_type=%d peer_ota_addr=",
		    desc->peer_ota_addr.type);
	data_printf(desc->peer_ota_addr.val, 6);
	MODLOG_DFLT(" peer_id_addr_type=%d peer_id_addr=",
		    desc->peer_id_addr.type);
	data_printf(desc->peer_id_addr.val, 6);
	MODLOG_DFLT(" conn_itvl=%d conn_latency=%d supervision_timeout=%d "
		    "encrypted=%d authenticated=%d bonded=%d\n",
		    desc->conn_itvl, desc->conn_latency,
		    desc->supervision_timeout,
		    desc->sec_state.encrypted,
		    desc->sec_state.authenticated,
		    desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void bleprph_advertise(void)
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

	/* C1 03 keyboard C2 03 mouse C2 02 multiple */
	fields.appearance = 0x03c2;
	fields.appearance_is_present = 1;

	/* Indicate that the TX power level field should be included; have the
	 * stack fill this value automatically.  This is done by assigning the
	 * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
	 */
	fields.tx_pwr_lvl_is_present = 1;
	fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

	name = ble_svc_gap_device_name();
	fields.name = (uint8_t *)name;
	fields.name_len = strlen(name);
	fields.name_is_complete = 1;

	fields.uuids16 = (ble_uuid16_t[]) {
		BLE_UUID16_INIT(BT_UUID_HIDS),
		BLE_UUID16_INIT(BLE_SVC_BAS_UUID16),
	};
	fields.num_uuids16 = 2;
	fields.uuids16_is_complete = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0) {
		MODLOG_DFLT("error setting advertisement data; rc=%d\n", rc);
		return;
	}

	/* Begin advertising. */
	memset(&adv_params, 0, sizeof adv_params);
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
	rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
			       &adv_params, bleprph_gap_event, NULL);
	if (rc != 0) {
		MODLOG_DFLT("error enabling advertisement; rc=%d\n", rc);
		return;
	}
}


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static void bleprph_power_control(uint16_t conn_handle)
{
	int rc;

	rc = ble_gap_read_remote_transmit_power_level(conn_handle, 0x01); // Attempting on LE 1M phy
	assert(rc == 0);

	rc = ble_gap_set_transmit_power_reporting_enable(conn_handle, 0x1, 0x1);
	assert(rc == 0);
}
#endif


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static int bleprph_gap_power_event(struct ble_gap_event *event, void *arg)
{

	switch (event->type) {
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
	struct ble_gap_conn_desc desc;
	int rc;

	switch (event->type) {
	case BLE_GAP_EVENT_CONNECT:
		/* A new connection was established or a connection attempt failed. */
		MODLOG_DFLT("connection %s; status=%d ",
			    event->connect.status == 0 ? "established" : "failed",
			    event->connect.status);
		if (event->connect.status == 0) {
			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			assert(rc == 0);
			bleprph_print_conn_desc(&desc);

			struct ble_gap_upd_params params;
			params.itvl_min = 6;
			params.itvl_max = 6;
			params.latency = 0;
			params.supervision_timeout = 100;

			rc = ble_gap_update_params(event->connect.conn_handle, &params);
		}
		MODLOG_DFLT("\n");

		if (event->connect.status != 0) {
			/* Connection failed; resume advertising. */
			bleprph_advertise();
			peri_conn_handle = 0;
		} else {
			peri_conn_handle = event->connect.conn_handle;
		}

		#if MYNEWT_VAL(BLE_POWER_CONTROL)
		bleprph_power_control(event->connect.conn_handle);

		ble_gap_event_listener_register(&power_control_event_listener,
						bleprph_gap_power_event, NULL);
		#endif
		mouse_ble_connected = true;
		return 0;

	case BLE_GAP_EVENT_DISCONNECT:
		MODLOG_DFLT("disconnect; reason=0x%02x ", (0xFF) & (event->disconnect.reason));
		bleprph_print_conn_desc(&event->disconnect.conn);
		MODLOG_DFLT("\n");

		peri_conn_handle = 0;
		/* Connection terminated; resume advertising. */
		bleprph_advertise();
		mouse_ble_connected = false;
		return 0;

	case BLE_GAP_EVENT_CONN_UPDATE:
		/* The central has updated the connection parameters. */
		MODLOG_DFLT("connection updated; status=%d ",
			    event->conn_update.status);
		rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
		assert(rc == 0);
		bleprph_print_conn_desc(&desc);
		MODLOG_DFLT("\n");
		return 0;

	case BLE_GAP_EVENT_ADV_COMPLETE:
		MODLOG_DFLT("advertise complete; reason=%d",
			    event->adv_complete.reason);
		bleprph_advertise();
		return 0;

	case BLE_GAP_EVENT_ENC_CHANGE:
		/* Encryption has been enabled or disabled for this connection. */
		MODLOG_DFLT("encryption change event; status=%d ",
			    event->enc_change.status);
		rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
		assert(rc == 0);
		bleprph_print_conn_desc(&desc);
		MODLOG_DFLT("\n");
		return 0;

	case BLE_GAP_EVENT_NOTIFY_TX:
		#if 0
		MODLOG_DFLT("notify_tx event; conn_handle=%d attr_handle=%d "
			    "status=%d is_indication=%d",
			    event->notify_tx.conn_handle,
			    event->notify_tx.attr_handle,
			    event->notify_tx.status,
			    event->notify_tx.indication);
		#endif
		return 0;

	case BLE_GAP_EVENT_SUBSCRIBE:
		MODLOG_DFLT("subscribe event; conn_handle=%d attr_handle=%d "
			    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
			    event->subscribe.conn_handle,
			    event->subscribe.attr_handle,
			    event->subscribe.reason,
			    event->subscribe.prev_notify,
			    event->subscribe.cur_notify,
			    event->subscribe.prev_indicate,
			    event->subscribe.cur_indicate);
		return 0;

	case BLE_GAP_EVENT_MTU:
		MODLOG_DFLT("mtu update event; conn_handle=%d cid=%d mtu=%d\n",
			    event->mtu.conn_handle,
			    event->mtu.channel_id,
			    event->mtu.value);
		return 0;

	case BLE_GAP_EVENT_REPEAT_PAIRING:
		/* We already have a bond with the peer, but it is attempting to
		 * establish a new secure link.  This app sacrifices security for
		 * convenience: just throw away the old bond and accept the new link.
		 */

		/* Delete the old bond. */
		rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
		assert(rc == 0);
		ble_store_util_delete_peer(&desc.peer_id_addr);

		/* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
		 * continue with the pairing operation.
		 */
		return BLE_GAP_REPEAT_PAIRING_RETRY;

	case BLE_GAP_EVENT_PASSKEY_ACTION:
		MODLOG_DFLT("PASSKEY_ACTION_EVENT started \n");
		struct ble_sm_io pkey = { 0 };

		if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
			pkey.action = event->passkey.params.action;
			pkey.passkey = 123456; // This is the passkey to be entered on peer
			MODLOG_DFLT("Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
			rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
			MODLOG_DFLT("ble_sm_inject_io result: %d\n", rc);
		} else if (event->passkey.params.action == BLE_SM_IOACT_INPUT ||
			   event->passkey.params.action == BLE_SM_IOACT_NUMCMP ||
			   event->passkey.params.action == BLE_SM_IOACT_OOB) {
			MODLOG_DFLT("BLE_SM_IOACT_INPUT, BLE_SM_IOACT_NUMCMP, BLE_SM_IOACT_OOB"
				    " bonding not supported!");
		}
		return 0;
	}
	return 0;
}

static void bleprph_on_reset(int reason)
{
	MODLOG_DFLT("Resetting state; reason=%d\n", reason);
}

#if CONFIG_EXAMPLE_RANDOM_ADDR
static void ble_app_set_addr(void)
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
	uint8_t addr_val[6] = { 0 };
	rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

	MODLOG_DFLT("Device Address: ");
	data_printf(addr_val, 6);
	MODLOG_DFLT("\n");
	/* Begin advertising. */
	bleprph_advertise();
}

void hog_send_mouse_value(void)
{
	uint8_t key_value = 0;
	int16_t x_value = 0;
	int16_t y_value = 0;
	int16_t roll_value = 0;

//	key_get_value_debounced(&key_value);
//	qdec_get_value(&roll_value);

	if (test_mode_auto_circle) {
		auto_circle(&x_value, &y_value);
	} else {
//		sensor_get_value(&x_value, &y_value);
	}

	if (key_value || x_value || y_value || roll_value) {
		uint8_t mouse_data[7] = { 0 };
		struct os_mbuf *om;

		mouse_data[0] = key_value;
		mouse_data[1] = x_value & 0xff;
		mouse_data[2] = x_value >> 8;
		mouse_data[3] = y_value & 0xff;
		mouse_data[4] = y_value >> 8;
		mouse_data[5] = roll_value & 0xff;
		mouse_data[6] = roll_value >> 8;

		om = ble_hs_mbuf_from_flat(&mouse_data, sizeof(mouse_data));
		ble_gatts_notify_custom(peri_conn_handle, hid_mouse_input_handle, om);
	}
}

void ble_report_thread_entry(void *parameter)
{
	while (1) {
		xSemaphoreTake(sem_timer, portMAX_DELAY);
		if (mouse_ble_connected) {
			hog_send_mouse_value();
		}
	}
}


#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"
	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


void mouse_ble_mode_init(void)
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
	ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

	ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
	#ifdef CONFIG_EXAMPLE_BONDING
	ble_hs_cfg.sm_bonding = 1;
	/* Enable the appropriate bit masks to make sure the keys
	 * that are needed are exchanged
	 */
	ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
	ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
	#endif

	#ifdef CONFIG_EXAMPLE_MITM
	ble_hs_cfg.sm_mitm = 1;
	#endif

	#ifdef CONFIG_EXAMPLE_USE_SC
	ble_hs_cfg.sm_sc = 1;
	#else
	ble_hs_cfg.sm_sc = 0;
	#endif

	#ifdef CONFIG_EXAMPLE_RESOLVE_PEER_ADDR
	/* Stores the IRK */
	ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;
	ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;
	#endif

	rc = gatt_svr_init();
	assert(rc == 0);

	/* Set the default device name. */
	rc = ble_svc_gap_device_name_set(ble_dev_name);
	assert(rc == 0);

	/* XXX Need to have template for store */
	ble_store_config_init();

	hs_thread_init();

	xTaskCreate(ble_report_thread_entry, "hid_report", 200, NULL, 2, &task_report_hd);
}

