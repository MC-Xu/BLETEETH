/**************************************************************************//**
* @file     pan_hal_i2c.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series I2C (Inter-Integrated Circuit) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"

I2C_HandleTypeDef* I2C_INST_PTR;

static void I2C_SendData(I2C_T* I2Cx, uint8_t Data)
{
    I2Cx->DATACMD =  Data & 0xFF;
}

bool HAL_I2C_Init(I2C_HandleTypeDef *i2c)
{
    uint32_t tmpreg = 0;
    uint32_t result = 0;
    uint32_t pclk;

    I2C_INST_PTR = i2c;
    
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_I2C0, ENABLE);

    // The IC_SAR holds the slave address when the I2C is operating as a slave
    if (i2c->I2C_InitObj.AddressMode == I2C_ADDR_10BIT)
    {
        i2c->I2C_InitObj.AckAddress = I2C_ACK_ADDR_10BIT;
    }
    else
    {
        i2c->I2C_InitObj.AckAddress = I2C_ACK_ADDR_7BIT;
    }

    I2C_Cmd(i2c->I2Cx, DISABLE);

    /*get system clock frequency*/
    pclk = CLK_GetPeripheralFreq(i2c->I2Cx);


    /*---------------------------- I2Cx CON Configuration ------------------------*/
    /*Disable I2C */

    tmpreg = i2c->I2Cx->CON;

    /*restart enable*/
    tmpreg |= (I2C_CON_IC_RESTART_EN);
    /* slave address mode*/
    if (I2C_ACK_ADDR_7BIT == i2c->I2C_InitObj.AckAddress)
    {
        tmpreg &= ~I2C_CON_IC_10BITADDR_SLAVE;
    }
    else // 10-bit addr
    {
        tmpreg |= I2C_CON_IC_10BITADDR_SLAVE;
    }

    tmpreg &= ~(I2C_CON_SPEED);
    /*---------------------------- I2Cx HCNT/LCNT Configuration ------------------------*/
    /* Configure speed in standard mode */
    result = pclk / i2c->I2C_InitObj.ClockSpeed;
    if (i2c->I2C_InitObj.ClockSpeed <= 100000)
    {
        /* Standart mode speed calculate: Tlow/Thigh = 1 */
        result /= 2;
        if (result < i2c->I2Cx->FS_SPKLEN + 7)
            return false;
        /*HCNT equals to LCNT*/
        i2c->I2Cx->SS_SCL_HCNT = result - i2c->I2Cx->FS_SPKLEN - 7;
        i2c->I2Cx->SS_SCL_LCNT = result - 1;

        tmpreg |= I2C_SPEED_STANDARD_MODE;
    }
    /* Configure speed in fast mode */
    else if (i2c->I2C_InitObj.ClockSpeed <= 1000000)
    {
        if (i2c->I2C_InitObj.DutyCycle == I2C_DutyCycle_2)
        {
            /* Fast mode speed calculate: Tlow/Thigh = 2 */
            result /= 3;
            if (result < i2c->I2Cx->FS_SPKLEN + 7)
                return false;
            i2c->I2Cx->FS_SCL_HCNT = result - i2c->I2Cx->FS_SPKLEN - 7;
            i2c->I2Cx->FS_SCL_LCNT = (result << 1) - 1;
        }
        else
        {
            /* Fast mode speed calculate: Tlow/Thigh = 16/9 */
            if (result * 9 / 25 < i2c->I2Cx->FS_SPKLEN + 7)
                return false;
            /* Set DUTY bit */
            i2c->I2Cx->FS_SCL_HCNT = result * 9 / 25 - i2c->I2Cx->FS_SPKLEN - 7;
            i2c->I2Cx->FS_SCL_LCNT = (result * 16 / 25) - 1;
        }

        tmpreg |= I2C_SPEED_FAST_MODE;
    }
    /* To use the I2C at 1 MHz (in fast mode plus ) */
    else
    {
        result /= 3;
        if (result < i2c->I2Cx->HS_SPKLEN + 7)
            return false;
        /* Set DUTY bit */
        i2c->I2Cx->HS_SCL_HCNT = result - i2c->I2Cx->HS_SPKLEN - 7;
        i2c->I2Cx->HS_SCL_LCNT = (result << 1) - 1;
        tmpreg |= I2C_SPEED_HIGH_MODE;
    }

    i2c->I2Cx->CON = tmpreg;

    /*data setup time: 250ns*/
    i2c->I2Cx->SDA_SETUP = (pclk) / 4000000 + 1;

    /* Set I2Cx Own Address1 and acknowledged address */
    i2c->I2Cx->SAR &= ~(I2C_TAR_TAR);
    /* Get 7-bit address from I2C_OwnAddress1*/
    i2c->I2Cx->SAR |= ((i2c->I2C_InitObj.OwnAddress));
    /*fifo threshold */
    i2c->I2Cx->TX_TL = I2C_TX_TL_4;
    i2c->I2Cx->RX_TL = I2C_RX_TL_4;

    // I2C_InitRole() begin
    if (i2c->I2C_InitObj.Role == I2C_ROLE_MASTER)
    {
        // 1. set master mode
        I2C_SetMode(i2c->I2Cx, I2C_ROLE_MASTER);
        // 2. set remote slave addr
        if (i2c->I2C_InitObj.AddressMode == I2C_ADDR_10BIT)
        {
            I2C_Set10bitAddress(i2c->I2Cx, i2c->I2C_InitObj.OwnAddress);
        }
        else {
            I2C_Set7bitAddress(i2c->I2Cx, i2c->I2C_InitObj.OwnAddress);
        }
        i2c->I2Cx->SDA_HOLD = ((0x3 << 16) | 0x3);
    }
    else
    {
        I2C_SetMode(i2c->I2Cx, I2C_ROLE_SLAVE);
    }
    
    I2C_DisableAllIT(i2c->I2Cx);

    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);
    // I2C_InitRole() end
    return 0;
}

