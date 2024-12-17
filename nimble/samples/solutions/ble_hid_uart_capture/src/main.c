/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <string.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
/* Mandatory services. */
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nimble/ble.h"

#include "nimble/pan107x/nimble_glue_spark.h"

/* Application-specified header. */
#include "blecent.h"
#include "uart_AT.h"
#include "blehr_sens.h"
#include "blehog_sens.h"
#include "hog.h"
#include "ll_api.h"
/* Connection handle */
uint16_t peri_conn_handle;
static uint16_t central_conn_handle;
static uint16_t central_charac_write_handle;
void ble_store_config_init(void);
void hex_to_string(uint8_t *buff_num, uint8_t length, uint8_t *buff_str_out);

static int blecent_gap_event(struct ble_gap_event *event, void *arg);

//config EXAMPLE_IO_TYPE
//int
//default 0 if BLE_SM_IO_CAP_DISP_ONLY
//default 1 if BLE_SM_IO_CAP_DISP_YES_NO
//default 2 if BLE_SM_IO_CAP_KEYBOARD_ONLY
//default 3 if BLE_SM_IO_CAP_NO_IO
//default 4 if BLE_SM_IO_CAP_KEYBOARD_DISP

#define CONFIG_EXAMPLE_IO_TYPE  3
#define CONFIG_EXAMPLE_RANDOM_ADDR  0
#define CONFIG_EXAMPLE_USE_SC  1
#define CONFIG_EXAMPLE_RESOLVE_PEER_ADDR 1
#define CONFIG_EXAMPLE_BONDING 1
#define CONFIG_EXAMPLE_MITM 1

/**
 * Application callback.  Called when the read of the ANS Supported New Alert
 * Category characteristic has completed.
 */
static int
blecent_on_read(uint16_t conn_handle,
		const struct ble_gatt_error *error,
		struct ble_gatt_attr *attr,
		void *arg)
{
	printf("Read complete; status=%d conn_handle=%d", error->status,
	       conn_handle);
	if (error->status == 0) {
		printf(" attr_handle=%d value=", attr->handle);
		print_mbuf(attr->om);
	}
	printf("\n");

	return 0;
}

/**
 * Application callback.  Called when the write to the ANS Alert Notification
 * Control Point characteristic has completed.
 */
static int
blecent_on_write(uint16_t conn_handle,
		 const struct ble_gatt_error *error,
		 struct ble_gatt_attr *attr,
		 void *arg)
{
	printf("Write complete; status=%d conn_handle=%d attr_handle=%d\n",
	       error->status, conn_handle, attr->handle);

	return 0;
}

/**
 * Application callback.  Called when the attempt to subscribe to notifications
 * for the ANS Unread Alert Status characteristic has completed.
 */
static int
blecent_on_subscribe(uint16_t conn_handle,
		     const struct ble_gatt_error *error,
		     struct ble_gatt_attr *attr,
		     void *arg)
{
	printf("Subscribe complete; status=%d conn_handle=%d "
	       "attr_handle=%d\n",
	       error->status, conn_handle, attr->handle);

	return 0;
}

/**
 * Performs three concurrent GATT operations against the specified peer:
 * 1. Reads the ANS Supported New Alert Category characteristic.
 * 2. Writes the ANS Alert Notification Control Point characteristic.
 * 3. Subscribes to notifications for the ANS Unread Alert Status
 *    characteristic.
 *
 * If the peer does not support a required service, characteristic, or
 * descriptor, then the peer lied when it claimed support for the alert
 * notification service!  When this happens, or if a GATT procedure fails,
 * this function immediately terminates the connection.
 */
