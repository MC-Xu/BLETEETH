/**************************************************************************//**
 * @file     clk.c
 * @version  V1.00
 * $Date: 16/02/22 9:39a $
 * @brief    Panchip series CLK driver source file
 *
 * @note
 * Copyright (C) 2016 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#include "PanSeries.h"
#include "pan_clk.h"
#include "pan_sys.h"

/** @addtogroup Panchip_Device_Driver Panchip Device Driver
  @{
*/

/** @addtogroup Panchip_CLK_Driver CLK Driver
  @{
*/


/** @addtogroup Panchip_CLK_EXPORTED_FUNCTIONS CLK Exported Functions
  @{
*/

/**
  * @brief  This function get HCLK frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetHCLKFreq(void)
{
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

/**
  * @brief  This function get CPU frequency. The frequency unit is Hz.
  * @return CPU frequency
  */
uint32_t CLK_GetCPUFreq(void)
{
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

/**
  * @brief  This function get APB1 frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetPCLK1Freq(void)
{
    uint32_t ahbclock,apb1_clock;
    uint32_t apb1_div;
    
    // update ahb clock
    SystemCoreClockUpdate();
    ahbclock =  SystemCoreClock;
    
    apb1_div = (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_APB1_DIV_Msk) >> CLK_TOPCTL_APB1_DIV_Pos;
    if(apb1_div)
			apb1_clock = ahbclock / (2 * apb1_div);
		else
			apb1_clock = ahbclock;
    
    return apb1_clock;
}

/**
  * @brief  This function get APB2 frequency. The frequency unit is Hz.
  * @return HCLK frequency
  */
uint32_t CLK_GetPCLK2Freq(void)
{
    uint32_t ahbclock,apb2_clock;
    uint32_t apb2_div;
    
    // update ahb clock
    SystemCoreClockUpdate();
    ahbclock =  SystemCoreClock;
    
    apb2_div = (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_APB2_DIV_Msk) >> CLK_TOPCTL_APB2_DIV_Pos;
    if(apb2_div)
			apb2_clock = ahbclock / (2 * apb2_div);
		else
			apb2_clock = ahbclock;
    
    return apb2_clock;
}

/**
  * @brief  This function set 16M ref clock source
  * @param[in]  u32ClkSrc is HCLK clock source. Including :
  *                  - \ref CLK_SYS_SRCSEL_RCH 
  *                  - \ref CLK_SYS_SRCSEL_XTH 
  *                  - \ref CLK_SYS_SRCSEL_RCL 
  *                  - \ref CLK_SYS_SRCSEL_XTL 
  *                  - \ref CLK_SYS_SRCSEL_DPLL
  * @return None
  */
void CLK_RefClkSrcConfig(uint32_t u32ClkSrc)
{
    uint32_t tmpreg = CLK->CLK_TOP_CTRL_3V;
    tmpreg &= ~CLK_TOPCTL_SYS_CLK_SEL_Msk;
    switch(u32ClkSrc)
    {
        case CLK_SYS_SRCSEL_RCH: 
            CLK->RCH_CTRL |= CLK_RCHCTL_RCH_EN_Msk;
#ifdef SYNC_3V_REG_MANUALLY
            CLK_Wait3vSyncReady();
#endif
			CLK_WaitClockReady(u32ClkSrc);
            tmpreg |= CLK_SYS_SRCSEL_RCH;
            break;
        case CLK_SYS_SRCSEL_XTH: 
			CLK_XthStartupConfig();
            CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
#ifdef SYNC_3V_REG_MANUALLY
            CLK_Wait3vSyncReady();
#endif
			CLK_WaitClockReady(u32ClkSrc);
            tmpreg |= CLK_SYS_SRCSEL_XTH;
            break;
        case CLK_SYS_SRCSEL_DPLL: 
            CLK->DPLL_CTRL |= CLK_DPLLCTL_DPLL_EN_Msk;
#ifdef SYNC_3V_REG_MANUALLY
            CLK_Wait3vSyncReady();
#endif
			CLK_WaitClockReady(u32ClkSrc);
            tmpreg |= CLK_SYS_SRCSEL_DPLL;
            break;
        default: break;
    }

	CLK->CLK_TOP_CTRL_3V = tmpreg;
    //wait xtal clock stable
//    CLK_WaitClockReady(u32ClkSrc);
	SystemCoreClockUpdate();
}

/**
  * @brief  This function set HCLK clock source
  * @param[in]  u32ClkSrc is HCLK clock source. Including :
  *                  - \ref CLK_DPLL_REF_CLKSEL_RCH
  *                  - \ref CLK_DPLL_REF_CLKSEL_XTH
  * @return None
  */
void CLK_SYSCLKConfig(uint32_t u32ClkSrc,uint32_t freq_out)
{
    if(u32ClkSrc == CLK_DPLL_REF_CLKSEL_RCH){
        CLK->DPLL_CTRL &= ~CLK_DPLLCTL_RCLK_SEL_Msk;
    }
    else if(u32ClkSrc == CLK_DPLL_REF_CLKSEL_XTH){
        CLK->DPLL_CTRL |= CLK_DPLLCTL_RCLK_SEL_Msk;
    }
    
    /* Config MCU DPLL*/         
	CLK_SetDpllOutputFreq(freq_out);
    //enable dpll clock
    CLK->DPLL_CTRL |= CLK_DPLLCTL_DPLL_EN_Msk;
#ifdef SYNC_3V_REG_MANUALLY
    CLK_Wait3vSyncReady();
#endif
    CLK_WaitClockReady(CLK_SYS_SRCSEL_DPLL);

    /* Update System Core Clock */
//    SystemCoreClockUpdate();
}

/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it. 
  * @param  CLK_AHBPeriph: specifies the AHB2 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *            @arg CLK_AHBPeriph_DMAC   
  *            @arg CLK_AHBPeriph_GPIO   
  *            @arg CLK_AHBPeriph_SYSTICK
  *            @arg CLK_AHBPeriph_APB1   
  *            @arg CLK_AHBPeriph_APB2   
  *            @arg CLK_AHBPeriph_AHB    
  *            @arg CLK_AHBPeriph_RF 
  *            @arg CLK_AHBPeriph_All
  * @param  NewState    : new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_AHBPeriphClockCmd(uint32_t CLK_AHBPeriph, FunctionalState NewState)
{
    (NewState != DISABLE)?(CLK->AHB_CLK_CTRL |= CLK_AHBPeriph):(CLK->AHB_CLK_CTRL &= ~CLK_AHBPeriph);
}

/**
  * @brief  Enables or disables the Low Speed APB (APB1) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it. 
  * @param  CLK_APB1Periph: specifies the APB1 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *            @arg CLK_APB1Periph_I2C0              
  *            @arg CLK_APB1Periph_SPI0       
  *            @arg CLK_APB1Periph_UART0     
  *            @arg CLK_APB1Periph_PWM01    
  *            @arg CLK_APB1Periph_PWM23      
  *            @arg CLK_APB1Periph_PWM45  
  *            @arg CLK_APB1Periph_PWM67   
  *            @arg CLK_APB1Periph_PWMEN    
  *            @arg CLK_APB1Periph_ADC    
  *            @arg CLK_APB1Periph_WDT   
  *            @arg CLK_APB1Periph_WWDT    
  *            @arg CLK_APB1Periph_TMR0    
  *            @arg CLK_APB1Periph_All
  * @param  NewState  : new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_APB1PeriphClockCmd(uint32_t CLK_APB1Periph, FunctionalState NewState)
{
  (NewState != DISABLE)?(CLK->APB1_CLK_CTRL |= CLK_APB1Periph):(CLK->APB1_CLK_CTRL &= ~CLK_APB1Periph);
}

/**
  * @brief  Enables or disables the High Speed APB (APB2) peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before 
  *         using it.
  * @param  CLK_APB2Periph: specifies the APB2 peripheral to gates its clock.
  *          This parameter can be any combination of the following values:
  *            @arg CLK_APB2Periph_SPI1 
  *            @arg CLK_APB2Periph_UART1
  *            @arg CLK_APB2Periph_TMR1 
  *            @arg CLK_APB2Periph_TMR2 
  *            @arg CLK_APB2Periph_All  
  * @param  NewState: new state of the specified peripheral clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CLK_APB2PeriphClockCmd(uint32_t CLK_APB2Periph, FunctionalState NewState)
{
    (NewState != DISABLE)?(CLK->APB2_CLK_CTRL |= CLK_APB2Periph):(CLK->APB2_CLK_CTRL &= ~CLK_APB2Periph); 
}

/**
  * @brief  This function check selected clock source status
  * @param[in]  u32ClkMask is selected clock source. Including
  *                  - \ref CLK_RCL_SELECT
  *                  - \ref CLK_RCH_SELECT
  *                  - \ref CLK_XTL_SELECT
  *                  - \ref CLK_XTH_SELECT
  *                  - \ref CLK_DPLL_SELECT
  * @return   0  clock is not stable
  *           1  clock is stable
  *
  * @details  To wait for clock ready by specified CLKSTATUS bit or timeout (~5ms)
  */
uint32_t CLK_WaitClockReady(uint32_t u32ClkMask)
{
    int32_t i32TimeOutCnt = 2160000;
    uint32_t u32ClkTmp = 0;
    while(1) 
    {
		switch(u32ClkMask){
				case CLK_SYS_SRCSEL_RCH :    u32ClkTmp = CLK->RCH_CTRL; break;
				case CLK_SYS_SRCSEL_XTH :    u32ClkTmp = CLK->XTH_CTRL; break;
//				case CLK_SYS_SRCSEL_RCL :    u32ClkTmp = CLK->RCL_CTRL_3V; break;
//				case CLK_SYS_SRCSEL_XTL :    u32ClkTmp = CLK->XTL_CTRL_3V; break;
				case CLK_SYS_SRCSEL_DPLL:    u32ClkTmp = CLK->DPLL_CTRL; break;
				default: break;
		}		
        if((u32ClkTmp & CLK_STABLE_STATUS_Msk) != CLK_STABLE_STATUS_Msk){
            if(i32TimeOutCnt-- <= 0)
             return 0;
        }else{
            break;
        }  
    }
    return 1;
}

/**
  * @brief  This function wait sync 3v clock locale stable
  * @param[in]  none
  * @return   0  clock sync is not stable
  *           1  clock sync is stable
  *
  * @details  To wait for clock ready by specified CLKSTATUS bit or timeout (~5ms)
  */
uint32_t CLK_Wait3vSyncReady(void)
{
    // Trigger manual 3v sync
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;
    // Force delay 60us+ (should be larger than 31.25us)
    SYS_delay_10nop(200);

    return 1;
}

/**
  * @brief  This function wait sync 3v clock locale stable by hardware
  * @param[in]  none
  * @return   none
  */
void CLK_Set3vSyncAuto(void)
{
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk;
}

/**
  * @brief This API is used to select wdt clock source
  * @param[in] u32clksel wdt clock selection. Could be 
  *             - \ref CLK_APB1_WDTSEL_MILLI_PULSE, \ref CLK_APB1_WDTSEL_RCL32K
  * @return none
  * @note This API is only used to select wdt clock source
  */
void CLK_SetWdtClkSrc(uint32_t u32clksel)
{
    if (u32clksel == CLK_APB1_WDTSEL_MILLI_PULSE) {
        // Enable Mili Pulse Clock if WDT selects it
        u32clksel |= CLK_APB1Periph_MILI_CLK;
    }
    CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_WDTSRC_SEL_Msk) | u32clksel;
}

