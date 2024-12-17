/**************************************************************************
 * @file     sys.h
 * @version  V1.00
 * $Revision: 15 $
 * $Date: 2023/11/08 $  
 * @brief    Panchip series SYS driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __PAN_SYS_H__
#define __PAN_SYS_H__


/**
 * @brief System Interface
 * @defgroup system_interface System Interface
 * @{
 */
#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @defgroup MULTI_FUNC_FLAG Io function define
 * @brief       Io function define constant definitions
 * @{ 
 */
#define SYS_MFP_TYPE_Msk(bit)       (1UL << ((bit) +16)) /*!< TYPE mask for Multiple Function Port */
#define SYS_MFP_ALT_Msk(bit)        (1UL << ((bit) + 8)) /*!< ALT mask for Multiple Function Port */
#define SYS_MFP_MFP_Msk(bit)        (1UL << ((bit)    )) /*!< MFP mask for Multiple Function Port */

#define SYS_MFP_GPIO                    0x00000000UL

/*P0*/
#define SYS_MFP_P00_GPIO				0x00000000UL
#define SYS_MFP_P00_SWD_CLK				0x00000001UL
#define SYS_MFP_P00_UART1_RX			0x00000100UL
#define SYS_MFP_P00_I2C0_SCL			0x00000101UL
#define SYS_MFP_P00_SPI0_CLK			0x00010000UL
#define SYS_MFP_P00_LL_DBG_14			0x00010001UL
#define SYS_MFP_P00_MDM_DBG_11			0x00010100UL
#define SYS_MFP_P00_RESERVED			0x00010101UL
#define SYS_MFP_P00_Msk					0x00010101UL

#define SYS_MFP_P01_GPIO				0x00000000UL
#define SYS_MFP_P01_SWD_DAT				0x00000002UL
#define SYS_MFP_P01_UART1_TX			0x00000200UL
#define SYS_MFP_P01_I2C0_SDA			0x00000202UL
#define SYS_MFP_P01_SPI0_CS				0x00020000UL
#define SYS_MFP_P01_LL_DBG_15			0x00020002UL
#define SYS_MFP_P01_MDM_DBG_12			0x00020200UL
#define SYS_MFP_P01_RESERVED			0x00020202UL
#define SYS_MFP_P01_Msk					0x00020202UL

#define SYS_MFP_P02_GPIO				0x00000000UL
#define SYS_MFP_P02_UART0_CTS			0x00000004UL
#define SYS_MFP_P02_SPI1_MISO			0x00000400UL
#define SYS_MFP_P02_PWM_CH3				0x00000404UL
#define SYS_MFP_P02_UART1_CTS			0x00040000UL
#define SYS_MFP_P02_LL_DBG_8			0x00040004UL
#define SYS_MFP_P02_MDM_DBG_19			0x00040400UL
#define SYS_MFP_P02_RESERVED			0x00040404UL
#define SYS_MFP_P02_Msk					0x00040404UL

#define SYS_MFP_P03_GPIO				0x00000000UL
#define SYS_MFP_P03_UART0_CTS			0x00000008UL
#define SYS_MFP_P03_SPI0_CS				0x00000800UL
#define SYS_MFP_P03_PWM_CH2				0x00000808UL
#define SYS_MFP_P03_TIMER0_CNT_OUT		0x00080000UL
#define SYS_MFP_P03_LL_DBG_8			0x00080008UL
#define SYS_MFP_P03_MDM_DBG_5			0x00080800UL
#define SYS_MFP_P03_RESERVED            0x00080808UL
#define SYS_MFP_P03_Msk                 0x00080808UL

#define SYS_MFP_P04_GPIO				0x00000000UL
#define SYS_MFP_P04_UART0_RTS			0x00000010UL
#define SYS_MFP_P04_SPI0_CLK			0x00001000UL
#define SYS_MFP_P04_PWM_CH3				0x00001010UL
#define SYS_MFP_P04_TIMER0_EXT			0x00100000UL
#define SYS_MFP_P04_LL_DBG_9			0x00100010UL
#define SYS_MFP_P04_MDM_DBG_6			0x00101000UL
#define SYS_MFP_P04_RESERVED			0x00101010UL
#define SYS_MFP_P04_Msk					0x00101010UL	/* func act as ADC CH0 if io is in analog mode */

#define SYS_MFP_P05_GPIO				0x00000000UL
#define SYS_MFP_P05_UART0_TX			0x00000020UL
#define SYS_MFP_P05_SPI0_MISO			0x00002000UL
#define SYS_MFP_P05_PWM_CH4				0x00002020UL
#define SYS_MFP_P05_EXT_STADC			0x00200000UL
#define SYS_MFP_P05_LL_DBG_12			0x00200020UL
#define SYS_MFP_P05_MDM_DBG_9			0x00202000UL
#define SYS_MFP_P05_RESERVED			0x00202020UL
#define SYS_MFP_P05_Msk					0x00202020UL

#define SYS_MFP_P06_GPIO				0x00000000UL
#define SYS_MFP_P06_UART0_RX			0x00000040UL
#define SYS_MFP_P06_SPI0_MISO			0x00004000UL
#define SYS_MFP_P06_PWM_CH5				0x00004040UL
#define SYS_MFP_P06_TIMER1_EXT			0x00400000UL
#define SYS_MFP_P06_LL_DBG_13			0x00400040UL
#define SYS_MFP_P06_MDM_DBG_3			0x00404000UL
#define SYS_MFP_P06_AHB_CLK				0x00404040UL
#define SYS_MFP_P06_Msk					0x00404040UL

#define SYS_MFP_P07_GPIO				0x00000000UL
#define SYS_MFP_P07_UART1_RX			0x00000080UL
#define SYS_MFP_P07_I2C0_SCL			0x00008000UL
#define SYS_MFP_P07_SPI0_MOSI			0x00008080UL
#define SYS_MFP_P07_PWM_CH0				0x00800000UL
#define SYS_MFP_P07_LL_DBG_10			0x00800080UL
#define SYS_MFP_P07_MDM_DBG_7			0x00808000UL
#define SYS_MFP_P07_RESERVED			0x00808080UL
#define SYS_MFP_P07_Msk					0x00808080UL	/* func act as ADC CH5 if io is in analog mode */

/*P1*/
#define SYS_MFP_P10_GPIO				0x00000000UL
#define SYS_MFP_P10_UART1_TX			0x00000001UL
#define SYS_MFP_P10_I2C0_SDA			0x00000100UL
#define SYS_MFP_P10_SPI0_MISO			0x00000101UL
#define SYS_MFP_P10_PWM_CH5				0x00010000UL
#define SYS_MFP_P10_LL_DBG_11			0x00010001UL
#define SYS_MFP_P10_MDM_DBG_8			0x00010100UL
#define SYS_MFP_P10_RESERVED			0x00010101UL
#define SYS_MFP_P10_Msk					0x00010101UL

#define SYS_MFP_P11_GPIO				0x00000000UL
#define SYS_MFP_P11_UART1_RTS			0x00000002UL
#define SYS_MFP_P11_SPI0_MOSI			0x00000200UL
#define SYS_MFP_P11_PWM_CH7				0x00000202UL
#define SYS_MFP_P11_CLK_32K				0x00020000UL
#define SYS_MFP_P11_LL_DBG_9			0x00020002UL
#define SYS_MFP_P11_MDM_DBG_0			0x00020200UL
#define SYS_MFP_P11_UART0_TX			0x00020202UL
#define SYS_MFP_P11_Msk					0x00020202UL

#define SYS_MFP_P12_GPIO				0x00000000UL
#define SYS_MFP_P12_UART0_RX			0x00000004UL
#define SYS_MFP_P12_PWM_CH4				0x00000400UL
#define SYS_MFP_P12_TIMER0_CNT_OUT		0x00000404UL
#define SYS_MFP_P12_UART1_TX			0x00040000UL
#define SYS_MFP_P12_LL_DBG_4			0x00040004UL
#define SYS_MFP_P12_MDM_DBG_17			0X00040400UL
#define SYS_MFP_P12_SPI0_MISO			0x00040404UL
#define SYS_MFP_P12_Msk					0x00040404UL	/* func act as ADC CH3 if io is in analog mode */

#define SYS_MFP_P13_GPIO				0x00000000UL
#define SYS_MFP_P13_UART0_RTS			0x00000008UL
#define SYS_MFP_P13_I2C0_SDA			0x00000800UL
#define SYS_MFP_P13_PWM_CH3				0x00000808UL
#define SYS_MFP_P13_LL_DBG_3			0x00080000UL
#define SYS_MFP_P13_MDM_DBG_16			0x00080008UL
#define SYS_MFP_P13_SPI1_CS				0x00080800UL
#define SYS_MFP_P13_SPI0_CS				0x00080808UL
#define SYS_MFP_P13_Msk					0x00080808UL	/* func act as USB DM if io is in analog mode */

#define SYS_MFP_P14_GPIO				0x00000000UL
#define SYS_MFP_P14_UART0_CTS			0x00000010UL
#define SYS_MFP_P14_I2C0_SCL			0x00001000UL
#define SYS_MFP_P14_PWM_CH2				0x00001010UL
#define SYS_MFP_P14_LL_DEBUG_2			0x00100000UL
#define SYS_MFP_P14_MDM_DBG_15			0x00100010UL
#define SYS_MFP_P14_SPI0_CLK			0x00101000UL
#define SYS_MFP_P14_RESERVED			0x00101010UL
#define SYS_MFP_P14_Msk					0x00101010UL	/* func act as USB DP if io is in analog mode */

#define SYS_MFP_P15_GPIO				0x00000000UL
#define SYS_MFP_P15_SPI0_CS				0x00000020UL
#define SYS_MFP_P15_PWM_CH5				0x00002000UL
#define SYS_MFP_P15_TIMER0_EXT			0x00002020UL
#define SYS_MFP_P15_UART0_RX			0x00200000UL
#define SYS_MFP_P15_LL_DBG_5			0x00200020UL
#define SYS_MFP_P15_MDM_DBG_18			0x00202000UL
#define SYS_MFP_P15_RESERVED			0x00202020UL
#define SYS_MFP_P15_Msk					0x00202020UL

#define SYS_MFP_P16_GPIO				0x00000000UL
#define SYS_MFP_P16_UART0_TX			0x00000040UL
#define SYS_MFP_P16_TIMER2_CNT_OUT		0x00004000UL
#define SYS_MFP_P16_PWM_CH0				0x00004040UL
#define SYS_MFP_P16_LL_DBG_0			0x00400000UL
#define SYS_MFP_P16_MDM_DBG_13			0x00400040UL
#define SYS_MFP_P16_SPI1_CLK			0x00404000UL
#define SYS_MFP_P16_I2C0_SCL			0x00404040UL
#define SYS_MFP_P16_Msk					0x00404040UL	/* func act as HOSC EXT if io is in analog mode */

#define SYS_MFP_P17_GPIO				0x00000000UL
#define SYS_MFP_P17_UART0_RX			0x00000080UL
#define SYS_MFP_P17_TIMER2_EXT			0x00008000UL
#define SYS_MFP_P17_PWM_CH1				0x00008080UL
#define SYS_MFP_P17_LL_DBG_1			0x00800000UL
#define SYS_MFP_P17_MDM_DBG_14			0x00800080UL
#define SYS_MFP_P17_RCH					0x00808000UL
#define SYS_MFP_P17_RESERVED			0x00808080UL
#define SYS_MFP_P17_Msk					0x00808080UL	/* func act as LOSC EXT if io is in analog mode */

/*P2*/
#define SYS_MFP_P20_GPIO				0x00000000UL
#define SYS_MFP_P20_SPI1_MISO			0x00000001UL
#define SYS_MFP_P20_PWM_CH6				0x00000100UL
#define SYS_MFP_P20_TADC_CLK			0x00000101UL
#define SYS_MFP_P20_LL_DBG_6			0x00010000UL
#define SYS_MFP_P20_MDM_DBG_2			0x00010001UL
#define SYS_MFP_P20_XTL_C1_CLK			0x00010100UL
#define SYS_MFP_P20_RESERVED			0x00010101UL
#define SYS_MFP_P20_Msk					0x00010101UL	/* func act as XTL C2 if io is in analog mode */

#define SYS_MFP_P21_GPIO				0x00000000UL
#define SYS_MFP_P21_SPI1_MOSI			0x00000002UL
#define SYS_MFP_P21_PWM_CH7				0x00000200UL
#define SYS_MFP_P21_TADC_VLD			0x00000202UL
#define SYS_MFP_P21_LL_DBG_7			0x00020000UL
#define SYS_MFP_P21_MDM_DBG_1			0x00020002UL
#define SYS_MFP_P21_XTL_C2_CLK			0x00020200UL
#define SYS_MFP_P21_RESERVED			0x00020202UL
#define SYS_MFP_P21_Msk					0x00020202UL	/* func act as XTL C1 if io is in analog mode */

#define SYS_MFP_P22_GPIO				0x00000000UL
#define SYS_MFP_P22_SPI1_CLK			0x00000004UL
#define SYS_MFP_P22_PWM_CH0				0x00000400UL
#define SYS_MFP_P22_RESERVED			0x00000404UL
#define SYS_MFP_P22_TADC_DATl			0x00040000UL
#define SYS_MFP_P22_MDM_DBG_0			0x00040004UL
#define SYS_MFP_P22_SPI0_MOSI			0x00040400UL
#define SYS_MFP_P22_RESERVED1			0x00040404UL
#define SYS_MFP_P22_Msk					0x00040404UL	/* func act as ADC CH2 & DFT IP & DFT_V if io is in analog mode */

#define SYS_MFP_P23_GPIO				0x00000000UL
#define SYS_MFP_P23_SPI1_CS				0x00000008UL
#define SYS_MFP_P23_PWM_CH1				0x00000800UL
#define SYS_MFP_P23_DPLL_DIV8			0x00000808UL
#define SYS_MFP_P23_TADC_DATH			0x00080000UL
#define SYS_MFP_P23_MDM_DBG_10			0x00080008UL
#define SYS_MFP_P23_I2C0_SCL			0x00080800UL
#define SYS_MFP_P23_RESERVED			0x00080808UL
#define SYS_MFP_P23_Msk					0x00080808UL	/* func act as ADC CH1 & DFT IN & DFT_I if io is in analog mode */

#define SYS_MFP_P24_GPIO				0x00000000UL
#define SYS_MFP_P24_SPI1_MISO			0x00000010UL
#define SYS_MFP_P24_UART1_RX			0x00001000UL
#define SYS_MFP_P24_PWM_CH2				0x00001010UL
#define SYS_MFP_P24_TIMER1_CNT_OUT		0x00100000UL
#define SYS_MFP_P24_MDM_DBG_4			0x00100010UL
#define SYS_MFP_P24_RESERVED			0x00101000UL
#define SYS_MFP_P24_RESERVED1			0x00101010UL
#define SYS_MFP_P24_Msk					0x00101010UL	/* func act as ADC CH6 & DFT QP if io is in analog mode */

#define SYS_MFP_P25_GPIO				0x00000000UL
#define SYS_MFP_P25_SPI1_MOSI			0x00000020UL
#define SYS_MFP_P25_UART1_TX			0x00002000UL
#define SYS_MFP_P25_PWM_CH3				0x00002020UL
#define SYS_MFP_P25_MDM_DBG_5			0x00200000UL
#define SYS_MFP_P25_RESERVED			0x00200020UL
#define SYS_MFP_P25_I2C0_SDA			0x00202000UL
#define SYS_MFP_P25_RESERVED1			0x00202020UL
#define SYS_MFP_P25_Msk					0x00202020UL	/* func act as ADC CH7 & DFT QN if io is in analog mode */

#define SYS_MFP_P26_GPIO				0x00000000UL
#define SYS_MFP_P26_UART1_RTS			0x00000040UL
#define SYS_MFP_P26_PWM_CH4				0x00004000UL
#define SYS_MFP_P26_SPI1_CS				0x00004040UL
#define SYS_MFP_P26_MDM_DBG_4			0x00400000UL
#define SYS_MFP_P26_MDM_DBG_10			0x00400040UL
#define SYS_MFP_P26_RESERVED			0x00404000UL
#define SYS_MFP_P26_RESERVED1			0x00404040UL
#define SYS_MFP_P26_Msk                 0x00404040UL

#define SYS_MFP_P27_GPIO				0x00000000UL
#define SYS_MFP_P27_UART1_CTS			0x00000080UL
#define SYS_MFP_P27_PWM_CH5				0x00008000UL
#define SYS_MFP_P27_SPI1_CLK			0x00008080UL
#define SYS_MFP_P27_RESERVED			0x00800000UL
#define SYS_MFP_P27_MDM_DBG_20			0x00800080UL
#define SYS_MFP_P27_XTH					0x00808000UL
#define SYS_MFP_P27_RESERVED1			0x00808080UL
#define SYS_MFP_P27_Msk					0x00808080UL

/*P3*/
#define SYS_MFP_P30_GPIO				0x00000000UL
#define SYS_MFP_P30_UART1_RX			0x00000001UL
#define SYS_MFP_P30_PWM_CH6				0x00000100UL
#define SYS_MFP_P30_SPI1_MISO			0x00000101UL
#define SYS_MFP_P30_MDM_DBG_10			0x00010000UL
#define SYS_MFP_P30_RESERVED			0x00010001UL
#define SYS_MFP_P30_RESERVED1			0x00010100UL
#define SYS_MFP_P30_RESERVED2			0x00010101UL
#define SYS_MFP_P30_Msk					0x00010101UL

#define SYS_MFP_P31_GPIO				0x00000000UL
#define SYS_MFP_P31_UART1_TX			0x00000002UL
#define SYS_MFP_P31_PWM_CH7				0x00000200UL
#define SYS_MFP_P31_SPI1_MOSI			0x00000202UL
#define SYS_MFP_P31_RESERVED			0x00020000UL
#define SYS_MFP_P31_RESERVED1			0x00020002UL
#define SYS_MFP_P31_RESERVED2			0x00020200UL
#define SYS_MFP_P31_RESERVED3			0x00020202UL
#define SYS_MFP_P31_Msk					0x00020202UL
/* @}*///end of group  MULTI_FUNC_FLAG

/**
  * @brief      Set pin function
  * @param[in]   Port GPIO port number.
  * @param[in]   Bit  The single or multiple pins of specified gpio port.
  * @param[in]   Func  pin function
  * @return     None
  */
#define SYS_SET_MFP(Port, Bit, Func)     \
            (SYS->Port##_MFP = (SYS->Port##_MFP & ~SYS_MFP_##Port##Bit##_Msk) | SYS_MFP_##Port##Bit##_##Func)




/**
  * @brief      Disable register write-protection function
  * @param      None
  * @return     None
  * @details    This function disable register write-protection function.
  *             To unlock the protected register to allow write access.
  */
__STATIC_INLINE void SYS_UnlockReg(void)
{
    do
     {
        SYS->REGLCTL = 0x59;
        SYS->REGLCTL = 0x16;
        SYS->REGLCTL = 0x88;
    }
    while(SYS->REGLCTL == 0);
}

/**
  * @brief      Enable register write-protection function
  * @return     None
  * @details    This function is used to enable register write-protection function.
  *             To lock the protected register to forbid write access.
  */
__STATIC_INLINE void SYS_LockReg(void)
{
    // (Workaround) To insure write-protected reg
    // is succefully wrote before lock
#if 1
    __NOP();__NOP();__NOP();__NOP();__NOP();
#else
    __ISB();
#endif
    SYS->REGLCTL = 0;
}

/**
  * @brief  This function check register write-protection bit setting
  * @return 0: Write-protection function is disabled.
  *         1: Write-protection function is enabled.
  */
uint32_t SYS_IsRegLocked(void);

/**
  * @brief       Set delay time
  * @param[in]   u32NopCnt nop count.
  * @return     None
  */
void SYS_delay_10nop(uint32_t u32NopCnt);

/*@}*/


#ifdef __cplusplus
}
#endif

#endif //__PAN_SYS_H__
