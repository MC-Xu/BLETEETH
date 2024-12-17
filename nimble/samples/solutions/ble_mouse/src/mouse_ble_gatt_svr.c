/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "mouse_ble_gatt_svr.h"

uint16_t hid_input_handle;
uint16_t hid_consumer_input_handle;
uint16_t hid_mouse_input_handle;

static struct hids_info info = {
	.version = 0x0000,
	.code = 0x00,
	.flags = HIDS_NORMALLY_CONNECTABLE,
};

static struct hids_report input = {
	.id = 0x01,
	.type = HIDS_INPUT,
};

static struct hids_report input_consumer = {
	.id = 0x02,
	.type = HIDS_INPUT,
};

static struct hids_report input_mouse = {
	.id = 0x03,
	.type = HIDS_INPUT,
};

static uint8_t ctrl_point;

static int gatt_svr_chr_access_hid(uint16_t conn_handle, uint16_t attr_handle,
				   struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_hid_input_report(uint16_t conn_handle, uint16_t attr_handle,
						struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_hid_consumer_report(uint16_t conn_handle, uint16_t attr_handle,
						   struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_hid_mouse_report(uint16_t conn_handle, uint16_t attr_handle,
						struct ble_gatt_access_ctxt *ctxt, void *arg);

const uint8_t report_map[] = {
	0x05, 0x01,             /* Usage Page(Generic Desktop) */
	0x09, 0x06,             /* Usage(Keyboard) */

	0xA1, 0x01,             /* Collection(Application) */
	0x85, 0x01,             /* Report Id(1) */
	/* Byte 0 */
	0x05, 0x07,             /* Usage Page(Key Codes) */
	0x19, 0xE0,             /* Usage Minimum(224) */
	0x29, 0xE7,             /* Usage Maximum(231) */
	0x15, 0x00,             /* Logical Minimum (0) */
	0x25, 0x01,             /* Logical Maximum (1) */
	0x75, 0x01,             /* Report Size (1) */
	0x95, 0x08,             /* Report Count (8) */
	0x81, 0x02,             /* Input(Data,Variable,Absolute) */
	/* Byte 1, Reserved */
	0x75, 0x08,             /* Report Size (8) */
	0x95, 0x01,             /* Report Count (1) */
	0x81, 0x01,             /* Input(Constant) */
	/* Byte 2~7 */
	0x05, 0x07,             /* Usage Page (Key Codes) */
	0x19, 0x00,             /* Usage Minimum (0) */
	0x29, 0x65,             /* Usage Maximum (101) */
	0x15, 0x00,             /* Logical Minimum (0) */
	0x25, 0x65,             /* Logical Maximum (101) */
	0x75, 0x08,             /* Report Size (8) */
	0x95, 0x06,             /* Report Count (6) */
	0x81, 0x00,             /* Input(Data, Array) */

	/* Report LEDs */
	/* bit 0~4 */
	0x05, 0x08,             /* Usage Page (LEDs) */
	0x19, 0x01,             /* Usage Minimum (1) */
	0x29, 0x05,             /* Usage Maximum (5) */
	0x75, 0x01,             /* Report Size (1) */
	0x95, 0x05,             /* Report Count (5) */
	0x91, 0x02,             /* Output: (Data, Variable, Absolute) */
	/* bit 5~7, Reserved */
	0x75, 0x03,             /* Report Size (3) */
	0x95, 0x01,             /* Report Count (1) */
	0x91, 0x01,             /* Output: (Constant) */
	0xC0,                   /* End Collection */

	/* Report Consumer */
	0x05, 0x0C,             /* Usage Page(Consumer) */
	0x09, 0x01,             /* Usage (Consumer Control) */
	0xA1, 0x01,             /* Collection(Application) */
	0x85, 0x02,             /* Report ID (2) */
	/* Byte0 */
	0x0A, 0x23, 0x02,       /* Usage (AC Home) */
	0x09, 0x70,             /* Usage (Brightness Down) */
	0x09, 0x6f,             /* Usage (Brightness Up) */
	0x0a, 0xae, 0x01,       /* Usage (AL Keyboard Layout) */
	0x0a, 0x21, 0x02,       /* Usage (AC Search) */
	0x15, 0x00,             /* Logical Minimum (0) */
	0x25, 0x01,             /* Logical Maximum (1) */
	0x75, 0x01,             /* Report Size (1) */
	0x95, 0x05,             /* Report Count (5) */
	0x81, 0x02,             /* Input: (Data, Variable, Absolute, Bit Field) */
	0x75, 0x03,             /* Report Size (3) */
	0x95, 0x01,             /* Report Count (1) */
	0x81, 0x01,             /* Input: (Constant) */
	/* Byte1 */
	0x09, 0xb6,             /* Usage (Scan Previous Track) */
	0x09, 0xcd,             /* Usage (Play/Pause) */
	0x09, 0xb5,             /* Usage (Scan Next Track) */
	0x09, 0xe2,             /* Usage (Mute) */
	0x09, 0xea,             /* Usage (Volume Decrement) */
	0x09, 0xe9,             /* Usage (Volume Increment) */
	0x09, 0x30,             /* Usage (Power) */
	0x15, 0x00,             /* Logical Minimum (0) */
	0x25, 0x01,             /* Logical Maximum (1) */
	0x75, 0x01,             /* Report Size (1) */
	0x95, 0x07,             /* Report Count (7) */
	0x81, 0x02,             /* Input: (Data, Variable, Absolute, Bit Field) */
	0x75, 0x01,             /* Report Size (1) */
	0x95, 0x01,             /* Report Count (1) */
	0x81, 0x01,             /* Input: (Constant) */
	0xC0,                   /* End Collection */

	0x05, 0x01,             /* Usage Page (Generic Desktop Ctrls) */
	0x09, 0x02,             /* Usage (Mouse) */
	0xA1, 0x01,             /* Collection (Application) */
	0x85, 0x03,             /* report id(3) */
	0x09, 0x01,             /* Usage (Pointer) */
	0xA1, 0x00,             /* Collection (Physical) */
	0x05, 0x09,             /* Usage Page (Button) */
	0x19, 0x01,             /* Usage Minimum (0x01) */
	0x29, 0x05,             /* Usage Maximum (0x05) */
	0x15, 0x00,             /* Logical Minimum (0) */
	0x25, 0x01,             /* Logical Maximum (1) */
	0x95, 0x05,             /* Report Count (5) */
	0x75, 0x01,             /* Report Size (1) */
	0x81, 0x02,             /* Input (Data,Var,Abs,No Wrap,Linear,...) */
	0x95, 0x01,             /* Report Count (1) */
	0x75, 0x03,             /* Report Size (3) */
	0x81, 0x03,             /* Input (Const,Var,Abs,No Wrap,Linear,...) */
	0x05, 0x01,             /* Usage Page (Generic Desktop Ctrls) */
	0x09, 0x30,             /* Usage (X) */
	0x09, 0x31,             /* Usage (Y) */
	0x09, 0x38,             /* Usage (Wheel) */
	0x16, 0x00, 0x80,       /* Logical Minimum(129) */
	0x26, 0xFF, 0x7f,       /* LogicalMaximum(127) */
	0x75, 0x10,             /* Report Size(16) */
	0x95, 0x03,             /* Report Count (3) */
	0x81, 0x06,             /* Input (Data,Var,Rel,No Wrap,Linear,...) */
	0xC0,                   /* End Collection */
	0xC0,                   /* End Collection */
};

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
	{
		/* Service: HIDS */
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS),
		.characteristics = (struct ble_gatt_chr_def[]) { {
									 /* Characteristic: hids information */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_INFO),
									 .access_cb = gatt_svr_chr_access_hid,
									 .flags = BLE_GATT_CHR_F_READ,
								 }, {
									 /* Characteristic: hids report map */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT_MAP),
									 .access_cb = gatt_svr_chr_access_hid,
									 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
								 }, {
									 /* Characteristic: hids inout report */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT),
									 .access_cb = gatt_svr_chr_access_hid_input_report,
									 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
									 .val_handle = &hid_input_handle,
									 .descriptors = (struct ble_gatt_dsc_def[]) { {
															      .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT_REF),
															      .access_cb = gatt_svr_chr_access_hid_input_report,
															      .att_flags = BLE_ATT_F_READ,
														      }, {
															      0
														      }, }
								 }, {
									 /* Characteristic: hids consumer report */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT),
									 .access_cb = gatt_svr_chr_access_hid_consumer_report,
									 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
									 .val_handle = &hid_consumer_input_handle,
									 .descriptors = (struct ble_gatt_dsc_def[]) { {
															      .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT_REF),
															      .access_cb = gatt_svr_chr_access_hid_consumer_report,
															      .att_flags = BLE_ATT_F_READ,
														      }, {
															      0
														      }, }
								 }, {
									 /* Characteristic: hids consumer report */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT),
									 .access_cb = gatt_svr_chr_access_hid_mouse_report,
									 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
									 .val_handle = &hid_mouse_input_handle,
									 .descriptors = (struct ble_gatt_dsc_def[]) { {
															      .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_REPORT_REF),
															      .access_cb = gatt_svr_chr_access_hid_mouse_report,
															      .att_flags = BLE_ATT_F_READ,
														      }, {
															      0
														      }, }
								 }, {
									 /* Characteristic: Body sensor location */
									 .uuid = BLE_UUID16_DECLARE(BT_UUID_HIDS_CTRL_POINT),
									 .access_cb = gatt_svr_chr_access_hid,
									 .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
								 }, {
									 0, /* No more characteristics in this service */
								 }, }

	}, {
		0, /* No more services. */
	},

};


