/**************************************************************************//**
* @file     pan_hal_spi.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series SPI (Serial Peripheral Interface) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/


#include "pan_hal.h"

SPI_HandleTypeDef spiHandleArray[2] = 
{
    {
        .pSpix = SPI0,  
        .initObj = {0},
        .interruptObj = {0},
        .xferStat = SPI_STAT_NULL,
        .pTxBuffPtr = NULL,
        .txXferSize = 0,
        .txXferCount = 0,
        .pRxBuffPtr = NULL,
        .rxXferSize = 0,
        .rxXferCount = 0,
        .IRQn = SPI0_IRQn,  // Adjust this to appropriate IRQn_Type value
        .rxIntCallback = NULL,
        .txIntCallback = NULL,
        .dmaSrc = DMAC_Peripheral_SPI0_Rx,
        .dmaDst = DMAC_Peripheral_SPI0_Tx,
        .errorCode = 0
    },
    {
        .pSpix = SPI1,  
        .initObj = {0},
        .interruptObj = {0},
        .xferStat = SPI_STAT_NULL,
        .pTxBuffPtr = NULL,
        .txXferSize = 0,
        .txXferCount = 0,
        .pRxBuffPtr = NULL,
        .rxXferSize = 0,
        .rxXferCount = 0,
        .IRQn = SPI1_IRQn,  // Adjust this to appropriate IRQn_Type value
        .rxIntCallback = NULL,
        .txIntCallback = NULL,
        .dmaSrc = DMAC_Peripheral_SPI1_Rx,
        .dmaDst = DMAC_Peripheral_SPI1_Tx,
        .errorCode = 0
    }
};



void HAL_SPI_Init(SPI_HandleTypeDef *pSpi)
{
    uint32_t tmpreg = 0;
    //uint8_t baud_div = CalcDivisorByBaudrate(pSpi);

    if(SPI_IsSpiEnabled(pSpi->pSpix))
        SPI_DisableSpi(pSpi->pSpix);

    tmpreg = pSpi->pSpix->CR0;

    tmpreg &= ~(SPI_CR0_SPH_Msk | SPI_CR0_SPO_Msk  | SPI_CR0_DSS_Msk);
    tmpreg |= (
               ((pSpi->initObj.clkPhase           << SPI_CR0_SPH_Pos) & SPI_CR0_SPH_Msk )  |   \
                ((pSpi->initObj.clkPolarity          << SPI_CR0_SPO_Pos) & SPI_CR0_SPO_Msk )  |   \
                ((pSpi->initObj.dataSize << SPI_CR0_DSS_Pos) & SPI_CR0_DSS_Msk )      \
              );
    tmpreg &= ~(SPI_CR0_FRF_Msk);
    tmpreg |= (pSpi->initObj.format << SPI_CR0_FRF_Pos);
    tmpreg &= ~(SPI_CR0_SCR_Msk);
    // tmpreg |= ((pSpi->SPI_InitStruct->SPI_baudRateDiv-1) << SPI_CR0_SCR_Pos);   //Fixed SCR to 0, only use CPSR to config baudrate
    //tmpreg |= ((baud_div-1) << SPI_CR0_SCR_Pos);   //Fixed SCR to 0, only use CPSR to config baudrate
    tmpreg |= ((pSpi->initObj.baudrateDiv-1) << SPI_CR0_SCR_Pos);   //Fixed SCR to 0, only use CPSR to config baudrate    
    pSpi->pSpix->CR0 = tmpreg;

    //Baudrate Config, expect_baudrate = apb_clk / ((1 + SCR) * CPSR)
    pSpi->pSpix->CPSR = 2;//SPI_InitStruct->SPI_baudRateDiv;

    //role Select
    if(pSpi->initObj.role == SPI_RoleSlave)
    {
        pSpi->pSpix->CR1 |= SPI_CR1_MS_Msk;
    }
    else
    {
        pSpi->pSpix->CR1 &= ~SPI_CR1_MS_Msk;
    }

    SPI_EnableSpi(pSpi->pSpix);
}

void HAL_SPI_DeInit(SPI_HandleTypeDef *pSpi){
    SPI_DisableSpi(pSpi->pSpix);
}

void HAL_SPI_Init_INT(SPI_HandleTypeDef *pSpi){
    SPI_EnableIrq(pSpi->pSpix, pSpi->interruptObj.Mode);
}

void HAL_SPI_DeInit_INT(SPI_HandleTypeDef *pSpi){
    SPI_DisableIrq(pSpi->pSpix, pSpi->interruptObj.Mode);
}


// Receive multiple data from specified SPI, and return the size of actual data received
static size_t HAL_SPI_RecvMultiDataAsync(SPI_T* pSpix, uint16_t* data, size_t expectRecvSize)
{
    size_t i = 0;
    for (; i < expectRecvSize; i++)
    {
        // Fetch data from Rx FIFO until it's empty
        if (!SPI_IsRxFifoEmpty(pSpix))
        {
            data[i] = SPI_ReceiveData(pSpix);
        }
        else
        {
            break;
        }
    }
    return i;
}

// Send multiple data to specified SPI, and return the size of actual data sent
static size_t HAL_SPI_SendMultiDataAsync(SPI_T* pSpix, const uint16_t* data, size_t expectSendSize)
{
    size_t i = 0;
    for (; i < expectSendSize; i++)
    {
        // Fill data into Tx FIFO until it's full
        if (!SPI_IsTxFifoFull(pSpix))
        {
            SPI_SendData(pSpix, data[i]);
        }
        else
        {
            break;
        }
    }
    return i;
}

void HAL_SPI_SendData(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, uint32_t timeout)
{    
	pSpi->txXferSize = size;
    pSpi->txXferCount =0;
    pSpi->pTxBuffPtr = pBuf;

    while (pSpi->txXferCount < pSpi->txXferSize)
    {
        if (!SPI_IsTxFifoFull(pSpi->pSpix))
        {
            SPI_SendData(pSpi->pSpix, pSpi->pTxBuffPtr[pSpi->txXferCount++]);
			(void)(pSpi->pSpix->DR);
        }
    }
	while(SPI_IsBusy(pSpi->pSpix)){}
}

void HAL_SPI_ReceiveData(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, uint32_t timeout)
{
    pSpi->rxXferCount = 0;
    pSpi->rxXferSize =size;
    pSpi->pRxBuffPtr = pBuf;

    if(pSpi->initObj.role == SPI_RoleMaster)
    {
		pSpi->txXferCount = 0;
		pSpi->txXferSize =size;

		while (!(pSpi->txXferCount == pSpi->txXferSize && pSpi->rxXferCount == pSpi->rxXferSize))
		{
			if (pSpi->txXferCount < pSpi->txXferSize)
			{
				if (!SPI_IsTxFifoFull(pSpi->pSpix)) {
					SPI_SendData(pSpi->pSpix, '\0');
					pSpi->txXferCount++;
				}
			}
			if (pSpi->rxXferCount < pSpi->rxXferSize)
			{
				if (!SPI_IsRxFifoEmpty(pSpi->pSpix)) {
					pSpi->pRxBuffPtr[pSpi->rxXferCount++] = SPI_ReceiveData(pSpi->pSpix);
				}
			}
		}
    }
    else if (pSpi->initObj.role == SPI_RoleSlave)
    {
        while (pSpi->rxXferCount < pSpi->rxXferSize)
        {
            if (!SPI_IsRxFifoEmpty(pSpi->pSpix))
            {
                pSpi->pRxBuffPtr[pSpi->rxXferCount++] = SPI_ReceiveData(pSpi->pSpix);
            }
        }
    }
}

void HAL_SPI_SendReceiveData (SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize, uint16_t* pRecvBuf, size_t recvBufSize)
{
    pSpi->rxXferCount = 0;
    pSpi->rxXferSize =recvBufSize;
	pSpi->pRxBuffPtr = pRecvBuf;
	pSpi->txXferCount = 0;
	pSpi->txXferSize =sendBufSize;
	pSpi->pTxBuffPtr = pSendBuf;

	while (!(pSpi->txXferCount == pSpi->txXferSize && pSpi->rxXferCount == pSpi->rxXferSize))
	{
		if (pSpi->txXferCount < pSpi->txXferSize)
		{
			pSpi->txXferCount += HAL_SPI_SendMultiDataAsync(pSpi->pSpix, (uint16_t*)(pSpi->pTxBuffPtr + pSpi->txXferCount), pSpi->txXferSize - pSpi->txXferCount);
		}
		if (pSpi->rxXferCount < pSpi->rxXferSize)
		{
			pSpi->rxXferCount += HAL_SPI_RecvMultiDataAsync(pSpi->pSpix, (uint16_t*)(pSpi->pRxBuffPtr + pSpi->rxXferCount), pSpi->rxXferSize - pSpi->rxXferCount);
		}
	}
}


void HAL_SPI_SendData_INT(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, SPI_CallbackFunc callback) 
{
    pSpi->xferStat = SPI_STAT_TX;
    /* Enable Interrupt of target SPI */
    // pSpi->interruptObj.Mode = SPI_IRQ_ALL;
    pSpi->txXferSize = size;
    pSpi->txXferCount =0;
    pSpi->pTxBuffPtr = pBuf;
    pSpi->txIntCallback = callback;
    // HAL_SPI_Init_INT(pSpi);
    SPI_EnableIrq(pSpi->pSpix, SPI_IRQ_TX_HALF_EMPTY);
    NVIC_EnableIRQ(pSpi->IRQn);        
}

