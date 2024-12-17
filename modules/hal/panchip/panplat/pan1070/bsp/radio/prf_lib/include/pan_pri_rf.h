/*
 * Copyright (C) 2021 Panchip Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 
 */
 
/**
 * @file
 * @brief    Panchip series pri_rf driver header file 
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 21/11/19 18:33 $ 
 */

#ifndef __PAN_PRI_RF_H__
#define __PAN_PRI_RF_H__
/**
 * @brief Private rf Interface
 * @defgroup private_rf_interface Private rf Interface
 * @{
 */
#ifdef __cplusplus
extern "C"
{
#endif

#include "PanSeries.h"

/**@defgroup PRI_RF_MODE_FLAG Private rf mode type 
 * @brief       Private rf mode type  definitions
 * @{ */
#define	PRI_RF_MODE_SEL_TX				(0)   /*!< Tx mode */
#define	PRI_RF_MODE_SEL_RX				(1)   /*!< Rx mode */
#define	PRI_RF_MODE_SEL_TRX				(2)   /*!< Tx mode and Rx mode */
/**@} */

/**@defgroup PRI_RF_ADR_LEN_FLAG Private rf address lenth type 
 * @brief       Private rf address lenth type definitions
 * @{ */
#define	PRI_RF_ADDR_BYTE_LEN_2			(0)     /*!< 2-bytes is invalid */
#define	PRI_RF_ADDR_BYTE_LEN_3			(1)     /*!< 3-bytes address */
#define	PRI_RF_ADDR_BYTE_LEN_4			(2)     /*!< 4-bytes address */
#define	PRI_RF_ADDR_BYTE_LEN_5			(3)     /*!< 5-bytes address */
/**@} */

/**@defgroup PRI_RF_CHIP_MODE_FLAG Private rf chip mode type 
 * @brief       Private rf chip mode type definitions
 * @{ */
#define	PRI_RF_CHIP_MODE_INVALID		(0)     /*!< Chip mode is invalid */
#define	PRI_RF_CHIP_MODE_BLE			(1)       /*!< Ble mode */
#define	PRI_RF_CHIP_MODE_297			(2)       /*!< 297 mode */
#define	PRI_RF_CHIP_MODE_NORDIC			(3)     /*!< Nordic mode */
/**@} */

/**@defgroup PRI_RF_BUf_MEM_FLAG Private rf buf memory address offset 
 * @brief       Private rf buf memory address offset definitions
 * @{ */
#define REG_FILE_OFST                       0x0000    /*!< Register address offset */
#define SEQ_RAM_OFST                        0x1000    /*!< Sequence ram address offset */
#define LIST_RAM_OFST                       0x8000    /*!< List ram address offset */
#define TX_RX_RAM_OFST                      0x8200    /*!< Tx and rx ram address offset */
#define CTE_IQ_RAM_OFST                     0xB7FC    /*!< Cte ram address offset */
/**@} */

/** 
 * @brief       Read 32 bit register value
 *
 * @param[in]   base_addr Module base address
 * @param[in]   reg_ofst  register address offset
 */
#define LLHWC_READ32_REG(base_addr, reg_ofst) \
		(*(volatile uint32_t *)((0x50020000) + (base_addr) + (reg_ofst)))
/** 
 * @brief       Write value to 32 bit register
 *
 * @param[in]   base_addr Module base address
 * @param[in]   reg_ofst  register address offset
 * @param[in]   data  register expect value
 */
#define LLHWC_WRITE32_REG(base_addr, reg_ofst, data) \
			(*(volatile uint32_t *)(((0x50020000) + (base_addr) + (reg_ofst))) = (data))
/** 
 * @brief       Read 4 byte with byte read mode
 *
 * @param[in]   pckt Base address
 * @param[in]   pos  address offset
 */
#define READ_4_BYTES(pckt, pos) (((uint32_t) (pckt)[pos]) | \
								(((uint32_t) (pckt)[pos + 1]) << 8) | \
								(((uint32_t) (pckt)[pos + 2]) << 16) | \
								(((uint32_t) (pckt)[pos + 3]) << 24))

/** 
 * @brief       Write value to register
 *
 * @param[in]   base Module base address
 * @param[in]   Reg  register address
 * @param[in]   Func  register function
 * @param[in]   Value  register expect value
 */
