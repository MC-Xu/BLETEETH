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

#include "utils/bd_addr.h"
#include "utils/defs_types.h"

/* Application-specified header. */
#include "blecent.h"
#include "blehr_sens.h"
#include "gw_svc.h"


#define APP_DATA_TEST_EN     1
#define APP_DATA_TOKEN       0xAA

/* Connection parameter config */
#define APP_MST_CONN_INTERVAL  BLE_GAP_CONN_ITVL_MS(50)
#define APP_SLV_CONN_INTERVAL  BLE_GAP_CONN_ITVL_MS(30)
#define APP_CONN_TIMEOUT       BLE_GAP_SUPERVISION_TIMEOUT_MS(2500)

/* Test Config */
#define APP_SLV_TEST_EN      0 /* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 1 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 0. */
#define APP_MST_TEST_EN      0 /* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 0 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 1. */

#define APP_SLV_DEVICE_ID    0 /* Slave device can be 0,1,2 */
#define APP_ADDR_MATCH_LEN   4 /* use for master address filte */

#if APP_MST_TEST_EN
bdAddr_t addrFilterTable [] = {
	{0xD0, 0x00, 0x00, 0x00, 0x01, 0x39},
};
#else

bdAddr_t addrFilterTable [] ={
	{0x66, 0x66, 0x66, 0x66, 0x01, 0xC2},
	{0x66, 0x66, 0x66, 0x66, 0x03, 0xC2},
	{0x66, 0x66, 0x66, 0x66, 0x04, 0xC2},
};
#endif

/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 1 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 0. */
#if APP_SLV_TEST_EN
	static const char *device_name = "mutil_conn_test_S";
	#undef APP_DATA_TOKEN
	#define APP_DATA_TOKEN       0x77
/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 0 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 1. */
#elif APP_MST_TEST_EN
	static const char *device_name = "mutil_conn_test_M";
	#undef APP_DATA_TOKEN
	#define APP_DATA_TOKEN       0x77
/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 2 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 3. */
#else
	static const char *device_name = "mutil_conn_107";
#endif

typedef struct{
	uint32_t   txPktCnt;
	uint32_t   rxPktCnt;
	uint16_t   connHandle;
	uint8_t    role;
	uint8_t    inUse;

	uint8_t    isEncCmpl;
	uint8_t    notifyEn;
	uint16_t   gwAttHandle;
	
	struct ble_npl_callout   connUpdTimer;
}conn_cb_t;

typedef struct{
	conn_cb_t    ccb[CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL];
	uint8_t      numMstConn;
	uint8_t      numSlvConn;
}app_cb_t;

app_cb_t appCb;

void app_conn_upd_cb(struct ble_npl_event *ev)
{
	//uint16_t connHandle = conn_handle;
	uint16_t connHandle = *(uint16_t *)ev->arg;

	struct ble_gap_upd_params new_param;
	new_param.itvl_max = APP_SLV_CONN_INTERVAL;
	new_param.itvl_min = APP_SLV_CONN_INTERVAL;
	new_param.latency  = 0;
	new_param.supervision_timeout = APP_CONN_TIMEOUT;
	new_param.max_ce_len = 0;
	new_param.min_ce_len = 0;

	int rc = ble_gap_update_params(connHandle, &new_param);
	if(rc){
		APP_TRACK_WRN("conn update failed - connHandle:%d, rc:%02x\r\n", connHandle, rc);
	}
}

void app_conn_upd_timer_start(conn_cb_t *pCcb)
{
	APP_TRACK_WRN("Conn Update Timer start...\n");
	ble_npl_callout_reset(&pCcb->connUpdTimer, pdMS_TO_TICKS(1000));
}


conn_cb_t *app_new_conn(uint16_t connHandle, uint8_t role)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->inUse == false){
			pCcb->inUse = true;
			pCcb->connHandle = connHandle;
			pCcb->role = role;
			pCcb->isEncCmpl = false;
			pCcb->notifyEn = false;
			pCcb->gwAttHandle = 0; // invalid handle
			pCcb->txPktCnt = 0;
			pCcb->rxPktCnt = 0;
			
			if(pCcb->connUpdTimer.handle == NULL){
				ble_npl_callout_init(&pCcb->connUpdTimer, nimble_port_get_dflt_eventq(), app_conn_upd_cb, &pCcb->connHandle);
			}
			
			if(role == BLE_GAP_ROLE_MASTER){
				appCb.numMstConn++;
			}else{
				appCb.numSlvConn++;
			}
			return pCcb;
		}
	}

	return NULL;
}