void HAL_SPI_ReceiveData_INT(SPI_HandleTypeDef *pSpi, uint16_t* pBuf, size_t size,SPI_CallbackFunc callback)
{
    pSpi->xferStat = SPI_STAT_RX;

    pSpi->rxXferSize = size;
    pSpi->rxXferCount =0;
    pSpi->pRxBuffPtr = pBuf;
    pSpi->rxIntCallback = callback;
    SPI_EnableIrq(pSpi->pSpix, SPI_IRQ_RX_HALF_FULL | SPI_IRQ_RX_TIMEOUT);
    NVIC_EnableIRQ(pSpi->IRQn);
}

void HAL_SPI_SendReceiveData_INT(SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize,SPI_CallbackFunc sendCallback, uint16_t* pRecvBuf, size_t recvBufSize,SPI_CallbackFunc recvCallback)
{
    pSpi->xferStat = SPI_STAT_RXTX;

    pSpi->txXferSize = sendBufSize;
    pSpi->txXferCount =0;
    pSpi->pTxBuffPtr = pSendBuf;
    pSpi->txIntCallback = sendCallback;

    pSpi->rxXferSize = recvBufSize;
    pSpi->rxXferCount =0;
    pSpi->pRxBuffPtr = pRecvBuf;
    pSpi->rxIntCallback = recvCallback;
    SPI_EnableIrq(pSpi->pSpix, SPI_IRQ_ALL);
    NVIC_EnableIRQ(pSpi->IRQn);
}