#define PRI_RF_WRITE_REG_VALUE(base,Reg,Func,Value)     \
        (base->Reg = (base->Reg & ~(Reg##_##Func##_Msk)) | ((Value << Reg##_##Func##_Pos) & Reg##_##Func##_Msk))
/** 
 * @brief       Read value to register
 *
 * @param[in]   base Module base address
 * @param[in]   Reg  register address
 * @param[in]   Func  register function
 */
#define PRI_RF_READ_REG_VALUE(base,Reg,Func)     \
        ((base->Reg & (Reg##_##Func##_Msk)) >> Reg##_##Func##_Pos)
/** 
 * @brief       Set function according to state
 *
 * @param[in]   base Module base address
 * @param[in]   Reg  register address
 * @param[in]   Func  register function
 * @param[in]   State  enable state
 */
#define PRI_RF_SET_FUNC_ENABLE(base,Reg,Func,State)     \
		((State == ENABLE) ? (base->Reg |= Reg##_##Func##_Msk) : (base->Reg &= ~Reg##_##Func##_Msk))
/**
  * @brief  This function used to enable or disable auto analysis payload function
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_AutoAnlsPayloadEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,DPY_EN,NewState);
}

/**
  * @brief  This function used to enable or disable crc check function
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_CrcCheckEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,CRC_EN,NewState);
}

/**
  * @brief  This function select crc check mode
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of crc16 select,if enable,crc16 selected,or crc8 selected
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_Crc16Select(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,CRC_SEL16,NewState);
}
/**
  * @brief  This function used to enable or disable scamble function
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_ScambleEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,SCR_EN,NewState);
}

/**
  * @brief  This function used to enable or disable enhance mode2(compatible with nordic)
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_NordicEnhanceEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,NRF_ENHANCE,NewState);
}
/**
  * @brief  This function used to adjuest enhance mode2 enable or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @return enable state
  */ 
__STATIC_INLINE uint8_t PRI_RF_IsNordicEnhance(PRI_RF_T *rf)
{
	return PRI_RF_READ_REG_VALUE(rf,R00_CTL,NRF_ENHANCE);
}
/**
  * @brief  This function used to enable or disable enhance mode
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_EnhanceEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,ENHANCE,NewState);
}
/**
  * @brief  This function used to adjuest enhance mode is enable or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @return enable state
  */ 
__STATIC_INLINE uint8_t PRI_RF_IsEnhance(PRI_RF_T *rf)
{
    return PRI_RF_READ_REG_VALUE(rf,R00_CTL,ENHANCE);
}
/**
  * @brief  This function used to set band width, 1Mbps or 2Mbps can selected
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: state of 2Mbps selected,if 1,2Mbps selected, or 1Mbps selected
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_BandWidth2mSel(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,BW_MODE,NewState);
}

/**
  * @brief  This function used to select chip mode
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  modeSel: chip mode select,including:
  *						\ref PRI_RF_CHIP_MODE_INVALID
  *						\ref PRI_RF_CHIP_MODE_BLE	
  *						\ref PRI_RF_CHIP_MODE_297	
  *						\ref PRI_RF_CHIP_MODE_NORDIC	
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_ChipModeSel(PRI_RF_T *rf,uint8_t modeSel)	
{
	PRI_RF_WRITE_REG_VALUE(rf,R00_CTL,CHIP_MODE,modeSel);
}
/**
  * @brief  This function used to get chip mode
  * @param  rf: where rf is a private rf peripheral base address  	
  * @return chip mode
  */ 
__STATIC_INLINE uint8_t PRI_RF_GetChipMode(PRI_RF_T *rf)	
{
	return PRI_RF_READ_REG_VALUE(rf,R00_CTL,CHIP_MODE);
}

/**
  * @brief  This function used to enable or disable rx ack if rx with payload
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_RxAckEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,RX_ACK_PAYLOAD_EN,NewState);
}
/**
  * @brief  This function used to adjust rx ack is enable or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @return enable state
  */ 
__STATIC_INLINE uint8_t PRI_RF_IsRxAckEn(PRI_RF_T *rf)
{
    return (rf->R00_CTL & R00_CTL_RX_ACK_PAYLOAD_EN_Msk) >> R00_CTL_RX_ACK_PAYLOAD_EN_Pos;
}
/**
  * @brief  This function used to enable or disable tx no ack if function enabled and
  *			tx works in enhance mode,then rx ack is no needed	
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_TxNoAckEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,TX_NOACK_EN,NewState);
}
/**
  * @brief  This function used to adjuest tx no ack is enable or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @return tx enable state
  */ 
__STATIC_INLINE uint8_t PRI_RF_IsTxNoAckEn(PRI_RF_T *rf)
{
    return (rf->R00_CTL & R00_CTL_TX_NOACK_EN_Msk) >> R00_CTL_TX_NOACK_EN_Pos;
}

/**
  * @brief  This function used to enable  tx or rx function
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of function control state,if enabled,rx function enabled,
  * 		or tx function enabled
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_TrxFuncSel(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R00_CTL,PRI_RX,NewState);
}

/**
  * @brief  This function used to adjust rx function is enable or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @return none
  */ 
__STATIC_INLINE uint8_t PRI_RF_IsRxSel(PRI_RF_T *rf)
{
    return PRI_RF_READ_REG_VALUE(rf,R00_CTL,PRI_RX);
}
/**
  * @brief  This function used to enable or disable all interrupt clear function
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_ClearAllIrqEn(PRI_RF_T *rf,FunctionalState NewState)
{
    PRI_RF_SET_FUNC_ENABLE(rf,R01_INT,IRQ_CLR_EN,NewState);
}


/**
  * @brief  This function used to set interrupt mask
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  msk: interrupt mask bit,including:
  *						R01_INT_CTL_RX_CRC_ERR_MASK_Msk    
  *						R01_INT_CTL_TX_TIMEOUT_IRQ_MASK_Msk
  *						R01_INT_CTL_TX_IRQ_MASK_Msk        
  *						R01_INT_CTL_RX_IRQ_MASK_Msk        
  * @param  NewState: new state of interrupt mask
  * @return none
  */
__STATIC_INLINE void PRI_RF_IntMask(PRI_RF_T *rf,uint32_t msk,FunctionalState NewState)
{
    (NewState)?(rf->R01_INT |= msk):(rf->R01_INT &= ~msk);
}

/**
  * @brief  This function used to read interrupt flag
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  msk: interrupt flag bit,including:
  *						R01_INT_CTL_RX_CRC_ERR_Msk    
  *						R01_INT_CTL_TX_TIMEOUT_IRQ_Msk
  *						R01_INT_CTL_TX_IRQ_Msk        
  *						R01_INT_CTL_RX_IRQ_Msk        
  * @return true,interrupt happened
  * @return false,interrupt does not happened
  */
__STATIC_INLINE bool PRI_RF_IntFlag(PRI_RF_T *rf,uint32_t msk)
{
    return (rf->R01_INT & msk)?(true):(false);
}

/**
  * @brief  This function used to force exit rx mode
  * @param  rf: where rf is a private rf peripheral base address 
  * @param  NewState: new state of force exit function
  * @return none
  */
__STATIC_INLINE void PRI_RF_ForceExitRx(PRI_RF_T *rf,FunctionalState NewState)
{
	PRI_RF_SET_FUNC_ENABLE(rf,R01_INT,EXIT_RX,NewState);
}

/**
  * @brief  This function used to set tx& rx transmit wait time 
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_SetTrxTransWaitTime(PRI_RF_T *rf,uint16_t time)
{
	PRI_RF_WRITE_REG_VALUE(rf,R02_TMR_CTL,TRX_TRANS_WAIT_TIME,time);
}


/**
  * @brief  This function used to set tx& rx transmit wait time 
  * @param  rf: where rf is a private rf peripheral base address
  * @param  pipe: where pipe is a rx pipe number, valid 0~7
  * @param  NewState: new state of enabling state
  * @return none
  */ 
__STATIC_INLINE void PRI_RF_EnableRxPipeAdr(PRI_RF_T *rf,uint16_t pipe, FunctionalState NewState)
{
	if (NewState)
		rf->R10_RX_ADDR_EN |= (0x1ul << (R10_RX_ADDR0_EN_Pos + pipe));
	else
		rf->R10_RX_ADDR_EN &= ~(0x1ul << (R10_RX_ADDR0_EN_Pos + pipe));
}



__STATIC_INLINE void PRI_RF_SetRxGoon(PRI_RF_T *rf, uint8_t en_flag)
{
	PRI_RF_WRITE_REG_VALUE(rf, R01_INT, PRI_RX_GOON, en_flag);
}

__STATIC_INLINE void PRI_RF_SetAddrMatchBit(PRI_RF_T *rf, uint8_t value)
{
	PRI_RF_WRITE_REG_VALUE(rf, R11_CFG, ADDR_ERR_THR, value);
}

__STATIC_INLINE void PRI_RF_SetPayloadEndian(PRI_RF_T *rf, uint8_t endian)
{
	PRI_RF_WRITE_REG_VALUE(rf, R01_INT, PRI_ENDIAN, endian);
}

__STATIC_INLINE void PRI_RF_MultiPreamble(PRI_RF_T *rf, uint8_t pre_length)
{
	PRI_RF_WRITE_REG_VALUE(rf, R11_CFG, PREAM_2BYTE_EN, 1);
	PRI_RF_WRITE_REG_VALUE(rf, R11_CFG, B250K_PREAM, pre_length);			// preamble length ctrl
}

__STATIC_INLINE void PRI_RF_StopRx(PRI_RF_T *rf, uint8_t en_flag)
{
	PRI_RF_WRITE_REG_VALUE(rf, R01_INT, EXIT_RX, en_flag);			// stop rx
}

__STATIC_INLINE void PRI_RF_RandomNumGenInit(PRI_RF_T *rf)
{
	rf->RNG1 |= RNG1_RNG_EN_Msk;
}

__STATIC_INLINE void PRI_RF_RandomNumGenReinit(PRI_RF_T *rf)
{
	rf->RNG1 &= ~(RNG1_RNG_EN_Msk | RNG1_RNG_RING_SCRMB_SEL_Msk);
}

__STATIC_INLINE uint32_t PRI_RF_GetRandomNum(PRI_RF_T *rf)
{
	return rf->RNG2;
}
/**
  * @brief  This function used to enable private radio ldo
  * @return none
  */ 
void ana_prf_ldo_en(void);
/**
  * @brief  This function used to disable private radio ldo
  * @return none
  */ 
void ana_prf_ldo_dis(void);

/**
  * @brief  This function used to set private rf payload lenth
  * @param  rf: where rf is a private rf peripheral base address 
  * @param  mode: tx or rx mode select,including:
  * 							\ref PRI_RF_MODE_SEL_TX
  * 							\ref PRI_RF_MODE_SEL_RX
  * @param  len: payload lenth 
  * @retval none
  */ 
void PRI_RF_SetTrxPayloadLen(PRI_RF_T *rf,uint8_t mode,uint8_t len);

/**
  * @brief  This function used to set private rf address byte lenth
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  len: address byte lenth ,including:
  *  						\ref PRI_RF_ADDR_BYTE_LEN_2			
  *  						\ref PRI_RF_ADDR_BYTE_LEN_3		
  *  						\ref PRI_RF_ADDR_BYTE_LEN_4		
  *  						\ref PRI_RF_ADDR_BYTE_LEN_5		
  * @retval none
  */ 
void PRI_RF_SetAddrByteLen(PRI_RF_T *rf,uint8_t len);

/**
  * @brief  This function used to set pid manual
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  mode: tx or rx mode select,including:
  * 							PRI_RF_MODE_SEL_TX
  * 							PRI_RF_MODE_SEL_RX
  * @param  pid: pid value
  * @retval none
  */ 
void PRI_RF_SetPidManual(PRI_RF_T *rf,uint8_t mode,uint8_t pid);

/**
  * @brief  This function used to set rx wait time 
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  NewState: new state of enabling state
  * @retval none
  */ 
void PRI_RF_SetRxWaitTime(PRI_RF_T *rf,uint16_t time);

/**
  * @brief  This function used to set private rf tx or rx address
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  mode: tx or rx mode select,including:
  * 							PRI_RF_MODE_SEL_TX
  * 							PRI_RF_MODE_SEL_RX
  * 							PRI_RF_MODE_SEL_TRX
  * @param  pipe: where pipe is a rx pipe selection, 1 bit represents a channel
  *         example: pipe set 7, pipe0\pipe1\pipe2 address need set a value
  * @param  addr: addr base address 
  * @retval none
  */ 
void PRI_RF_SetTrxAddr(PRI_RF_T *rf,uint8_t mode,uint8_t pipe,uint32_t *addr);

/**
  * @brief  This function used to set private rf tx or rx ram start address
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  mode: tx or rx mode select,including:
  * 						\ref PRI_RF_MODE_SEL_TX
  * 						\ref PRI_RF_MODE_SEL_RX
  * 						\ref PRI_RF_MODE_SEL_TRX
  * @param  addr: addr base address 
  * @retval none
  */ 
void PRI_RF_SetTrxRamStartAddr(PRI_RF_T *rf,uint8_t mode,uint32_t addr);

/**
  * @brief  This function used to adjust private rf tx or rx ram state is ready or not
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  mode: tx or rx mode select,including:
  * 						\ref PRI_RF_MODE_SEL_TX
  * 						\ref PRI_RF_MODE_SEL_RX
  * 						\ref PRI_RF_MODE_SEL_TRX
  * @retval true ram is ready
  * @retval false ram is not ready
  */ 
bool PRI_RF_IsTrxRamReady(PRI_RF_T *rf,uint8_t mode);

/**
  * @brief  This function used to set private rf ram ready state
  * @param  rf: where rf is a private rf peripheral base address  
  * @param  mode: tx or rx mode select,including:
  * 							\ref PRI_RF_MODE_SEL_TX
  * 							\ref PRI_RF_MODE_SEL_RX
  * 							\ref PRI_RF_MODE_SEL_TRX
  * @param  ready: ready state 
  * @retval none
  */ 
void PRI_RF_SetTrxRamReady(PRI_RF_T *rf,uint8_t mode,uint8_t ready);


/**@} */
#ifdef __cplusplus
}
#endif

#endif /* __PRI_RF_H__ */
