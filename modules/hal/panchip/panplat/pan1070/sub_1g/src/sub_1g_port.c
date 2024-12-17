/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file sub_1g_port.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include "sub_1g_port.h"
#include "pan_spi.h"
#include "pan_gpio.h"

#define DELAY_1M      __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();//1.25M

rf_port_t rf_port=
{
    .antenna_init = rf_antenna_init,
    .tcxo_init = rf_tcxo_init,
    .set_tx = rf_antenna_tx,
    .set_rx = rf_antenna_rx,
    .antenna_close = rf_antenna_close,
    .tcxo_close = rf_tcxo_close,
    .spi_readwrite = spi_readwritebyte,
    .spi_cs_high = spi_cs_set_high,
    .spi_cs_low = spi_cs_set_low,
    .delayms = rf_delay_ms,
    .delayus = rf_delay_us,
};

/**
 * @brief spi_readwritebyte
 * @param[in] <tx_data> spi readwritebyte value
 * @return result
 */
uint8_t spi_readwritebyte(uint8_t wdata)
{
    uint8_t i;
    uint8_t rdata = 0;
    
    for(i = 0; i < 8; i++)
    {
        SCK_LOW;
        DELAY_1M;
        if(wdata & 0x80)
        {
            SPI_DATA_HIGH;
        }
        else
        {
            SPI_DATA_LOW;
        }

        wdata <<= 1;
        DELAY_1M;
        SCK_HIGH;
        DELAY_1M;
		
        rdata <<= 1;
		
        if(SPI_MISO_LEVEL)
        {
           rdata |= 0x01;
        }
    }
    
    SCK_LOW;
    
    return rdata;
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_high(void)
{
    CSN_HIGH;
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_low(void)
{
    CSN_LOW;
}

/**
 * @brief rf_delay_ms
 * @param[in] <time> ms
 * @return none
 */
void rf_delay_ms(uint32_t time)
{
	SYS_delay_10nop(SystemCoreClock / 10000 * time);
}

/**
 * @brief rf_delay_us
 * @param[in] <time> us
 * @return none
 */
void rf_delay_us(uint32_t time)
{
	SYS_delay_10nop(SystemCoreClock / 1000000 * time / 10);
}

/**
 * @brief do rf TX/RX IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
    rf_set_gpio_output(MODULE_GPIO_RX);
    rf_set_gpio_output(MODULE_GPIO_TX);
    rf_set_gpio_input(MODULE_GPIO_CAD_IRQ);

    rf_set_gpio_state(MODULE_GPIO_RX, 0);
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_CAD_IRQ, 0);
}

/**
 * @brief do rf XTAL IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
//	rf_set_gpio_output(MODULE_GPIO_TCXO);
//	rf_set_gpio_state(MODULE_GPIO_TCXO, 1);
}

/**
 * @brief close rf XTAL IO
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
//	rf_set_gpio_output(MODULE_GPIO_TCXO);
//	rf_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief change rf IO to rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief change rf IO to tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
    rf_set_gpio_state(MODULE_GPIO_RX, 0);
    rf_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change rf IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_RX, 0);
}