uint32_t HAL_SPI_SendData_DMA(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size,SPI_CallbackFunc callback) 
{
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);               
    SPI_EnableDmaTx(pSpi->pSpix);

	pSpi->xferStat = SPI_STAT_TX;
    pSpi->txXferCount =0;
    pSpi->txXferSize = size;
    pSpi->pTxBuffPtr =pBuf;
    uint32_t dmaChNum = 0xFF;
    /* Get free DMA channel */
    dmaChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_DSTTFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_ERR);

    dma_mem2spi_config.PeripheralDst = pSpi->dmaDst;
    DMAC_Channel_Array[dmaChNum].ConfigTmp = dma_mem2spi_config;
    DMAC_Channel_Array[dmaChNum].CallbackSpi = callback;
//    DMAC_Channel_Array[dmaChNum].StopFlag = false;

    DMAC_Channel_Array[dmaChNum].pBuffPtr = (uint32_t *)pSpi->pTxBuffPtr;
	DMAC_Channel_Array[dmaChNum].XferSize = pSpi->txXferSize;
	DMAC_Channel_Array[dmaChNum].PeriMode = DMAC_Peri_SPI;    
    DMAC_SetChannelConfig(DMA, dmaChNum, &DMAC_Channel_Array[dmaChNum].ConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc =  DMAC_Channel_Array[dmaChNum].ConfigTmp.DataWidthSrc;
    // uint32_t DataWidthInByteSrc = 1 <<  DMAC_Channel_Array[dmaChNum].ConfigTmp.DataWidthSrc;
    uint32_t IsNotDivisible = pSpi->txXferSize % DataWidthInByteSrc;
    uint32_t BlockSize = pSpi->txXferSize / DataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* Start DMA Tx channel */
    DMAC_StartChannel(DMA, dmaChNum, pSpi->pTxBuffPtr, (void*)&(pSpi->pSpix->DR), BlockSize);
    return dmaChNum;
}