/**
  * @brief This API is used to select wwdt clock source
  * @param[in] wwdt The base address of wwdt module
  * @param[in] u32clksel wwdt clock selection. Could be 
  *             - \ref CLK_APB1_WDTSEL_MILLI_PULSE, \ref CLK_APB1_WDTSEL_RCL32K
  * @return none
  * @note This API is only used to select wwdt clock source
  */
void CLK_SetWwdtClkSrc(uint32_t u32clksel)
{
    if (u32clksel == CLK_APB1_WWDTSEL_MILLI_PULSE) {
        // Enable Mili Pulse Clock if WWDT selects it
        u32clksel |= CLK_APB1Periph_MILI_CLK;
    }
    CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_WWDTSRC_SEL_Msk) | u32clksel;
}

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
void CLK_SetTmrClkSrc(TIMER_T *timer, uint32_t u32clksel)
{
    if(timer == TIMER0) {        
        CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_TMR0SRC_SEL_Msk) | u32clksel;
    }
    else if(timer == TIMER1){
        CLK->APB2_CLK_CTRL = (CLK->APB2_CLK_CTRL & ~CLK_APB2CLK_TMR1SRC_SEL_Msk) | u32clksel;
    }
    else if(timer == TIMER2){
        CLK->APB2_CLK_CTRL = (CLK->APB2_CLK_CTRL & ~CLK_APB2CLK_TMR2SRC_SEL_Msk) | u32clksel;
    }
}

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
void CLK_SetPwmClkSrc(uint32_t u32ChNum, uint32_t u32clksel)
{
    switch (u32ChNum) {
    case 0:
    case 1:
        CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_PWM01_CLK_SEL_Msk) | u32clksel;
        break;
    case 2:
    case 3:
        CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_PWM23_CLK_SEL_Msk) | u32clksel;
        break;
    case 4:
    case 5:
        CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_PWM45_CLK_SEL_Msk) | u32clksel;
        break;
    case 6:
    case 7:
        CLK->APB1_CLK_CTRL = (CLK->APB1_CLK_CTRL & ~CLK_APB1CLK_PWM67_CLK_SEL_Msk) | u32clksel;
        break;
    default:
        break;
    }
}

