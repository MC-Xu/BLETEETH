/**************************************************************************//**
 * @file     system_PanSeries.c
 * @version  V1.00
 * @brief    Panchip series SoC system clock init code and assert handler
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#include "PanSeries.h"
#include "pan_clk.h"
#include "pan_fmc.h"
#include "pan_sys.h"
#include "pan_efuse.h"

/*
 * Global variables defined here should be used after RAM RW/ZI data init.
 */
uint32_t SystemCoreClock;
bool isFtDataValid = true;
float SocTemperature = 22; /* Treat default SoC temperature as 22 C */
/*
 * Global variables here to be used in SystemInit() function should reside
 * in flash, since RAM RW/ZI data are not initialized at that time.
 */
const uint32_t PanFlashLineMode = CONFIG_FLASH_LINE_MODE;
const bool PanFlashEnhanceEnable = false;
uint32_t vec_remap_adr = 0x20007000;

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface and other modules which should
  *         be init in early boot stage.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
    ANA->CPU_ADDR_REMAP_CTRL =0;
    
    // Speed up flash clock from default 16MHz to 32MHz
    CLK_SetFlashClkDiv(CLK_FLASH_CLKDIV_1);
    // Switch flash to 4-Line mode
    FMC_SetFlashMode(FLCTL, PanFlashLineMode, PanFlashEnhanceEnable);
    // Init I-Cache
    InitIcache(FLCTL, PanFlashLineMode);

#ifndef SYNC_3V_REG_MANUALLY
    // Auto sync value to 3v registers
    // Note that this auto-sync mechanism works for all registers needed
    // doing 3v sync except Px_DINOFF regs for P46/P47/P56 in GPIO module.
    CLK_Set3vSyncAuto();
#endif

#if CONFIG_SOC_DCDC_PAN1070
    uint32_t tmpreg = ANA->LP_BUCK_3V;
    // Increase buck default output voltage
    tmpreg &= (~ANAC_BUCK_VOUT_Msk_3v);
    tmpreg |= 0x6 << ANAC_BUCK_VOUT_Pos_3v;
    // Enable DCDC-LDO auto switching when SoC vbat voltage drop off to about 1.8v and then uprise again
    tmpreg |= ANAC_BUCK_VOL_LIMIT_EN_Msk_3v;
    // Enable buck output
    tmpreg &= ~0x1;
    ANA->LP_BUCK_3V = tmpreg;
#endif

    // Disable eFuse VDD for power saving purpose
    EFUSE->EF_VDD = 0;
    // Clear the internal XOCAP trim of XTH and we prefer to use the external capacitor instead
    CLK->XTH_CTRL &= ~CLK_XTHCTL_XO_CAP_SEL_Msk;
    // Disable SW pull-down res to avoid high power leakage in low temperature
    ANA->LP_SW &= ~ANAC_HP_DVDD_PD_Msk;
    // Forcely reset GPIO debounce clk source to AHB before configure db cycle
    GPIO_DB->DBCTL &= ~GP_DBCTL_DBCLKSRC_Msk;
    // Other optimization configs (From task 4372)
    ANA->LP_PTAT_POLY |= ANAC_PTAT_TMP_TRIM_Msk;
    CLK->XTL_CTRL_3V = (CLK->XTL_CTRL_3V & ~CLK_XTLCTL_CORE_BIAS_Msk_3v) | (0x7u << CLK_XTLCTL_CORE_BIAS_Pos_3v);
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate(void)
{

#ifdef FPGA_MODE
    //Fixed 32M System Clock on FPGA
    SystemCoreClock = FREQ_32MHZ;
#else
    uint32_t div = 0;
    uint32_t freq_out = 0;
		div = (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_AHB_DIV_Msk) >> CLK_TOPCTL_AHB_DIV_Pos;
		if(div == 0)
			div = 1;
		else
			div += 1;

		if(CLK->CLK_TOP_CTRL_3V & CLK_SYS_SRCSEL_DPLL){
			freq_out = (CLK->DPLL_CTRL & CLK_DPLLCTL_FREQ_OUT_Msk);
			if(freq_out == CLK_DPLL_OUT_64M){
				SystemCoreClock = FREQ_64MHZ  / div;
			}
			else{
				SystemCoreClock = FREQ_48MHZ / div;
			}
		}
		else{
			SystemCoreClock = FREQ_32MHZ / div;
		}
#endif
}

