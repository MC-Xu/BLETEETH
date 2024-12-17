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
#include "ble_rgb_sens.h"
#include "rgb_light.h"

uint16_t rgb_c2002_handle;
uint16_t rgb_c2003_handle;

static int gatt_svr_chr_access_rgb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);


static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
	{
			/* Service: Heart-rate */
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID128_DECLARE(RGB_SERVICE_UUID),
		.characteristics = (struct ble_gatt_chr_def[]) { {
			/* Characteristic: C2003 characteristic */
			.uuid = BLE_UUID128_DECLARE(C2003_UUID),
			.access_cb = gatt_svr_chr_access_rgb,
			.val_handle = &rgb_c2003_handle,
			.flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
		}, {
			/* Characteristic: C2002 characteristic */
			.uuid = BLE_UUID128_DECLARE(C2002_UUID),
			.access_cb = gatt_svr_chr_access_rgb,
			.val_handle = &rgb_c2002_handle,
			.flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
		}, {
			0, /* No more characteristics in this service */
		}, }
	},

	{
		0, /* No more services */
	},
};

static int gatt_svr_chr_access_rgb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    /* Sensor location, set to "Chest" */
    int rc;
	uint16_t om_len;
	uint8_t rgb[20];
	
    if (attr_handle == rgb_c2003_handle) {
        return 0; /* do not implement read and write */
    } else if (attr_handle == rgb_c2002_handle) {
		
		om_len = OS_MBUF_PKTLEN(ctxt->om);
		
		rc = ble_hs_mbuf_to_flat(ctxt->om, rgb, sizeof(rgb), &om_len);
		write_rgb(rgb, om_len);
		if (rc != 0) {
			return BLE_ATT_ERR_UNLIKELY;
		}
	}

    return BLE_ATT_ERR_UNLIKELY;
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

