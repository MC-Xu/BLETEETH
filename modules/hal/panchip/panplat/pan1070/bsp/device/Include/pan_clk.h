/**************************************************************************
 * @file     pan_clk.h
 * @version  V1.00
 * $Revision: 3$
 * $Date: 2023/11/08 $ 
 * @brief    Panchip series CLK driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/ 
#ifndef __PAN_CLK_H__
#define __PAN_CLK_H__

/**
 * @brief Clk Interface
 * @defgroup clk_interface Clk Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif




#define CLK_APB1_WDTSEL_MILLI_PULSE             (0x00010000UL)   
#define CLK_APB1_WDTSEL_RCL32K                  (0x00000000UL)

#define CLK_APB1_WWDTSEL_MILLI_PULSE            (0x00000000UL)  /*!< Wwdt clk source select apb_clk / 2048 */
#define CLK_APB1_WWDTSEL_RCL32K                 (0x00020000UL)  /*!< Wwdt clk source select rcl 32k */

#define CLK_APB1_TMR0SEL_APB1CLK                (0x00000000UL)  /*!< Timer0 clk source select apb_clk */
#define CLK_APB1_TMR0SEL_RCL32K                 (0x00040000UL)  /*!< Timer0 clk source select rcl 32k */
#define CLK_APB1_TMR0SEL_TM0                    (0x00080000UL)  /*!< Timer0 clk source select externel input */

#define CLK_APB2_TMR1SEL_APB2CLK                (0x00000000UL)  /*!< Timer1 clk source select apb_clk */
#define CLK_APB2_TMR1SEL_RCL32K                 (0x00000100UL)  /*!< Timer1 clk source select rcl 32k */
#define CLK_APB2_TMR1SEL_TM1                    (0x00000200UL)  /*!< Timer1 clk source select externel input */
                                               
#define CLK_APB2_TMR2SEL_APB2CLK                (0x00000000UL)  /*!< Timer2 clk source select apb_clk */
#define CLK_APB2_TMR2SEL_RCL32K                 (0x00000400UL)  /*!< Timer2 clk source select rcl 32k */
#define CLK_APB2_TMR2SEL_TM2                    (0x00000800UL)  /*!< Timer2 clk source select externel input */

#define CLK_APB1_PWM_CH01_SEL_APB               (0x00000000UL)  /*!< PWM CH01 clk source select apb clk */
#define CLK_APB1_PWM_CH01_SEL_CLK32K            (0x00100000UL)  /*!< PWM CH01 clk source select 32k clk */
#define CLK_APB1_PWM_CH23_SEL_APB               (0x00000000UL)  /*!< PWM CH23 clk source select apb clk */
#define CLK_APB1_PWM_CH23_SEL_CLK32K            (0x00200000UL)  /*!< PWM CH23 clk source select 32k clk */
#define CLK_APB1_PWM_CH45_SEL_APB               (0x00000000UL)  /*!< PWM CH45 clk source select apb clk */
#define CLK_APB1_PWM_CH45_SEL_CLK32K            (0x00400000UL)  /*!< PWM CH45 clk source select 32k clk */
#define CLK_APB1_PWM_CH67_SEL_APB               (0x00000000UL)  /*!< PWM CH67 clk source select apb clk */
#define CLK_APB1_PWM_CH67_SEL_CLK32K            (0x00800000UL)  /*!< PWM CH67 clk source select 32k clk */

/** @defgroup CLK_FREQUENCT_FLAG Clk frequence
 * @brief       Clk frequence definitions
 * @{ */

 #define FREQ_1MHZ                            (1000000)
 #define FREQ_8MHZ                            (8000000)
 #define FREQ_16MHZ                           (16000000)
 #define FREQ_25MHZ                           (25000000)
 #define FREQ_32MHZ                           (32000000)
 #define FREQ_48MHZ                           (48000000)
 #define FREQ_64MHZ                           (64000000)
 #define FREQ_96MHZ                           (96000000)
 #define FREQ_50MHZ                           (50000000)
 #define FREQ_72MHZ                           (72000000)
 #define FREQ_100MHZ                          (100000000)
/**@} */


/** @defgroup SYS_CLK_SOURCE_FLAG System clk source select
 * @brief       System clk source definitions
 * @{ */