static void
blecent_read_write_subscribe(const struct peer *peer)
{
	const struct peer_chr *chr;
	const struct peer_dsc *dsc;
	uint8_t value[2];
	int rc;

	/* Read the supported-new-alert-category characteristic. */
	chr = peer_chr_find_uuid(peer,
				 BT_UUID_CUSTOM_SERVICE_VAL,
				 BT_UUID_CUSTOM_CHARAC_VAL1);
	if (chr == NULL) {
		printf("Error: Peer doesn't support the Supported New "
		       "Alert Category characteristic\n");
		goto err;
	}

	rc = ble_gattc_read(peer->conn_handle, chr->chr.val_handle,
			    blecent_on_read, NULL);
	if (rc != 0) {
		printf("Error: Failed to read characteristic; rc=%d\n",
		       rc);
		goto err;
	}

	/* Write two bytes (99, 100) to the alert-notification-control-point
	 * characteristic.
	 */
	chr = peer_chr_find_uuid(peer,
				 BT_UUID_CUSTOM_SERVICE_VAL,
				 BT_UUID_CUSTOM_CHARAC_VAL2);
	if (chr == NULL) {
		printf("Error: Peer doesn't support the Alert "
		       "Notification Control Point characteristic\n");
		goto err;
	}

	value[0] = 99;
	value[1] = 100;
	central_conn_handle = peer->conn_handle;
	central_charac_write_handle = chr->chr.val_handle;
	rc = ble_gattc_write_flat(peer->conn_handle, chr->chr.val_handle,
				  value, sizeof value, blecent_on_write, NULL);
	if (rc != 0) {
		printf("Error: Failed to write characteristic; rc=%d\n",
		       rc);
	}

	/* Subscribe to notifications for the Unread Alert Status characteristic.
	 * A central enables notifications by writing two bytes (1, 0) to the
	 * characteristic's client-characteristic-configuration-descriptor (CCCD).
	 */
	dsc = peer_dsc_find_uuid(peer,
				 BT_UUID_CUSTOM_SERVICE_VAL,
				 BT_UUID_CUSTOM_CHARAC_VAL1,
				 BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
	if (dsc == NULL) {
		printf("Error: Peer lacks a CCCD for the Unread Alert "
		       "Status characteristic\n");
		goto err;
	}

	value[0] = 1;
	value[1] = 0;
	rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,
				  value, sizeof value, blecent_on_subscribe, NULL);
	if (rc != 0) {
		printf("Error: Failed to subscribe to characteristic; "
		       "rc=%d\n", rc);
		goto err;
	}

	return;

err:
	/* Terminate the connection. */
	ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

/**
 * Called when service discovery of the specified peer has completed.
 */
static void
blecent_on_disc_complete(const struct peer *peer, int status, void *arg)
{

	if (status != 0) {
		/* Service discovery failed.  Terminate the connection. */
		printf("Error: Service discovery failed; status=%d "
		       "conn_handle=%d\n", status, peer->conn_handle);
		ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
		return;
	}

	/* Service discovery has completed successfully.  Now we have a complete
	 * list of services, characteristics, and descriptors that the peer
	 * supports.
	 */
	printf("Service discovery complete; status=%d "
	       "conn_handle=%d\n", status, peer->conn_handle);

	/* Now perform three concurrent GATT procedures against the peer: read,
	 * write, and subscribe to notifications.
	 */
	blecent_read_write_subscribe(peer);

	TGT_SendMultiData("CONN DONE\n", sizeof("CONN DONE\n") - 1);
}

/**
 * Initiates the GAP general discovery procedure.
 */
void blecent_scan(void)
{
	uint8_t own_addr_type;
	struct ble_gap_disc_params disc_params;
	int rc;

	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		printf("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Tell the controller to filter duplicates; we don't want to process
	 * repeated advertisements from the same device.
	 */
	disc_params.filter_duplicates = 0;

	/**
	 * Perform a passive scan.  I.e., don't send follow-up scan requests to
	 * each advertiser.
	 */
	disc_params.passive = 1;

	/* Use defaults for the rest of the parameters. */
	disc_params.itvl = 60;
	disc_params.window = 50;
	disc_params.filter_policy = 0;
	disc_params.limited = 0;

	rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params,
			  blecent_gap_event, NULL);
	if (rc != 0) {
		printf("Error initiating GAP discovery procedure; rc=%d\n",
		       rc);
	}
}

/**
 * Indicates whether we should tre to connect to the sender of the specified
 * advertisement.  The function returns a positive result if the device
 * advertises connectability and support for the Alert Notification service.
 */
static int
blecent_should_connect(const struct ble_gap_disc_desc *disc)
{
	struct ble_hs_adv_fields fields;
	int rc;
	int i;

	/* The device has to be advertising connectability. */
	if (disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
	    disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND) {

		return 0;
	}

	rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);
	if (rc != 0) {
		return rc;
	}

	/* The device has to advertise support for the Alert Notification
	 * service (BT_UUID_CUSTOM_SERVICE_VAL).
	 */
//    uint8_t uart_service_uuid[16] =
//    { 0xff, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
//      0x78, 0x56, 0x34, 0x12,
//      0x78, 0x56, 0x34, 0x12 };
//     const ble_uuid128_t *uuid = BT_UUID_CUSTOM_SERVICE_VAL;

	for (i = 0; i < fields.num_uuids128; i++) {
		if (!memcmp(BLE_UUID128(&fields.uuids128[i])->value, BLE_UUID128(BT_UUID_CUSTOM_SERVICE_VAL)->value, 16)) {
			// if (!memcmp(fields.uuids128[i].value, uuid->value, 16)) {
			return 1;
		}
	}
	return 0;
}

