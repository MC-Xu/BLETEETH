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
#include "ble_uart.h"

uint16_t uart_handle;

static int
gatt_svr_chr_access_uart(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(UART_BASE_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID128_DECLARE(UART_RX_SERVICE_UUID),
            .access_cb = gatt_svr_chr_access_uart,
            .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
        }, {
            .uuid = BLE_UUID128_DECLARE(UART_TX_SERVICE_UUID),
            .access_cb = gatt_svr_chr_access_uart,
			.val_handle = &uart_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },

	{
		0, /* No more services */
	},
};

static int
gatt_svr_chr_access_uart(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint8_t data[256] = {0};
    volatile uint16_t uuid;
	uint16_t om_len;
    int rc;

	if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
		uuid = ble_uuid_u16(ctxt->chr->uuid);
		om_len = OS_MBUF_PKTLEN(ctxt->om);
		rc = ble_hs_mbuf_to_flat(ctxt->om, data, sizeof(data), &om_len);
		if (rc != 0) {
			assert(0);
			return BLE_ATT_ERR_UNLIKELY;
		} 
		ble_uart_write(data, om_len);
	} else if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
		uuid = ble_uuid_u16(ctxt->chr->uuid);
	}

	return 0;
}


void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
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

int
gatt_svr_init(void)
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