#define CLK_SYS_SRCSEL_RCH         	          ((uint32_t)0x00000000)  /*!< System clk source frequence select as rch */
#define CLK_SYS_SRCSEL_XTH        	          ((uint32_t)0x00000100)  /*!< System clk source frequence select as xth */
#define CLK_SYS_SRCSEL_DPLL         	        ((uint32_t)0x00000200)  /*!< System clk source frequence select as dpll */
/**@} */

/*SYS CLK DPLL SELECTED*/
/** @defgroup DPLL_SOURCE_FLAG Dpll clk reference source select
 * @brief       Dpll clk reference source select definitions
 * @{ */
#define CLK_DPLL_REF_CLKSEL_RCH         	  ((uint32_t)0x00000000)
#define CLK_DPLL_REF_CLKSEL_XTH         	  ((uint32_t)0x00000008)
#define CLK_DPLL_OUT_48M       	              ((uint32_t)0x00000000)
#define CLK_DPLL_OUT_64M        	          ((uint32_t)0x00000004)
/**@} */


/** @defgroup APB1_PERIPHERAL_CLK_FLAG Apb1 clk enable
 * @brief       Apb1 clk enable definitions
 * @{ */

#define CLK_APB1Periph_I2C0               ((uint32_t)0x00000001)
#define CLK_APB1Periph_SPI0               ((uint32_t)0x00000002)
#define CLK_APB1Periph_UART0              ((uint32_t)0x00000008)
#define CLK_APB1Periph_PWM0_CH01          ((uint32_t)0x00000010)
#define CLK_APB1Periph_PWM0_CH23          ((uint32_t)0x00000020)
#define CLK_APB1Periph_PWM0_CH45          ((uint32_t)0x00000040)
#define CLK_APB1Periph_PWM0_CH67          ((uint32_t)0x00000080)
#define CLK_APB1Periph_PWM0_EN            ((uint32_t)0x00000100)
#define CLK_APB1Periph_ADC                ((uint32_t)0x00000200)
#define CLK_APB1Periph_WDT                ((uint32_t)0x00000400)
#define CLK_APB1Periph_WWDT               ((uint32_t)0x00000800)
#define CLK_APB1Periph_TMR0               ((uint32_t)0x00001000)
#define CLK_APB1Periph_MILI_CLK           ((uint32_t)0x00002000)
#define CLK_APB1Periph_All                ((uint32_t)0x00003ffb)
/**@} */

  
/** @defgroup APB2_PERIPHERAL_CLK_FLAG Apb2 clk enable
 * @brief       Apb2 clk enable definitions
 * @{ */

#define CLK_APB2Periph_SPI1               ((uint32_t)0x00000002)
#define CLK_APB2Periph_UART1              ((uint32_t)0x00000008)
#define CLK_APB2Periph_TMR1               ((uint32_t)0x00000010)
#define CLK_APB2Periph_TMR2               ((uint32_t)0x00000020)
#define CLK_APB2Periph_All                ((uint32_t)0x0000003a)
/**@} */

/** @defgroup AHB_PERIPHERAL_CLK_FLAG Ahb peripheral clk enable
 * @brief       Ahb peripheral clk enable definitions
 * @{ */
 
#define CLK_AHBPeriph_DMAC                ((uint32_t)0x00000001)
#define CLK_AHBPeriph_GPIO                ((uint32_t)0x00000002)
#define CLK_AHBPeriph_SYSTICK             ((uint32_t)0x00000004)
#define CLK_AHBPeriph_APB1                ((uint32_t)0x00000008)
#define CLK_AHBPeriph_APB2                ((uint32_t)0x00000010)
#define CLK_AHBPeriph_AHB                 ((uint32_t)0x00000020)
#define CLK_AHBPeriph_BLE_32M             ((uint32_t)0x00000040)
#define CLK_AHBPeriph_BLE_32K             ((uint32_t)0x00000080)

#define CLK_AHBPeriph_ROM                 ((uint32_t)0x00000400)
#define CLK_AHBPeriph_EFUSE               ((uint32_t)0x00000800)
#define CLK_AHBPeriph_USB_AHB             ((uint32_t)0x00002000)
#define CLK_AHBPeriph_USB_48M             ((uint32_t)0x00004000)
#define CLK_AHBPeriph_All                 ((uint32_t)0x00006CFF)
/**@} */

/** @defgroup CLK_SRC_FLAG Clk reference source
 * @brief       Clk reference source definitions
 * @{ */
