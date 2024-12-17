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
#include "ble_cad.h"

#include "sub_1g_base.h"
#include "sub_1g_port.h"

#define MAC_EVT_TX_CAD_NONE     0x00
#define MAC_EVT_TX_CAD_TIMEOUT  0x01
#define MAC_EVT_TX_CAD_ACTIVE   0x02
#define PREAM_TIME_MS                   100

/* Connection handle */
static uint16_t conn_handle;
static bool notify_state;

static const char *device_name = "b+cad rx";
static uint8_t mu_data[] = { 0xD1, 0x07, 0xc9, 0x7a, 0xbb, 0x8f, 0xdd, 0x4b, 0x00, 0x11 };

#define ADV_UUID    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x20, 0x40, 0x6e
static int blehr_gap_event(struct ble_gap_event *event, void *arg);

static uint8_t blehr_addr_type;

static bool subscribe_flag = false;
volatile uint32_t cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;
static uint32_t one_chirp_time;
extern struct RxDoneMsg RxDoneParams;

void cad_rx_init(void);
void cad_rx(void);
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

		rc = ble_gatts_notify_custom(conn_handle, cad_handle, om);
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
		} else {
			conn_handle = event->connect.conn_handle;
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

	case BLE_GAP_EVENT_SUBSCRIBE:
		notify_state = event->subscribe.cur_notify;
		if (subscribe_flag) {
			rf_set_mode(RF_MODE_STB3);
			rf_refresh();
			subscribe_flag = false;
		} else {
			cad_rx_init();
			subscribe_flag = true;
		}
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

void GPIO3_IRQHandler(void)
{
	GPIO_ClrAllIntFlag(P3);
	cad_tx_detect_flag = MAC_EVT_TX_CAD_ACTIVE;
	cad_rx();
}

void cad_gpio_init(void)
{
	SYS_SET_MFP(P3, 1, GPIO);
	GPIO_SetMode(P3, BIT1, GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P3, BIT1);
	GPIO_EnablePulldownPath(P3, BIT1);
	GPIO_EnableInt(P3, 1, GPIO_INT_RISING);
	GPIO_ClrAllIntFlag(P3);
	__NVIC_ClearPendingIRQ(GPIO3_IRQn);
	NVIC_EnableIRQ(GPIO3_IRQn);
}

void cad_spi_init(void)
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

void cad_rx_init(void)
{
	uint32_t ret = 0;
	uint32_t bw, sf;

	ret = rf_init();
	if (ret != OK) {
		printf(" RF Init Fail\n");
		return;
	}
	printf("3029 radio init ok\n");
	rf_set_default_para();
	rf_set_bw(BW_500K);
	rf_set_sf(SF_5);

	sf = rf_get_sf();
	bw = rf_get_bw();
	one_chirp_time = rf_get_chirp_time(bw, sf);

	rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3);
	rf_enter_continous_rx();
}
void cad_rx(void)
{
	uint32_t i, rx_max_times = 0;

	if (check_cad_rx_inactive(one_chirp_time) == LEVEL_ACTIVE) {
		printf("cad\n");
		rx_max_times = 0;

		while (rf_get_recv_flag() != RADIO_FLAG_RXDONE) {
			rf_irq_process();
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

		rf_delay_us(10);
		rx_max_times++;
		if (rx_max_times < one_chirp_time / 10 * 2) { /*2-chirp-cad-check*/
			if (CHECK_CAD() != 1) {
				printf("break cad err\n");
				return;
			}
		}
		if (rx_max_times >= (PREAM_TIME_MS * 100 + one_chirp_time * 30)) { /*chirp-timeout*/
			printf("break cad timeout\n");
			return;
		}
	} else {
		printf("sleep\n");
		rf_deepsleep();
		rf_delay_us(one_chirp_time);
		rf_init();
		rf_set_default_para();
		rf_set_bw(BW_500K);
		rf_set_sf(SF_5);
		rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3);
		rf_enter_continous_rx();
	}
}


void app_init(void)
{
	int rc;

	#if CONFIG_BOOT_ENABLE
	print_version_info();
	#endif

	printf("app started\n");

	/** set public address*/
	uint8_t pub_mac[6] = { 8, 1, 3, 4, 5, 6 };

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

	cad_spi_init();
	cad_gpio_init();

	hs_thread_init();
}
