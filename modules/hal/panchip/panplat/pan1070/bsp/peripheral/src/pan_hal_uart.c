/**************************************************************************//**
* @file     pan_hal_uart.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series UART (Universal Asynchronous Receiver-Transmitter) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#include "pan_hal.h"


UART_HandleTypeDef UART_Handle_Array[2] =
    {
        {.pUartx = UART0,
         .initObj = {0},
         .interruptObj = {0},
         .pTxBuffPtr = NULL,
         .txXferSize = 0,
         .txXferCount = 0,
         .pRxBuffPtr = NULL,
         .rxXferSize = 0,
         .rxXferCount = 0,
         .IRQn = UART0_IRQn,
         .rxIntCallback = NULL,
         .txIntCallback = NULL,
         .dmaSrc = DMAC_Peripheral_UART0_Rx,
         .dmaDst = DMAC_Peripheral_UART0_Tx,
         .errorCode = 0},
        {.pUartx = UART1,
         .initObj = {0},
         .interruptObj = {0},
         .pTxBuffPtr = NULL,
         .txXferSize = 0,
         .txXferCount = 0,
         .pRxBuffPtr = NULL,
         .rxXferSize = 0,
         .rxXferCount = 0,
         .IRQn = UART1_IRQn,
         .rxIntCallback = NULL,
         .txIntCallback = NULL,
         .dmaSrc = DMAC_Peripheral_UART1_Rx,
         .dmaDst = DMAC_Peripheral_UART1_Tx,
         .errorCode = 0}};

void HAL_DelayMs(uint32_t t)
{
    for (uint32_t i = 0; i < t; i++)
    {
        SYS_delay_10nop(0x1080);
    }
}



bool HAL_UART_Init(UART_HandleTypeDef *pUart)
{
    uint32_t tmpreg = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    uint64_t apbclock = 0x00;

	if(pUart->pUartx == UART0)
	{
		CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
	}
	else if(pUart->pUartx == UART1)
	{
		CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
	}
	
    /*---------------------------- UART BRR Configuration -----------------------*/
    /* Configure the UART Baud Rate */
    apbclock = CLK_GetPeripheralFreq((void *)pUart->pUartx);
    /*unlock to enable write & read divisor register*/
    pUart->pUartx->LCR |= UART_LCR_DLAB_Msk;
    /* Determine the integer part baud_rate_divisor =  PCLK*100 / (16*required_baud_rate)*/
    integerdivider = ((25 * apbclock) / (4 * (pUart->initObj.baudRate)));

    // Too high baudrate (too small divider) would cause DLL/DLH be all 0 which means UART disabled,
    // thus return false if this happens.
    if (integerdivider < 100)
        return false;

    tmpreg = (integerdivider / 100);
    pUart->pUartx->RBR_THR_DLL = tmpreg & 0xFF;
    pUart->pUartx->IER_DLH = (tmpreg & 0xFF00) >> 8;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * tmpreg);

    /* Implement the fractional part in the register */
    pUart->pUartx->DLF = ((((fractionaldivider * 16) + 50) / 100));
    pUart->pUartx->LCR &= ~UART_LCR_DLAB_Msk;

    /*---------------------------- UART Line Configuration -----------------------*/
    tmpreg = pUart->pUartx->LCR;
    tmpreg &= ~(UART_LCR_SP_Msk | UART_LCR_EPS_Msk | UART_LCR_PEN_Msk | UART_LCR_STOP_Msk | UART_LCR_DLS_Msk);
    tmpreg |= (pUart->initObj.format);
    pUart->pUartx->LCR = tmpreg;

    /*-------------------------------enable fifo----------------------------------*/
    pUart->pUartx->SCR |= UART_FCR_FIFOE_Msk;
    pUart->pUartx->IIR_FCR = pUart->pUartx->SCR;
    UART_EnableFifo(pUart->pUartx);

    return true;
}

void HAL_UART_SendData(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size)
{
    pUart->pTxBuffPtr = pbuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;
    while (pUart->txXferCount < pUart->txXferSize)
    {
        UART_SendData(pUart->pUartx, pUart->pTxBuffPtr[pUart->txXferCount++]);
        while (!(UART_GetLineStatus(pUart->pUartx) & UART_LSR_TEMT_Msk))
            ;
    }
}

