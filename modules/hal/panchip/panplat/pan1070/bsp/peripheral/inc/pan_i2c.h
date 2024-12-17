/**************************************************************************//**
 * @file     pan_i2c.h
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 2023/11/08  $
 * @brief    Panchip series I2C driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __PAN_I2C_H__
#define __PAN_I2C_H__

/**
 * @brief I2c Interface
 * @defgroup i2c_interface I2c Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief Structure with i2c init feature.
 *
 * @param I2C_ClockSpeed 	Specifies the clock frequency.
 * @param I2C_Mode				Specifies the I2C mode.This parameter can be a value of
 *													  \ref PAN_I2C_MODE_MASTER,\ref PAN_I2C_MODE_SLAVE
 * @param I2C_DutyCycle		Specifies the I2C fast mode duty cycle.This parameter
 *													 can be a value of \ref I2C_DutyCycle_16_9, \ref I2C_DutyCycle_2
 * @param I2C_OwnAddress1	Specifies the first device own address,
 *													 This parameter can be a 7-bit or 10-bit address.
 * @param I2C_AcknowledgedAddress		Specifies if 7-bit or 10-bit address is acknowledged.
 *													 This parameter can be a value of 
 *														\ref I2C_AcknowledgedAddress_7bit, \ref I2C_AcknowledgedAddress_10bit
 */
typedef struct
{
  uint32_t I2C_ClockSpeed;          /*!< Specifies the clock frequency.
                                         This parameter must be set to a value lower than 400kHz */

  uint16_t I2C_Mode;                /*!< Specifies the I2C mode.
                                         This parameter can be a value of @ref I2C_mode */

  uint16_t I2C_DutyCycle;           /*!< Specifies the I2C fast mode duty cycle.
                                         This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */

  uint16_t I2C_OwnAddress1;         /*!< Specifies the first device own address.
                                         This parameter can be a 7-bit or 10-bit address. */

  uint16_t I2C_AcknowledgedAddress; /*!< Specifies if 7-bit or 10-bit address is acknowledged.
                                         This parameter can be a value of @ref I2C_acknowledged_address */
}I2C_InitTypeDef;

/* Exported constants --------------------------------------------------------*/


#define I2C_DYNAMIC_TAR_UPDATE 0


/** @defgroup I2C_mode
 * @brief    I2C operational modes
 * @{
 */
#define I2C_MODE_MASTER  ((uint16_t)0x0021) /*!< I2C Master Mode */
#define I2C_MODE_SLAVE   ((uint16_t)0x0000) /*!< I2C Slave Mode */

/**
 * @brief    Check if the I2C mode is valid.
 * @param    MODE: I2C mode to check.
 * @retval   true if the mode is valid, false otherwise.
 */
#define IS_I2C_MODE(MODE) (((MODE) == I2C_MODE_MASTER) || ((MODE) == I2C_MODE_SLAVE))

/** @} */

/** @defgroup I2C_speed_mode
 * @brief    I2C communication speed modes
 * @{
 */
#define I2C_SPEED_STANDARD_MODE (uint16_t)(0x0002)  /*!< Standard Mode (up to 100 Kbps) */
#define I2C_SPEED_FAST_MODE     (uint16_t)(0x0004)  /*!< Fast Mode (up to 400 Kbps) */
#define I2C_SPEED_HIGH_MODE     (uint16_t)(0x0006)  /*!< High-Speed Mode (up to 3.4 Mbps) */
/** @} */


/** @defgroup I2C_duty_cycle
 * @brief    I2C fast mode duty cycle options
 * @{
 */
#define I2C_DutyCycle_16_9                      ((uint16_t)0x4000) /*!< I2C fast mode Tlow/Thigh = 16/9 */
#define I2C_DutyCycle_2                         ((uint16_t)0xBFFF) /*!< I2C fast mode Tlow/Thigh = 2 */
#define I2C_DutyCycle_1_1                       ((uint16_t)0x0800)
#define I2C_DutyCycle_1_2                       ((uint16_t)0x0400)



/* @brief    I2C fast mode duty cycle validation */
#define IS_I2C_DUTY_CYCLE(CYCLE)  (((CYCLE) == I2C_DutyCycle_16_9) || \
                                   ((CYCLE) == I2C_DutyCycle_2))
/** @} */