#define CLK_DivideSource_Ahb              (1)
#define CLK_DivideSource_Apb1             (2)
#define CLK_DivideSource_Apb2             (4)
/**@} */

/** @defgroup CLK_SRC_FLAG Clk reference source
 * @brief       Clk reference source definitions
 * @{ */
#define CLK_RCL_SELECT                    (0)    /*!< Clk source reference set as rcl */
#define CLK_RCH_SELECT                    (1)    /*!< Clk source reference set as rch */
#define CLK_XTL_SELECT                    (2)    /*!< Clk source reference set as xtl */
#define CLK_XTH_SELECT                    (3)    /*!< Clk source reference set as xth */
#define CLK_DPLL_SELECT                   (4)    /*!< Clk source reference set as dpll */
/**@} */

#define CLK_STABLE_STATUS_Pos             (24)  /*!< Clk stable register position */
#define CLK_STABLE_STATUS_Msk             (0x1ul << CLK_STABLE_STATUS_Pos)    /*!< Clk stable register mask value */

#define CLKTRIM_CALC_CLK_SEL_32K		  (0)
#define CLKTRIM_CALC_CLK_SEL_32M		  (1)

#define CLKTRIM_QDEC_CLK_SEL_APB		  (0)       /*!< Qdec clk source selecet apb clk */
#define CLKTRIM_QDEC_CLK_SEL_32K		  (1)       /*!< Qdec clk source selecet 32K */

#define CLKTRIM_KSCAN_CLK_SEL_APB		  (0)
#define CLKTRIM_KSCAN_CLK_SEL_32K		  (1)


/** @defgroup CLK_RST_MODULE Module Reset Control Register Constants
 * @brief Module Reset Control Register constant definitions
 * @{
 */
#define DMA_RST     ((0x0<<24) | CLK_IPRST0_DMARST_Pos      ) /*!< DMA_RST is one of the CLK_ResetModule() parameter */
#define LL_RST      ((0x0<<24) | CLK_IPRST0_RFRST_Pos       ) /*!< LL_RST is one of the CLK_ResetModule() parameter */
#define EFUSE_RST   ((0x0<<24) | CLK_IPRST0_EFUSERST_Pos    ) /*!< EFUSE_RST is one of the CLK_ResetModule() parameter */
#define USB_RST     ((0x0<<24) | CLK_IPRST0_USBRST_Pos      ) /*!< USB_RST is one of the CLK_ResetModule() parameter */
#define MDMSTB_RST  ((0x0<<24) | CLK_IPRST0_MDMSTDBYRST_Pos ) /*!< MDMSTB_RST is one of the CLK_ResetModule() parameter */
#define MDM_RST     ((0x0<<24) | CLK_IPRST0_MDMRST_Pos      ) /*!< MDM_RST is one of the CLK_ResetModule() parameter */
#define I2C0_RST    ((0x4<<24) | CLK_IPRST1_I2C0RST_Pos     ) /*!< I2C0_RST is one of the CLK_ResetModule() parameter */
#define SPI0_RST    ((0x4<<24) | CLK_IPRST1_SPI0RST_Pos     ) /*!< SPI0_RST is one of the CLK_ResetModule() parameter */
#define SPI1_RST    ((0x4<<24) | CLK_IPRST1_SPI1RST_Pos     ) /*!< SPI0_RST is one of the CLK_ResetModule() parameter */
#define UART0_RST   ((0x4<<24) | CLK_IPRST1_UART0RST_Pos    ) /*!< UART0_RST is one of the CLK_ResetModule() parameter */
#define UART1_RST   ((0x4<<24) | CLK_IPRST1_UART1RST_Pos    ) /*!< UART0_RST is one of the CLK_ResetModule() parameter */
#define PWM0_RST    ((0x4<<24) | CLK_IPRST1_PWM0RST_Pos     ) /*!< PWM0_RST is one of the CLK_ResetModule() parameter */
#define ADC_RST     ((0x4<<24) | CLK_IPRST1_ADCRST_Pos      ) /*!< ADC_RST is one of the CLK_ResetModule() parameter */
#define WDT_RST     ((0x4<<24) | CLK_IPRST1_WDTRST_Pos      ) /*!< WDT_RST is one of the CLK_ResetModule() parameter */
#define WWDT_RST    ((0x4<<24) | CLK_IPRST1_WWDTRST_Pos     ) /*!< WWDT_RST is one of the CLK_ResetModule() parameter */
#define TMR0_RST    ((0x4<<24) | CLK_IPRST1_TMR0RST_Pos     ) /*!< TMR0_RST is one of the CLK_ResetModule() parameter */
#define TMR1_RST    ((0x4<<24) | CLK_IPRST1_TMR1RST_Pos     ) /*!< TMR1_RST is one of the CLK_ResetModule() parameter */
#define TMR2_RST    ((0x4<<24) | CLK_IPRST1_TMR2RST_Pos     ) /*!< TMR2_RST is one of the CLK_ResetModule() parameter */
#define GPIO_RST    ((0x4<<24) | CLK_IPRST1_GPIORST_Pos     ) /*!< GPIO_RST is one of the CLK_ResetModule() parameter */
#define CLKTRIM_RST ((0x4<<24) | CLK_IPRST1_TRIMRST_Pos     ) /*!< CLKTRIM_RST is one of the CLK_ResetModule() parameter */
/** @} */ // End of CLK_RST_MODULE

