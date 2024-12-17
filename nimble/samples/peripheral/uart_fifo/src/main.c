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


#define UART_RECV_BUF_SIZE		512

uint8_t revDataBuf[UART_RECV_BUF_SIZE];


#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;
	
	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

void uart_data_send(UART_HandleTypeDef *pUart, uint8_t *pBuf, size_t size)
{
	pUart->pTxBuffPtr = pBuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;

	HAL_UART_Init_INT(pUart);
}

void tx_callback(UART_Cb_Flag_Opt i,uint8_t *pOutPtr,uint16_t size)
{
    printf("tx complete\n");
} 

void rx_callback(UART_Cb_Flag_Opt flag,uint8_t *pOutPtr,uint16_t size)
{
    if (flag == UART_CB_FLAG_RX_FINISH)
    {
        printf("rx finish\n");
    }
    else if (flag == UART_CB_FLAG_RX_TIMEOUT)
    {
		uart_data_send(&UART1_OBJ, pOutPtr, size);
        printf("rx timeout\n");
    }
    else if (flag == UART_CB_FLAG_RX_BUFFFULL)
    {
        printf("rx buff full\n");
    }
}

static void uart1_init(void)
{
	SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
	
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
	

void app_init(void)
{
    printf("app started\n");
    
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
	uart1_init();
}