uint32_t CLK_GetPeripheralFreq(void * Peripheral)
{
    uint8_t u8ApbDiv = 0;
	  uint32_t u32AhbClk,u32Pclk;

	/*get system clock frequency*/
    u32AhbClk = CLK_GetHCLKFreq(); 
    
    if((I2C0    == (I2C_T *)   Peripheral) 
     ||(UART0   == (UART_T *)  Peripheral)
     ||(SPI0    == (SPI_T *)   Peripheral)
     ||(TIMER0  == (TIMER_T *) Peripheral))
    {
        u8ApbDiv = (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_APB1_DIV_Msk) >> CLK_TOPCTL_APB1_DIV_Pos;
    }
    else if((SPI1    == (SPI_T *)    Peripheral) 
          ||(UART1   == (UART_T *)   Peripheral)
          ||(TIMER1  == (TIMER_T *)  Peripheral)
          ||(TIMER2  == (TIMER_T *)  Peripheral))
        u8ApbDiv = (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_APB2_DIV_Msk) >> CLK_TOPCTL_APB2_DIV_Pos;
    
    if(u8ApbDiv)
        u32Pclk = u32AhbClk / (2 * u8ApbDiv);
    else
        u32Pclk = u32AhbClk;
    return u32Pclk;
}

/**
  * @brief  This function clear the selected system reset source
  * @param[in]  u32Src is system reset source. Including:
  *                  - \ref CLK_RSTSTS_CHIPRF_Msk
  *                  - \ref CLK_RSTSTS_PINRF_Msk
  *                  - \ref CLK_RSTSTS_WDTRF_Msk
  *                  - \ref CLK_RSTSTS_LVRRF_Msk
  *                  - \ref CLK_RSTSTS_BODRF_Msk
  *                  - \ref CLK_RSTSTS_SYSRF_Msk
  *                  - \ref CLK_RSTSTS_PORRF_Msk
  *                  - \ref CLK_RSTSTS_CPURF_Msk
  * @return None
  */