/** @defgroup I2C_transfer_direction
 * @{
 */
#define  I2C_Direction_Transmitter              ((uint16_t)0x0000)
#define  I2C_Direction_Receiver                 ((uint16_t)0x0100)
#define IS_I2C_DIRECTION(DIRECTION)             (((DIRECTION) == I2C_Direction_Transmitter) || \
                                                 ((DIRECTION) == I2C_Direction_Receiver))
/** @} */

/** @defgroup I2C_acknowledged_address
 * @{
 */
#define I2C_AcknowledgedAddress_7bit            ((uint16_t)0x4000)
#define I2C_AcknowledgedAddress_10bit           ((uint16_t)0xC000)
#define IS_I2C_ACKNOWLEDGE_ADDRESS(ADDRESS)     (((ADDRESS) == I2C_AcknowledgedAddress_7bit) || \
                                                 ((ADDRESS) == I2C_AcknowledgedAddress_10bit))
/** @} */

/** @defgroup I2C_Cmd  
 * @{
 */
#define I2C_CMD_WR                              ((uint8_t)0x00)
#define I2C_CMD_RD                              ((uint8_t)0x01)
#define I2C_CMD_STOP                            ((uint8_t)0x02)
#define I2C_CMD_RESTART                         ((uint8_t)0x04)
/** @} */

/** @defgroup I2C_Receive_FIFO_Threshold_Value
 * @{
 */
#define I2C_RX_TL_0                             ((uint8_t)0x00)
#define I2C_RX_TL_1                             ((uint8_t)0x01)
#define I2C_RX_TL_2                             ((uint8_t)0x02)
#define I2C_RX_TL_3                             ((uint8_t)0x03)
#define I2C_RX_TL_4                             ((uint8_t)0x04)
#define I2C_RX_TL_5                             ((uint8_t)0x05)
#define I2C_RX_TL_6                             ((uint8_t)0x06)
#define I2C_RX_TL_7                             ((uint8_t)0x07)
#define I2C_RX_TL_8                             ((uint8_t)0x08)
/** @} */

/** @defgroup I2C_Transmit_FIFO_Threshold_Value
 * @{
 */
#define I2C_TX_TL_0                             ((uint8_t)0x00)
#define I2C_TX_TL_1                             ((uint8_t)0x01)
#define I2C_TX_TL_2                             ((uint8_t)0x02)
#define I2C_TX_TL_3                             ((uint8_t)0x03)
#define I2C_TX_TL_4                             ((uint8_t)0x04)
#define I2C_TX_TL_5                             ((uint8_t)0x05)
#define I2C_TX_TL_6                             ((uint8_t)0x06)
#define I2C_TX_TL_7                             ((uint8_t)0x07)
#define I2C_TX_TL_8                             ((uint8_t)0x08)
/** @} */

/** @defgroup I2C_interrupts_definition
* @{
*/

#define I2C_IT_RX_UNDER                         ((uint16_t)0x0001)
#define I2C_IT_RX_OVER                          ((uint16_t)0x0002)
#define I2C_IT_RX_FULL                          ((uint16_t)0x0004)
#define I2C_IT_TX_OVER                          ((uint16_t)0x0008)
#define I2C_IT_TX_EMPTY                         ((uint16_t)0x0010)
#define I2C_IT_RD_REQ                           ((uint16_t)0x0020)
#define I2C_IT_TX_ABORT                         ((uint16_t)0x0040)
#define I2C_IT_RX_DONE                          ((uint16_t)0x0080)
#define I2C_IT_ACTIVITY                         ((uint16_t)0x0100)
#define I2C_IT_STOP_DET                         ((uint16_t)0x0200)
#define I2C_IT_START_DET                        ((uint16_t)0x0400)
#define I2C_IT_GEN_CALL                         ((uint16_t)0x0800)
#define I2C_IT_MST_ON_HOLD                      ((uint16_t)0x2000)
#define I2C_IT_ALL                              ((uint16_t)0x2FFF)

#define IS_I2C_CLEAR_IT(IT)                     ((((IT) & (uint16_t)0x0FFF) == 0x00) && ((IT) != (uint16_t)0x00))