static void HAL_I2C_SetAddress(I2C_HandleTypeDef *i2c, uint16_t DevAddress)
{
    if (i2c->I2C_InitObj.AddressMode == I2C_ADDR_10BIT)
    {
        I2C_Set10bitAddress(i2c->I2Cx, DevAddress);
    }
    else
    {
        I2C_Set7bitAddress(i2c->I2Cx, (uint8_t)DevAddress);
    }

    i2c->I2Cx->SDA_HOLD = ((0x3 << 16) | 0x3);
}

void HAL_I2C_DeInit(I2C_HandleTypeDef *i2c) {
    I2C_Cmd(i2c->I2Cx, DISABLE);
	I2C_DisableAllIT(i2c->I2Cx);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_I2C0, DISABLE);
}

void HAL_I2C_Init_INT(I2C_HandleTypeDef *i2c)
{
    I2C_Cmd(i2c->I2Cx, DISABLE);
    I2C_DisableAllIT(i2c->I2Cx);
    I2C_ITConfig(i2c->I2Cx, I2C_IT_START_DET | I2C_IT_RX_FULL | I2C_IT_RD_REQ | I2C_IT_STOP_DET, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);

    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_0);
    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_0);
}

void HAL_I2C_DeInit_INT(I2C_HandleTypeDef *i2c)
{
    I2C_DisableAllIT(i2c->I2Cx);
}

void HAL_I2C_Master_SendData(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, uint32_t Timeout)
{

    HAL_I2C_SetAddress(i2c, DevAddress);
    
    while (Size)
    {
        while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF))
            ;
        if (Size > 1)
            i2c->I2Cx->DATACMD = ((I2C_CMD_WR << 8) | (*Buf & 0xFF));
        else
        {
            if (I2C_CON_MASTER_MODE & i2c->I2Cx->CON)
                i2c->I2Cx->DATACMD = ((I2C_CMD_STOP << 8) | (*Buf & 0xFF));
            else
                i2c->I2Cx->DATACMD = ((I2C_CMD_WR << 8) | (*Buf & 0xFF));
        }
        Buf++;
        Size--;
    }
}