/**
 * @defgroup BOD_Threshold_Selection Brown Out Detector Threshold Voltage Selection Constants
 * @brief Constants for configuring the Brown Out Detector threshold voltage selection.
 * @{
 */
#define CLK_BODCTL_BOD_INT_EN           (0UL<<CLK_BODCTL_BODRSTEN_Pos_3v)  /*!< Brown-out Interrupt Enable */
#define CLK_BODCTL_BOD_RST_EN           (1UL<<CLK_BODCTL_BODRSTEN_Pos_3v)  /*!< Brown-out Reset Enable */
#define CLK_BODCTL_BODSEL_1_75V         (0UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 1.75V */
#define CLK_BODCTL_BODSEL_1_95V         (1UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 1.95V */
#define CLK_BODCTL_BODSEL_2_15V         (2UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 2.15V */
#define CLK_BODCTL_BODSEL_2_35V         (3UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 2.35V */
#define CLK_BODCTL_BODSEL_2_55V         (4UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 2.55V */
#define CLK_BODCTL_BODSEL_2_75V         (5UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 2.75V */
#define CLK_BODCTL_BODSEL_2_95V         (6UL<<CLK_BODCTL_BODSEL_Pos_3v)    /*!< Setting Brown Out Detector Threshold Voltage as 2.95V */
/** @} */ // End of BOD_Threshold_Selection


/**
 * @defgroup BOD_Debounce_Selection Brown Out Detector Debounce Selection Constants
 * @brief Constants for configuring the debounce selection for the Brown Out Detector.
 * @{
 */
#define CLK_BLDBCTL_BODDBSEL_2POW0      0x00000001UL
#define CLK_BLDBCTL_BODDBSEL_2POW1      0x00000002UL
#define CLK_BLDBCTL_BODDBSEL_2POW2      0x00000004UL
#define CLK_BLDBCTL_BODDBSEL_2POW3      0x00000008UL
#define CLK_BLDBCTL_BODDBSEL_2POW4      0x00000010UL
#define CLK_BLDBCTL_BODDBSEL_2POW5      0x00000020UL
/**@} */

/**
 * @defgroup LVR_Debounce_Selection Low Voltage Reset Debounce Selection Constants
 * @brief Constants for configuring the debounce selection for the Low Voltage Reset.
 * @{
 */
#define CLK_BLDBCTL_LVRDBSEL_2POW0      0x00000100UL
#define CLK_BLDBCTL_LVRDBSEL_2POW1      0x00000200UL
#define CLK_BLDBCTL_LVRDBSEL_2POW2      0x00000400UL
#define CLK_BLDBCTL_LVRDBSEL_2POW3      0x00000800UL
#define CLK_BLDBCTL_LVRDBSEL_2POW4      0x00001000UL
#define CLK_BLDBCTL_LVRDBSEL_2POW5      0x00002000UL
/**@} */

/**
  * @brief  Configures the xth clock.
  * @retval None
  */
__STATIC_INLINE void CLK_XthStartupConfig(void)
{
    CLK->XTH_CTRL |= (CLK_XTHCTL_XTH_TST_EN_Msk | CLK_XTHCTL_START_FAST_Msk);
}