void app_release_conn(uint16_t connHandle)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->connHandle == connHandle){
			pCcb->inUse = false;
			pCcb->connHandle = 0xFFFF;
			pCcb->isEncCmpl = false;
			pCcb->notifyEn = false;
			pCcb->gwAttHandle = 0; // invalid handle
			pCcb->txPktCnt = 0;
			pCcb->rxPktCnt = 0;
			
			if(ble_npl_callout_is_active(&pCcb->connUpdTimer)){
				ble_npl_callout_stop(&pCcb->connUpdTimer);
			}

			if(pCcb->role == BLE_GAP_ROLE_MASTER){
				if(appCb.numMstConn>0) appCb.numMstConn--;
			}else{
				if(appCb.numSlvConn>0) appCb.numSlvConn--;
			}
			return;
		}
	}
}

conn_cb_t *app_get_conn(uint16_t connHandle)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->connHandle == connHandle){
			return pCcb;
		}
	}

	return NULL;
}

bool_t app_is_addr_contain(uint8_t addr[6])
{
	for(int i=0;i<COUNTOF(addrFilterTable); i++)
	{
		if(!memcmp(addr, &addrFilterTable[i], APP_ADDR_MATCH_LEN)){
			return true;
		}
	}
	return false;
}

#if APP_DATA_TEST_EN
static struct ble_npl_callout data_send_timer;
static volatile uint8_t data_send_enable = 0;

extern void app_data_timer_start(void);

uint8_t buf[256];

void app_rx_cnt(uint16_t connHandle, struct os_mbuf *om)
{
	uint8_t *pdata = buf;
	uint16_t len = 0;
	conn_cb_t *pCcb = NULL;

	ble_hs_mbuf_to_flat(om, buf, sizeof(buf), &len);

	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->inUse && pCcb->connHandle == connHandle){
			APP_TRACK_INFO("Rx: connHandle:%d, dataLen:%d, rxCnt:%d\n", connHandle, len, pCcb->rxPktCnt);
			//APP_TRACK_DATA(pdata, len);

			uint32_t rxCnt = 0;
			rxCnt = (pdata[3]<<24) | (pdata[2]<<16) | (pdata[1]<<8) | pdata[0];

			if(pCcb->rxPktCnt != rxCnt){
				APP_TRACK_ERR("rxCnt error\n");
				goto done;
			}

			uint8_t token = APP_DATA_TOKEN;
			for(int i=4; i<len; i++)
			{
				if(pdata[i] != token){
					APP_TRACK_ERR("data error\n");
					goto done;
				}
			}

			pCcb->rxPktCnt++; //
			break;
		}
	}

	return;

done:
	APP_TRACK_ERR("Rx error(handle:%d, rxLen:%d, rxCnt:%d):\n", connHandle, len, pCcb->rxPktCnt);
	APP_TRACK_DATA(pdata, len);
}

uint8_t data[244] = {
	0,   0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,//80

	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,//80

	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77, 0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,//80

	0x77,0x77,0x77,0x77,
};

