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
#include "services/gap/ble_svc_gap.h"
#include "ble_plhd.h"

#include "sub_1g_base.h"
#include "sub_1g_port.h"

#define TX_LEN  (100)

/* Connection handle */
static uint16_t conn_handle;
static bool notify_state;

#ifdef RX_DEVICE
static const char *device_name = "b+plhd rx";
#else
static const char *device_name = "b+plhd tx";
#endif
static uint8_t mu_data[] = { 0xD1, 0x07, 0xc9, 0x7a, 0xbb, 0x8f, 0xdd, 0x4b, 0x00, 0x11 };

#define ADV_UUID    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x20, 0x40, 0x6e
static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;
static bool subscribe_flag = false;

static uint8_t tx_test_buf[TX_LEN] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, };
static uint32_t current_period_time = 1000;
#ifdef RX_DEVICE
static uint8_t payload_header_len = PLHD_LEN8;
#endif
static struct ble_npl_callout plhd_period_tx_timer;

extern struct RxDoneMsg RxDoneParams;

void single_rx(void);
void single_tx(void);
void plhd_init(void);
/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void blehr_advertise(void)
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

	fields.name = (uint8_t *)device_name;
	fields.name_len = strlen(device_name);
	fields.name_is_complete = 1;

	fields.mfg_data = mu_data;
	fields.mfg_data_len = sizeof(mu_data);
	fields.mfg_data_len = sizeof(mu_data);

	rc = ble_gap_adv_set_fields(&fields);

	if (rc != 0) {

		printf("error setting advertisement data; rc=%d\n", rc);
		return;
	}

	memset(&fields, 0, sizeof(fields));

	fields.uuids128 = (ble_uuid128_t[]) {
		BLE_UUID128_INIT(ADV_UUID)
	};
	fields.num_uuids128 = 1;
	fields.name_is_complete = 1;

	rc = ble_gap_adv_rsp_set_fields(&fields);

	if (rc != 0) {

		printf("error sd setting advertisement data; rc=%d\n", rc);
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




/* This functions simulates heart beat and notifies it to the client */
void notify_data(uint8_t *data, uint16_t len)
{
	int rc;
	struct os_mbuf *om;

	if (notify_state) {
		om = ble_hs_mbuf_from_flat(data, len);

		rc = ble_gatts_notify_custom(conn_handle, plhd_handle, om);
		assert(rc == 0);
	}
}

static int blehr_gap_event(struct ble_gap_event *event, void *arg)
{
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
		} else   {
			conn_handle = event->connect.conn_handle;
		}

		break;

	case BLE_GAP_EVENT_DISCONNECT:
		printf("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
		conn_handle = BLE_HS_CONN_HANDLE_NONE; /* reset conn_handle */

		#ifndef RX_DEVICE
		ble_npl_callout_stop(&plhd_period_tx_timer);
		#endif
		/* Connection terminated; resume advertising */
		blehr_advertise();
		break;

	case BLE_GAP_EVENT_ADV_COMPLETE:
		printf("adv complete\n");
		blehr_advertise();
		break;

	case BLE_GAP_EVENT_SUBSCRIBE:
		notify_state = event->subscribe.cur_notify;
		#ifndef RX_DEVICE
		if (subscribe_flag) {
			ble_npl_callout_stop(&plhd_period_tx_timer);
			subscribe_flag = false;
		} else {
			ble_npl_callout_reset(&plhd_period_tx_timer, pdMS_TO_TICKS(current_period_time));
			subscribe_flag = true;
		}
		#else
		if (subscribe_flag) {
			rf_set_mode(RF_MODE_STB3);
			rf_refresh();
			subscribe_flag = false;
		} else {
			plhd_init();
			subscribe_flag = true;
		}
		#endif
		break;

	case BLE_GAP_EVENT_MTU:
		printf("mtu update event; conn_handle=%d mtu=%d\n",
		       event->mtu.conn_handle,
		       event->mtu.value);

		break;

	}

	return 0;
}

