/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    main.c
 *
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "nimble/ble.h"
#include "nimble/pan107x/nimble_glue_spark.h"

#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "ble_uart.h"


/* Connection handle */
static uint16_t conn_handle;

static bool notify_state;

static const char *device_name = "panchip_uart";

static uint8_t own_addr_type;

static int blehr_gap_event(struct ble_gap_event *event, void *arg);
void ble_store_config_init(void);

static uint8_t blehr_addr_type;

#define CONFIG_EXAMPLE_RANDOM_ADDR         0

#define UART_RECV_BUF_SIZE		512

uint8_t revDataBuf[UART_RECV_BUF_SIZE];
uint16_t rx_data_size;


static void print_addr(uint8_t* addr)
{
    for(uint8_t i=0; i<6; i++)
    {
        printf("%02x ", addr[i]);
    }
    printf("\n");
}

/**
 * Logs information about a connection to the console.
 */
static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    printf("handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    printf(" our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    printf(" peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    printf(" peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    printf(" conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

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

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));

    /*
     * Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                    BLE_HS_ADV_F_BREDR_UNSUP;

	fields.uuids128 = (ble_uuid128_t[]) {
		BLE_UUID128_INIT(UART_BASE_SERVICE_UUID)
	};
	fields.num_uuids128 = 1;
	fields.uuids128_is_complete = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0) {
        
        printf("error setting advertisement data; rc=%d\n", rc);
        return;
    }
	
	memset(&fields, 0, sizeof(fields));

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&fields);
    if (rc != 0) {
        
        printf("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    
    rc = ble_gap_adv_start(blehr_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, blehr_gap_event, NULL);
    if (rc != 0) {
        printf("error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

static int
blehr_gap_event(struct ble_gap_event *event, void *arg)
{
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
            conn_handle = 0;
        }
        else {
          conn_handle = event->connect.conn_handle;
          #if LOW_POWER_TESET_CI_100MS || LOW_POWER_TESET_CI_1000MS || LOW_POWER_TESET_LATENCY_100MS || LOW_POWER_TESET_LATENCY_1000MS
            timer_to_change_connection_params();
          #endif
        }
        
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
        conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

        /* Connection terminated; resume advertising */
        blehr_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("adv complete\n");
        blehr_advertise();
        break;

	case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        printf("encryption change event; status=%d ",
                    event->enc_change.status);
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        printf("\n");
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
	#if (LOW_POWER_TESET_LATENCY_100MS == 0 && LOW_POWER_TESET_LATENCY_1000MS == 0)
        printf("subscribe event; cur_notify=%d\n value handle; "
                          "val_handle=%d\n",
                    event->subscribe.cur_notify, uart_handle);
        if (event->subscribe.attr_handle == uart_handle) {
            notify_state = event->subscribe.cur_notify;
        }
	#endif
        break;

    case BLE_GAP_EVENT_MTU:
        printf("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
    
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
        printf("PASSKEY_ACTION_EVENT started \n");
        struct ble_sm_io pkey = {0};

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = 123456; // This is the passkey to be entered on peer
            printf("Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            printf("ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT ||
                  event->passkey.params.action == BLE_SM_IOACT_NUMCMP ||
                  event->passkey.params.action == BLE_SM_IOACT_OOB ) {
            printf("BLE_SM_IOACT_INPUT, BLE_SM_IOACT_NUMCMP, BLE_SM_IOACT_OOB"
                          " bonding not supported!");
        }
        return 0;
    }

    return 0;
}

static void
bleprph_on_reset(int reason)
{
    printf("Resetting state; reason=%d\n", reason);
}

static void
bleprph_on_sync(void)
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
        printf("error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    printf("Device Address: ");
    print_addr(addr_val);
    printf("\n");

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


void uart_data_send(UART_HandleTypeDef *pUart, uint8_t *Buf, size_t Size)
{
	pUart->pTxBuffPtr = Buf;
    pUart->txXferSize = Size;
    pUart->txXferCount = 0;

	HAL_UART_Init_INT(pUart);
}

void tx_callback(UART_Cb_Flag_Opt i,uint8_t *OutPtr,uint16_t Size)
{
    printf("tx complete\n");
} 

void rx_callback(UART_Cb_Flag_Opt flag,uint8_t *OutPtr,uint16_t Size)
{
    if (flag == UART_CB_FLAG_RX_FINISH)
    {
        printf("rx finish\n");
    }
    else if (flag == UART_CB_FLAG_RX_TIMEOUT)
    {
		extern void ble_app_user_evt_post(void);
		
		rx_data_size = Size;
		ble_app_user_evt_post();

        printf("rx timeout\n");
    }
    else if (flag == UART_CB_FLAG_RX_BUFFFULL)
    {
        printf("rx buff full\n");
    }
}

static void uart1_init(void)
{
	#ifdef IP_101x
	SYS_SET_MFP(P0, 1, UART1_TX);
    SYS_SET_MFP(P0, 0, UART1_RX);
    GPIO_EnableDigitalPath(P0, BIT1);
	#else
	SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
	#endif
	
	UART1_OBJ.initObj.baudRate = 115200;
	UART1_OBJ.initObj.format = HAL_UART_FMT_8_N_1;  
	UART1_OBJ.pRxBuffPtr = revDataBuf;
    UART1_OBJ.rxXferSize = UART_RECV_BUF_SIZE;
    UART1_OBJ.rxIntCallback = rx_callback;
	UART1_OBJ.txIntCallback = tx_callback;

    UART1_OBJ.interruptObj.switchFlag = ENABLE;
    UART1_OBJ.interruptObj.interruptMode = HAL_UART_INT_THR_EMPTY | HAL_UART_INT_RECV_DATA_AVL;

	HAL_UART_Init(&UART1_OBJ);

	HAL_UART_Init_INT(&UART1_OBJ);

	UART_SetTxTrigger(UART1_OBJ.pUartx, UART_TX_FIFO_EMPTY);
	UART_SetRxTrigger(UART1_OBJ.pUartx, UART_RX_FIFO_TWO_LESS_THAN_FULL);
	NVIC_EnableIRQ(UART1_OBJ.IRQn);
}

void ble_uart_write(uint8_t *data, uint16_t len)
{
	uart_data_send(&UART1_OBJ, data, len);
}

void ble_uart_notify(uint8_t *data, uint16_t len)
{
	int rc;
    struct os_mbuf *om;
	
	if (notify_state) {
		om = ble_hs_mbuf_from_flat(data, len);

		rc = ble_gatts_notify_custom(conn_handle, uart_handle, om);

		assert(rc == 0);
	}
}

void ble_app_user_evt(struct ble_npl_event *ev)
{
	ble_uart_notify(revDataBuf, rx_data_size);
}

void app_init(void)
{
    int rc;

    printf("app started\n");
	
	#if CONFIG_BOOT_ENABLE
	print_version_info();
	#endif
	
	uart1_init();
    
    /** set public address*/
    uint8_t pub_mac[6]={6,2,3,4,5,6};

	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif

    db_set_bd_address(pub_mac);
    
        /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;


    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    assert(rc == 0);
	
	/* XXX Need to have template for store */
    ble_store_config_init();

	hs_thread_init();
}