void app_data_tx(void)
{
	int rc = 0;
	
	if(data_send_enable == false)
		return;

	//struct os_mbuf *om = ble_hs_mbuf_from_flat(data, sizeof data);

	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		conn_cb_t *pCcb = &appCb.ccb[i];
		//if(pCcb->inUse && pCcb->gwAttHandle != 0 && pCcb->notifyEn && pCcb->isEncCmpl)
		if(pCcb->inUse && pCcb->gwAttHandle != 0 && pCcb->notifyEn)
		{
			//data[0] = pCcb->connHandle & 0xFF;

			data[0] = pCcb->txPktCnt & 0xFF;
			data[1] = (pCcb->txPktCnt>>8) & 0xFF;
			data[2] = (pCcb->txPktCnt>>16) & 0xFF;
			data[3] = (pCcb->txPktCnt>>24) & 0xFF;

			if(pCcb->role == BLE_GAP_ROLE_MASTER){
				struct os_mbuf *om = ble_hs_mbuf_from_flat(data, sizeof data);
				rc = ble_gattc_write_no_rsp(pCcb->connHandle, pCcb->gwAttHandle, om);
			}
			else{
				struct os_mbuf *om = ble_hs_mbuf_from_flat(data, sizeof data);
				rc = ble_gatts_notify_custom(pCcb->connHandle, pCcb->gwAttHandle, om);
			}
			if(rc == 0){
				pCcb->txPktCnt++;
				APP_TRACK_INFO("Tx: connHandle:%d, TxCnt:%d\n", pCcb->connHandle, pCcb->txPktCnt);
				
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
	}

	if(rc){
		APP_TRACK_WRN("Push data Failed!\r\n");
	}
}
	
void app_data_send_cb(struct ble_npl_event *ev)
{
	app_data_tx();
	
	//app_data_timer_start();
	ble_npl_callout_reset(&data_send_timer, pdMS_TO_TICKS(1000));
}

void app_data_timer_start(void)
{
	if(ble_npl_callout_is_active(&data_send_timer)){
		return;
	}
	APP_TRACK_INFO("start send data timer\n");
	ble_npl_callout_reset(&data_send_timer, pdMS_TO_TICKS(10000));
}

void app_data_timer_stop(void)
{
	data_send_enable = false;
	ble_npl_callout_stop(&data_send_timer);
}

void app_data_send_timer_init(void)
{
	ble_npl_callout_init(&data_send_timer, nimble_port_get_dflt_eventq(), app_data_send_cb, NULL);
}
#endif

void key_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
	
	GPIO_SetMode(P0, BIT(6), GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P0, BIT(6));
	GPIO_EnablePullupPath(P0, BIT(6));
	
	GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    NVIC_EnableIRQ(GPIO0_IRQn);
}

void GPIO0_IRQHandler(void)
{
	if(GPIO_GetIntFlag(P0,  BIT(6))) 
	{
		GPIO_ClrIntFlag(P0, BIT(6));
		data_send_enable = !data_send_enable;
		printf("data send %s...\n", data_send_enable ? "enable" : "disable");
	}
}


int app_gap_event_cb(struct ble_gap_event *event, void *arg);
void app_scan_start(void);

/**
 * Application callback.  Called when the read of the ANS Supported New Alert
 * Category characteristic has completed.
 */
int blecent_on_read(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg)
{
	APP_TRACK_INFO("Read complete; status=%d conn_handle=%d\n", error->status, conn_handle);

	if (error->status == 0)
	{
		APP_TRACK_INFO(" -attr_handle=%d value=", attr->handle);
		print_mbuf(attr->om);
	}

	return 0;
}

/**
 * Application callback.  Called when the write to the ANS Alert Notification
 * Control Point characteristic has completed.
 */
int blecent_on_write(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg)
{
	APP_TRACK_INFO("Write complete; status=%d conn_handle=%d attr_handle=%d\n", error->status, conn_handle,
					attr->handle);
	return 0;
}

/**
 * Application callback.  Called when the attempt to subscribe to notifications
 * for the ANS Unread Alert Status characteristic has completed.
 */
int blecent_on_subscribe(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr,
				void *arg)
{
	APP_TRACK_INFO("Subscribe complete; status=%d conn_handle=%d " "attr_handle=%d\n", error->status, conn_handle,
					attr->handle);

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
static void blecent_read_write_subscribe(const struct peer *peer)
{
	const struct peer_chr *chr;
	const struct peer_dsc *dsc;
	uint8_t value[2];
	int rc;

    chr = peer_chr_find_uuid(peer,
    				BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E),
					BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E));

    if (chr == NULL) {
        APP_TRACK_ERR("Error: Peer doesn't support the Alert "
                           "Notification Control Point characteristic\n");
        goto err;
    }

    conn_cb_t *pCcb = app_get_conn(peer->conn_handle);
    if(pCcb == NULL){
    	APP_TRACK_ERR("No Found GW Charic \n");
    	goto err;
    }
    pCcb->gwAttHandle = chr->chr.val_handle;


	/* Subscribe to notifications for the Unread Alert Status characteristic.
	 * A central enables notifications by writing two bytes (1, 0) to the
	 * characteristic's client-characteristic-configuration-descriptor (CCCD).
	 */
	dsc = peer_dsc_find_uuid(peer,
					BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E),
					BLE_UUID128_DECLARE(0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E),
					BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));

	if (dsc == NULL)
	{
		APP_TRACK_ERR("Error: Don't find battery level CCCD characteristic\n");
		goto err;
	}

	value[0] = 1;
	value[1] = 0;
	rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, value, sizeof value, blecent_on_subscribe, NULL);
	if (rc != 0)
	{
		APP_TRACK_ERR("Error: Failed to subscribe to characteristic; rc=%d\n", rc);
		goto err;
	}

	pCcb->notifyEn = true;

	return;