void CLK_ClearResetSrc(uint32_t u32Src)
{
    CLK->RSTSTS |= u32Src;
}

/**
  * @brief  This function get the system reset source register value
  * @return Reset source
  */
uint32_t CLK_GetResetSrc(void)
{
    return (CLK->RSTSTS);
}

/**
  * @brief This function reset chip.
  * @details Setting this bit will reset the whole chip, including cpu core and all peripherals,
  *          and this bit will automatically return to 0. The CHIPRST is the same as the POR reset,
  *          all the chip controllers is reset and the chip settings from flash are also reload.
  * @note This bit is write protected. Do SYS_UnlockReg() before setting this bit.
  * @return None
  */
void CLK_ResetChip(void)
{
    CLK->IPRST0 |= CLK_IPRST0_CHIPRST_Msk;
    while (1) {
        /* Busy wait until SoC reset */
    }
}

/**
  * @brief  This function reset system from Flash Mode to ROM Mode (with Panchip USB DFU Enabled).
  * @details The FTOR reset is used to reset system and switch from flash mode to rom mode. If
  *          this bit is set, reset range is completely consistent with RCCRESETREQ (sys reset),
  *          and this bit will automatically return to 0.
  * @note This bit is write protected. Do SYS_UnlockReg() before setting this bit.
  * @return None
  */