void HAL_I2C_Master_ReceiveData(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, uint32_t Timeout)
{
    HAL_I2C_SetAddress(i2c, DevAddress);
    
    while (Size)
    {
        if (I2C_CON_MASTER_MODE & i2c->I2Cx->CON)
        {
            while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF))
            {
            }
            if (Size > 1)
            {
                I2C_SendCmd(i2c->I2Cx, I2C_CMD_RD);
            }
            else
            {
                I2C_SendCmd(i2c->I2Cx, I2C_CMD_RD | I2C_CMD_STOP);
            }
        }
        while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_RFNE))
        {
        }
        *Buf = (uint8_t)i2c->I2Cx->DATACMD;
        Buf++;
        Size--;
    }
}

void HAL_I2C_Master_SendData_INT(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    i2c->pTxBuffPtr = Buf;
    i2c->TxXferSize = Size;
    i2c->TxXferCount = 0;
    i2c->TxIntCallback = Callback;
    I2C_Cmd(i2c->I2Cx, DISABLE);
    HAL_I2C_SetAddress(i2c, DevAddress);
    I2C_DisableAllIT(i2c->I2Cx);
    I2C_ITConfig(i2c->I2Cx, I2C_IT_TX_EMPTY | I2C_IT_START_DET | I2C_IT_RX_FULL | I2C_IT_RD_REQ | I2C_IT_STOP_DET, ENABLE);

    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);
    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_4);
    NVIC_EnableIRQ(i2c->IRQn);

    while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF));
    I2C_SendDataCmd(I2C0, *i2c->pTxBuffPtr, I2C_CMD_WR);

    i2c->TxXferCount++;
}

void HAL_I2C_Master_ReceiveData_INT(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    i2c->pRxBuffPtr = Buf;
    i2c->RxXferSize = Size;
    i2c->RxXferCount = 0;
    i2c->RxIntCallback = Callback;
    I2C_Cmd(i2c->I2Cx, DISABLE);
    HAL_I2C_SetAddress(i2c, DevAddress);
    I2C_DisableAllIT(i2c->I2Cx);
    I2C_ITConfig(i2c->I2Cx, I2C_IT_START_DET | I2C_IT_RX_FULL | I2C_IT_RD_REQ | I2C_IT_STOP_DET, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);
    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_0);

    NVIC_EnableIRQ(i2c->IRQn);

    while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF));
    I2C_SendCmd(i2c->I2Cx, I2C_CMD_RD);

}

void HAL_I2C_Master_SendData_DMA(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    HAL_I2C_SetAddress(i2c, DevAddress);
    i2c->pTxBuffPtr = Buf;
    i2c->TxXferSize = Size;
    i2c->TxXferCount = 0;

    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_0);
    I2C_DMATransferDataLevel(i2c->I2Cx, I2C_TX_TL_0);

    I2C_DMACmd(i2c->I2Cx, ENABLE);
    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    uint32_t DmaChNum = 0xFF;
    DmaChNum = DMAC_AcquireChannel(DMA);

    dma_mem2i2c_config.PeripheralDst = i2c->DMA_DST;
    DMAC_Channel_Array[DmaChNum].ConfigTmp = dma_mem2i2c_config;
    DMAC_Channel_Array[DmaChNum].CallbackI2c = Callback;
    DMAC_Channel_Array[DmaChNum].XferFlag = false;
    DMAC_Channel_Array[DmaChNum].pBuffPtr = (uint32_t *)i2c->pTxBuffPtr;
    DMAC_Channel_Array[DmaChNum].XferSize = i2c->TxXferSize;
    DMAC_Channel_Array[DmaChNum].PeriMode = DMAC_Peri_I2C;
    /*enable dma transfer interrupt*/
    DMAC_ClrIntFlagMsk(DMA, DmaChNum, DMAC_FLAG_INDEX_TFR);
    
    DMAC_SetChannelConfig(DMA, DmaChNum, &DMAC_Channel_Array[DmaChNum].ConfigTmp);
    /*start dma tx channel*/

    DMAC_StartChannel(DMA, DmaChNum, i2c->pTxBuffPtr, (void *)&(i2c->I2Cx->DATACMD), i2c->TxXferSize);
}