/**
  * @brief  Configures the Low Speed AHB clock (HCLK).
  * @param  u32ClkDiv: defines the AHB clock divider. This clock is derived from 
  *         the AHB clock (HCLK).
  *          This parameter can be 0~15,hclk = hclk / (u32ClkDiv + 1):
  * @retval None
  */
__STATIC_INLINE void CLK_HCLKConfig(uint32_t u32ClkDiv)
{
    CLK->CLK_TOP_CTRL_3V = (CLK->CLK_TOP_CTRL_3V & (~CLK_TOPCTL_AHB_DIV_Msk)) | (u32ClkDiv << CLK_TOPCTL_AHB_DIV_Pos);
}

/**
  * @brief  Configures the Low Speed APB clock (PCLK1).
  * @param  u32ClkDiv: defines the APB1 clock divider. This clock is derived from 
  *         the AHB clock (HCLK).
  * 		This parameter can be 0~15,PCLK1 = hclk / (u32ClkDiv * 2):
  * @retval None
  */
__STATIC_INLINE void CLK_PCLK1Config(uint32_t u32ClkDiv )
{
    CLK->CLK_TOP_CTRL_3V = (CLK->CLK_TOP_CTRL_3V & (~CLK_TOPCTL_APB1_DIV_Msk))| (u32ClkDiv << CLK_TOPCTL_APB1_DIV_Pos);
}

/**
  * @brief  Configures the High Speed APB clock (PCLK2).
  * @param  CLK_HCLK: defines the APB2 clock divider. This clock is derived from 
  *         the AHB clock (HCLK).
  * 		This parameter can be 0~15,PCLK2 = hclk / (u32ClkDiv * 2):
  * @retval None
  */
__STATIC_INLINE void CLK_PCLK2Config(uint32_t u32ClkDiv)
{
    CLK->CLK_TOP_CTRL_3V = (CLK->CLK_TOP_CTRL_3V & (~CLK_TOPCTL_APB2_DIV_Msk))| (u32ClkDiv << CLK_TOPCTL_APB2_DIV_Pos);
}



/**
* @brief  This function  set DPLL divider
* @param[in]  div is dpll clk dividor ratio,4 bit width
* @return   none
*
*/
//__STATIC_INLINE void CLK_SetDpllDiv(uint32_t div)
//{
//   if(div > 0xff)
//		 CLK->DPLL_CTRL &= ~CLK_DPLLCTL_DIV_Msk;
//	 else
//		 CLK->DPLL_CTRL = ((CLK->DPLL_CTRL & ~CLK_DPLLCTL_DIV_Msk) | div);
//}

//__STATIC_INLINE uint32_t CLK_GetDpllDiv(void)
//{
//		return (CLK->DPLL_CTRL & CLK_DPLLCTL_DIV_Msk);
//}

/**
* @brief  This function set DPLL frequence
* @param[in]  freq is target frequency,it could be:
*							CLK_DPLL_OUT_48M
*							CLK_DPLL_OUT_32M
* @return   none
*
*/
__STATIC_INLINE void CLK_SetDpllOutputFreq(uint32_t freq)
{
    CLK->DPLL_CTRL &= ~CLK_DPLLCTL_FREQ_OUT_Msk;
    CLK->DPLL_CTRL |= freq;
}

/**
* @brief  This function set temperature gain value
* @param[in]  gain is target gain value
* @return   none
*
*/
//__STATIC_INLINE void CLK_SetTemperatureGain(uint32_t gain)
//{
//    ANA->ANA_TEMP = (ANA->ANA_TEMP & ~ANAC_TEMP_GAIN_CTL_Msk) | (gain<<ANAC_TEMP_GAIN_CTL_Pos);
//}

/**
* @brief  This function set temperature gain value
* @param[in]  gain is target gain value
* @return   none
*
*/
//__STATIC_INLINE void CLK_EnableTempDetect(void)
//{
//		ANA->ANA_TEMP |= ANAC_TEMP_ENABLE_Msk;
//}


/**
* @brief  This function used to enable clktrim peripheral
* @param[in]  NewState: new state of the clk.This parameter can be: ENABLE or DISABLE.
* @return   none
*
*/
__STATIC_INLINE void CLK_EnableClkTrim(FunctionalState NewState)
{
	(NewState != ENABLE)?(CLK->MEAS_CLK_CTRL &= ~CLK_MEASCLK_TRIM_CLK_EN_Msk):(CLK->MEAS_CLK_CTRL |= CLK_MEASCLK_TRIM_CLK_EN_Msk);
}