void CLK_ResetSystemToRomMode(void)
{
    CLK->IPRST0 |= CLK_IPRST0_FTORRST_Msk;
    while (1) {
        /* Busy wait until SoC reset */
    }
}

/**
  * @brief  This function reset selected modules.
  * @param[in]  u32ModuleIndex is module index. Including :
  *                  - \ref DMA_RST
  *                  - \ref LL_RST
  *                  - \ref EFUSE_RST
  *                  - \ref USB_RST
  *                  - \ref MDMSTB_RST
  *                  - \ref MDM_RST
  *                  - \ref I2C0_RST
  *                  - \ref SPI0_RST
  *                  - \ref SPI1_RST
  *                  - \ref UART0_RST
  *                  - \ref UART1_RST
  *                  - \ref PWM0_RST
  *                  - \ref ADC_RST
  *                  - \ref WDT_RST
  *                  - \ref WWDT_RST
  *                  - \ref TMR0_RST
  *                  - \ref TMR1_RST
  *                  - \ref TMR2_RST
  *                  - \ref GPIO_RST
  *                  - \ref CLKTRIM_RST
  * @return None
  */
void CLK_ResetModule(uint32_t u32ModuleIndex)
{
    *(volatile uint32_t *)((uint32_t)&(CLK->IPRST0) + (u32ModuleIndex>>24)) |= 1<<(u32ModuleIndex & 0x00ffffff);
    *(volatile uint32_t *)((uint32_t)&(CLK->IPRST0) + (u32ModuleIndex>>24)) &= ~(1<<(u32ModuleIndex & 0x00ffffff));
}