/**
 * Connects to the sender of the specified advertisement of it looks
 * interesting.  A device is "interesting" if it advertises connectability and
 * support for the Alert Notification service.
 */
static void
blecent_connect_if_interesting(const struct ble_gap_disc_desc *disc)
{
	uint8_t own_addr_type;
	int rc;

	/*Filter adv by rssi*/
	if (disc->rssi < -70) {
		return;
	}

	/* Don't do anything if we don't care about this advertiser. */
	if (!blecent_should_connect(disc)) {
		return;
	}

	/* Scanning must be stopped before a connection can be initiated. */
	rc = ble_gap_disc_cancel();
	if (rc != 0) {
		printf("Failed to cancel scan; rc=%d\n", rc);
		return;
	}

	/* Figure out address to use for connect (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		printf("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for
	 * timeout.
	 */
	rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, NULL,
			     blecent_gap_event, NULL);
	if (rc != 0) {
		printf("Error: Failed to connect to device; addr_type=%d "
		       "addr=%s\n; rc=%d",
		       disc->addr.type, addr_str(disc->addr.val), rc);
		return;
	}
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that is
 * established.  blecent uses the same callback for all connections.
 *
 * @param event                 The event being signalled.
 * @param arg                   Application-specified argument; unused by
 *                                  blecent.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