/**
* @brief  This function used to select calculate clk source
* @param[in]  src: clk source, including:
*				CLKTRIM_CALC_CLK_SEL_32K
*				CLKTRIM_CALC_CLK_SEL_32M
* @return   none
*
*/
__STATIC_INLINE void CLK_SelectClkTrimSrc(uint32_t src)
{
	CLK->MEAS_CLK_CTRL = (CLK->MEAS_CLK_CTRL & ~CLK_MEASCLK_TRIM_CLK_SEL_Msk)| (src << CLK_MEASCLK_TRIM_CLK_SEL_Pos);
}

/**
* @brief  This function is used to set calculated high speed clock source (HSCK = RCH/EXT_CLK) divisor of CLKTRIM
* @param[in]  div: clock divisor (9 bits),
*                   0:      No division, cal_clk = HSCK
*                   others: Divided by 2*div, cal_clk = HSCK / (2*div)
* @return   none
*/
__STATIC_INLINE void CLK_SetClkTrimCalClkDiv(uint16_t div)
{
    uint32_t reg = CLK->MEAS_CLK_CTRL & ~CLK_MEASCLK_TRIM_CLK_DIV_Msk;
    CLK->MEAS_CLK_CTRL = reg | ((div << CLK_MEASCLK_TRIM_CLK_DIV_Pos) & CLK_MEASCLK_TRIM_CLK_DIV_Msk);
}

/**
 * @brief Clock divider settings.
 *
 * This enumeration defines the available flash clock divider settings for flash operations.
 */
enum {
    CLK_FLASH_CLKDIV_1  = 0x00, /*!< Divide-by-1 (No Division) */
    CLK_FLASH_CLKDIV_2  = 0x01, /*!< Divide-by-2 */
    CLK_FLASH_CLKDIV_4  = 0x02, /*!< Divide-by-4 */
    CLK_FLASH_CLKDIV_8  = 0x04, /*!< Divide-by-8 */
    CLK_FLASH_CLKDIV_16 = 0x08, /*!< Divide-by-16 */
    CLK_FLASH_CLKDIV_30 = 0x0F, /*!< Divide-by-30 */
};


/**
* @brief  This function is used to set flash clk divisor.
* @param[in]  div: clock divisor
* @return   none
*/
__STATIC_FORCEINLINE void CLK_SetFlashClkDiv(uint8_t div)
{
    uint32_t reg = CLK->AHB_CLK_CTRL & (~CLK_AHBCLK_SPI_FLASH_DIV_Msk);
    CLK->AHB_CLK_CTRL = reg | ((div << CLK_AHBCLK_SPI_FLASH_DIV_Pos) & CLK_AHBCLK_SPI_FLASH_DIV_Msk);
}

/**
  * @brief  This function get HCLK frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetHCLKFreq(void);

 /**
   * @brief  This function get CPU frequency. The frequency unit is Hz.
   * @return CPU frequency
   */
uint32_t CLK_GetCPUFreq(void);

/**
  * @brief  This function get APB1 frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetPCLK1Freq(void);

/**
  * @brief  This function get APB2 frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetPCLK2Freq(void);

 /**
    * @brief  This function set 16M ref clock source
    * @param[in]  u32ClkSrc is HCLK clock source. Including :
    *                  - \ref CLK_SYS_SRCSEL_RCH 
    *                  - \ref CLK_SYS_SRCSEL_XTH 
    *                  - \ref CLK_SYS_SRCSEL_DPLL
    * @return None
    */
void CLK_RefClkSrcConfig(uint32_t u32ClkSrc);
/**
  * @brief  This function set HCLK clock source
  * @param[in]  u32ClkSrc is HCLK clock source. Including :
  *                  - \ref CLK_DPLL_REF_CLKSEL_RCH
  *                  - \ref CLK_DPLL_REF_CLKSEL_XTH
  * @param[in]  freq_out is output frequence clock
  * @return None
  */
void CLK_SYSCLKConfig(uint32_t u32ClkSrc,uint32_t freq_out);
/**
  * @brief  This function set CPU frequency divider. The frequency unit is Hz.
  * @param[in]  u32ClkDiv is ahb clock division
  * @return HCLK frequency
  */