err:
	/* Terminate the connection. */
	ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

/**
 * Called when service discovery of the specified peer has completed.
 */
static void blecent_on_disc_complete(const struct peer *peer, int status, void *arg)
{
	if (status != 0)
	{
		/* Service discovery failed.  Terminate the connection. */
		APP_TRACK_ERR("Error: Service discovery failed; status=%d " "conn_handle=%d\n", status, peer->conn_handle);
		ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
		return;
	}

	/* Service discovery has completed successfully.  Now we have a complete
	 * list of services, characteristics, and descriptors that the peer
	 * supports.
	 */
	APP_TRACK_INFO("Service discovery complete; status=%d " "conn_handle=%d\n", status, peer->conn_handle);

	/* Now perform three concurrent GATT procedures against the peer: read,
	 * write, and subscribe to notifications.
	 */
	blecent_read_write_subscribe(peer);

	if(appCb.numMstConn < CONFIG_BT_MAX_NUM_OF_CENTRAL){
		app_scan_start();
	}
}

/**
 * Initiates the GAP general discovery procedure.
 */
void app_scan_start(void)
{
	if(ble_gap_disc_active()){
		return;
	}

	uint8_t own_addr_type;
	struct ble_gap_disc_params disc_params;
	int rc;

	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0)
	{
		APP_TRACK_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Tell the controller to filter duplicates; we don't want to process
	 * repeated advertisements from the same device.
	 */
	disc_params.filter_duplicates = 1;

	/**
	 * Perform a passive scan.  I.e., don't send follow-up scan requests to
	 * each advertiser.
	 */
	disc_params.passive = 1;

	/* Use defaults for the rest of the parameters. */
	disc_params.itvl = 0;
	disc_params.window = 0;
	disc_params.filter_policy = 0;
	disc_params.limited = 0;

	rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, app_gap_event_cb, NULL);
	if (rc != 0)
	{
		APP_TRACK_ERR("Error initiating GAP discovery procedure; rc=%d\n", rc);
	}

	APP_TRACK_INFO("Starting Scan...\n");
}

static int blecent_should_connect(const struct ble_gap_disc_desc *disc)
{
	struct ble_hs_adv_fields fields;
	int rc;
	int i;

	if (disc->rssi < -60){
		return 0;
	}

	/* The device has to be advertising connectability. */
	if(disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
	   disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND){
		return 0;
	}

	/* Address Filter */
	if(!app_is_addr_contain((uint8_t *)disc->addr.val)) {
		return 0;
	}
	else{
		return 1;
	}
	
	return 0;
}

static void blecent_connect_if_interesting(const struct ble_gap_disc_desc *disc)
{
	uint8_t own_addr_type;
	int rc;

	/* Don't do anything if we don't care about this advertiser. */
	if (!blecent_should_connect(disc)){
		return;
	}

	/* Scanning must be stopped before a connection can be initiated. */
	rc = ble_gap_disc_cancel();
	if (rc != 0){
		APP_TRACK_ERR("Failed to cancel scan; rc=%d\n", rc);
		return;
	}

	/* Figure out address to use for connect (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		APP_TRACK_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for
	 * timeout.
	 */
	struct ble_gap_conn_params conn_params = {
		.scan_itvl   = 0x0010,
		.scan_window = 0x0010,
		.itvl_min = APP_MST_CONN_INTERVAL,
		.itvl_max = APP_MST_CONN_INTERVAL,
		.latency  = 0,
		.supervision_timeout = APP_CONN_TIMEOUT,
		.min_ce_len = 0,
		.max_ce_len = 0,
	};
	rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, &conn_params, app_gap_event_cb, NULL);
	if (rc != 0){
		APP_TRACK_ERR("Error: create connect failed; addr_type=%d addr=%s; rc=%d\n", disc->addr.type,
						addr_str(disc->addr.val), rc);
		app_scan_start();
	}
}