blecent_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
	struct ble_hs_adv_fields fields;
	int rc;

	switch (event->type) {
	case BLE_GAP_EVENT_DISC:
		rc = ble_hs_adv_parse_fields(&fields, event->disc.data,
					     event->disc.length_data);
		if (rc != 0) {
			return 0;
		}

		/* An advertisment report was received during GAP discovery. */
		// print_adv_fields(&fields);

		/* Try to connect to the advertiser if it looks interesting. */
		blecent_connect_if_interesting(&event->disc);
		return 0;

	case BLE_GAP_EVENT_CONNECT:
		/* A new connection was established or a connection attempt failed. */
		if (event->connect.status == 0) {
			/* Connection successfully established. */
			printf("Connection established ");

			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			assert(rc == 0);
			print_conn_desc(&desc);
			printf("\n");

			/* Remember peer. */
			rc = peer_add(event->connect.conn_handle);
			if (rc != 0) {
				printf("Failed to add peer; rc=%d\n", rc);
				return 0;
			}

			/* Perform service discovery. */
			rc = peer_disc_all(event->connect.conn_handle,
					   blecent_on_disc_complete, NULL);
			if (rc != 0) {
				printf("Failed to discover services; rc=%d\n", rc);
				return 0;
			}
		} else {
			/* Connection attempt failed; resume scanning. */
			printf("Error: Connection failed; status=%d\n",
			       event->connect.status);
			blecent_scan();
		}

		return 0;

	case BLE_GAP_EVENT_DISCONNECT:
		/* Connection terminated. */
		printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
		print_conn_desc(&event->disconnect.conn);
		printf("\n");

		/* Forget about peer. */
		peer_delete(event->disconnect.conn.conn_handle);

		/* Resume scanning. */
		blecent_scan();
		return 0;

	case BLE_GAP_EVENT_DISC_COMPLETE:
		printf("discovery complete; reason=%d\n",
		       event->disc_complete.reason);
		return 0;

	case BLE_GAP_EVENT_ENC_CHANGE:
		/* Encryption has been enabled or disabled for this connection. */
		printf("cen encrypt change; status=%d ",
		       event->enc_change.status);
		rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
		assert(rc == 0);
		print_conn_desc(&desc);
		return 0;

	case BLE_GAP_EVENT_NOTIFY_RX:
		/* Peer sent us a notification or indication. */
		printf("received %s; conn_handle=%d attr_handle=%d "
		       "attr_len=%d\n",
		       event->notify_rx.indication ?
		       "indication" :
		       "notification",
		       event->notify_rx.conn_handle,
		       event->notify_rx.attr_handle,
		       OS_MBUF_PKTLEN(event->notify_rx.om));

		data_printf(OS_MBUF_DATA(event->notify_rx.om, uint8_t *), OS_MBUF_PKTLEN(event->notify_rx.om));

		TGT_SendMultiData("Peri Notify:", sizeof("Peri Notify:") - 1);
		TGT_SendMultiData(OS_MBUF_DATA(event->notify_rx.om, uint8_t *), OS_MBUF_PKTLEN(event->notify_rx.om));

		/* Attribute data is contained in event->notify_rx.attr_data. */
		return 0;

	case BLE_GAP_EVENT_MTU:
		printf("mtu update event; conn_handle=%d cid=%d mtu=%d\n",
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

	default:
		return 0;
	}
}


static bool notify_state_00c8;
static bool notify_state_ff81;
static bool notify_state_ff03;

static int bleph_gap_event(struct ble_gap_event *event, void *arg);

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

	printf("blehr_advertise\n");
	/*
	 *  Set the advertisement data included in our advertisements:
	 *     o Flags (indicates advertisement type and other general info)
	 *     o Advertising tx power
	 *     o Device name
	 */
	memset(&fields, 0, sizeof(fields));

	/* Set the default device name. */
	rc = ble_svc_gap_device_name_set((const char*)g_fmc_config_data.device_name);
	assert(rc == 0);
	/*
	 * Advertise two flags:
	 *      o Discoverability in forthcoming advertisement (general)
	 *      o BLE-only (BR/EDR unsupported)
	 */
	fields.flags = BLE_HS_ADV_F_DISC_GEN |
		       BLE_HS_ADV_F_BREDR_UNSUP;

	fields.name = (uint8_t *)g_fmc_config_data.device_name;
	fields.name_len = g_fmc_config_data.name_length;
	fields.name_is_complete = 1;
	
	uint8_t data[8] = {0x00, 0x00, 0x00, 0x15, 0x83, 0x7c, 0xae, 0xdf};
    fields.mfg_data = data;
    fields.mfg_data_len = 9;

    fields.appearance = BLE_SVC_GAP_APPEARANCE_GEN_HID + 4;
    fields.appearance_is_present = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0) {

		printf("error setting advertisement data; rc=%d\n", rc);
		return;
	}

	/* Begin advertising */
	memset(&adv_params, 0, sizeof(adv_params));
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

//	ble_att_set_preferred_mtu(247);
	rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER,
			       &adv_params, bleph_gap_event, NULL);
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
void peri_notify(uint8_t *data, uint8_t len)
{
	struct os_mbuf *om;

	if (!notify_state_00c8) {
		printf("00c8 notify_state %d\n", notify_state_00c8);
		return;
	}

	om = ble_hs_mbuf_from_flat(data, len);

	ble_gatts_notify_custom(peri_conn_handle, hrs_hrm_handle, om);
}

