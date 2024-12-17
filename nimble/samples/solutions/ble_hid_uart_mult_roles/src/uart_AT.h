/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "nimble_syscfg.h"

#ifndef __UART_AT_H__
#define __UART_AT_H__

/**
 * @brief Fmc Interface
 * @defgroup fmc_interface Fmc Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define BT_UUID_CUSTOM_SERVICE_VAL                  (BLE_UUID128_DECLARE(BT_UUID_CUSTOM_SERVICE_VAL_ARG))
#define BT_UUID_CUSTOM_CHARAC_VAL1                  (BLE_UUID128_DECLARE(BT_UUID_CUSTOM_CHARAC_VAL1_ARG))
#define BT_UUID_CUSTOM_CHARAC_VAL2                  (BLE_UUID128_DECLARE(BT_UUID_CUSTOM_CHARAC_VAL2_ARG))

#define BT_UUID_CUSTOM_SERVICE_VAL_ARG              0xff, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12
#define BT_UUID_CUSTOM_CHARAC_VAL1_ARG              0xf1, 0xde, 0xbc, 0xfa, 0x78, 0x56, 0x34, 0x12, 0x71, 0x56, 0x34, 0x12, 0x73, 0x56, 0x34, 0x12
#define BT_UUID_CUSTOM_CHARAC_VAL2_ARG              0xf2, 0xde, 0xbc, 0xea, 0x78, 0x56, 0x34, 0x12, 0x72, 0x56, 0x34, 0x12, 0x74, 0x56, 0x34, 0x12

#define VND_MAX_LEN                                 200
#define BAUDRATE                                    115200
#define TGT_UART                                    UART1
#define RX_BUFFER_SIZE                              240
#define CONFIG_BT_DEVICE_NAME                       "n_muart"
#define DEFAULT_MAC_ADDR                            { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 }
#define DEFAULT_BOND_ADDR                           { 0x11, 0x22, 0x33, 0x44, 0x66, 0x88 }
#define DEFAULT_CONFIG_DATA_ADD                     0x0007c000

#define MYNEWT_VAL_APP_BLE_THREAD_STACK_SIZE        512

void uart_at_init(void);
void TGT_SendMultiData(const uint8_t *data, uint16_t size);

typedef enum {
	AT_NONE,
	AT_SCAN,
	AT_SEND_DATA,
} at_sem_flag_t;

typedef struct {
	uint32_t baudrate;
	uint8_t own_mac[6];
	uint8_t device_name[28];
	uint8_t name_length;
	uint8_t bond_mac[6];
	uint32_t passkey;
	uint32_t rst_flag;
} fmc_data;

void default_data_init(void);
void data_printf(uint8_t *data, uint32_t len);
void app_ble_thread_init(void);

extern fmc_data g_fmc_config_data;
extern SemaphoreHandle_t xSemaphore_app;
extern void blecent_scan(void);
extern void peri_notify(uint8_t *data, uint8_t len);
extern void central_write(uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __UART_AT_H__ */