void app_adv_start(void)
{
	if(ble_gap_adv_active()){
		return;
	}

    int rc;
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    struct ble_hs_adv_fields rsp_fields;

    /* set adv parameters */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min  = ADV_INTR_30_MS;
    adv_params.itvl_max  = ADV_INTR_35_MS;

    /* Set Adv Data */
    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN|BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    ASSERT(rc == 0, rc);

#if 0
    /* Set Scan Rsp Data */
    memset(&rsp_fields, 0, sizeof(rsp_fields));
    rsp_fields.name = (uint8_t *)device_name;
    rsp_fields.name_len = strlen(device_name);
    rsp_fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&fields);
    ASSERT(rc == 0, rc);
#endif

    TRACK_INFO("Starting advertising...\n");

    /* As own address type we use hard-coded value, because we generate
       NRPA and by definition it's random */
    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                           &adv_params, app_gap_event_cb, NULL);
    ASSERT(rc == 0, rc);
}

int app_gap_event_cb(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
	struct ble_hs_adv_fields fields;
	int rc;

	switch (event->type)
	{
	case BLE_GAP_EVENT_DISC:
		rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
		if (rc != 0){
			return 0;
		}

		/* An advertisment report was received during GAP discovery. */
		//print_adv_fields(&fields);

		/* Try to connect to the advertiser if it looks interesting. */
		blecent_connect_if_interesting(&event->disc);
		return 0;

	case BLE_GAP_EVENT_CONNECT:
		/* A new connection was established or a connection attempt failed. */
		if (event->connect.status == 0)
		{
			/* Connection successfully established. */
			APP_TRACK_INFO("=== Connection established ===\n");

			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			ASSERT(rc==0, 0);
			print_conn_desc(&desc);

			conn_cb_t *pCcb = app_new_conn(desc.conn_handle, desc.role);
			ASSERT(pCcb != NULL, 0);

			rc = ble_gap_set_data_len(desc.conn_handle, MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE, 2120);
			ASSERT(rc==0, 0);

			if(desc.role == BLE_GAP_ROLE_MASTER)
			{
				rc = ble_gattc_exchange_mtu(desc.conn_handle, NULL, NULL);
				ASSERT(rc == 0, 0);

				/* Remember peer. */
				rc = peer_add(event->connect.conn_handle);
				if (rc != 0){
					APP_TRACK_ERR("Failed to add peer; rc=%d\n", rc);
					return 0;
				}

				/* Perform service discovery. */
				rc = peer_disc_all(event->connect.conn_handle, blecent_on_disc_complete, NULL);
				if (rc != 0){
					APP_TRACK_ERR("Failed to discover services; rc=%d\n", rc);
					return 0;
				}
				else{
					APP_TRACK_WRN("sdp start\n");
				}
			}
			else //slave
			{
			#if !APP_SLV_TEST_EN
				if(desc.conn_itvl < APP_SLV_CONN_INTERVAL || desc.supervision_timeout < APP_CONN_TIMEOUT){
					app_conn_upd_timer_start(pCcb);
				}
			#endif
								
				if(appCb.numSlvConn < CONFIG_BT_MAX_NUM_OF_PERIPHERAL){
					app_adv_start();
				}
			}

			#if APP_DATA_TEST_EN
				app_data_timer_start();
			#endif
		}
		else
		{
			/* Connection attempt failed; resume scanning. */
			APP_TRACK_ERR("Error: Connection failed; status=%d\n", event->connect.status);

			if(event->connect.conn_handle < CONFIG_BT_MAX_NUM_OF_CENTRAL){
				app_scan_start();
			}
			else{
				app_adv_start();
			}
		}
		return 0;

	case BLE_GAP_EVENT_DISCONNECT:
	{
		/* Connection terminated. */
		desc = event->disconnect.conn;
		
		APP_TRACK_INFO("=== Disconnect Cmpl ===\n -reason:0x%02x \n", event->disconnect.reason);
		print_conn_desc(&desc);

		app_release_conn(event->disconnect.conn.conn_handle);

		/* Resume scanning. */
		if(desc.role == BLE_GAP_ROLE_MASTER)
		{
			/* Forget about peer. */
			peer_delete(event->disconnect.conn.conn_handle);

			if(appCb.numMstConn < CONFIG_BT_MAX_NUM_OF_CENTRAL){
				app_scan_start();
			}
		}
		else{
			if(appCb.numSlvConn < CONFIG_BT_MAX_NUM_OF_PERIPHERAL){
				app_adv_start();
			}
		}

	#if APP_DATA_TEST_EN
		if(appCb.numMstConn == 0 && appCb.numSlvConn == 0){
			app_data_timer_stop();
		}
	#endif
		return 0;
	}
	case BLE_GAP_EVENT_DISC_COMPLETE:
		APP_TRACK_INFO("scan discovery complete; reason=%d\n", event->disc_complete.reason);
		app_scan_start();
		//app_adv_start();
		return 0;

	case BLE_GAP_EVENT_ENC_CHANGE:
		/* Encryption has been enabled or disabled for this connection. */
		APP_TRACK_WRN("encryption change event; status=%d \n", event->enc_change.status);

		rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
		ASSERT(rc==0, 0);
		print_conn_desc(&desc);

		conn_cb_t *pCcb = app_get_conn(event->subscribe.conn_handle);
		if(pCcb == NULL){
			APP_TRACK_ERR("Invalid connHandle\n");
			return 0;
		}

		pCcb->isEncCmpl = true;
		return 0;

	case BLE_GAP_EVENT_SUBSCRIBE:
	{
		conn_cb_t *pCcb = app_get_conn(event->subscribe.conn_handle);
		if(pCcb == NULL){
			APP_TRACK_ERR("subscribe failed\n");
			return 0;
		}

		pCcb->gwAttHandle = event->subscribe.attr_handle;
		pCcb->notifyEn = true;

		return 0;
	}
	case BLE_GAP_EVENT_NOTIFY_RX:
	{
		struct os_mbuf *om = event->notify_rx.om;
		app_rx_cnt(event->notify_rx.conn_handle, om);
		return 0;
	}
	case BLE_GAP_EVENT_NOTIFY_TX:
		//APP_TRACK_INFO("notify send ok(connHandle:%d, attHandle:%d)\n",
		//				event->notify_tx.conn_handle, event->notify_tx.attr_handle);
		return 0;

	case BLE_GAP_EVENT_MTU:
		APP_TRACK_INFO("MTU update event \r\n" " -conn_handle=%d\n" " -cid=%d\n" " -mtu=%d\n", event->mtu.conn_handle,
						event->mtu.channel_id, event->mtu.value);
		return 0;

	case BLE_GAP_EVENT_REPEAT_PAIRING:
		/* We already have a bond with the peer, but it is attempting to
		 * establish a new secure link.  This app sacrifices security for
		 * convenience: just throw away the old bond and accept the new link.
		 */

		/* Delete the old bond. */
		rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
		ASSERT(rc==0, 0);
		ble_store_util_delete_peer(&desc.peer_id_addr);

		/* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
		 * continue with the pairing operation.
		 */
		return BLE_GAP_REPEAT_PAIRING_RETRY;
	
	case BLE_GAP_EVENT_CONN_UPDATE:
		APP_TRACK_INFO("Conn update Cmpl...\n");
		rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
		ASSERT(rc==0, 0);
		print_conn_desc(&desc);
		break;

	default:
		break;
	}

	return 0;
}

