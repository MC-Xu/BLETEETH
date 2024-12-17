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
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "blehog_sens.h"


uint16_t hid_input_handle;
uint16_t hid_consumer_input_handle;

enum {
	HIDS_REMOTE_WAKE                = BIT(0),
	HIDS_NORMALLY_CONNECTABLE       = BIT(1),
};

struct hids_info {
	uint16_t version;       /* version number of base USB HID Specification */
	uint8_t code;           /* country HID Device hardware is localized for. */
	uint8_t flags;
}__attribute__((packed));

struct hids_report {
	uint8_t id;     /* report id */
	uint8_t type;   /* report type */
} __attribute__((packed));

static struct hids_info info = {
	.version = 0x0000,
	.code = 0x00,
	.flags = HIDS_NORMALLY_CONNECTABLE,
};

enum {
	HIDS_INPUT      = 0x01,
	HIDS_OUTPUT     = 0x02,
	HIDS_FEATURE    = 0x03,
};

static struct hids_report input = {
	.id = 0x01,
	.type = HIDS_INPUT,
};

static struct hids_report input_consumer = {
	.id = 0x02,
	.type = HIDS_INPUT,
};


static uint8_t ctrl_point;

static uint8_t report_map[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x02,        // Usage (Mouse)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x03,        //   Report ID (3)
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x05, 0x09,        //     Usage Page (Button)
	0x19, 0x01,        //     Usage Minimum (0x01)
	0x29, 0x03,        //     Usage Maximum (0x03)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x75, 0x01,        //     Report Size (1)
	0x95, 0x03,        //     Report Count (3)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x05,        //     Report Size (5)
	0x95, 0x01,        //     Report Count (1)
	0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
	0x09, 0x38,        //     Usage (Wheel)
	0x15, 0x81,        //     Logical Minimum (-127)
	0x25, 0x7F,        //     Logical Maximum (127)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x01,        //     Report Count (1)
	0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x0C,        //     Report Size (12)
	0x95, 0x02,        //     Report Count (2)
	0x09, 0x30,        //     Usage (X)
	0x09, 0x31,        //     Usage (Y)
	0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
	0x16, 0x01, 0xF8,  //     Logical Minimum (-2047)
	0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x0C,        //     Usage Page (Consumer)
	0x15, 0x81,        //     Logical Minimum (-127)
	0x25, 0x7F,        //     Logical Maximum (127)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x02,        //     Report Count (2)
	0x0A, 0x38, 0x02,  //     Usage (AC Pan)
	0x0A, 0x2F, 0x02,  //     Usage (AC Zoom)
	0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0xC0,              // End Collection
	0x05, 0x0C,        // Usage Page (Consumer)
	0x09, 0x01,        // Usage (Consumer Control)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x02,        //   Report ID (2)
	0x09, 0x30,        //   Usage (Power)
	0x0A, 0x23, 0x02,  //   Usage (AC Home)
	0x0A, 0x24, 0x02,  //   Usage (AC Back)
	0x09, 0x40,        //   Usage (Menu)
	0x09, 0xE9,        //   Usage (Volume Increment)
	0x09, 0xEA,        //   Usage (Volume Decrement)
	0x09, 0xCD,        //   Usage (Play/Pause)
	0x0A, 0x2F, 0x02,  //   Usage (AC Zoom)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0xA0,        //   Usage (VCR Plus)
	0x09, 0x82,        //   Usage (Mode Step)
	0x0A, 0x2D, 0x02,  //   Usage (AC Zoom In)
	0x0A, 0x2E, 0x02,  //   Usage (AC Zoom Out)
	0x09, 0xB3,        //   Usage (Fast Forward)
	0x09, 0xB4,        //   Usage (Rewind)
	0x09, 0xB5,        //   Usage (Scan Next Track)
	0x09, 0xB6,        //   Usage (Scan Previous Track)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              // End Collection
};



static int gatt_svr_chr_access_hid(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_hid_input_report(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_hid_consumer_report(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);


static const struct ble_gatt_svc_def gatt_svr_svcs_hid[] = {
    {
        /* Service: Heart-rate */
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

    switch (uuid)
    {
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

    switch (uuid)
    {
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

int gatt_svr_hid_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(gatt_svr_svcs_hid);
    if (rc != 0) {
        return rc;
    }
    
    rc = ble_gatts_add_svcs(gatt_svr_svcs_hid);
    if (rc != 0) {
        return rc;
    }
    
    return 0;
}

