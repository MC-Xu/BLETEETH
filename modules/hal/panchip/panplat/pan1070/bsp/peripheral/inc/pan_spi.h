/**************************************************************************
 * @file     pan_spi.h
 * @version  V1.10
 * $Revision: 2 $
 * $Date:    2023/11/08  $  
 * @brief    PAN series SPI driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __PAN_SPI_H__
#define __PAN_SPI_H__

/**
 * @brief Spi Interface
 * @defgroup spi_interface Spi Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**@defgroup SPI_IRQ_FLAG Spi interrupt event type
 * @brief       Spi interrupt event type constants definitions
 * @{ */ 
typedef enum SPI_Irq
{
    SPI_IRQ_TX_HALF_EMPTY   = 0x08,     //Transmit FIFO Half Empty Interrupt
    SPI_IRQ_RX_HALF_FULL    = 0x04,     //Receive FIFO Half Full Interrupt
    SPI_IRQ_RX_TIMEOUT      = 0x02,     //Receive FIFO Not Empty and No Read prior to Timeout Interrupt
    SPI_IRQ_RX_OVERRUN      = 0x01,     //Receive FIFO Overflow Interrupt
    SPI_IRQ_ALL             = 0x0f      //All interrupts
} SPI_IrqDef;
/**@} */

/**@defgroup SPI_WORK_MODE_FLAG Spi operating mode
 * @brief       Spi operating mode constants definitions
 * @{ */ 
typedef enum SPI_Role
{
    SPI_RoleMaster  = 0x0,
    SPI_RoleSlave   = 0x1
} SPI_RoleDef;  
/**@} */

#if 0
/*Transfer Mode. Selects the mode of transfer for serial communication. This field
does not affect the transfer duplicity. Only indicates whether the receive or transmit
data are valid.*/
typedef enum SPI_TransferMode
{
    Spi_dir_full_duplex = 0x0,
    Spi_dir_tx_only     = 0x1,
    Spi_dir_rx_only     = 0x2,
    Spi_dir_read_eeprom = 0x3
} SPI_TransferModeDef;
#endif

/**@defgroup SPI_DATA_SIZE_FLAG Spi data frame size
 * @brief       Spi data frame size constants definitions
 * @{ */ 
typedef enum SPI_DataFrameSize
{
    SPI_DataFrame_4b    = 0x03,
    SPI_DataFrame_5b    = 0x04,
    SPI_DataFrame_6b    = 0x05,
    SPI_DataFrame_7b    = 0x06,
    SPI_DataFrame_8b    = 0x07,
    SPI_DataFrame_9b    = 0x08,
    SPI_DataFrame_10b   = 0x09,
    SPI_DataFrame_11b   = 0x0a,
    SPI_DataFrame_12b   = 0x0b,
    SPI_DataFrame_13b   = 0x0c,
    SPI_DataFrame_14b   = 0x0d,
    SPI_DataFrame_15b   = 0x0e,
    SPI_DataFrame_16b   = 0x0f
} SPI_DataFrameSizeDef;
/**@} */

/**@defgroup SPI_BAUD_DIV_FLAG Spi baudrate divider
 * @brief       Spi baudrate divider constants definitions
 * @{ */
typedef enum SPI_BaudRateDiv
{
    SPI_BaudRateDiv_2   = 0x02,
    SPI_BaudRateDiv_4   = 0x04,
    SPI_BaudRateDiv_6   = 0x06,
    SPI_BaudRateDiv_8   = 0x08,
    SPI_BaudRateDiv_16  = 0x10,
    SPI_BaudRateDiv_32  = 0x20,
    SPI_BaudRateDiv_48  = 0x30,
    SPI_BaudRateDiv_64  = 0x40,
    SPI_BaudRateDiv_96  = 0x60,
    SPI_BaudRateDiv_128 = 0x80,
    SPI_BaudRateDiv_160 = 0xA0,
    SPI_BaudRateDiv_192 = 0xC0,
    SPI_BaudRateDiv_224 = 0xE0,
    SPI_BaudRateDiv_240 = 0xF0,
    SPI_BaudRateDiv_250 = 0xFA,
} SPI_BaudRateDivDef;
/**@} */