static void HAL_I2C_Master_SendData_DMA_U16_STC(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint16_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    HAL_I2C_SetAddress(i2c, DevAddress);
    i2c->TxXferSize = Size;
    i2c->TxXferCount = 0;

    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_0);
    I2C_DMATransferDataLevel(i2c->I2Cx, I2C_TX_TL_0);

    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_3);
    I2C_DMAReceiveDataLevel(i2c->I2Cx, I2C_RX_TL_3);

    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    uint32_t DmaChNum = 0xFF;
    DmaChNum = DMAC_AcquireChannel(DMA);

    dma_mem2i2c_config.PeripheralDst = i2c->DMA_DST;
    DMAC_Channel_Array[DmaChNum].ConfigTmp = dma_mem2i2c_u16_config;
    DMAC_Channel_Array[DmaChNum].CallbackI2c = Callback;
    DMAC_Channel_Array[DmaChNum].XferFlag = false;
    DMAC_Channel_Array[DmaChNum].pBuffPtr = (uint32_t *)Buf;
    DMAC_Channel_Array[DmaChNum].XferSize = i2c->TxXferSize;
    DMAC_Channel_Array[DmaChNum].PeriMode = DMAC_Peri_I2C;
    /*enable dma transfer interrupt*/
    DMAC_ClrIntFlagMsk(DMA, DmaChNum, DMAC_FLAG_INDEX_TFR);
    
    DMAC_SetChannelConfig(DMA, DmaChNum, &DMAC_Channel_Array[DmaChNum].ConfigTmp);
    /*start dma tx channel*/

    DMAC_StartChannel(DMA, DmaChNum, Buf, (void *)&(i2c->I2Cx->DATACMD), i2c->TxXferSize);

}

static uint16_t cmd_buf[256]={0};

void HAL_I2C_Master_ReceiveData_DMA(I2C_HandleTypeDef *i2c, uint16_t DevAddress, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    uint16_t i;
	for(i=0;i<Size-1;i++){
        cmd_buf[i] = (I2C_CMD_RD << 8);
    }
    cmd_buf[i] = ((I2C_CMD_RD|I2C_CMD_STOP) << 8);


    HAL_I2C_Master_SendData_DMA_U16_STC(i2c,DevAddress, cmd_buf, Size, NULL);

    i2c->pRxBuffPtr = Buf;
    i2c->RxXferSize = Size;
    i2c->RxXferCount = 0;

    uint32_t DmaChNum = 0xFF;
    /*get free dma channel;*/
    DmaChNum = DMAC_AcquireChannel(DMA);
    /*enable dma transfer interrupt*/
    DMAC_ClrIntFlagMsk(DMA, DmaChNum, DMAC_FLAG_INDEX_TFR);

    dma_i2c2mem_config.PeripheralSrc = i2c->DMA_SRC;
    DMAC_Channel_Array[DmaChNum].ConfigTmp = dma_i2c2mem_config;
    DMAC_Channel_Array[DmaChNum].CallbackI2c = Callback;
    DMAC_Channel_Array[DmaChNum].XferFlag = false;
    DMAC_Channel_Array[DmaChNum].pBuffPtr = (uint32_t*)i2c->pRxBuffPtr;
    DMAC_Channel_Array[DmaChNum].XferSize = i2c->RxXferSize;
    DMAC_Channel_Array[DmaChNum].PeriMode = DMAC_Peri_I2C;
    DMAC_SetChannelConfig(DMA, DmaChNum, &DMAC_Channel_Array[DmaChNum].ConfigTmp);
    /*start dma tx channel*/
    DMAC_StartChannel(DMA, DmaChNum, (void *)&(i2c->I2Cx->DATACMD), i2c->pRxBuffPtr, i2c->RxXferSize);
    I2C_DMACmd(i2c->I2Cx, ENABLE);
}