#define IS_I2C_GET_IT(IT)                       (((IT) == I2C_IT_RX_UNDER) || ((IT) == I2C_IT_RX_OVER) || \
                                                 ((IT) == I2C_IT_RX_FULL)  || ((IT) == I2C_IT_TX_OVER) || \
                                                 ((IT) == I2C_IT_TX_EMPTY) || ((IT) == I2C_IT_RD_REQ) || \
                                                 ((IT) == I2C_IT_TX_ABORT) || ((IT) == I2C_IT_RX_DONE) || \
                                                 ((IT) == I2C_IT_ACTIVITY) || ((IT) == I2C_IT_STOP_DET) || \
                                                 ((IT) == I2C_IT_START_DET)|| ((IT) == I2C_IT_GEN_CALL))
/** @} */

/** @defgroup I2C_flags_definition
 * @{
 */
#define I2C_FLAG_SLV_ACTIVITY                   ((uint32_t)0x00000040)
#define I2C_FLAG_MST_ACTIVITY                   ((uint32_t)0x00000020)
#define I2C_FLAG_RFF                  			((uint32_t)0x00000010)
#define I2C_FLAG_RFNE                  			((uint32_t)0x00000008)
#define I2C_FLAG_TFE                    		((uint32_t)0x00000004)
#define I2C_FLAG_TFNF                   		((uint32_t)0x00000002)
#define I2C_FLAG_ACTIVITY                     	((uint32_t)0x00000001)
#define IS_I2C_CLEAR_FLAG(FLAG)                 ((((FLAG) & (uint16_t)0x007F) == 0x00) && ((FLAG) != (uint16_t)0x00))
#define IS_I2C_GET_FLAG(FLAG)                   (((FLAG) == I2C_FLAG_ACTIVITY) || ((FLAG) == I2C_FLAG_TFNF) || \
                                                 ((FLAG) == I2C_FLAG_TFE) || ((FLAG) == I2C_FLAG_RFNE) || \
                                                 ((FLAG) == I2C_FLAG_RFF) || ((FLAG) == I2C_FLAG_MST_ACTIVITY) || \
                                                 ((FLAG) == I2C_FLAG_SLV_ACTIVITY))
/** @} */

/** @defgroup I2C_Mask_Definitions
 * @brief    I2C mask definitions for various flags and interrupt enables.
 * @{
 */
#define FLAG_MASK       ((uint32_t)0x007F0FFF)  /*!< I2C FLAG mask */
#define ITEN_MASK       ((uint16_t)0x2FFF)     /*!< I2C Interrupt Enable mask */
#define IT_FLAG_MASK    ((uint16_t)0x2FFF)     /*!< I2C FLAG mask */
/** @} */

/**
  * @brief  Set I2C mode .
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Mode: role of the I2Cx peripheral.
  *          This parameter can be: I2C_MODE_MASTER or I2C_MODE_SLAVE.
  * @note   This function should be called before enabling
            the I2C Peripheral.
  * @retval None
  */
__STATIC_INLINE void I2C_SetMode(I2C_T* I2Cx, uint16_t Mode)
{
    (Mode == I2C_MODE_MASTER)?(I2Cx->CON |= (I2C_CON_IC_SLAVE_DISABLE | I2C_CON_MASTER_MODE)):(I2Cx->CON &= ~(I2C_CON_IC_SLAVE_DISABLE | I2C_CON_MASTER_MODE));
}
/**
  * @brief  Set I2C Tx fifo threshold value .
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  ThresholdValue:Transmit FIFO Threshold Level
  *          This parameter can be @I2C_Transmit_FIFO_Threshold_Value.
  * @note   Controls the level of entries (or below) that trigger the TX_EMPTY interrupt (bit 4 in
  *         IC_RAW_INTR_STAT register). The valid range is 0-8, with the additional
  *         restriction that it may not be set to value larger than the depth of the buffer. If an
  *         attempt is made to do that, the actual value set will be the maximum depth of the
  *         buffer.A value of 0 sets the threshold for 0 entries, and a value of 8 sets the threshold for
  *         8 entries.
  * @retval None
  */
__STATIC_INLINE void I2C_SetTxTirggerLevel(I2C_T* I2Cx, uint8_t ThresholdValue)
{
    I2Cx->TX_TL = (I2Cx->TX_TL & ~I2C_TX_TL) | (ThresholdValue & 0xF);
}