#define SPI_BAUD_RATE_DIV(x)	((x)?(x):(256))

/**@defgroup SPI_POLARITY_FLAG Spi clock polarity
 * @brief       Spi clock polarity constants definitions,Serial Clock Polarity, Used to 
 *							select the polarity of the inactive serial clock, which is held inactive when
 *							the DW_apb_ssi master is not actively transferring data on the serial bus
 * @{ */
typedef enum SPI_ClockPol
{
    SPI_ClockPolarityLow    = 0x0,
    SPI_ClockPolarityHigh   = 0x1
} SPI_ClockPolDef;
/**@} */

/**@defgroup SPI_PHASE_FLAG Spi clock phase
 * @brief       Spi clock phase constants definitions
 * @{ */
typedef enum SPI_ClockPhase
{
    SPI_ClockPhaseFirstEdge     = 0x0,
    SPI_ClockPhaseSecondEdge    = 0x1
} SPI_ClockPhaseDef;
/**@} */

/**@defgroup SPI_FORMAT_FLAG Spi frame format
 * @brief       Spi frame format constants definitions
 * @{ */
typedef enum SPI_FrameFormat
{
    SPI_FormatMotorola     = 0x00,
    SPI_FormatTi           = 0x01,
    SPI_FormatMicrowire    = 0x02,
} SPI_FrameFormatDef;
/**@} */

/**
 * @brief Enumeration of SPI Wire Number.
 */
typedef enum SPI_WireNum
{
    SPI_WireNumThree  = 0x01, /*!< 3-wire SPI communication */
    SPI_WireNumFour   = 0x00  /*!< 4-wire SPI communication */
} SPI_WireNumDef;

/**
 * @brief Enumeration of SPI Command/Data Selection.
 */
typedef enum SPI_CmdDat
{
    SPI_CommandSelect  = 0x00, /*!< SPI command selection */
    SPI_DataSelect     = 0x01  /*!< SPI data selection */
} SPI_CmdDatDef;


/**
 * @brief Enumeration of SPI Write/Read Selection.
 */
typedef enum SPI_WriteReadSel
{
    SPI_3WireRead    = 0x00, /*!< 3-wire SPI read selection */
    SPI_3WireWrite   = 0x01  /*!< 3-wire SPI write selection */
} SPI_WriteReadSelDef;

/** 
 * @brief Structure with spi init feature.
 *
 * @param SPI_role 						Specifies the SPI operating mode.\ref SPI_WORK_MODE_FLAG
 * @param SPI_dataFrameSize		Specifies the SPI data size	\ref SPI_DATA_SIZE_FLAG
 * @param SPI_CPOL						Specifies the SPI data size \ref SPI_POLARITY_FLAG
 * @param SPI_CPHA						Specifies the clock active edge for the bit capture \ref SPI_PHASE_FLAG
 * @param SPI_baudRateDiv			Specifies the Baud Rate Divisor used to configure the transmit and receive
 *																 SCK clock \ref SPI_BAUD_DIV_FLAG
 * @param SPI_format					Specifies the SPI format \ref SPI_FORMAT_FLAG
 */
typedef struct
{
    SPI_RoleDef SPI_role;                       /*!< Specifies the SPI operating mode */
    SPI_DataFrameSizeDef SPI_dataFrameSize;     /*!< Specifies the SPI data size */
    SPI_ClockPolDef SPI_CPOL;                   /*!< Specifies the serial clock steady state */
    SPI_ClockPhaseDef SPI_CPHA;                 /*!< Specifies the clock active edge for the bit capture */
//    SPI_BaudRateDivDef SPI_baudRateDiv;         /*!< Specifies the Baud Rate Divisor used to configure the transmit and receive SCK clock. */
    uint16_t SPI_baudRateDiv;         /*!< Specifies the Baud Rate Divisor used to configure the transmit and receive SCK clock. */
    SPI_FrameFormatDef SPI_format;              /*!< Specifies the SPI format */
} SPI_InitTypeDef;