void HAL_I2C_Slave_SendData(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, uint32_t Timeout)
{
    while (1)
    {
        if (I2C_GetRawITStatus(i2c->I2Cx, I2C_IT_RD_REQ))
        {
            while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF))
                ;
            if (I2C_CON_MASTER_MODE & i2c->I2Cx->CON)
                i2c->I2Cx->DATACMD = ((I2C_CMD_STOP << 8) | (*Buf & 0xFF));
            else
                i2c->I2Cx->DATACMD = ((I2C_CMD_WR << 8) | (*Buf & 0xFF));
            Buf++;
            I2C_ClearITPendingBit(i2c->I2Cx, I2C_IT_RD_REQ);
        }
        if (I2C_GetRawITStatus(i2c->I2Cx, I2C_IT_STOP_DET))
        {
            I2C_ClearITPendingBit(i2c->I2Cx, I2C_IT_STOP_DET);
            break;
        }
    }
}

void HAL_I2C_Slave_ReceiveData(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, uint32_t Timeout)
{
    while (Size)
    {
        if (I2C_CON_MASTER_MODE & i2c->I2Cx->CON)
        {
            while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_TFNF))
            {
            }
            if (Size > 1)
            {
                I2C_SendCmd(i2c->I2Cx, I2C_CMD_RD);
            }
            else
            {
                I2C_SendCmd(i2c->I2Cx, I2C_CMD_RD | I2C_CMD_STOP);
            }
        }
        while (!I2C_GetFlagStatus(i2c->I2Cx, I2C_FLAG_RFNE))
        {
        }
        *Buf = (uint8_t)i2c->I2Cx->DATACMD;
        Buf++;
        Size--;
    }
}

void HAL_I2C_Slave_SendData_INT(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    // i2c->Xfer_Dir = I2C_Tx_Dir;
    i2c->pTxBuffPtr = Buf;
    i2c->TxXferSize = Size;
    i2c->TxXferCount = 0;
    i2c->TxIntCallback = Callback;
    I2C_Cmd(i2c->I2Cx, DISABLE);
    I2C_DisableAllIT(i2c->I2Cx);
    I2C_ITConfig(i2c->I2Cx, I2C_IT_START_DET | I2C_IT_RX_FULL | I2C_IT_RD_REQ | I2C_IT_STOP_DET, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);

//    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_0);
    NVIC_EnableIRQ(i2c->IRQn);
}

void HAL_I2C_Slave_ReceiveData_INT(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    i2c->pRxBuffPtr = Buf;
    i2c->RxXferSize = Size;
    i2c->RxXferCount = 0;
    i2c->RxIntCallback = Callback;

    I2C_Cmd(i2c->I2Cx, DISABLE);
    I2C_DisableAllIT(i2c->I2Cx);
    I2C_ITConfig(i2c->I2Cx, I2C_IT_START_DET | I2C_IT_RX_FULL | I2C_IT_RD_REQ | I2C_IT_STOP_DET, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->I2Cx, ENABLE);

    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_8);
    NVIC_EnableIRQ(i2c->IRQn);
}