/**
  * @brief  Set I2C Rx fifo threshold value .
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  ThresholdValue:Receive FIFO Threshold Level
  *          This parameter can be @I2C_Receive_FIFO_Threshold_Value.
  * @note   Controls the level of entries (or below) that trigger the RX_FULL interrupt (bit 2 in
  *         IC_RAW_INTR_STAT register). The valid range is 0-8, with the additional
  *         restriction that it may not be set to value larger than the depth of the buffer. If an
  *         attempt is made to do that, the actual value set will be the maximum depth of the
  *         buffer.A value of 0 sets the threshold for 0 entries, and a value of 8 sets the threshold for
  *         8 entries.
  * @retval None
  */
__STATIC_INLINE void I2C_SetRxTirggerLevel(I2C_T* I2Cx, uint8_t ThresholdValue)
{
    I2Cx->RX_TL = (I2Cx->RX_TL & ~I2C_RX_TL) | (ThresholdValue & 0xF);
}
/**
  * @brief  Enables or disables the specified I2C peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2Cx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void I2C_Cmd(I2C_T* I2Cx, FunctionalState NewState)
{
    (NewState != DISABLE)?(I2Cx->IC_ENABLE |= I2C_ENABLE_ENABLE):(I2Cx->IC_ENABLE &= (uint32_t)~((uint32_t)I2C_ENABLE_ENABLE));
}
/**
  * @brief  Enables or disables the specified I2C general call feature.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C General call.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void I2C_GeneralCallCmd(I2C_T* I2Cx, FunctionalState NewState)
{
    (NewState != DISABLE)?(I2Cx->TAR = (I2Cx->TAR | I2C_TAR_SPECIAL) & (~I2C_TAR_GC_OR_START)):(I2Cx->TAR = I2Cx->TAR);
}

/**
  * @brief  Sends a data byte through the I2Cx peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Data: Byte to be transmitted..
            Cmd : I2C_CMD_RD/I2C_CMD_WR, I2C_CMD_RESTART, or I2C_CMD_STOP
  * @retval None
  */
__STATIC_INLINE void I2C_SendDataCmd(I2C_T* I2Cx, uint8_t Data, uint8_t Cmd)
{
    I2Cx->DATACMD = ((Cmd << 8 ) | (Data & 0xFF));
}
/**
  * @brief  Sends a data byte through the I2Cx peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Cmd: I2C_CMD_RD/I2C_CMD_WR,I2C_CMD_RESTART, or I2C_CMD_STOP
  * @retval None
  */
__STATIC_INLINE void I2C_SendCmd(I2C_T* I2Cx, uint8_t Cmd)
{
    I2Cx->DATACMD |= (Cmd << 8);
}
/**
  * @brief  Returns the most recent received data by the I2Cx peripheral.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval The value of the received data.
  */
__STATIC_INLINE uint8_t I2C_ReceiveData(I2C_T* I2Cx)
{
    return (uint8_t)I2Cx->DATACMD;
}
/**
  * @brief  Aborting I2C Transfers.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval None
  */
__STATIC_INLINE void I2C_AbortTransfer(I2C_T* I2Cx)
{
    I2Cx->IC_ENABLE |= I2C_ENABLE_ABORT;
}
/**
  * @brief  Enables or disables the specified I2C DMA requests.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  NewState: new state of the I2C DMA transfer.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void I2C_DMACmd(I2C_T* I2Cx, FunctionalState NewState)
{
    (NewState != DISABLE)?(I2Cx->DMA_CR |= (I2C_DMA_CR_RDMAE | I2C_DMA_CR_TDMAE)):(I2Cx->DMA_CR &= (uint32_t)~((uint32_t)(I2C_DMA_CR_RDMAE | I2C_DMA_CR_TDMAE)));
}
/**
  * @brief  DMA Transmit Data Level Register.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Level: This parameter controls the level at which a DMA request is made by the transmit logic
  * @retval None
  */
__STATIC_INLINE void I2C_DMATransferDataLevel(I2C_T* I2Cx, uint8_t Level)
{
    I2Cx->DMA_TDLR |= (Level & 0x07);
}
/**
  * @brief  DMA Receive Data Level Register.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  Level: This parameter controls the level at which a DMA request is made by the receive logic.
  * @retval None
  */