uint32_t HAL_SPI_ReceiveData_DMA(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, SPI_CallbackFunc callback) 
{
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

	pSpi->xferStat = SPI_STAT_RX;
    pSpi->rxXferCount =0;
    pSpi->rxXferSize = size;
    pSpi->pRxBuffPtr =pBuf;
    // Initialize the DMA controller
    uint32_t dmaChNum = 0xff;
    /* Get free DMA channel */
    dmaChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_SRCTFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_ERR);

    dma_spi2mem_config.PeripheralSrc = pSpi->dmaSrc;
    DMAC_Channel_Array[dmaChNum].ConfigTmp = dma_spi2mem_config;
    DMAC_Channel_Array[dmaChNum].CallbackSpi = callback;
//    DMAC_Channel_Array[dmaChNum].StopFlag = false;

    DMAC_Channel_Array[dmaChNum].pBuffPtr = (uint32_t *)pSpi->pRxBuffPtr;
	DMAC_Channel_Array[dmaChNum].XferSize = pSpi->rxXferSize;
	DMAC_Channel_Array[dmaChNum].PeriMode = DMAC_Peri_SPI;
    DMAC_SetChannelConfig(DMA, dmaChNum, &DMAC_Channel_Array[dmaChNum].ConfigTmp);

    /* Condition check */
    uint32_t RxDataWidthInByteSrc =  DMAC_Channel_Array[dmaChNum].ConfigTmp.DataWidthSrc;
    uint32_t RxIsNotDivisible = pSpi->rxXferSize % RxDataWidthInByteSrc;
    uint32_t RxBlockSize = pSpi->rxXferSize / RxDataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

	/*We should clear rx fifo before rx start*/
	while (!SPI_IsRxFifoEmpty(pSpi->pSpix)) {
		(void)(pSpi->pSpix->DR);
	}
	SPI_EnableDmaRx(pSpi->pSpix);
	/* Start DMA Rx channel */
    DMAC_StartChannel(DMA, dmaChNum, (void*)&(pSpi->pSpix->DR), pSpi->pRxBuffPtr, RxBlockSize);

    return dmaChNum;
}

static uint32_t HAL_SPI_SendData_DMA_ForSR(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size,SPI_CallbackFunc callback) 
{
    SPI_EnableDmaTx(pSpi->pSpix);

	pSpi->xferStat = SPI_STAT_RXTX;
    pSpi->txXferCount =0;
    pSpi->txXferSize = size;
    pSpi->pTxBuffPtr =pBuf;
    uint32_t dmaChNum = 0xFF;
    /* Get free DMA channel */
    dmaChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_DSTTFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_ERR);

    dma_mem2spi_config.PeripheralDst = pSpi->dmaDst;
    DMAC_Channel_Array[dmaChNum].ConfigTmp = dma_mem2spi_config;
    DMAC_Channel_Array[dmaChNum].CallbackSpi = callback;