void HAL_I2C_Slave_SendData_DMA(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    i2c->pTxBuffPtr = Buf;
    i2c->TxXferSize = Size;
    i2c->TxXferCount = 0;

    I2C_SetTxTirggerLevel(i2c->I2Cx, I2C_TX_TL_3);
    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_3);
    I2C_DMATransferDataLevel(i2c->I2Cx, I2C_TX_TL_3);
    I2C_DMAReceiveDataLevel(i2c->I2Cx, I2C_RX_TL_3);

    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    uint32_t DmaChNum = 0xFF;
    /*get free dma channel;*/
    DmaChNum = DMAC_AcquireChannel(DMA);
    /*enable dma transfer interrupt*/
    DMAC_ClrIntFlagMsk(DMA, DmaChNum, DMAC_FLAG_INDEX_TFR);

    dma_mem2i2c_config.PeripheralDst = i2c->DMA_DST;
    DMAC_Channel_Array[DmaChNum].ConfigTmp = dma_mem2i2c_config;
    DMAC_Channel_Array[DmaChNum].CallbackI2c = Callback;
    DMAC_Channel_Array[DmaChNum].XferFlag = false;
    DMAC_Channel_Array[DmaChNum].pBuffPtr = (uint32_t *)i2c->pTxBuffPtr;
    DMAC_Channel_Array[DmaChNum].XferSize = i2c->TxXferSize;
    DMAC_Channel_Array[DmaChNum].PeriMode = DMAC_Peri_I2C;
    DMAC_SetChannelConfig(DMA, DmaChNum, &DMAC_Channel_Array[DmaChNum].ConfigTmp);
    /*start dma tx channel*/
    DMAC_StartChannel(DMA, DmaChNum, i2c->pTxBuffPtr, (void *)&(i2c->I2Cx->DATACMD), i2c->TxXferSize/4);
    while(!I2C_GetRawITStatus(i2c->I2Cx, I2C_IT_RD_REQ)){}
    I2C_DMACmd(i2c->I2Cx, ENABLE);
    I2C_ClearITPendingBit(i2c->I2Cx, I2C_IT_RD_REQ);
}

void HAL_I2C_Slave_ReceiveData_DMA(I2C_HandleTypeDef *i2c, uint8_t *Buf, uint32_t Size, I2C_CallbackFunc Callback)
{
    i2c->pRxBuffPtr = Buf;
    i2c->RxXferSize = Size;
    i2c->RxXferCount = 0;

    I2C_SetRxTirggerLevel(i2c->I2Cx, I2C_RX_TL_3);
    I2C_DMAReceiveDataLevel(i2c->I2Cx, I2C_RX_TL_3);

    I2C_DMACmd(i2c->I2Cx, ENABLE);
    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    uint32_t DmaChNum = 0xFF;
    DmaChNum = DMAC_AcquireChannel(DMA);
    DMAC_ClrIntFlagMsk(DMA, DmaChNum, DMAC_FLAG_INDEX_TFR);

    dma_i2c2mem_config.PeripheralSrc = i2c->DMA_SRC;
    DMAC_Channel_Array[DmaChNum].ConfigTmp = dma_i2c2mem_config;
    DMAC_Channel_Array[DmaChNum].CallbackI2c = Callback;
    DMAC_Channel_Array[DmaChNum].XferFlag = false;
    DMAC_Channel_Array[DmaChNum].pBuffPtr = (uint32_t *)i2c->pRxBuffPtr;
    DMAC_Channel_Array[DmaChNum].XferSize = i2c->RxXferSize;
    DMAC_Channel_Array[DmaChNum].PeriMode = DMAC_Peri_I2C;
    DMAC_SetChannelConfig(DMA, DmaChNum, &DMAC_Channel_Array[DmaChNum].ConfigTmp);
    /*start dma tx channel*/
    DMAC_StartChannel(DMA, DmaChNum, (void *)&(i2c->I2Cx->DATACMD), i2c->pRxBuffPtr, i2c->RxXferSize);

}