__STATIC_INLINE void I2C_DMAReceiveDataLevel(I2C_T* I2Cx, uint8_t Level)
{
    I2Cx->DMA_RDLR |= (Level & 0x07);
}
/**
  * @brief  Checks whether the specified I2C interrupt has occurred or not.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt source to check.
  *          This parameter can be one of the following values:
  *          @arg I2C_IT_RX_UNDER
  *          @arg I2C_IT_RX_OVER
  *          @arg I2C_IT_RX_FULL
  *          @arg I2C_IT_TX_OVER
  *          @arg I2C_IT_TX_EMPTY
  *          @arg I2C_IT_RD_REQ
  *          @arg I2C_IT_TX_ABRT
  *          @arg I2C_IT_RX_DONE
  *          @arg I2C_IT_ACTIVITY
  *          @arg I2C_IT_STOP_DET
  *          @arg I2C_IT_START_DET
  *          @arg I2C_IT_GEN_CALL
  *          @arg I2C_IT_MST_ON_HOLD
  * @retval The new state of I2C_IT (SET or RESET).
  */
__STATIC_INLINE ITStatus I2C_GetITStatus(I2C_T* I2Cx, uint16_t I2C_IT)
{
    return ((I2Cx->INTR_STAT & (I2C_IT & IT_FLAG_MASK)) != (uint32_t)RESET)?(SET):(RESET);
}
/**
  * @brief  Checks whether the specified I2C interrupt has occurred or not.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the interrupt source to check.
  *          This parameter can be one of the following values:
  *             @arg I2C_IT_RX_UNDER
  *             @arg I2C_IT_RX_OVER
  *             @arg I2C_IT_RX_FULL
  *             @arg I2C_IT_TX_OVER
  *             @arg I2C_IT_TX_EMPTY
  *             @arg I2C_IT_RD_REQ
  *             @arg I2C_IT_TX_ABRT
  *             @arg I2C_IT_RX_DONE
  *             @arg I2C_IT_ACTIVITY
  *             @arg I2C_IT_STOP_DET
  *             @arg I2C_IT_START_DET
  *             @arg I2C_IT_GEN_CALL
  *             @arg I2C_IT_MST_ON_HOLD
  * @retval The new state of I2C_IT (SET or RESET).
  */
__STATIC_INLINE ITStatus I2C_GetRawITStatus(I2C_T* I2Cx, uint16_t I2C_IT)
{
    return ((I2Cx->RAW_INTR_STAT & (I2C_IT & IT_FLAG_MASK)) != (uint32_t)RESET)?(SET):(RESET);
}
/**
  * @brief  Enables or disables the specified I2C interrupts.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  I2C_IT: specifies the I2C interrupts sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *             @arg I2C_IT_RX_UNDER
  *             @arg I2C_IT_RX_OVER
  *             @arg I2C_IT_RX_FULL
  *             @arg I2C_IT_TX_OVER
  *             @arg I2C_IT_TX_EMPTY
  *             @arg I2C_IT_RD_REQ
  *             @arg I2C_IT_TX_ABRT
  *             @arg I2C_IT_RX_DONE
  *             @arg I2C_IT_ACTIVITY
  *             @arg I2C_IT_STOP_DET
  *             @arg I2C_IT_START_DET
  *             @arg I2C_IT_GEN_CALL
  * @param  NewState: new state of the specified I2C interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void I2C_ITConfig(I2C_T* I2Cx, uint16_t I2C_IT, FunctionalState NewState)
{
    (NewState != DISABLE)?(I2Cx->INTR_MASK |= I2C_IT):(I2Cx->INTR_MASK &= (uint16_t)~I2C_IT);
}

/**
  * @brief  Enables or disables all the specified I2C interrupts.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval None
  */
__STATIC_INLINE void I2C_DisableAllIT(I2C_T* I2Cx)
{
    I2Cx->INTR_MASK = 0x0;
}
/**
  * @brief  Clears all the I2Cx's interrupt pending bits.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval None
  */
__STATIC_INLINE void I2C_ClearAllITPendingBit(I2C_T* I2Cx)
{
    (void)(I2Cx->CLR_INTR);
}

/**
  * @brief  get the source of tx_abort.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @retval true or false
  */
__STATIC_INLINE bool I2C_AbortSrcCheck(I2C_T* I2Cx,uint32_t IC_MSK)
{
    return (I2Cx->TX_ABRT_SRC & IC_MSK)?(true):(false);
}


