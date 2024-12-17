/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MOUSE_BLE_GATT_SVR_
#define __MOUSE_BLE_GATT_SVR_

#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "host/ble_hs.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BT_UUID_HIDS                          0x1812    /* HID value */
#define BT_UUID_HIDS_REPORT_REF               0x2908    /* HID Report Characteristic REF UUID value */
#define BT_UUID_HIDS_INFO                     0x2a4a    /* HID Information Characteristic UUID value */
#define BT_UUID_HIDS_REPORT_MAP               0x2a4b    /* HID Report Map Characteristic UUID value */
#define BT_UUID_HIDS_CTRL_POINT               0x2a4c    /* HID Control Point Characteristic UUID value */
#define BT_UUID_HIDS_REPORT                   0x2a4d    /* HID Report Characteristic UUID value */
#define BT_UUID_HIDS_PROTOCOL_MODE            0x2a4e    /* HID Protocol Mode Characteristic UUID value */
#define BT_UUID_RSC_MEASUREMENT               0x2a53    /* RSC Measurement Characteristic UUID value */

enum {
	HIDS_REMOTE_WAKE                = BIT(0),
	HIDS_NORMALLY_CONNECTABLE       = BIT(1),
};

enum {
	HIDS_INPUT      = 0x01,
	HIDS_OUTPUT     = 0x02,
	HIDS_FEATURE    = 0x03,
};

struct hids_info {
	uint16_t version;       /* version number of base USB HID Specification */
	uint8_t code;           /* country HID Device hardware is localized for. */
	uint8_t flags;
} __attribute__((packed));

struct hids_report {
	uint8_t id;     /* report id */
	uint8_t type;   /* report type */
} __attribute__((packed));

extern uint16_t hid_input_handle;
extern uint16_t hid_consumer_input_handle;
extern uint16_t hid_mouse_input_handle;

int gatt_svr_init(void);
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
#ifdef __cplusplus
}
#endif

#endif