static bool HwParamDataVerify(OTP_STRUCT_T *p_opt)
{
    uint8_t calc_checksum = 0;

    for (size_t i = 0x1E; i < 0x7B; i++)
    {
        calc_checksum += p_opt->d8[i];
    }

    if (calc_checksum != p_opt->m.ft_checksum)
    {
        return false;   // Calib data checksum failed
    }

    if (p_opt->m.ft_version >= 2)
    {
        calc_checksum = 0;
        for (size_t i = 0x80; i < 0xFF; i++)
        {
            calc_checksum += p_opt->d8[i];
        }

        if (calc_checksum != p_opt->m_v2.ft_checksum2)
        {
            return false;   // Calib data checksum2 failed
        }
    }

    return true;
}

__WEAK void ADC_SetCalirationParams(OTP_STRUCT_T *otp)
{
    return;
}

__WEAK void PW_ParamsSet(OTP_STRUCT_T *otp)
{
    return;
}

__WEAK void FMC_ParamsSet(OTP_STRUCT_T *otp)
{
    return;
}

bool SystemHwParamLoader(OTP_STRUCT_T *otp)
{
    /*
     * NOTE: We should make sure flash enhance mode is disabled before read
     *       data from flash.
     */
    FMC_ReadInfoArea(FLCTL, 0x0, CMD_DREAD, (uint8_t *)otp, sizeof(OTP_STRUCT_T));

    if (!HwParamDataVerify(otp))
    {
        /* Mark chip as NoFT */
        isFtDataValid = false;
        /* Exit if Hw Parameter Data Checking Failed */
        memset(&otp->d8[0x00], 0x00, sizeof(OTP_STRUCT_T));  // clear the global otp structure
        return false;
    }

#if CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE
    otp->m.lph_ldo_trim = (otp->m.lph_ldo_trim + CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE > 15) ? 15 : otp->m.lph_ldo_trim + CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE;
    if (otp->m.ft_version >= 2)
    {
        otp->m_v2.lph_ldo_vref_trim = (otp->m_v2.lph_ldo_vref_trim + CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE > 7) ? 7 : otp->m_v2.lph_ldo_vref_trim + CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE;
    }
#endif

    // Note:
    // 1. Clock XTH and XTL may need Board-Level calibration on customers MP stage
    // 2. ADC calibration data is loaded when calling ADC APIs (e.g. ADC_Init(), ADC_OutputVoltage(), ..)
    ANA->LP_PTAT_POLY = (ANA->LP_PTAT_POLY & ~(0x7u << 3) & ~(0x7u << 16)) | (otp->m.pmu_vbg_trim << 3) | (otp->m.ipoly_trim << 16);
    ANA->LP_HP_LDO = (ANA->LP_HP_LDO & ~(0xFu << 3) & ~(0x7u << 20)) | (otp->m.hp_ldo_trim << 3) | (otp->m.flash_ldo_trim << 20);
    ANA->LP_LP_LDO_3V = (ANA->LP_LP_LDO_3V & ~(0xFu << 1) & ~(0xFu << 17)) | (otp->m.lpl_ldo_trim << 1) | (otp->m.lph_ldo_trim << 17);
    ANA->LP_ANA_LDO = (ANA->LP_ANA_LDO & ~(0xFu << 3)) | (otp->m.ana_ldo_trim << 3);
    ANA->ANA_RFFE_LDO = (ANA->ANA_RFFE_LDO & ~(0xFu << 3)) | (otp->m.rffe_ldo_trim << 3);
    ANA->LP_FSYN_LDO = (ANA->LP_FSYN_LDO & ~(0xFu << 3)) | (otp->m.fsyn_ldo_trim << 3);
    ANA->ANA_ADC_LDO = (ANA->ANA_ADC_LDO & ~(0xFu << 3)) | (otp->m.adc_ldo_trim << 3);
    ANA->ANA_VCO_LDO = (ANA->ANA_VCO_LDO & ~(0xFu << 3)) | (otp->m.vco_ldo_trim << 3);
    ANA->ANA_MISC_3V = (ANA->ANA_MISC_3V & ~(0x7u << 0)) | (otp->m.bod_vref_trim << 0);
    CLK->RCH_CTRL = (CLK->RCH_CTRL & ~(0xFFu << 8)) | (otp->m.rch_trim << 8);
    CLK->RCL_CTRL_3V = (CLK->RCL_CTRL_3V & ~(0xFFFu << 4)) | (otp->m.rcl_coarse_trim << 4) | (otp->m.rcl_fine_trim << 8);
    otp->m.buck_imax_cal_trim = (otp->m.buck_imax_cal_trim + 4 > 0x1F) ? 0x1F : (otp->m.buck_imax_cal_trim + 4);
    ANA->ANA_RESERVED_3V = (ANA->ANA_RESERVED_3V & ~(0x1u << 21)) | ((otp->m.buck_out_trim & 0x1u) << 21);   // lsb of buck vout trim code
    ANA->LP_BUCK_3V = (ANA->LP_BUCK_3V & ~(0xFu << 2) & ~(0x1Fu << 9) & ~(0x1Fu << 14))
            | ((otp->m.buck_out_trim >> 1) << 2) | (otp->m.buck_imax_cal_trim << 9) | (otp->m.buck_zero_cal_trim << 14);
    ANA->ANA_DFT = (ANA->ANA_DFT & ~(0xFFu << 24)) | (otp->m.battery_trim << 24);

    if (otp->m.ft_version >= 2)
    {
        if (otp->m.ft_version < 4)
        {
            otp->m_v2.lph_ldo_vref_trim++;
            if (otp->m_v2.lph_ldo_vref_trim > 7)
            {
                otp->m_v2.lph_ldo_vref_trim = 7;
            }
        }
        ANA->LP_LP_LDO_3V = (ANA->LP_LP_LDO_3V & ~(0x7u << 21)) | (otp->m_v2.lph_ldo_vref_trim << 21);
        ANA->ANA_RESERVED_3V |= (0x1u << 24);  // LPLDOH enhance mode (default enabled, coordinate with lph vref trim)
        if (otp->m.ft_version >= 7)
        {
            // Start from this ft version, we change LPLDOH calibrate flow to MODE2 (indtead of MODE1 with enhance mode),
            // hence we should switch LPLDOH to MODE2 before we use it.
            // Note that lph_ldo_trim would take no effect anymore when we switch LPLDOH to MODE2
            ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;
        }
    }

    if (otp->m.ft_version >= 6)
    {
        ANA->LP_PTAT_POLY = (ANA->LP_PTAT_POLY & ~(0x3u << 1)) | (otp->m_v2.ptat_temp_trim << 1);
        CLK->XTL_CTRL_3V = (CLK->XTL_CTRL_3V & ~(0x7u << 4)) | (otp->m_v2.xtl_core << 4);
        CLK->XTH_CTRL = (CLK->XTH_CTRL & ~(0x1u << 10)) | (otp->m_v2.xth_icore << 10);
        CLK->RCH_CTRL = (CLK->RCH_CTRL & ~(0x3u << 4)) | (otp->m_v2.rch_bias << 4);
        CLK->DPLL_CTRL = (CLK->DPLL_CTRL & ~(0x3u << 4) & ~(0x3u << 6) & ~(0x7u << 8) & ~(0x3u << 11))
            | (otp->m_v2.dpll_icp_bias << 4) | (otp->m_v2.dpll_icp_ctrl << 6)  | (otp->m_v2.dpll_kvco_ctrl << 8)  | (otp->m_v2.dpll_vco_freq_trim << 11);
    }

    /*
     * Storing adc params related calibration. if adc symbol is not linked, 
     * this function would be handled as weak function.
     */
    ADC_SetCalirationParams(otp);

    /*
     * Storing power params related calibration.
     */
    PW_ParamsSet(otp);

    /*
     * Storing ft params for fmc use.
     */
    FMC_ParamsSet(otp);

    FMC_GetFlashUniqueId(FLCTL);
    FMC_GetFlashJedecId(FLCTL);
    FMC_SetFlashCapacity(FLCTL);

#ifdef SYNC_3V_REG_MANUALLY
    CLK_Wait3vSyncReady();
#endif

    return true;
}


#if USE_ASSERT

/**
 * @brief      Assert Error Message
 *
 * @param[in]  file  the source file name
 * @param[in]  line  line number
 *
 * @return     None
 *
 * @details    The function prints the source file name and line number where
 *             the ASSERT_PARAM() error occurs, and then stops in an infinite loop.
 */
void AssertError(uint8_t * file, uint32_t line)
{

    SYS_DBG(" wrong parameters.\r\n", file, line);

    /* Infinite loop */
    while(1) ;
}
#endif