/**
  * @brief  set slave address.
  * @param  I2Cx: where x can be 1, 2 or 3 to select the I2C peripheral.
  * @param  addr: slave address.
  * @retval true or false
  */
__STATIC_INLINE void I2C_SetSlaveAddr(I2C_T* I2Cx,uint32_t addr)
{
    I2Cx->SAR = (I2Cx->SAR & ~I2C_TAR_TAR) | addr;
}


/**
  * @brief  Initializes the I2Cx peripheral according to the specified
  *         parameters in the I2C_InitStruct.
  *
  * @note   To use the I2C at 400 KHz (in fast mode), the PCLK1 frequency
  *         (I2C peripheral input clock) must be a multiple of 10 MHz.
  *
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  I2C_InitStruct: pointer to a I2C_InitTypeDef structure that contains
  *         the configuration information for the specified I2C peripheral.
  * @note   This function should be called before enabling
            the I2C Peripheral.
  * @retval false	init fail
  * @retval true	init succeed
  *
  *	@code:
	* I2C_InitTypeDef  I2C_InitStructure;
  * 
	* I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	* I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_OWN_ADDRESS7;
	* I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	* I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED_100K;
	* I2C_Cmd(I2Cx, DISABLE);
	* bool ret = I2C_Init(I2Cx, &I2C_InitStructure);
  *
  * @endcode
  */
bool I2C_Init(I2C_T* I2Cx, I2C_InitTypeDef* I2C_InitStruct);
/**
  * @brief  Configure the target address for any master transaction.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  Address: specifies the slave 7-bit address which will be transmitted
  * @return None.
  */
void I2C_Set7bitAddress(I2C_T* I2Cx, uint8_t Address);
/**
  * @brief  Configure the target address for any master transaction.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  Address: specifies the slave 10-bit address which will be transmitted
  * @return None.
  */
void I2C_Set10bitAddress(I2C_T* I2Cx, uint16_t Address);

/**
  * @brief  Checks whether the specified I2C flag is set or not.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  I2C_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *          @arg I2C_FLAG_SLV_ACTIVITY
  *          @arg I2C_FLAG_MST_ACTIVITY
  *          @arg I2C_FLAG_RFF
  *          @arg I2C_FLAG_TFE
  *          @arg I2C_FLAG_TFNF
  *          @arg I2C_FLAG_ACTIVITY
  *          @arg I2C_FLAG_MST_ACTIVITY
  * @return The new state of I2C_FLAG (SET or RESET).
  */
FlagStatus I2C_GetFlagStatus(I2C_T* I2Cx, uint32_t I2C_FLAG);
/**
  * @brief  Clears the I2Cx's interrupt pending bits.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  I2C_IT: specifies the interrupt pending bit to clear.
  *             @arg I2C_IT_RX_UNDER
  *             @arg I2C_IT_RX_OVER
  *             @arg I2C_IT_RX_FULL
  *             @arg I2C_IT_TX_OVER
  *             @arg I2C_IT_TX_EMPTY
  *             @arg I2C_IT_RD_REQ
  *             @arg I2C_IT_TX_ABRT
  *             @arg I2C_IT_RX_DONE
  *             @arg I2C_IT_ACTIVITY
  *             @arg I2C_IT_STOP_DET
  *             @arg I2C_IT_START_DET
  *             @arg I2C_IT_GEN_CALL
  *             @arg I2C_IT_MST_ON_HOLD
  * @return None
  */
void I2C_ClearITPendingBit(I2C_T* I2Cx, uint16_t I2C_IT);

/**
  * @brief  read data from the I2Cx peripheral.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  WriteBuf: cache of stored readings.
  * @param  Size: number of data read from I2C peripheral.
  * @return none.
  */
void I2C_Write(I2C_T* I2Cx,uint8_t *WriteBuf,uint32_t Size);

/**
  * @brief  write data to the I2Cx peripheral.
  * @param  I2Cx: where I2Cx is a i2c peripheral base address.
  * @param  ReadBuf: cache of stored writings.
  * @param  Size: number of data read from I2C peripheral.
  * @return None.
  */
void I2C_Read(I2C_T* I2Cx,uint8_t *ReadBuf,uint32_t Size);

/**@} */

#ifdef __cplusplus
}
#endif

#endif //__PAN_I2C_H__

/*** (C) COPYRIGHT 2016 Panchip Technology Corp. ***/