//    DMAC_Channel_Array[dmaChNum].StopFlag = false;

    DMAC_Channel_Array[dmaChNum].pBuffPtr = (uint32_t *)pSpi->pTxBuffPtr;
	DMAC_Channel_Array[dmaChNum].XferSize = pSpi->txXferSize;
	DMAC_Channel_Array[dmaChNum].PeriMode = DMAC_Peri_SPI;    
    DMAC_SetChannelConfig(DMA, dmaChNum, &DMAC_Channel_Array[dmaChNum].ConfigTmp);

    return dmaChNum;
}


static uint32_t HAL_SPI_ReceiveData_DMA_ForSR(SPI_HandleTypeDef *pSpi, uint16_t *pBuf, size_t size, SPI_CallbackFunc callback) 
{
	pSpi->xferStat = SPI_STAT_RXTX;
    pSpi->rxXferCount =0;
    pSpi->rxXferSize = size;
    pSpi->pRxBuffPtr =pBuf;
    // Initialize the DMA controller
    uint32_t dmaChNum = 0xff;
    /* Get free DMA channel */
    dmaChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_SRCTFR);
    DMAC_ClrIntFlagMsk(DMA, dmaChNum, DMAC_FLAG_INDEX_ERR);


    dma_spi2mem_config.PeripheralSrc = pSpi->dmaSrc;
    DMAC_Channel_Array[dmaChNum].ConfigTmp = dma_spi2mem_config;
    DMAC_Channel_Array[dmaChNum].CallbackSpi = callback;
//    DMAC_Channel_Array[dmaChNum].StopFlag = false;

    DMAC_Channel_Array[dmaChNum].pBuffPtr = (uint32_t *)pSpi->pRxBuffPtr;
	DMAC_Channel_Array[dmaChNum].XferSize = pSpi->rxXferSize;
	DMAC_Channel_Array[dmaChNum].PeriMode = DMAC_Peri_SPI;
    DMAC_SetChannelConfig(DMA, dmaChNum, &DMAC_Channel_Array[dmaChNum].ConfigTmp);

	while (!SPI_IsRxFifoEmpty(pSpi->pSpix)) {
		(void)(pSpi->pSpix->DR);
	}
    SPI_EnableDmaRx(pSpi->pSpix);

    return dmaChNum;
}

void HAL_SPI_SendReceiveData_DMA(SPI_HandleTypeDef *pSpi, uint16_t* pSendBuf, size_t sendBufSize,SPI_CallbackFunc sendCallback, uint16_t* pRecvBuf, size_t recvBufSize,SPI_CallbackFunc recvCallback)
{
    uint32_t rxChNum = 0xFF;
    uint32_t txChNum = 0xFF;
	/* We should clear rx fifo before rx start */

    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);
    txChNum = HAL_SPI_SendData_DMA_ForSR(pSpi,pSendBuf,sendBufSize,sendCallback);

    rxChNum = HAL_SPI_ReceiveData_DMA_ForSR(pSpi,pRecvBuf,recvBufSize,recvCallback);
    /* Condition check */
    uint32_t RxDataWidthInByteSrc =  DMAC_Channel_Array[rxChNum].ConfigTmp.DataWidthSrc;
    uint32_t RxIsNotDivisible = pSpi->rxXferSize % RxDataWidthInByteSrc;
    uint32_t RxBlockSize = pSpi->rxXferSize / RxDataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc


    /* Condition check */
    uint32_t TxDataWidthInByteSrc =  DMAC_Channel_Array[txChNum].ConfigTmp.DataWidthSrc;
    // uint32_t DataWidthInByteSrc = 1 <<  DMAC_Channel_Array[dmaChNum].ConfigTmp.DataWidthSrc;
    uint32_t TxIsNotDivisible = pSpi->txXferSize % TxDataWidthInByteSrc;
    uint32_t TxBlockSize = pSpi->txXferSize / TxDataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* Start DMA Rx channel */
    DMAC_StartChannel(DMA, rxChNum, (void*)&(pSpi->pSpix->DR), pSpi->pRxBuffPtr, RxBlockSize);
    /* Start DMA Tx channel */
    DMAC_StartChannel(DMA, txChNum, pSpi->pTxBuffPtr, (void*)&(pSpi->pSpix->DR), TxBlockSize);
}

