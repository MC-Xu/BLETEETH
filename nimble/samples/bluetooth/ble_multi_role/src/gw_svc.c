/*
 ********************************************************************************
 * @FileName  : gw_svc.c
 * @CreateDate: 2021-2-19
 * @Author    : GaoQiu
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 ********************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "sysinit/sysinit.h"
#include "gw_svc.h"

uint16_t gwRxValHandle;
uint16_t gwTxValHandle;

static uint8_t gwRxVal[64];

extern void app_rx_cnt(uint16_t connHandle, struct os_mbuf *om);

__WEAK void app_rx_cnt(uint16_t connHandle, struct os_mbuf *om)
{
	//TRACK_INFO("Rx data 1(handle:%d):\r\n", *ctxt->chr->val_handle);
	//TRACK_DATA(ctxt->om->om_data, ctxt->om->om_len);
	//memcpy(gwRxVal, ctxt->om->om_data, min(ctxt->om->om_len, sizeof(gwRxVal)));
}

int ble_svc_gw_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	int rc = 0;

	//uint8_t *uuid128 = BLE_UUID128(ctxt->chr->uuid)->value;

	if(ctxt->chr->val_handle == &gwRxValHandle)
	{
		switch(ctxt->op)
		{
		case BLE_GATT_ACCESS_OP_READ_CHR:
			rc = os_mbuf_append(ctxt->om, &gwRxVal[0], sizeof gwRxVal);
			return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

		case BLE_GATT_ACCESS_OP_WRITE_CHR:
			app_rx_cnt(conn_handle, ctxt->om);
			return 0;

		default:
			return BLE_ATT_ERR_UNLIKELY;
		}
	}
	return 0;
}

static const struct ble_gatt_svc_def ble_svc_gw_defs[] =
{
	/*** GW Service. */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E),
        .characteristics = (struct ble_gatt_chr_def[])
		{
        	/*** GW RX characteristic */
        	{
				.uuid = BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E),
				.access_cb = ble_svc_gw_access,
				.val_handle = (uint16_t *)&gwRxValHandle,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
			},

        	/*** GW TX characteristic */
        	{
				.uuid = BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E),
				.access_cb = ble_svc_gw_access,
				.val_handle = (uint16_t *)&gwTxValHandle,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
			},

			/* No more characteristics in this service. */
			{
				0,
			}
		},
    },

	/* No more services. */
    {
        0,
    },
};


/**
 * Initialize the GW Service.
 */
int  ble_svc_gw_init(void)
{
    int rc;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    rc = ble_gatts_count_cfg(ble_svc_gw_defs);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_gw_defs);
    SYSINIT_PANIC_ASSERT(rc == 0);
	
	return rc;
}