static void
blehr_on_sync(void)
{
	int rc;

	/* Use privacy */
	rc = ble_hs_id_infer_auto(0, &blehr_addr_type);
	assert(rc == 0);

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


void plhd_period_tx_cb(struct ble_npl_event *e)
{
	single_tx();
	ble_npl_callout_reset(&plhd_period_tx_timer, pdMS_TO_TICKS(current_period_time));
}

void period_notify_init(void)
{
	ble_npl_callout_init(&plhd_period_tx_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(), plhd_period_tx_cb, NULL);
}

void plhd_param_change(uint32_t plhd_val)
{
	#ifdef RX_DEVICE
	if (plhd_val == 8) {
		payload_header_len = PLHD_LEN8;
	} else if (plhd_val == 16) {
		payload_header_len = PLHD_LEN16;
	} else {
		payload_header_len = PLHD_LEN8;
	}
	rf_set_plhd_rx_on(5, payload_header_len);
	printf("Set payload header len %d\n", plhd_val);
	#endif
}

void GPIO1_IRQHandler(void)
{
	GPIO_ClrAllIntFlag(P1);
	rf_irq_process();

	if (rf_get_recv_flag() != RADIO_FLAG_IDLE) {
		single_rx();
	}
}

void plhd_gpio_init(void)
{
	SYS_SET_MFP(P1, 7, GPIO);
	GPIO_SetMode(P1, BIT7, GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P1, BIT7);
	GPIO_EnablePulldownPath(P1, BIT7);
	GPIO_EnableInt(P1, 7, GPIO_INT_RISING);
	GPIO_ClrAllIntFlag(P1);
	__NVIC_ClearPendingIRQ(GPIO1_IRQn);
	NVIC_EnableIRQ(GPIO1_IRQn);
}

void plhd_spi_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

	/* 3740AU33BBA */
	SYS_SET_MFP(P1, 4, GPIO);                       // PAD_CSN_3V
	SYS_SET_MFP(P1, 5, GPIO);                       // PAD_SCK_3V
	SYS_SET_MFP(P3, 0, GPIO);                       // PAD_MISO_3V
	SYS_SET_MFP(P1, 6, GPIO);                       // PAD_MOSI_3V

	SYS_SET_MFP(P1, 3, GPIO);                       // reset pin
	SYS_SET_MFP(P1, 7, GPIO);                       // IRQ pin

	GPIO_SetMode(P1, BIT3, GPIO_MODE_OUTPUT);       // reset

	P14 = 1;                                        // cs = 1;
	P13 = 1;
	P15 = 0;                                        // sck = 0;

	GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT);       // cs
	GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT);       // sck
	GPIO_SetMode(P1, BIT6, GPIO_MODE_OUTPUT);       // mosi

	GPIO_SetMode(P3, BIT0, GPIO_MODE_INPUT);        // miso
	GPIO_SetMode(P1, BIT7, GPIO_MODE_INPUT);        // irq

	SYS_SET_MFP(P2, 3, GPIO);                       // test pin
	GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT);       // test
	P23 = 0;
}

void plhd_init(void)
{
	uint32_t ret = 0;

	for (uint8_t i = 0; i < TX_LEN - 0x10; i++) {
		tx_test_buf[0x10 + i] = 0x30 + i;
	}

	ret = rf_init();
	if (ret != OK) {
		printf(" 3029 radio init fail\n");
		return;
	}
	printf("3029 radio init ok\n");
	rf_set_default_para();
	#ifdef RX_DEVICE
	rf_set_plhd_rx_on(5, payload_header_len);
	rf_enter_continous_rx();
	#endif
	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	rf_set_recv_flag(RADIO_FLAG_IDLE);
}

void single_rx(void)
{
	uint32_t i;

	if (rf_get_recv_flag() == RADIO_FLAG_PLHDRXDONE) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		printf("###Plhd Rev :  ");
		for (i = 0; i < RxDoneParams.PlhdSize; i++) {
			printf("0x%02x ", RxDoneParams.PlhdPayload[i]);
		}
		printf("\n");
	}

	if (rf_get_recv_flag() == RADIO_FLAG_RXDONE) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		printf("###Rx len  %d: ", RxDoneParams.Size);
		for (i = 0; i < RxDoneParams.Size; i++) {
			printf("0x%02x ", RxDoneParams.Payload[i]);
		}
		printf("\n");
		return;
	}
	if ((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR)) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		printf("Rxerr\n");
		return;
	}
}

void single_tx(void)
{
	static uint8_t tx_count = 0;
	uint32_t tx_time = 0;

	rf_set_transmit_flag(RADIO_FLAG_IDLE);

	if (rf_single_tx_data(tx_test_buf, TX_LEN, &tx_time) != OK) {
		printf("tx fail \r\n");
	}

	while (rf_get_transmit_flag() != RADIO_FLAG_TXDONE) {
		rf_irq_process();
	}

	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	printf("Tx cnt %d\r\n", tx_count++);
}

void app_init(void)
{
	int rc;

	#if CONFIG_BOOT_ENABLE
	print_version_info();
	#endif

	printf("app started\n");

	/** set public address*/
	#ifdef RX_DEVICE
	uint8_t pub_mac[6] = { 8, 7, 3, 4, 5, 6 };
	#else
	uint8_t pub_mac[6] = { 8, 9, 3, 4, 5, 6 };
	#endif

	#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
	#endif

	// get_mac_addr_from_info(pub_mac); /* get mac addr from flash or efuse */
	db_set_bd_address(pub_mac);

	/* Initialize the NimBLE host configuration */
	ble_hs_cfg.sync_cb = blehr_on_sync;

	rc = gatt_svr_init();
	assert(rc == 0);

	/* Set the default device name */
	rc = ble_svc_gap_device_name_set(device_name);
	assert(rc == 0);

	plhd_spi_init();
	#ifndef RX_DEVICE
	period_notify_init();
	plhd_init();
	#endif
	plhd_gpio_init();

	hs_thread_init();
}