/**
  * @brief  Synchronous serial port enable
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_EnableSpi(SPI_T* SPIx)
{
    uint32_t tmpreg = 0;

    tmpreg = SPIx->CR1;
    tmpreg |= SPI_CR1_SSE_Msk;
    SPIx->CR1 = tmpreg;
}

/**
  * @brief  Synchronous serial port disable
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_DisableSpi(SPI_T* SPIx)
{
    uint32_t tmpreg = 0;

    tmpreg = SPIx->CR1;
    tmpreg &= ~SPI_CR1_SSE_Msk;
    SPIx->CR1 = tmpreg;
}

/**
  * @brief  get synchronous serial port status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsSpiEnabled(SPI_T* SPIx)
{
    uint32_t tmpreg = 0;

    tmpreg = SPIx->CR1;
    if(tmpreg & SPI_CR1_SSE_Msk)
        return true;
    else
        return false;
}

/**
  * @brief  DMA for the transmit FIFO is enabled
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_EnableDmaTx(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    tmpreg |= SPI_DMACR_TDMAE_Msk;
    SPIx->DMACR = tmpreg;
}

/**
  * @brief  DMA for the transmit FIFO is disabled
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_DisableDmaTx(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    tmpreg &= ~SPI_DMACR_TDMAE_Msk;
    SPIx->DMACR = tmpreg;
}

/**
  * @brief  get DMA for the transmit FIFO status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsDmaTxEnabled(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    if(tmpreg & SPI_DMACR_TDMAE_Msk)
        return true;
    else
        return false;
}

/**
  * @brief  DMA for the receive FIFO is enabled
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_EnableDmaRx(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    tmpreg |= SPI_DMACR_RDMAE_Msk;
    SPIx->DMACR = tmpreg;
}

/**
  * @brief  DMA for the receive FIFO is disabled
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_DisableDmaRx(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    tmpreg &= ~SPI_DMACR_RDMAE_Msk;
    SPIx->DMACR = tmpreg;
}

/**
  * @brief  get DMA for the receive FIFO status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsDmaRxEnabled(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->DMACR;
    if(tmpreg & SPI_DMACR_RDMAE_Msk)
        return true;
    else
        return false;
}

/**
  * @brief  Send data,DR is the data register and is 16-bits wide
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE void SPI_SendData(SPI_T* SPIx, uint32_t Data)
{
    SPIx->DR = Data;
}

/**
  * @brief  Receive data,DR is the data register and is 16-bits wide
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval None
  */
__STATIC_INLINE uint32_t SPI_ReceiveData(SPI_T* SPIx)
{
    return SPIx->DR;
}

/**
  * @brief  Interruput mask Enable
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: where x can be 
  *          SPI_IRQ_TX_HALF_EMPTY 
  *          SPI_IRQ_RX_HALF_FULL 
  *          SPI_IRQ_RX_TIMEOUT
  *          SPI_IRQ_RX_OVERRUN
  *          SPI_IRQ_ALL 
  * @retval None
  */
__STATIC_INLINE void SPI_EnableIrq(SPI_T* SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->IMSC;
    tmpreg |= irq;
    SPIx->IMSC = tmpreg;
}

/**
  * @brief  Interruput mask Disable
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: where x can be 
  *          \ref SPI_IRQ_TX_HALF_EMPTY 
  *          \ref SPI_IRQ_RX_HALF_FULL 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  *          \ref SPI_IRQ_ALL 
  * @return None
  */
__STATIC_INLINE void SPI_DisableIrq(SPI_T* SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->IMSC;
    tmpreg &= ~irq;
    SPIx->IMSC = tmpreg;
}

