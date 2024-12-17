/**************************************************************************
 * @file     pan_lp.h
 * @version  V1.00
 * $Revision: 3 $
 * $Date:    2023/11/08 $  
 * @brief    Panchip series lowpower driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef		__PAN_LP_H__
#define		__PAN_LP_H__

/**
 * @brief Lowpower Interface
 * @defgroup lowpower_interface Lowpower Interface
 * @{
 */
#ifdef __cplusplus
extern "C"
{
#endif


#define	LP_EXT_P56_WK_EN								(1)		/*!< Gpio p56 wake up enable */
#define	LP_EXT_P56_WK_DISABLE							(0)		/*!< Gpio p56 wake up disable */

/**@defgroup LP_GPIO_WK_EDGE_FLAG Lowpower gpio wake up edge 
 * @brief       Lowpower gpio wake up edge select definitions
 * @{ */
#define	LP_EXT_P56_WK_EDGE_LOW							(0)		/*!< Gpio p56 wake up by falling edge */
#define	LP_EXT_P56_WK_EDGE_HIGH							(1)		/*!< Gpio p56 wake up by rising edge */
/**@} */

#define LP_SLPTMR_CH0                   (0)
#define LP_SLPTMR_CH1                   (1)
#define LP_SLPTMR_CH2                   (2)

#define LP_MODE_SEL_SLEEP_MODE			(0)
#define LP_MODE_SEL_DEEPSLEEP_MODE		(1)
#define LP_MODE_SEL_STANDBY_M1_MODE		(2)
#define LP_MODE_SEL_STANDBY_M0_MODE		(3)

#define LP_DEEPSLEEP_MODE1		(1)
#define LP_DEEPSLEEP_MODE2		(2)
#define LP_DEEPSLEEP_MODE3		(3)

#define LP_STANDBY_M1_MODE_SEL	LP_STANDBY_M1_MODE1
#define LP_STANDBY_M1_MODE1		(1)
#define LP_STANDBY_M1_MODE2		(2)
#define LP_STANDBY_M1_MODE3		(3)

#define LP_STBM1_WAKEUP_SRC_GPIO        BIT0
#define LP_STBM1_WAKEUP_SRC_SLPTMR      BIT1

#define LP_RETENTION_SRAM_NONE          (0)
#define LP_RETENTION_SRAM_BLOCK0        BIT0 /* 32 KB */
#define LP_RETENTION_SRAM_BLOCK1        BIT1 /* 16 KB */
#define LP_RETENTION_SRAM_DECRYPT       BIT2 /* 256 B */
#define LP_RETENTION_SRAM_PHY_REGS      BIT3 /* 256 B PHY SEQ RAM + PHY Registers */
#define LP_RETENTION_SRAM_LL            BIT4 /* 8 KB */
#define LP_RETENTION_SRAM_ALL           (BIT0 | BIT1 | BIT2 | BIT3 | BIT4)

/**
* @brief  This function enable lowpower intterrupt
* @param[in]  ana: where ana is analog module
* @param[in] NewState: new state of interrupt mask
*					ANAC_INT_LP_INT_EN_Msk
* @return   none
*/
__STATIC_INLINE void LP_EnableInt(ANA_T *ana,FunctionalState NewState)
{
	(NewState == ENABLE)?(ana->LP_INT_CTRL |= ANAC_INT_LP_INT_EN_Msk):(ana->LP_INT_CTRL &= ~ANAC_INT_LP_INT_EN_Msk);
}

/**
* @brief  This function wait to clear wake up flag
* @param[in]  ana: where ana is analog module
* @param[in]  u32Mask: bit need to be cleared
* @return   none
*/
__STATIC_INLINE void LP_ClearWakeFlag(ANA_T *ana,uint32_t u32Mask)
{
    ana->LP_INT_CTRL |= u32Mask;
}

/**
* @brief  This function enable hpldo ready or not(0: need wait ready, 1: do not care ready)
* @param[in]  ana: where ana is analog module
* @param[in] NewState: new state of hpldo ready bypass signal
* @return   none
*/
__STATIC_INLINE void LP_HpldoRdyBypassEn(ANA_T *ana,FunctionalState NewState)
{
    (NewState == ENABLE)?(ana->LP_DLY_CTRL_3V |= ANAC_HPLDO_RDY_BYPASS_Msk):(ana->LP_DLY_CTRL_3V &= ~ANAC_HPLDO_RDY_BYPASS_Msk);
}

/**
* @brief  This function enable fast clk delay or not
* @param[in]  ana: where ana is analog module
* @param[in] NewState: new state of hpldo ready bypass signal
* @return   none
*/
__STATIC_INLINE void LP_FastClkDelayEn(ANA_T *ana,FunctionalState NewState)
{
    (NewState == ENABLE)?(ana->LP_DLY_CTRL_3V |= ANAC_32KCLK_DLY_EN_Msk):(ana->LP_DLY_CTRL_3V &= ~ANAC_32KCLK_DLY_EN_Msk);
}

/**
  * @brief  This function enable gpio p56 wake up 
  * @param[in]  ana: where ana is analog module base address
  * @param[in]  NewState: enable or disable
  * @param[in]  WkEdge: p56 wake up edge select,0-->low,1-->high 
  * @return   none
  */
void LP_SetExternalWake(ANA_T *ana,uint8_t WkEdge);
/**
* @brief  This function set sleep time
* @param[in]  ana: where ana is analog module
* @param[in]  u32ClkCnt: where u32ClkCnt is 32k clock cnt num
* @param[in]  idx: channel index of sleeptimer, can be 0, 1, 2
* @return   none
*/
void LP_SetSleepTime(ANA_T *ana,uint32_t u32ClkCnt,uint8_t idx);

/**
  * @brief  This function get current 32K timer counter
  * @param[in]  ana: where ana is analog module
  * @return  Current 32K timer counter
  */
uint32_t LP_GetSlptmrCurrCount(ANA_T *ana);

/**
 * @brief  This function sets the LPLDOH (Low-Power Low-Dropout Regulator) delay count using 32kHz clock cycles.
 * @param[in]  ana: Analog module base address
 * @param[in]  u16Clk32Cnt: Number of 32kHz clock cycles for delay
 * @return   None
 */
void LP_SetLpldohDelay(ANA_T *ana,uint16_t u16Clk32Cnt);
/**
  * @brief  This function used to set digital reset time
  * @param[in]  ana: where ana is analog module base address
  * @param[in]  u8Clk32Cnt: where u8Clk32Cnt is 32k clock period cnt
  * @return   none
  */
void LP_SetWakeDelay(ANA_T *ana,uint16_t u16Clk32Cnt);
/**
 * @brief  This function sets the sleep mode for the specified analog module.
 * @param[in]  ana: Analog module base address
 * @param[in]  mode: Sleep mode configuration
 * @return   None
 */
void LP_SetSleepMode(ANA_T *ana,uint8_t mode);

/**
* @brief  This function set sleep mode config
* @param[in]  ana: Select analog module
* @param[in]  enterCyclically: Enable ARM Sleep-On-Exit Feature or not
* @return   none
*/
void LP_EnterSleepMode(ANA_T *ana, bool enterCyclically);

/**
 * @brief  This function sets the deep sleep mode configuration.
 * @param[in]  ana: Analog module base address
 * @param[in]  enterCyclically: Determines whether to cycle into low power
 * @param[in]  powerCtrl: Power control configuration
 * @param[in]  dpMode: Deep sleep mode configuration
 * @return   None
 */
void LP_EnterDeepSleepMode(ANA_T *ana, bool enterCyclically, uint8_t powerCtrl, uint8_t dpMode);

/**
 * @brief  This function sets the Standby Mode 1 configuration.
 * @param[in]  ana: Analog module base address
 * @param[in]  powerCtrl: Power control of SRAMS in lp mode
 * @param[in]  wakeupWithoutReset: Do not reset (continue run) after waking up
 */
void LP_EnterStandbyMode1(ANA_T *ana, uint8_t powerCtrl, bool wakeupWithoutReset);

/**
 * @brief  This function sets the Standby Mode 0 configuration.
 * @param[in]  ana: Analog module base address
 * @param[in]  enableClk32k: Enable 32K low speed clock in lp mode for special purpose, such as BOD/LVD wakeup
 */
void LP_EnterStandbyMode0(ANA_T *ana, bool enableClk32k);

/**@} */
#ifdef __cplusplus
}
#endif
#endif /* __PAN_LP_H__ */