void HAL_UART_ReceiveData(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size, uint32_t timeout)
{
    UART_SetRxTrigger(pUart->pUartx, UART_RX_FIFO_HALF_FULL);

    pUart->pRxBuffPtr = pbuf;
    pUart->rxXferSize = size;
    pUart->rxXferCount = 0;
    while (pUart->rxXferCount < pUart->rxXferSize)
    {
        while ((UART_GetLineStatus(pUart->pUartx) & UART_LSR_DR_Msk))
        {
            pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
        }
    }
}

void HAL_UART_SendData_INT(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size, UART_CallbackFunc callback)
{
    pUart->pTxBuffPtr = pbuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;
    pUart->txIntCallback = callback;
    UART_SetTxTrigger(pUart->pUartx, UART_TX_FIFO_EMPTY);

    pUart->interruptObj.switchFlag = ENABLE;
    pUart->interruptObj.interruptMode = HAL_UART_INT_THR_EMPTY;
    HAL_UART_Init_INT(pUart);
    NVIC_EnableIRQ(pUart->IRQn);
}

void HAL_UART_ReceiveData_INT(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size,uint32_t timeout,UART_CallbackFunc callback)
{
    pUart->pRxBuffPtr = pbuf;
    pUart->rxXferSize = size;
    pUart->rxXferCount = 0;
    pUart->rxIntCallback = callback;
	UART_SetRxTrigger(pUart->pUartx, UART_RX_FIFO_TWO_LESS_THAN_FULL);

    pUart->interruptObj.switchFlag = ENABLE;
    pUart->interruptObj.interruptMode = HAL_UART_INT_RECV_DATA_AVL;
    HAL_UART_Init_INT(pUart);
    pUart->interruptObj.interruptMode = HAL_UART_INT_LINE_STATUS;
    HAL_UART_Init_INT(pUart);
    pUart->interruptObj.interruptMode = HAL_UART_INT_ALL;
    HAL_UART_Init_INT(pUart);
    NVIC_EnableIRQ(pUart->IRQn);
}



void HAL_UART_SendData_DMA(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size, UART_CallbackFunc callback)
{
    pUart->interruptObj.switchFlag = ENABLE;
    pUart->interruptObj.interruptMode = HAL_UART_INT_THR_EMPTY;
    HAL_UART_Init_INT(pUart);

    pUart->pTxBuffPtr = pbuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;

    UART_ResetTxFifo(pUart->pUartx);
    UART_SetTxTrigger(pUart->pUartx, UART_TX_FIFO_HALF_FULL);
    UART_EnablePtime(pUart->pUartx); //Enable Programmable THRE Interrupt Mode

    uint32_t txChNum = 0xFF;
    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

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
    uint32_t BlockSize = size / DataWidthInByteSrc; // BlockSize = DataLen / DataWidthInByteSrc

    /* Start DMA Tx channel */
    DMAC_StartChannel(DMA, txChNum, pbuf, (void *)&(pUart->pUartx->RBR_THR_DLL), BlockSize);
}

void HAL_UART_ReceiveData_DMA(UART_HandleTypeDef *pUart, uint8_t *pbuf, size_t size, uint32_t timeout,UART_CallbackFunc callback)
{
    pUart->pRxBuffPtr = pbuf;
    pUart->rxXferSize = size;
    pUart->rxXferCount = 0;
    UART_ResetRxFifo(pUart->pUartx);

    UART_SetRxTrigger(pUart->pUartx, UART_RX_FIFO_HALF_FULL);
    uint32_t rxChNum = 0xFF;
    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);
    rxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_SRCTFR);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_BLK);
    DMAC_ClrIntFlagMsk(DMA, rxChNum, DMAC_FLAG_INDEX_ERR);

    // dma_uart2mem_config.PeripheralDst = pUart->dmaSrc;
    DMAC_Channel_Array[rxChNum].ConfigTmp = dma_uart2mem_config;
    DMAC_Channel_Array[rxChNum].ConfigTmp.PeripheralSrc = pUart->dmaSrc;
    DMAC_Channel_Array[rxChNum].CallbackUart = callback;
    DMAC_Channel_Array[rxChNum].PeriMode = DMAC_Peri_UART;
    DMAC_Channel_Array[rxChNum].pBuffPtr = (uint32_t*)pUart->pRxBuffPtr;
    DMAC_Channel_Array[rxChNum].XferSize = pUart->rxXferSize;
    DMAC_SetChannelConfig(DMA, rxChNum, &DMAC_Channel_Array[rxChNum].ConfigTmp);

    /* Condition check */
    uint32_t dataWidthInByteSrc = 1 <<DMAC_Channel_Array[rxChNum].ConfigTmp.DataWidthSrc;
    uint32_t blockSize = size / dataWidthInByteSrc;    //blockSize = DataLen / dataWidthInByteSrc
    /* Start DMA Rx channel */
    DMAC_StartChannel(DMA, rxChNum, (void*)&(pUart->pUartx->RBR_THR_DLL), pUart->pRxBuffPtr, blockSize);
}