void ff81_notify(uint8_t *data, uint8_t len)
{
	struct os_mbuf *om;

	if (!notify_state_ff81) {
		printf("ff81 notify_state %d\n", notify_state_ff81);
		return;
	}

	om = ble_hs_mbuf_from_flat(data, len);

	ble_gatts_notify_custom(peri_conn_handle, ff80_handle, om);
}

void ff03_notify(uint8_t *data, uint8_t len)
{
	struct os_mbuf *om;

	if (!notify_state_ff03) {
		printf("ff03 notify_state %d\n", notify_state_ff03);
		return;
	}

	om = ble_hs_mbuf_from_flat(data, len);

	ble_gatts_notify_custom(peri_conn_handle, ff00_handle, om);
}

void central_write(uint8_t *data, uint8_t len)
{
	int rc;

	rc = ble_gattc_write_flat(central_conn_handle, central_charac_write_handle,
				  data, len, blecent_on_write, NULL);
	if (rc != 0) {
		printf("Error: Failed to write characteristic; rc=%d\n",
		       rc);
	}

}

static int
btshell_on_mtu(uint16_t conn_handle, const struct ble_gatt_error *error,
	       uint16_t mtu, void *arg)
{
	switch (error->status) {
	case 0:
		printf("mtu exchange complete: conn_handle=%d mtu=%d\n",
		       conn_handle, mtu);
		break;

	default:
		printf("mtu exchange err\n");
		break;
	}

	return 0;
}

static int
bleph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_sm_io pkey = {0};
	struct ble_gap_conn_desc desc;
	int rc;

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
		} else {
			peri_conn_handle = event->connect.conn_handle;
			printf("connect to phone peri_conn_handle %d\n", peri_conn_handle);
		}
//		ble_gattc_exchange_mtu(event->connect.conn_handle, btshell_on_mtu, NULL);

		break;

	case BLE_GAP_EVENT_DISCONNECT:
		printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
		peri_conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

		/* Connection terminated; resume advertising */
		blehr_advertise();
		break;

	case BLE_GAP_EVENT_ADV_COMPLETE:
		printf("adv complete\n");
		blehr_advertise();
		break;

	case BLE_GAP_EVENT_SUBSCRIBE:  
		if (event->subscribe.attr_handle == hrs_hrm_handle) {
			notify_state_00c8 = event->subscribe.cur_notify;
			printf("subscribe event; cur_notify=%d\n value handle; "
		       "00c8 val_handle=%d\n",
		       event->subscribe.cur_notify, hrs_hrm_handle);
		}else if (event->subscribe.attr_handle == ff00_handle) {
			notify_state_ff03 = event->subscribe.cur_notify;
			printf("subscribe event; cur_notify=%d\n value handle; "
		       "ff00 val_handle=%d\n",
		       event->subscribe.cur_notify, ff00_handle);
			
			uint8_t data1[2] = {0x01, 0x07};
			ff03_notify(data1, 2);
			
			vTaskDelay(100);
			uint8_t data2[3] = {0x02, 0xB6, 0x00};
			ff03_notify(data2, 3);
		} else if (event->subscribe.attr_handle == ff80_handle) {
			notify_state_ff81 = event->subscribe.cur_notify;
			printf("subscribe event; cur_notify=%d\n value handle; "
		       "ff80 val_handle=%d\n",
		       event->subscribe.cur_notify, ff80_handle);
		}  else if (event->subscribe.attr_handle == hid_consumer_input_handle) {
			printf("subscribe event; cur_notify=%d\n value handle; "
		       "hid val_handle=%d\n",
		       event->subscribe.cur_notify, hid_consumer_input_handle);
		} 
		break;

	case BLE_GAP_EVENT_MTU:
		printf("mtu update event; conn_handle=%d mtu=%d\n",
		       event->mtu.conn_handle,
		       event->mtu.value);
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        printf("prph encryt change; status=%d\n",
                    event->enc_change.status);
		break;
	
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
        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = g_fmc_config_data.passkey; // This is the passkey to be entered on peer
            printf("Enter passkey %" PRIu32 " on the peer side\n", pkey.passkey);
            int rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            printf("ble_sm_inject_io result: %d\n", rc);
        }
		break;
	}

	return 0;
}

