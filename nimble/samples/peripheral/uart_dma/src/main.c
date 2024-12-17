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


#define UART_RECV_BUF_SIZE		128
#define UART_SEND_BUF_SIZE		128
#define UART_RECV_TEST_SIZE		32

uint8_t revDataBuf[UART_RECV_BUF_SIZE];
uint8_t sendDataBuf[UART_SEND_BUF_SIZE];

uint32_t txChNum = 0xFF;
uint32_t rxChNum = 0xFF;


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
	DMAC_StartChannel(DMA, txChNum, pBuf, (void *)&(pUart->pUartx->RBR_THR_DLL), size);
}

void uart_data_Recv(UART_HandleTypeDef *pUart, size_t Size)
{
	DMAC_StartChannel(DMA, rxChNum, (void*)&(pUart->pUartx->RBR_THR_DLL), pUart->pRxBuffPtr, Size);
}

void rx_callback(UART_Cb_Flag_Opt flag,uint8_t *pOutPtr,uint16_t Size)
{
	printf("rx done\n");

    uart_data_send(&UART1_OBJ, pOutPtr, Size);
}

void tx_callback(UART_Cb_Flag_Opt i,uint8_t *pOutPtr,uint16_t Size)
{
    printf("tx complete\n");

	uart_data_Recv(&UART1_OBJ, UART_RECV_TEST_SIZE);
} 

static void hal_uart_dma_recv_init(UART_HandleTypeDef *uart, uint8_t *Buf, size_t Size, UART_CallbackFunc Callback)
{
	uart->pRxBuffPtr = Buf;
    uart->rxXferSize = Size;
    uart->rxXferCount = 0;
    UART_ResetRxFifo(uart->pUartx);
    UART_SetRxTrigger(uart->pUartx, UART_RX_FIFO_HALF_FULL);

    rxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_SRCTFR);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_BLK);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_ERR);

    DMAC_Channel_Array[rxChNum].ConfigTmp = dma_uart2mem_config;
    DMAC_Channel_Array[rxChNum].ConfigTmp.PeripheralSrc = uart->dmaSrc;
    DMAC_Channel_Array[rxChNum].CallbackUart = Callback;
    DMAC_Channel_Array[rxChNum].PeriMode = DMAC_Peri_UART;
    DMAC_Channel_Array[rxChNum].pBuffPtr = (uint32_t*)uart->pRxBuffPtr;
    DMAC_Channel_Array[rxChNum].XferSize = uart->rxXferSize;
    DMAC_SetChannelConfig(DMA, rxChNum, &DMAC_Channel_Array[rxChNum].ConfigTmp);

    /* Condition check */
    uint32_t dataWidthInByteSrc = 1 <<DMAC_Channel_Array[rxChNum].ConfigTmp.DataWidthSrc;
    uint32_t blockSize = Size / dataWidthInByteSrc;
	
	printf("Recv BlockSize %d,Dma Ch%d\n",blockSize,rxChNum);
}

static void hal_uart_dma_send_init(UART_HandleTypeDef *pUart, uint8_t *pBuf, size_t size, UART_CallbackFunc callback)
{
    pUart->pTxBuffPtr = pBuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;

    UART_ResetTxFifo(pUart->pUartx);
    UART_SetTxTrigger(pUart->pUartx, UART_TX_FIFO_HALF_FULL);
    UART_EnablePtime(pUart->pUartx); //Enable Programmable THRE Interrupt Mode

    /* Get free DMA channel */
    txChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, txChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, txChNum, DMAC_FLAG_INDEX_DSTTFR);
    DMAC_ClrIntFlagMsk(DMA, txChNum, DMAC_FLAG_INDEX_BLK);
    DMAC_ClrIntFlagMsk(DMA, txChNum, DMAC_FLAG_INDEX_ERR);

    dma_mem2uart_config.PeripheralDst = pUart->dmaDst;
    DMAC_Channel_Array[txChNum].ConfigTmp = dma_mem2uart_config;
    DMAC_Channel_Array[txChNum].CallbackUart = callback;
    DMAC_Channel_Array[txChNum].PeriMode = DMAC_Peri_UART;
    DMAC_Channel_Array[txChNum].pBuffPtr = (uint32_t*)pUart->pTxBuffPtr;
    DMAC_Channel_Array[txChNum].XferSize = pUart->txXferSize;
    DMAC_SetChannelConfig(DMA, txChNum, &DMAC_Channel_Array[txChNum].ConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc = 1 << DMAC_Channel_Array[txChNum].ConfigTmp.DataWidthSrc;
    uint32_t BlockSize = size / DataWidthInByteSrc; 

	printf("Send BlockSize %d,Dma Ch%d\n",BlockSize, txChNum);
}

static void uart1_init(void)
{
	SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);

	UART1_OBJ.initObj.baudRate = 115200;
	UART1_OBJ.initObj.format = HAL_UART_FMT_8_N_1;  
	HAL_UART_Init(&UART1_OBJ);

	HAL_DMA_Init();
	
	hal_uart_dma_recv_init(&UART1_OBJ, revDataBuf, UART_RECV_BUF_SIZE, rx_callback);
	hal_uart_dma_send_init(&UART1_OBJ, sendDataBuf, UART_SEND_BUF_SIZE, tx_callback);

	uart_data_Recv(&UART1_OBJ, UART_RECV_TEST_SIZE);
}
	

void app_init(void)
{
    printf("app started\n");
    
	
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
	uart1_init();
}