/**
  * @brief  Get Interruput mask status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: where x can be 
  *          \ref SPI_IRQ_TX_HALF_EMPTY 
  *          \ref SPI_IRQ_RX_HALF_FULL 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  *          \ref SPI_IRQ_ALL 
  * @return None
  */
__STATIC_INLINE bool SPI_IsIrqEnabled(SPI_T* SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg;

    tmpreg = SPIx->IMSC;
    if(tmpreg & irq )
        return true;
    else
        return false;
}

/**
  * @brief  Get Masked Interruput status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral. 
  * @retval SPI_IrqDef: where return value can be 
  *          SPI_IRQ_TX_HALF_EMPTY 
  *          SPI_IRQ_RX_HALF_FULL 
  *          SPI_IRQ_RX_TIMEOUT
  *          SPI_IRQ_RX_OVERRUN
  *          SPI_IRQ_ALL 
  */
__STATIC_INLINE SPI_IrqDef SPI_GetMskedActiveIrq(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->MIS;
    tmpreg &= SPI_IRQ_ALL;

    return (SPI_IrqDef)tmpreg;
}

/**
  * @brief  Get Masked Interruput status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: where x can be 
  *          \ref SPI_IRQ_TX_HALF_EMPTY 
  *          \ref SPI_IRQ_RX_HALF_FULL 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  *          \ref SPI_IRQ_ALL 
  * @return true/false
  */
__STATIC_INLINE bool SPI_IsIrqActive(SPI_T * SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->MIS;
    if(tmpreg & irq )
        return true;
    else
        return false;
}

/**
  * @brief  Get raw Interruput status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: where x can be 
  *          \ref SPI_IRQ_TX_HALF_EMPTY 
  *          \ref SPI_IRQ_RX_HALF_FULL 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  *          \ref SPI_IRQ_ALL 
  * @return true/false
  */
__STATIC_INLINE bool SPI_IsRawIrqActive(SPI_T * SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->RIS;
    if(tmpreg & irq )
        return true;
    else
        return false;
}

/**
  * @brief  clear Interruput status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  irq: can be 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  * @return None
  */
__STATIC_INLINE void SPI_ClearIrq(SPI_T * SPIx, SPI_IrqDef irq)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->ICR;
    tmpreg |= irq ;
    SPIx->ICR = tmpreg;
}
/**
  * @brief  Get Raw Interruput status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral. 
  * @return SPI_IrqDef: where return value can be 
  *          \ref SPI_IRQ_TX_HALF_EMPTY 
  *          \ref SPI_IRQ_RX_HALF_FULL 
  *          \ref SPI_IRQ_RX_TIMEOUT
  *          \ref SPI_IRQ_RX_OVERRUN
  *          \ref SPI_IRQ_ALL 
  */
__STATIC_INLINE SPI_IrqDef SPI_GetRawActiveIrq(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->RIS;
    tmpreg &= SPI_IRQ_ALL;

    return (SPI_IrqDef)tmpreg;
}

/**
  * @brief  Get Tx FIFO empty status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsTxFifoEmpty(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;
    tmpreg = SPIx->SR;

    if(tmpreg & SPI_SR_TFE_Msk)
        return true;
    else
        return false;
}
/**
  * @brief  Get Tx FIFO full status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsTxFifoFull(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->SR;

    if(tmpreg & SPI_SR_TFNF_Msk)
        return false;
    else
        return true;
}
/**
  * @brief  Get Tx FIFO empty status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsRxFifoEmpty(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->SR;

    if(tmpreg & SPI_SR_RFNE_Msk)
        return false;
    else
        return true;

}
/**
  * @brief  Get Tx FIFO full status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsRxFifoFull(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->SR;

    if(tmpreg & SPI_SR_RFF_Msk)
        return true;
    else
        return false;

}
/**
  * @brief  Get Synchronous serial port busy status
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval true/false
  */