void CLK_HCLKConfig(uint32_t u32ClkDiv);
/**
  * @brief  This function set APB1 frequency divider. The frequency unit is Hz.
  * @param[in]  u32ClkDiv is is APB1 clock division
  * @return HCLK frequency
  */
void CLK_PCLK1Config(uint32_t u32ClkDiv );
/**
  * @brief  This function set APB2 frequency divider. The frequency unit is Hz.
  * @param[in]  u32ClkDiv is is APB2 clock division  
  * @return HCLK frequency
  */
void CLK_PCLK2Config(uint32_t u32ClkDiv);

/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it. 
  * @param  CLK_AHBPeriph: specifies the AHB2 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *             \ref AHB_PERIPHERAL_CLK_FLAG
  * @param  NewState    : new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_AHBPeriphClockCmd(uint32_t CLK_AHBPeriph, FunctionalState NewState);

/**
  * @brief  Enables or disables the Low Speed APB (APB1) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it. 
  * @param  CLK_APB1Periph: specifies the APB1 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *            \ref APB1_PERIPHERAL_CLK_FLAG
  * @param  NewState  : new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_APB1PeriphClockCmd(uint32_t CLK_APB1Periph, FunctionalState NewState);

/**
  * @brief  Enables or disables the High Speed APB (APB2) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it.
  * @param  CLK_APB2Periph: specifies the APB2 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *            \ref APB2_PERIPHERAL_CLK_FLAG 
  * @param  NewState: new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_APB2PeriphClockCmd(uint32_t CLK_APB2Periph, FunctionalState NewState);

/**
  * @brief  This function check selected clock source status
  * @param[in]  u32ClkMask is selected clock source. Including
  *                  - \ref CLK_RCL_SELECT
  *                  - \ref CLK_RCH_SELECT
  *                  - \ref CLK_DPLL_SELECT
  * @retval   0  clock is not stable
  * @retval   1  clock is stable
  *
  * @details  To wait for clock ready by specified CLKSTATUS bit or timeout (~5ms)
  */
uint32_t CLK_WaitClockReady(uint32_t u32ClkMask);

/**
  * @brief  This function wait sync 3v clock locale stable
  * @param[in]  none
  * @retval   0  clock sync is not stable
  * @retval   1  clock sync is stable
  *
  * @details  To wait for clock ready by specified CLKSTATUS bit or timeout (~5ms)
  */
uint32_t CLK_Wait3vSyncReady(void);

/**
* @brief  This function wait sync 3v clock locale stable by hardware
* @return   none
*/
void CLK_Set3vSyncAuto(void);
/**
  * @brief This API is used to select wdt clock source
  * @param[in] u32clksel wdt clock selection. Could be 
  *             - \ref CLK_APB1_WDTSEL_MILLI_PULSE, \ref CLK_APB1_WDTSEL_RCL32K
  * @return none
  * @note This API is only used to select wdt clock source
  */
void CLK_SetWdtClkSrc(uint32_t u32clksel);

/**
  * @brief This API is used to select wwdt clock source
  * @param[in] u32clksel wwdt clock selection. Could be 
  *             - \ref CLK_APB1_WWDTSEL_MILLI_PULSE, \ref CLK_APB1_WWDTSEL_RCL32K
  * @return none
  * @note This API is only used to select wwdt clock source
  */
void CLK_SetWwdtClkSrc(uint32_t u32clksel);

/**
  * @brief This API is used to SELECT timer clock source
  * @param[in] timer The base address of Timer module
  * @param[in] u32clksel timer clock selection. Could be 
  *             - \ref CLK_APB_TMR0SEL_APB1CLK, \ref CLK_APB_TMR0SEL_LIRC,\ref CLK_APB_TMR0SEL_TM0
  *             - \ref CLK_APB_TMR1SEL_APB2CLK, \ref CLK_APB_TMR1SEL_LIRC,\ref CLK_APB_TMR0SEL_TM1
  *             - \ref CLK_APB_TMR2SEL_APB2CLK, \ref CLK_APB_TMR2SEL_LIRC,\ref CLK_APB_TMR0SEL_TM2
  * @return none
  * @note This API is only used to select timer clock source
  */
void CLK_SetTmrClkSrc(TIMER_T *timer, uint32_t u32clksel);

