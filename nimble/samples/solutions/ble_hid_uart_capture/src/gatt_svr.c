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
#include "blehr_sens.h"
#include "uart_AT.h"
#include "ll_api.h"

uint16_t hrs_hrm_handle;
uint16_t ff00_handle;
uint16_t ff80_handle;

static int
gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
gatt_svr_chr_access_uart_read_write(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
gatt_svr_chr_access_ff80(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /* Service: vendor ble uart */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BT_UUID_CUSTOM_SERVICE_VAL,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: Heart-rate measurement */
            .uuid = BLE_UUID16_DECLARE(0x00DC),
            .access_cb = gatt_svr_chr_access_uart_read_write,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
        }, {
            /* Characteristic: Body sensor location */
            .uuid = BLE_UUID16_DECLARE(0x00C8),
            .access_cb = gatt_svr_chr_access_heart_rate,
			.val_handle = &hrs_hrm_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
	
	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xff80),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0xff82),
            .access_cb = gatt_svr_chr_access_ff80,
			.val_handle = &ff80_handle,
            .flags = BLE_GATT_CHR_F_WRITE_NO_RSP  | BLE_GATT_CHR_F_WRITE,
        }, {
            .uuid = BLE_UUID16_DECLARE(0xff81),
			.access_cb = gatt_svr_chr_access_heart_rate,
			.val_handle = &ff80_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        }, {
            0, /* No more characteristics in this service */
        }, }
	},

	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xff00),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0xff02),
			.access_cb = gatt_svr_chr_access_ff80,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_WRITE_NO_RSP  | BLE_GATT_CHR_F_WRITE,
        }, {
            .uuid = BLE_UUID16_DECLARE(0xff01),
			.access_cb = gatt_svr_chr_access_heart_rate,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        },{
            .uuid = BLE_UUID16_DECLARE(0xff03),
			.access_cb = gatt_svr_chr_access_heart_rate,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        },{
            0, /* No more characteristics in this service */
        }, }
	},
	
	{
		0, /* No more services */
	},
};

static int
gatt_svr_chr_access_heart_rate(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    printf("gatt_svr_chr_access_heart_rate\n");
    /* Sensor location, set to "Chest" */
    static uint8_t body_sens_loc = 0x01;
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_HRS_BODY_SENSOR_LOC_UUID) {
        rc = os_mbuf_append(ctxt->om, &body_sens_loc, sizeof(body_sens_loc));

        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int
gatt_svr_chr_access_uart_read_write(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint8_t rx_data[256];
	uint16_t rx_len;

	rx_len = OS_MBUF_PKTLEN(ctxt->om);
	os_mbuf_copydata(ctxt->om, 0, rx_len, rx_data);
    printf("gatt_svr_chr_access_uart_read_write %d\n",rx_len);

//	data_printf(rx_data, rx_len);
    TGT_SendMultiData(rx_data, OS_MBUF_PKTLEN(ctxt->om));
    return 0;

}

static int
gatt_svr_chr_access_ff80(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    /* Sensor location, set to "Chest" */
	uint8_t rx_data[256];
	uint16_t rx_len;
	
	const uint8_t version_data[] = {0x41, 0x54, 0x2B, 0x47, 0x46, 0x57, 0x56, 0x45, 0x52, 0x3F, 0x0D, 0x0A};
	const uint8_t addr_data[] = {0x41, 0x54, 0x2B, 0x4C, 0x42, 0x44, 0x41, 0x44, 0x44, 0x52, 0x3F, 0x0D};

	rx_len = OS_MBUF_PKTLEN(ctxt->om);
	os_mbuf_copydata(ctxt->om, 0, rx_len, rx_data);
	
	data_printf(rx_data, rx_len);
	if(!memcmp(rx_data, version_data, rx_len)) {
		ff81_notify((uint8_t *)"\r\nBR2141e-s(A02)_DH_210104_r5723\r\n", strlen("\r\nBR2141e-s(A02)_DH_210104_r5723\r\n"));
		
		vTaskDelay(100);
		
		ff81_notify((uint8_t *)"\r\nOK\r\n", strlen("\r\nOK\r\n"));
	} else if(!memcmp(rx_data, addr_data, rx_len)) {
		char str[50];
		
		sprintf(str, "\r\n+LBDADDR:%llx\r\n", LL_GetBdAddr());
		ff81_notify((uint8_t *)str, strlen(str));
		
		vTaskDelay(100);
		ff81_notify((uint8_t *)"\r\nOK\r\n", strlen("\r\nOK\r\n"));
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