static void on_reset(int reason)
{
	APP_TRACK_ERR("Resetting state; reason:0x%02x\n", reason);
}

static void on_sync(void)
{
	int rc;
	
	/* Make sure we have proper identity address set (public preferred) */
	rc = ble_hs_util_ensure_addr(0);
	ASSERT(rc==0, 0);

#if CONFIG_BT_MAX_NUM_OF_PERIPHERAL
	/* Adv */
	app_adv_start();
#endif

#if CONFIG_BT_MAX_NUM_OF_CENTRAL 
	/* Scan */
	app_scan_start();
#endif
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
	
#if APP_SLV_TEST_EN 
	memset(&data[0], 0xAA, sizeof(data));
	memcpy(pub_mac, &addrFilterTable[APP_SLV_DEVICE_ID], 6);
#endif
	
#if APP_MST_TEST_EN
	memset(&data[0], 0xAA, sizeof(data));
#endif

    db_set_bd_address(pub_mac);

    /* Configure the host. */
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb  = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);
	
	ble_svc_gap_init();
    rc = ble_svc_gw_init();
    assert(rc == 0);

    /* Initialize data structures to track connected peers. */
    rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 10, 32, 32);
    assert(rc == 0);

    /* Persist config store init*/
    ble_store_config_init();
	
#if APP_DATA_TEST_EN
	app_data_send_timer_init();
#endif

	key_init();

	hs_thread_init();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("task overflow:%s\r\n", pcTaskName);
    while(1);
}