void I2C0_IRQHandler()
{
    if (I2C_GetITStatus(I2C0, I2C_IT_TX_EMPTY))
    {
        while (I2C_GetFlagStatus(I2C0, I2C_FLAG_TFNF) == 0);
        if (I2C_INST_PTR->I2C_InitObj.Role == I2C_ROLE_MASTER)
        {
                if ((I2C_INST_PTR->TxXferCount + 1) == I2C_INST_PTR->TxXferSize) {
                  I2C_INST_PTR->I2Cx->DATACMD = (((I2C_CMD_STOP) << 8) | (I2C_INST_PTR->pTxBuffPtr[I2C_INST_PTR->TxXferCount] & 0xFF));
                } else {
                  I2C_INST_PTR->I2Cx->DATACMD = ((I2C_CMD_WR << 8) | (I2C_INST_PTR->pTxBuffPtr[I2C_INST_PTR->TxXferCount] & 0xFF));
                }
                
                I2C_INST_PTR->TxXferCount++;
                if (I2C_INST_PTR->TxXferCount >= I2C_INST_PTR->TxXferSize)
                {
                    I2C_ITConfig(I2C_INST_PTR->I2Cx, I2C_IT_TX_EMPTY, DISABLE);
                    if (I2C_INST_PTR->TxIntCallback != NULL)
                    {
                        I2C_INST_PTR->TxIntCallback(I2C_CB_FLAG_STOP, I2C_INST_PTR->pTxBuffPtr, I2C_INST_PTR->TxXferCount);
                    }
                }
        }
    }
    if (I2C_GetITStatus(I2C_INST_PTR->I2Cx, I2C_IT_START_DET))
    {
        I2C_ClearITPendingBit(I2C_INST_PTR->I2Cx, I2C_IT_START_DET);
    }
    if (I2C_GetITStatus(I2C_INST_PTR->I2Cx, I2C_IT_TX_ABORT))
    {
        I2C_ClearITPendingBit(I2C_INST_PTR->I2Cx, I2C_IT_TX_ABORT);
    }
    if (I2C_GetITStatus(I2C_INST_PTR->I2Cx, I2C_IT_RX_FULL))
    {
        while (I2C_GetFlagStatus(I2C_INST_PTR->I2Cx, I2C_FLAG_RFNE))
        {
			I2C_INST_PTR->pRxBuffPtr[I2C_INST_PTR->RxXferCount++] = I2C_ReceiveData(I2C_INST_PTR->I2Cx);
            if (I2C_INST_PTR->I2C_InitObj.Role == I2C_ROLE_MASTER)
			{
				if(I2C_INST_PTR->RxXferCount < (I2C_INST_PTR->RxXferSize)){
                    if ((I2C_INST_PTR->RxXferCount + 1) == I2C_INST_PTR->RxXferSize)
                    {
                        I2C_SendCmd(I2C_INST_PTR->I2Cx, I2C_CMD_RD|I2C_CMD_STOP);
                    }else {
                        I2C_SendCmd(I2C_INST_PTR->I2Cx, I2C_CMD_RD);
                    }
                }
            }
            
            if (I2C_INST_PTR->RxXferCount == I2C_INST_PTR->RxXferSize)
            {
                if (I2C_INST_PTR->RxIntCallback != NULL)
                {
                    I2C_INST_PTR->RxIntCallback(I2C_CB_FLAG_STOP, I2C_INST_PTR->pRxBuffPtr, I2C_INST_PTR->RxXferCount);
                }
            }
        }
        I2C_ClearITPendingBit(I2C_INST_PTR->I2Cx, I2C_IT_RX_FULL);
    }

    if (I2C_GetITStatus(I2C_INST_PTR->I2Cx, I2C_IT_STOP_DET))
    {

        while (I2C_GetFlagStatus(I2C_INST_PTR->I2Cx, I2C_FLAG_RFNE))
        {
        }


        I2C_ClearITPendingBit(I2C_INST_PTR->I2Cx, I2C_IT_STOP_DET);
    }

    if (I2C_GetITStatus(I2C_INST_PTR->I2Cx, I2C_IT_RD_REQ))
    {
        I2C_ClearITPendingBit(I2C0, I2C_IT_RD_REQ);
        while (I2C_GetFlagStatus(I2C_INST_PTR->I2Cx, I2C_FLAG_TFNF) == 0);// wait tx fifo no full
        if (I2C_INST_PTR->I2C_InitObj.Role == I2C_ROLE_SLAVE)
        {
            I2C_SendData(I2C0, I2C_INST_PTR->pTxBuffPtr[I2C_INST_PTR->TxXferCount]);
            
            I2C_INST_PTR->TxXferCount++;
            if (I2C_INST_PTR->TxXferCount >= I2C_INST_PTR->TxXferSize)
            {
                if (I2C_INST_PTR->TxIntCallback != NULL)
                {
                    I2C_INST_PTR->TxIntCallback(I2C_CB_FLAG_STOP, I2C_INST_PTR->pTxBuffPtr, I2C_INST_PTR->TxXferCount);
                }
            }
        }
    
        
    }
}