__STATIC_INLINE bool SPI_IsBusy(SPI_T* SPIx)
{
    uint32_t tmpreg = 0x00;

    tmpreg = SPIx->SR;
    if(tmpreg & SPI_SR_BUSY_Msk)
        return true;
    else
        return false;
}

/**
  * @brief  Tx Lsb Send function control
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval none
  */
__STATIC_INLINE void SPI_TxLsbEnable(SPI_T* SPIx,bool enable)
{
	(enable) ? (SPIx->CR1 |= SPI_CR1_TX_LSB_EN_Msk) : (SPIx->CR1 &= ~SPI_CR1_TX_LSB_EN_Msk);
}

/**
  * @brief  Rx Lsb Send function control
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @retval none
  */
__STATIC_INLINE void SPI_RxLsbEnable(SPI_T* SPIx,bool enable)
{
	(enable) ? (SPIx->CR1 |= SPI_CR1_RX_LSB_EN_Msk) : (SPIx->CR1 &= ~SPI_CR1_RX_LSB_EN_Msk);
}

/**
  * @brief  set spi wire number,which controlled the spi work mode
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  num: where num can be 
  *             SPI_WireNumThree
  *             SPI_WireNumFour
  * @retval none
  */
__STATIC_INLINE void SPI_SetWireNum(SPI_T* SPIx,SPI_WireNumDef num)
{
	(num == SPI_WireNumThree) ? (SPIx->CR1 |= SPI_CR1_SPI_MODE_SEL_Msk) : (SPIx->CR1 &= ~SPI_CR1_SPI_MODE_SEL_Msk);
}

/**
  * @brief  Distinguish whether there is a DC output line in 3-wire SPI mode, 
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  NewState: new state of spi dc output line state
  *                 enable-> dc output line
  *                 disable-> no dc output line
  * @retval none
  */
__STATIC_INLINE void SPI_DataCmdLineEnable(SPI_T* SPIx, FunctionalState NewState)
{
	(NewState == ENABLE) ? (SPIx->CR1 |= SPI_CR1_CMD_DAT_EN_Msk) : (SPIx->CR1 &= ~SPI_CR1_CMD_DAT_EN_Msk);
}


/**
  * @brief  Write read control in 3-wire SPI mode, 
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  cmdDat: where cmdDat can be 
  *             SPI_DataSelect
  *             SPI_CommandSelect
  * @retval none
  */
__STATIC_INLINE void SPI_DataCmdSelCmdDat(SPI_T* SPIx, SPI_CmdDatDef cmdDat)
{
	(cmdDat == SPI_DataSelect) ? (SPIx->CR1 |= SPI_CR1_CMD_DAT_CTRL_Msk) : (SPIx->CR1 &= ~SPI_CR1_CMD_DAT_CTRL_Msk);
}

/**
  * @brief  Write read control in 3-wire SPI mode 
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  wr: where wr can be 
  *             SPI_3WireRead
  *             SPI_3WireWrite
  * @retval none
  */
__STATIC_INLINE void SPI_WriteReadCtrl(SPI_T* SPIx, SPI_WriteReadSelDef wr)
{
	(wr == SPI_3WireWrite) ? (SPIx->CR1 |= SPI_CR1_WR_CTRL_Msk) : (SPIx->CR1 &= ~SPI_CR1_WR_CTRL_Msk);
}


/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *         parameters in the SPI_InitStruct .
  * @param  SPIx: where x can be 1, 2 to select the
  *         SPI peripheral.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that contains
  *         the configuration information for the specified SPI peripheral.
  * @return None
  */
void SPI_Init(SPI_T* SPIx, SPI_InitTypeDef* SPI_InitStruct);

/**@} */

#ifdef __cplusplus
}
#endif

#endif //__PAN_SPI_H__

/*** (C) COPYRIGHT 2023 Panchip Technology Corp. ***/