/**
  * @brief This API is used to select pwm counting clock source
  * @param[in] u32ChNum   pwm channel. Could be 0 ~ 7
  * @param[in] u32clksel  pwm counting clock selection. Could be
  *                       - \ref CLK_APB1_PWM_CH01_SEL_APB, \ref CLK_APB1_PWM_CH01_SEL_CLK32K
  *                       - \ref CLK_APB1_PWM_CH23_SEL_APB, \ref CLK_APB1_PWM_CH23_SEL_CLK32K
  *                       - \ref CLK_APB1_PWM_CH45_SEL_APB, \ref CLK_APB1_PWM_CH45_SEL_CLK32K
  *                       - \ref CLK_APB1_PWM_CH67_SEL_APB, \ref CLK_APB1_PWM_CH67_SEL_CLK32K
  * @return none
  * @note PWM channel 0/1, 2/3, 4/5, 6/7 share same clock source.
  */
void CLK_SetPwmClkSrc(uint32_t u32ChNum, uint32_t u32clksel);

/**
  * @brief This API is used to get peripheral clk frequence
  * @param[in] Peripheral The base address of peripheral module
  * @return none
  * @note This API is only used to select timer clock source
  */
uint32_t CLK_GetPeripheralFreq(void * Peripheral);

/**
 * @brief Reset the entire chip.
 *
 * This function resets the entire chip.
 *
 * @return None
 */
void CLK_ResetChip(void);

/**
 * @brief Reset the system to ROM mode.
 *
 * This function resets the system to ROM mode.
 *
 * @return None
 */
void CLK_ResetSystemToRomMode(void);

/**
 * @brief Reset a specified module.
 *
 * This function resets a specified module based on the provided module index.
 *
 * @param[in] u32ModuleIndex The index of the module to reset.
 *
 * @return None
 */
void CLK_ResetModule(uint32_t u32ModuleIndex);

/**
 * @brief Configure the BOD (Brown-Out Detector) with the specified mode and BOD level.
 *
 * This function configures the BOD with the specified mode and BOD level.
 *
 * @param[in] i32Mode The BOD mode.
 * @param[in] u32BODLevel The BOD level.
 *
 * @return None
 */
void CLK_ConfigureBOD(int32_t i32Mode, uint32_t u32BODLevel);

/**
 * @brief Enable the BOD (Brown-Out Detector).
 *
 * This function enables the BOD.
 *
 * @return None
 */
void CLK_EnableBOD(void);

/**
 * @brief Disable the BOD (Brown-Out Detector).
 *
 * This function disables the BOD.
 *
 * @return None
 */
void CLK_DisableBOD(void);

/**
 * @brief Get the BOD (Brown-Out Detector) status.
 *
 * This function returns the status of the BOD.
 *
 * @return The BOD status.
 */
uint32_t CLK_GetBODStatus(void);

/**
 * @brief Get the BOD (Brown-Out Detector) interrupt flag.
 *
 * This function returns the BOD interrupt flag.
 *
 * @return The BOD interrupt flag.
 */
uint32_t CLK_GetBODIntFlag(void);

/**
 * @brief Clear the BOD (Brown-Out Detector) interrupt flag.
 *
 * This function clears the BOD interrupt flag.
 *
 * @return None
 */
void CLK_ClearBODIntFlag(void);

/**
 * @brief Set the BOD (Brown-Out Detector) debounce time.
 *
 * This function sets the BOD debounce time.
 *
 * @param[in] u32BodDBTime The BOD debounce time to set.
 *
 * @return None
 */
void CLK_SetBODDebounceTime(uint32_t u32BodDBTime);

/**
 * @brief Enable the LVR (Low-Voltage Reset).
 *
 * This function enables the LVR.
 *
 * @return None
 */
void CLK_EnableLVR(void);

/**
 * @brief Disable the LVR (Low-Voltage Reset).
 *
 * This function disables the LVR.
 *
 * @return None
 */
void CLK_DisableLVR(void);

/**
 * @brief Set the LVR (Low-Voltage Reset) debounce time.
 *
 * This function sets the LVR debounce time.
 *
 * @param[in] u32LvrDBTime The LVR debounce time to set.
 *
 * @return None
 */
void CLK_SetLVRDebounceTime(uint32_t u32LvrDBTime);



/**@} */

#ifdef __cplusplus
}
#endif

#endif //__PAN_CLK_H__

/*** (C) COPYRIGHT 2023 Panchip Technology Corp. ***/