static void SPI_HandleReceivedData(SPI_HandleTypeDef *pSpi)
{
    while (!SPI_IsRxFifoEmpty(pSpi->pSpix))
    {
        pSpi->pRxBuffPtr[pSpi->rxXferCount++] = SPI_ReceiveData(pSpi->pSpix);

        if (pSpi->rxXferCount >= pSpi->rxXferSize) // Rx buffer full                                                 // if (pSpi->rxXferCount >= pSpi->rxXferSize)   //Rx buffer full
        {
            if (pSpi->rxIntCallback != NULL)
            {
                pSpi->rxIntCallback(SPI_CB_FLAG_INT, pSpi->pRxBuffPtr, pSpi->rxXferCount);
            }
            SPI_DisableIrq(pSpi->pSpix, SPI_IRQ_RX_HALF_FULL|SPI_IRQ_RX_TIMEOUT);
            break;
        }
    }
}

static void SPI_HandleTransmittingData(SPI_HandleTypeDef *pSpi)
{
    while (!SPI_IsTxFifoFull(pSpi->pSpix))
    {
        if (pSpi->txXferCount < pSpi->txXferSize)
        {
            SPI_SendData(pSpi->pSpix, pSpi->pTxBuffPtr[pSpi->txXferCount++]);
			if ((!SPI_IsRxFifoEmpty(pSpi->pSpix)) && (pSpi->xferStat == SPI_STAT_RXTX)) {
				pSpi->pRxBuffPtr[pSpi->rxXferCount++] = SPI_ReceiveData(pSpi->pSpix);
			}
        }
        if (pSpi->txXferCount >= pSpi->txXferSize)
        {
			if (pSpi->initObj.role == SPI_RoleMaster) {
				while(SPI_IsBusy(pSpi->pSpix)){}
			}
            if (pSpi->txIntCallback != NULL)
            {
                pSpi->txIntCallback(SPI_CB_FLAG_INT, pSpi->pTxBuffPtr, pSpi->txXferCount);
            }
			SPI_DisableIrq(pSpi->pSpix, SPI_IRQ_TX_HALF_EMPTY);
            break;
        }
    }
	if (pSpi->xferStat == SPI_STAT_TX) {
		while (!SPI_IsRxFifoEmpty(pSpi->pSpix)) {
			(void)(pSpi->pSpix->DR);
		}
	}
}
void SPI_HandleProc(SPI_HandleTypeDef *pSpi)
{
   if (SPI_IsIrqActive(pSpi->pSpix, SPI_IRQ_RX_OVERRUN))
   {
       SPI_ClearIrq(pSpi->pSpix, SPI_IRQ_RX_OVERRUN); // Clear overrun INT flag manually
       return;
   }

   if (SPI_IsIrqActive(pSpi->pSpix, SPI_IRQ_RX_HALF_FULL))
   {
       SPI_HandleReceivedData(pSpi);
   }
   if (SPI_IsIrqActive(pSpi->pSpix, SPI_IRQ_RX_TIMEOUT))
   {
       SPI_HandleReceivedData(pSpi);
   }

   if (SPI_IsIrqActive(pSpi->pSpix, SPI_IRQ_TX_HALF_EMPTY))
   {
       SPI_HandleTransmittingData(pSpi);
   }
}

void SPI0_IRQHandler(void)
{
    SPI_HandleProc(&spiHandleArray[0]);
}

void SPI1_IRQHandler(void)
{
    SPI_HandleProc(&spiHandleArray[1]);
}