/**
  * @brief  This function configure BOD function.
  *         Configure BOD reset or interrupt mode and set Brown-out voltage level.
  *         Enable Brown-out function
  * @param[in]  i32Mode is reset or interrupt mode. Including :
  *                  - \ref CLK_BODCTL_BOD_INT_EN
  *                  - \ref CLK_BODCTL_BOD_RST_EN
  * @param[in]  u32BODLevel is Brown-out voltage level. Including :
  *                  - \ref CLK_BODCTL_BODSEL_1_75V
  *                  - \ref CLK_BODCTL_BODSEL_1_95V
  *                  - \ref CLK_BODCTL_BODSEL_2_15V
  *                  - \ref CLK_BODCTL_BODSEL_2_35V
  *                  - \ref CLK_BODCTL_BODSEL_2_55V
  *                  - \ref CLK_BODCTL_BODSEL_2_75V
  *                  - \ref CLK_BODCTL_BODSEL_2_95V
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_ConfigureBOD(int32_t i32Mode, uint32_t u32BODLevel)
{
    uint32_t tmpreg = CLK->BODCTL_3V;
    tmpreg = (tmpreg & ~CLK_BODCTL_BODRSTEN_Msk_3v) | i32Mode;
    tmpreg = (tmpreg & ~CLK_BODCTL_BODSEL_Msk_3v) | u32BODLevel;
    CLK->BODCTL_3V = tmpreg;
}

/**
  * @brief  This function enable BOD function.
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_EnableBOD(void)
{
     CLK->BODCTL_3V |= CLK_BODCTL_BODEN_Msk_3v;
}

/**
  * @brief  This function disable BOD function.
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_DisableBOD(void)
{
     CLK->BODCTL_3V &= ~CLK_BODCTL_BODEN_Msk_3v;
}

/**
  * @brief  This function get Brown-out detector output status
  * @return 0: System voltage is higher than BOD_SEL setting or BODEN is 0.
  *         1: System voltage is lower than BOD_SEL setting.
  * @note   If the BODEN is 0, this function always return 0.
  */
uint32_t CLK_GetBODStatus(void)
{
    return (CLK->BODCTL_3V & CLK_BODCTL_BODOUT_Msk) ? 1 : 0;
}

/**
  * @brief      Get Brown-out detector interrupt flag
  * @retval     0   Brown-out detect interrupt flag is not set.
  * @retval     1   Brown-out detect interrupt flag is set.
  */
uint32_t CLK_GetBODIntFlag(void)
{
    return (CLK->BODCTL_3V & CLK_BODCTL_BODIF_Msk) ? 1 : 0;
}

/**
  * @brief      Clear Brown-out detector interrupt flag
  * @param      None
  * @return     None
  */
void CLK_ClearBODIntFlag(void)
{
    CLK->BODCTL_3V |= CLK_BODCTL_BODIF_Msk;
}

/**
  * @brief  This function set BOD debounce time.
  * @param[in]  u32BodDBTime is BOD debounce time in slow clock cycles. Including:
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW0
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW1
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW2
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW3
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW4
  *                  - \ref CLK_BLDBCTL_BODDBSEL_2POW5
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_SetBODDebounceTime(uint32_t u32BodDBTime)
{
    CLK->BLDBCTL_3V = (CLK->BLDBCTL_3V & ~CLK_BLDBCTL_BODDB_SEL_Msk_3v) | u32BodDBTime;
}

/**
  * @brief  This function enable LVR function.
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_EnableLVR(void)
{
     CLK->BODCTL_3V |= CLK_BODCTL_LVREN_Msk_3v;
}

/**
  * @brief  This function disable LVR function.
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_DisableLVR(void)
{
     CLK->BODCTL_3V &= ~CLK_BODCTL_LVREN_Msk_3v;
}

/**
  * @brief  This function set LVR debounce time.
  * @param[in]  u32LvrDBTime is LVR debounce time in slow clock cycles. Including:
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW0
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW1
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW2
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW3
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW4
  *                  - \ref CLK_BLDBCTL_LVRDBSEL_2POW5
  * @note This function operates the 3v-always-on register, please ensure the auto-3v-sync mechanism is on, otherwise
  *       you need to do the high-voltage-sync manually.
  * @return None
  */
void CLK_SetLVRDebounceTime(uint32_t u32LvrDBTime)
{
    CLK->BLDBCTL_3V = (CLK->BLDBCTL_3V & ~CLK_BLDBCTL_LVRDB_SEL_Msk_3v) | u32LvrDBTime;
}

/*@}*/ /* end of group Panchip_CLK_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group Panchip_CLK_Driver */

/*@}*/ /* end of group Panchip_Device_Driver */

/*** (C) COPYRIGHT 2016 Panchip Technology Corp. ***/