static int gatt_svr_chr_access_hid(uint16_t conn_handle, uint16_t attr_handle,
				   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint16_t uuid;
	int rc;

	uuid = ble_uuid_u16(ctxt->chr->uuid);

	switch (uuid) {
	case BT_UUID_HIDS_INFO: {
		rc = os_mbuf_append(ctxt->om, &info, sizeof(struct hids_info));
	} break;
	case BT_UUID_HIDS_REPORT_MAP: {
		rc = os_mbuf_append(ctxt->om, report_map, sizeof(report_map));
	} break;
	case BT_UUID_HIDS_CTRL_POINT: {
		uint16_t om_len;
		om_len = OS_MBUF_PKTLEN(ctxt->om);

		if (om_len != sizeof(ctrl_point)) {
			return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
		}
		rc = ble_hs_mbuf_to_flat(ctxt->om, &ctrl_point, om_len, NULL);
	} break;
	default: break;
	}
	return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int gatt_svr_chr_access_hid_input_report(uint16_t conn_handle, uint16_t attr_handle,
						struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint16_t uuid;
	int rc;

	uuid = ble_uuid_u16(ctxt->chr->uuid);

	switch (uuid) {
	case BT_UUID_HIDS_REPORT: {
		rc = os_mbuf_append(ctxt->om, NULL, 0);
	} break;
	case BT_UUID_HIDS_REPORT_REF: {
		rc = os_mbuf_append(ctxt->om, &input, sizeof(struct hids_report));
	} break;
	default: break;
	}

	return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int gatt_svr_chr_access_hid_consumer_report(uint16_t conn_handle, uint16_t attr_handle,
						   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint16_t uuid;
	int rc;

	uuid = ble_uuid_u16(ctxt->chr->uuid);

	switch (uuid) {
	case BT_UUID_HIDS_REPORT: {
		rc = os_mbuf_append(ctxt->om, NULL, 0);
	} break;
	case BT_UUID_HIDS_REPORT_REF: {
		rc = os_mbuf_append(ctxt->om, &input_consumer, sizeof(struct hids_report));
	} break;
	default: break;
	}

	return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int gatt_svr_chr_access_hid_mouse_report(uint16_t conn_handle, uint16_t attr_handle,
						struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint16_t uuid;
	int rc;

	uuid = ble_uuid_u16(ctxt->chr->uuid);

	switch (uuid) {
	case BT_UUID_HIDS_REPORT: {
		rc = os_mbuf_append(ctxt->om, NULL, 0);
	} break;
	case BT_UUID_HIDS_REPORT_REF: {
		rc = os_mbuf_append(ctxt->om, &input_mouse, sizeof(struct hids_report));
	} break;
	default: break;
	}

	return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
	char buf[BLE_UUID_STR_LEN];

	switch (ctxt->op) {
	case BLE_GATT_REGISTER_OP_SVC:
		printf("registered service %s with handle=%d\n",
		       ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
		       ctxt->svc.handle);
		break;

	case BLE_GATT_REGISTER_OP_CHR:
		printf("registering characteristic %s with "
		       "def_handle=%d val_handle=%d\n",
		       ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
		       ctxt->chr.def_handle,
		       ctxt->chr.val_handle);
		break;

	case BLE_GATT_REGISTER_OP_DSC:
		printf("registering descriptor %s with handle=%d\n",
		       ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
		       ctxt->dsc.handle);
		break;

	default:
		assert(0);
		break;
	}
}

int gatt_svr_init(void)
{
	int rc;

	rc = ble_gatts_count_cfg(gatt_svr_svcs);
	if (rc != 0) {
		return rc;
	}

	rc = ble_gatts_add_svcs(gatt_svr_svcs);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