static void
blecent_on_reset(int reason)
{
	printf("Resetting state; reason=%d\n", reason);
}

static void
blecent_on_sync(void)
{
	int rc;
	uint8_t device_mac[8] = {0};
	uint8_t mac_str[12] = {0};

	/* Make sure we have proper identity address set (public preferred) */
	rc = ble_hs_util_ensure_addr(0);
	assert(rc == 0);
	
//	put_le64(device_mac, LL_GetBdAddr());
//	swap_in_place(device_mac, BD_ADDR_LEN);
//	hex_to_string(device_mac, BD_ADDR_LEN, mac_str);
//	memcpy(g_fmc_config_data.device_name + g_fmc_config_data.name_length, &mac_str[8], 4);
//	g_fmc_config_data.name_length += 4;

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



/**
 * main
 *
 * All application logic and NimBLE host work is performed in default task.
 *
 * @return int NOTE: this function should never return!
 */
void app_init(void)
{
	uart_at_init();

	int rc;

#if CONFIG_BOOT_ENABLE
    print_version_info();
#endif

	/** set public address*/
	uint8_t pub_mac[6];
	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif
	memcpy(pub_mac, g_fmc_config_data.own_mac, 6);
	db_set_bd_address(pub_mac);

	/* Configure the host. */
	ble_hs_cfg.reset_cb = blecent_on_reset;
	ble_hs_cfg.sync_cb = blecent_on_sync;
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

	ble_svc_gap_init();
	
	rc = gatt_svr_init();
	assert(rc == 0);
	
	rc = gatt_svr_hid_init();
	assert(rc == 0);

	/* Initialize data structures to track connected peers. */
	rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 32, 32, 32);
	assert(rc == 0);

	/* Persist config store init*/
	ble_store_config_init();
	
	hog_init();

	hs_thread_init();
	app_ble_thread_init();

}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("task overflow:%s\r\n", pcTaskName);
	while (1);
}

/*
 * This is the application hook function running in OS IDLE task. Before use this, the
 * macro configUSE_IDLE_HOOK should be enabled in FreeRTOSConfig.h.
 *
 * Note that the function definition is a bit different with FreeRTOS original one, that
 * is, here we add a return value for flexibility.
 *
 * Return Value:
 * - false:   Continue run the following code after vApplicationIdleHook() in IDLE task,
 *            which is equivalent to the original FreeRTOS implementation.
 * - true:    Avoid run following code after vApplicationIdleHook() in IDLE task, and this
 *            could prevent SoC entering DeepSleep flow when CONFIG_PM enabled.
 */
bool vApplicationIdleHook(void)
{
//	extern bool uart_running;
//	#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
//	extern void WDT_Stop(void);
//	extern void WDT_Start(void);
//	#endif

//	if (!uart_running) {
//		__disable_irq();

//		CLK->CLK_TOP_CTRL_3V = (CLK->CLK_TOP_CTRL_3V & (~(CLK_TOPCTL_AHB_DIV_Msk | CLK_TOPCTL_APB1_DIV_Msk | CLK_TOPCTL_APB2_DIV_Msk)))
//				       | (15 << CLK_TOPCTL_AHB_DIV_Pos) | (0 << CLK_TOPCTL_APB1_DIV_Pos) | (0 << CLK_TOPCTL_APB2_DIV_Pos);

//		#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
//         WDT_Stop();
//		#endif
//		__WFI();
//		#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
//		WDT_Start();
//		#endif

//		CLK->CLK_TOP_CTRL_3V = (CLK->CLK_TOP_CTRL_3V & (~(CLK_TOPCTL_AHB_DIV_Msk |  CLK_TOPCTL_APB1_DIV_Msk | CLK_TOPCTL_APB2_DIV_Msk)))
//				       | (1 << CLK_TOPCTL_AHB_DIV_Pos) | (4 << CLK_TOPCTL_APB1_DIV_Pos) | (4 << CLK_TOPCTL_APB2_DIV_Pos);

//		__enable_irq();
//	} else {

//	}

	return false;
}
