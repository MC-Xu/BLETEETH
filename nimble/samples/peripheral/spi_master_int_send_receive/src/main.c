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
#include "timers.h"
#include "semphr.h"

#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pan_hal.h"

//#define printf(...)

static uint16_t master_send_buf[] = 
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
};

static bool master_send_done_flag = false;

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

static void spi_tx_callback(SPI_CbFlagOpt flag,uint16_t* pBuf,uint16_t size)
{
	master_send_done_flag = true;
}

static void spi_rx_callback(SPI_CbFlagOpt flag,uint16_t* pBuf,uint16_t size)
{
    printf("master receive done! receive %d data as interrupt mode\n", size);
}

void hal_bsp_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_SPI0, ENABLE);

	SYS_SET_MFP(P1, 1, SPI0_MOSI);
	SYS_SET_MFP(P0, 5, SPI0_MISO);
	SYS_SET_MFP(P0, 3, SPI0_CS);
	SYS_SET_MFP(P0, 4, SPI0_CLK);

	GPIO_EnableDigitalPath(P0, BIT4);   //SPI CLK is input pin while act as master
	GPIO_EnableDigitalPath(P0, BIT3);   //SPI CS is input pin while act as master
	GPIO_EnableDigitalPath(P1, BIT1);   //SPI MOSI is input pin while act as slave
	GPIO_EnableDigitalPath(P0, BIT5);   //SPI MISO is input pin while act as master

    SPI_InitOptDef spiInitStruct = {
        .format = SPI_FormatMotorola,
        .dataSize = SPI_DataFrame_16b,
        .clkPhase = SPI_ClockPhaseSecondEdge,
        .clkPolarity = SPI_ClockPolarityLow,
        .baudrateDiv = SPI_BaudRateDiv_32
    };
	
	spiInitStruct.role = SPI_RoleMaster;
	SPI0_OBJ.initObj = spiInitStruct;
	HAL_SPI_Init(&SPI0_OBJ);
}

void spi_master_send_receive_interrupt_test(void)
{
	uint16_t master_receive_buf[160] = {0, };
	uint16_t master_send_receive_cnt = sizeof(master_send_buf) / sizeof(master_send_buf[0]);

	/* Transmit data */
	HAL_SPI_SendData_INT(&SPI0_OBJ, master_send_buf, master_send_receive_cnt, spi_tx_callback);
	while(false == master_send_done_flag){}
	master_send_done_flag = false;
	HAL_SPI_DeInit(&SPI0_OBJ);
	printf("master send data: \n");
	for (uint16_t cnt = 0; cnt < master_send_receive_cnt; cnt++) {
		printf("0x%04x,", master_send_buf[cnt]);
		if (cnt % 0x10 == 0xf) {
			printf("\n");
		}
	}
	
	printf("master turn to receive mode \n");
	SYS_delay_10nop(10000);
	HAL_SPI_Init(&SPI0_OBJ);
	/* Receive data */
	for (uint16_t cnt = 0; cnt < master_send_receive_cnt; cnt++) {
		master_send_buf[cnt] = 0;
		master_receive_buf[cnt] = 0;
	}
	HAL_SPI_SendReceiveData_INT(&SPI0_OBJ, master_send_buf, master_send_receive_cnt, spi_tx_callback,
								master_receive_buf, master_send_receive_cnt, spi_rx_callback);
	while(false == master_send_done_flag){}
	master_send_done_flag = false;
	HAL_SPI_DeInit(&SPI0_OBJ);
	printf("receive data: \n");
	for (uint16_t cnt = 0; cnt < master_send_receive_cnt; cnt++) {
		printf("0x%04x,", SPI0_OBJ.pRxBuffPtr[cnt]);
		if (cnt % 0x10 == 0xf) {
			printf("\n");
		}
	}
	
	printf("master turn to quasi mode \n");
	/*delay to wait slave ready here*/
	SYS_delay_10nop(100000);
	HAL_SPI_Init(&SPI0_OBJ);
	/* Receive data */
	for (uint16_t cnt = 0; cnt < master_send_receive_cnt; cnt++) {
		master_send_buf[cnt] = cnt;
		master_receive_buf[cnt] = 0;
	}
	HAL_SPI_SendReceiveData_INT(&SPI0_OBJ, master_send_buf, master_send_receive_cnt, spi_tx_callback,
								master_receive_buf, master_send_receive_cnt, spi_rx_callback);
	while(false == master_send_done_flag){}
	master_send_done_flag = false;
	HAL_SPI_DeInit(&SPI0_OBJ);
	printf("receive data: \n");
	for (uint16_t cnt = 0; cnt < master_send_receive_cnt; cnt++) {
		printf("0x%04x,", SPI0_OBJ.pRxBuffPtr[cnt]);
		if (cnt % 0x10 == 0xf) {
			printf("\n");
		}
	}
}

void app_init(void)
{
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
    hal_bsp_init();
	
	spi_master_send_receive_interrupt_test();
}