void HAL_UART_Init_INT(UART_HandleTypeDef *pUart)
{
    if (pUart->interruptObj.switchFlag == ENABLE)
    {
        pUart->pUartx->IER_DLH |= ((pUart->interruptObj.interruptMode << UART_IER_ALL_IRQ_Pos) & UART_IER_ALL_IRQ_Msk);
    }
    else if (pUart->interruptObj.switchFlag == DISABLE)
    {
        pUart->pUartx->IER_DLH &= ~((pUart->interruptObj.interruptMode << UART_IER_ALL_IRQ_Pos) & UART_IER_ALL_IRQ_Msk);
    }
}

static void UART_HandleLineStatus(UART_HandleTypeDef *pUart)
{
    uint32_t lineStatus = UART_GetLineStatus(pUart->pUartx);

    // Filter line error
    if ((lineStatus & (UART_LINE_OVERRUN_ERR | UART_LINE_PARITY_ERR | UART_LINE_FRAME_ERR | UART_LINE_RECV_FIFO_EMPTY)) != 0x0)
    {

    }

    // Handle THRE event
    if (lineStatus & UART_LINE_THRE)
    {
        if ((pUart->pUartx->IER_DLH & UART_IER_EPTI_Msk) == 0)
        {

        }

        else if ((pUart->pUartx->IIR_FCR & UART_IIR_FIFOSE_Msk) != 0)
        {

        }
    }
}

static void UART_HandleModemStatus(UART_HandleTypeDef *pUart)
{
    uint32_t modemStatus = UART_GetModemStatus(pUart->pUartx);
}

static void UART_HandleReceivedData(UART_HandleTypeDef *pUart,UART_Cb_Flag_Opt flag)
{
    while (!UART_IsRxFifoEmpty(pUart->pUartx))
    {
        pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
    }
    
    if (flag == UART_CB_FLAG_RX_TIMEOUT)
    {
		if(pUart->rxXferCount >= pUart->rxXferSize)
		{
			pUart->rxXferCount = pUart->rxXferSize;
		}

        if (pUart->rxIntCallback != NULL)
        {
            pUart->rxIntCallback(flag,pUart->pRxBuffPtr,pUart->rxXferCount);
        }
        pUart->rxXferCount = 0;
    }
}

static void UART_HandleTransmittingData(UART_HandleTypeDef *pUart)
{
    while (!UART_IsTxFifoFull(pUart->pUartx))
    {
        if (pUart->txXferCount < pUart->txXferSize)
        {
            UART_SendData(pUart->pUartx, pUart->pTxBuffPtr[pUart->txXferCount++]);
        }
        else
        {
            UART_DisableIrq(pUart->pUartx, UART_IRQ_THR_EMPTY); // Disable THRE Interrupt after transmitting done
            if (pUart->txIntCallback != NULL)
            {
                pUart->txIntCallback(UART_CB_FLAG_TX_FINISH, pUart->pTxBuffPtr,pUart->txXferCount);
            }
            break;
        }
    }   
}

static void UART_HandleProc(UART_HandleTypeDef *pUart)
{
    UART_EventDef event = UART_GetActiveEvent(pUart->pUartx);

    switch (event)
    {
    case UART_EVENT_LINE:
        UART_HandleLineStatus(pUart);
        break;
    case UART_EVENT_DATA:
        UART_HandleReceivedData(pUart,UART_CB_FLAG_RX_FINISH);
        break;
    case UART_EVENT_TIMEOUT:
        UART_HandleReceivedData(pUart,UART_CB_FLAG_RX_TIMEOUT);
        break;
    case UART_EVENT_THR_EMPTY:
        UART_HandleTransmittingData(pUart);
        break;
    case UART_EVENT_MODEM:
        UART_HandleModemStatus(pUart);
        break;
    case UART_EVENT_NONE:
        /* Just ignore this event. */
        break;
    default:
        break;
    }
}

void UART0_IRQHandler(void)
{
    UART_HandleProc(&UART_Handle_Array[0]);
}

void UART1_IRQHandler(void)
{
    UART_HandleProc(&UART_Handle_Array[1]);
}


/*** (C) COPYRIGHT 2023 Panchip Technology Corp. ***/
