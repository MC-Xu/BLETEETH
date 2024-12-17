/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file sub_1g_port.h
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#ifndef __pan3029_PORT_H_
#define __pan3029_PORT_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "PanSeries.h"
#include "pan_sys.h"
#include "pan_spi.h"
#include "pan_gpio.h"
#include "sub_1g_base.h"

#define SPI_WRITE_CHECK         1
#define MODULE_GPIO_TX          0
#define MODULE_GPIO_RX          10
#define MODULE_GPIO_TCXO        10
#define MODULE_GPIO_CAD_IRQ     11

//3740AU33BBA 
#define CSN_LOW              P14 = 0
#define CSN_HIGH             P14 = 1

#define SCK_LOW              P15 = 0
#define SCK_HIGH             P15 = 1

#define SPI_DATA_LOW         P16 = 0
#define SPI_DATA_HIGH        P16 = 1

#define SPI_MISO_LEVEL       P30


typedef struct {
    void (*antenna_init)(void);
    void (*tcxo_init)(void);
    void (*set_tx)(void);
    void (*set_rx)(void);
    void (*antenna_close)(void);
    void (*tcxo_close)(void);
    uint8_t (*spi_readwrite)(uint8_t tx_data);
    void (*spi_cs_high)(void);
    void (*spi_cs_low)(void);
    void (*delayms)(uint32_t time);
    void (*delayus)(uint32_t time);
} rf_port_t;

extern rf_port_t rf_port;

uint8_t spi_readwritebyte(uint8_t tx_data);
void spi_cs_set_high(void);
void spi_cs_set_low(void);
void rf_delay_ms(uint32_t time);
void rf_delay_us(uint32_t time);
void rf_antenna_init(void);
void rf_tcxo_init(void);
void rf_tcxo_close(void);
void rf_antenna_rx(void);
void rf_antenna_tx(void);
void rf_antenna_close(void);

#endif
