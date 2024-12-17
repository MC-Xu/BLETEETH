#include "PanSeries.h"
#include "pan_lp.h"
#include "pan_clk.h"

extern const uint32_t PanFlashLineMode;
extern const bool PanFlashEnhanceEnable;

/**
* @brief  This function enable gpio p56 wake up 
* @param[in]  ana: where ana is analog module
* @param[in]  WkEdge: wake up edge select,0-->low,1-->high 
* @return   none
*/
void LP_SetExternalWake(ANA_T *ana,uint8_t WkEdge)
{
	if(WkEdge){
		ana->LP_FL_CTRL_3V |= ANAC_FL_EXT_WAKEUP_SEL_Msk_3v;
	}else{
		ana->LP_FL_CTRL_3V &= ~ANAC_FL_EXT_WAKEUP_SEL_Msk_3v;
	}
}

/**
* @brief  This function set sleep time
* @param[in]  ana: where ana is analog module
* @param[in]  u32ClkCnt: where u32ClkCnt is 32k clock cnt num
* @param[in]  idx: channel index of sleeptimer, can be 0, 1, 2
* @return   none
*/
void LP_SetSleepTime(ANA_T *ana,uint32_t u32ClkCnt,uint8_t idx)
{
	((__IO uint32_t *)(&(ana)->LP_SPACING_TIME0))[idx] = u32ClkCnt;
}

/**
* @brief  This function get current 32K timer counter
* @param[in]  ana: where ana is analog module
* @return  Current 32K timer counter
*/
uint32_t LP_GetSlptmrCurrCount(ANA_T *ana)
{
    return ana->LP_SLPTMR;
}

/**
* @brief  This function set delay time used for standby_m0 mode for wait LPLDOH ready
* @param[in]  ana: where ana is analog module
* @param[in]  u8Clk32Cnt: where u8Clk32Cnt is 32k clock period cnt
* @return   none
*/
void LP_SetLpldohDelay(ANA_T *ana,uint16_t u16Clk32Cnt)
{
	uint32_t tmp_reg;

	tmp_reg = ana->LP_DLY_CTRL_3V;
	tmp_reg &= ~(ANAC_LPLDOH_DLY_TIME_Msk_3v);
	tmp_reg |= (u16Clk32Cnt << ANAC_LPLDOH_DLY_TIME_Pos_3v);
	ana->LP_DLY_CTRL_3V = tmp_reg;
}

/**
* @brief  This function set delay time used for fast clock ready
* @param[in]  ana: where ana is analog module
* @param[in]  u8Clk32Cnt: where u8Clk32Cnt is 32k clock period cnt
* @return   none
*/
void LP_SetWakeDelay(ANA_T *ana,uint16_t u16Clk32Cnt)
{
	uint32_t tmp_reg;

	tmp_reg = ana->LP_DLY_CTRL_3V;
	tmp_reg &= ~(ANAC_32KCLK_DLY_TIME_Msk);
	tmp_reg |= (u16Clk32Cnt << ANAC_32KCLK_DLY_TIME_Pos);
	ana->LP_DLY_CTRL_3V = tmp_reg;
}

/**
* @brief  This function set delay time used for fast clock ready
* @param[in]  ana: where ana is analog module
* @param[in]  mode: where mode is sleep mode select
*                        LP_MODE_SEL_SLEEP_MODE
*                        LP_MODE_SEL_DEEPSLEEP_MODE
*                        LP_MODE_SEL_STANDBY_M1_MODE
*                        LP_MODE_SEL_STANDBY_M0_MODE
*
* @return   none
*/

void LP_SetSleepMode(ANA_T *ana,uint8_t mode)
{
	uint32_t tmp_reg;

	tmp_reg = ana->LP_FL_CTRL_3V;
	tmp_reg &= ~(ANAC_FL_SLEEP_MODE_SEL_Msk);
	tmp_reg |= (mode << ANAC_FL_SLEEP_MODE_SEL_Pos);
	ana->LP_FL_CTRL_3V = tmp_reg;
}


/**
* @brief  This function set sleep mode config
* @param[in]  ana: Select analog module
* @param[in]  enterCyclically: Enable ARM Sleep-On-Exit Feature or not
* @return   none
*/
void LP_EnterSleepMode(ANA_T *ana, bool enterCyclically)
{
    LP_EnableInt(ana, ENABLE);

    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ana->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }

    LP_SetSleepMode(ana, LP_MODE_SEL_SLEEP_MODE);
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

    if (enterCyclically) {
        SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    } else {
        SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
    }

    __WFI();
}

void LP_EnterDeepSleepMode(ANA_T *ana, bool enterCyclically, uint8_t powerCtrl, uint8_t dpMode)
{
    uint32_t nvicInt;

    ana->LP_INT_CTRL |= ANAC_INT_LP_INT_EN_Msk | ANAC_INT_SLEEP_TMR_INT_EN_Msk | ANAC_INT_SLEEP_TMR_WK_EN_Msk;

    if (dpMode == LP_DEEPSLEEP_MODE1) {
        ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
        ana->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
        ana->LP_FL_CTRL_3V |= ANAC_LDO_POWER_CTL_Msk | ANAC_FL_LDO_ISOLATE_EN_Msk;
        ana->LP_FL_CTRL_3V &= ~ANAC_LDOL_POWER_CTL_Msk;
    } else if (dpMode == LP_DEEPSLEEP_MODE2) {
        ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
        ana->LP_LP_LDO_3V &= ~ANAC_LPLDO_L_EN_Msk;
        ana->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk;
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_LDO_ISOLATE_EN_Msk;
    } else if (dpMode == LP_DEEPSLEEP_MODE3) {
        ana->LP_LP_LDO_3V &= ~ANAC_LPLDO_H_EN_Msk_3v;
        ana->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
        ana->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk;
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_LDO_ISOLATE_EN_Msk;
    }

    // Enable LPDOH mode 2 to save power
    // NOTE: This option is now configured in SystemHwParamLoader() flow due to ft version.
#if 0
    ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;
#endif

#if CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Keep flash power in Deepsleep mode
#if CONFIG_FLASH_LDO_EN
    ana->LP_FL_CTRL_3V |= ANAC_FL_FLASH_LP_EN_Msk;
    ana->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#else
    ana->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ana->LP_FL_CTRL_3V |= ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_FLASH_LDO_EN */
    // Configure rdp wait cnt for auto dp use
    FMC_SetRdpWaitCount(FLCTL, 0x400);
#else
    // Power down flash in Deepsleep mode
    ana->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ana->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_KEEP_FLASH_POWER_IN_LP_MODE */

    /* Enable flash auto-dp no matter flash power-down or not in deepsleep mode */
    FMC_EnableAutoDp(FLCTL);

    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ana->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }

    // Configure power of SRAM
    ana->LP_FL_CTRL_3V = ((powerCtrl & 0x1f) << 24u) | (ana->LP_FL_CTRL_3V & 0xe0FFFFFF);

    // Configure LP delay
    ana->LP_DLY_CTRL_3V &= ~0x3ff;
    ana->LP_DLY_CTRL_3V |= 0x5;

    // Mask all interrupts
    __disable_irq();

    // Set LP irq to lowest priority
    NVIC_SetPriority(LP_IRQn, 3);

    /* Store and Clear NVIC Interrupt Enable Register */
    nvicInt = NVIC->ISER[0U];
    NVIC->ICER[0U] = 0xFFFFFFFF;
    /* Enable LP IRQ to make sure related ISR would trigger after wake up */
    NVIC_EnableIRQ(LP_IRQn);

    // Ensure next wfi enter deepsleep mode
    LP_SetSleepMode(ana, LP_MODE_SEL_DEEPSLEEP_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Wait 3v sync ready manually
    ana->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;
    while (ana->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {}

    if (enterCyclically) {
        SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    } else {
        SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
    }

    __WFI();

    // Ensure next wfi enter sleep mode
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    LP_SetSleepMode(ana, LP_MODE_SEL_SLEEP_MODE);

    /* Disable flash auto-dp after deepsleep wakeup */
    FMC_DisableAutoDp(FLCTL);

#if ((!CONFIG_KEEP_FLASH_POWER_IN_LP_MODE) && (CONFIG_FLASH_LINE_MODE == FLASH_X4_MODE))
    /* Set en_burst_wrap in FMC */
    FLCTL->X_FL_X_MODE |= (0x1 << 16);
    /* Re-issue burst_wrap command to flash if is X4 mode */
    FLCTL->X_FL_CTL = (0 << 8) | (0x05 << 0);
    FLCTL->X_FL_WD[0] = CMD_BURST_READ;
    FLCTL->X_FL_WD[4] = BURST_READ_MODE_32 << 5;
    FLCTL->X_FL_TRIG = CMD_TRIG;
    while (FLCTL->X_FL_TRIG) {
        __NOP();
    }
#endif

    /* Restore NVIC Interrupt Enable Register [xxx:107 must]*/
    NVIC->ISER[0U] = nvicInt;
}

__ASM static void LP_WfiWithCoreRegsBackupAndResume(void)
{
    push {r0-r7}    /* Backup r0 ~ r7 */
    mov r0, r8
    mov r1, r9
    mov r2, r10
    mov r3, r11
    mov r4, r12
    mov r5, lr
    push {r0-r5}    /* Backup r8 ~ r12 and lr */
    wfi             /* Trigger hw to enter low power mode */
    pop {r0-r5}     /* Restore r8 ~ r12 and lr */
    mov r8, r0
    mov r9, r1
    mov r10, r2
    mov r11, r3
    mov r12, r4
    mov lr, r5
    pop {r0-r7}     /* Restore r0 ~ r7 */
    bx lr           /* Function return */
}

void LP_EnterStandbyMode1(ANA_T *ana, uint8_t powerCtrl, bool wakeupWithoutReset)
{
    ana->LP_INT_CTRL |= ANAC_INT_LP_INT_EN_Msk | ANAC_INT_SLEEP_TMR_INT_EN_Msk | ANAC_INT_SLEEP_TMR_WK_EN_Msk;

    /* StandbyM1 Power Mode 1, use both LPLDOH and LPLDOL */
    ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ana->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
    ana->LP_FL_CTRL_3V |= ANAC_FL_LDO_ISOLATE_EN_Msk;
    ana->LP_FL_CTRL_3V &= ~(ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk);

    /* LPDOH switch to mode 2 for better power consumption performance */
    ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;

    /* Power down Flash in lp mode discard the dedicated Flash LDO enabled or not */
    ana->LP_FL_CTRL_3V = (ana->LP_FL_CTRL_3V & ~(0x3 << 12u)) | (0x0 << 12u);

    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ana->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ana->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ana->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }

    /* Configure retention SRAM modules in low power mode */
    ana->LP_FL_CTRL_3V = ((powerCtrl & 0x1f) << 24u) | (ana->LP_FL_CTRL_3V & 0xe0FFFFFF);

    /* Set digital delay with 32k tick unit */
    ana->LP_DLY_CTRL_3V &= ~0x3ff;
    ana->LP_DLY_CTRL_3V |= 5;

    /* Insure we are going to enter hw standby mode 1 */
    LP_SetSleepMode(ana, LP_MODE_SEL_STANDBY_M1_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Mask all interrupts
    __disable_irq();

    /* Trigger 3v sync */
    ana->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;

    /* Clear all lowpower related int flags if any */
    ana->LP_INT_CTRL = ana->LP_INT_CTRL;
#if 0
    /* Reset all hw peripheral modules except eFuse and GPIO */
    CLK->IPRST0 = 0x1CC;
    CLK->IPRST0 = 0x0;
    CLK->IPRST1 = 0x17FFF;
    CLK->IPRST1 = 0x0;
#endif
    /* Disable all IRQs and clear all pending IRQs on NVIC */
    NVIC->ICER[0U] = 0xFFFFFFFF;
    NVIC->ICPR[0U] = 0xFFFFFFFF;

    /* Wait until 3v sync done */
    while (ana->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {
        __NOP();
    }

    if (!wakeupWithoutReset) {

        /* Reset CPU Vector Remap register to avoid issue after waking up */
        ana->CPU_ADDR_REMAP_CTRL = 0;

        /* Trigger hw to enter low power mode */
        __WFI();

        /*
         * ======== (Now SoC is expected in HW Standby Mode 1 and would never return back here) =========
         */
    } else {
        /*
         * Enable CPU core regs retention in standby mode 1
         * The CPU core registers PC, MSP, PSP and CONTROL would automatically
         * be saved and restored by hardware in SoC standby mode 1.
         */
        ana->LP_FL_CTRL_3V |= ANAC_FL_CPU_RETENTION_EN_Msk;

        /* Backup the FMC remap register in case we configure it in bootloader */
        uint32_t fmc_remap_bkp = FLCTL->X_FL_REMAP_ADDR;

        /*
         * 1. Backup CPU core registers which are not auto-saved by hardware
         * 2. Enter SoC Standby Mode 1 (WFI)
         * 3. Restore previous backup CPU core registers
         */
        LP_WfiWithCoreRegsBackupAndResume();

        /* Restore FMC remap register after waking up */
        FLCTL->X_FL_REMAP_ADDR = fmc_remap_bkp;

        // Disable CPU core regs retention after wakeup
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_CPU_RETENTION_EN_Msk;

        /* Reset SoC lp mode to sleep mode (1.2v area, do not need 3v sync) */
        ana->LP_FL_CTRL_3V = (ana->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk)
            | (LP_MODE_SEL_SLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

        /* Restore fmc and flash status */
        FMC_SetFlashMode(FLCTL, PanFlashEnhanceEnable, PanFlashEnhanceEnable);
        /* Reinit I-Cache */
        InitIcache(FLCTL, PanFlashEnhanceEnable);
        /*
         * Clear StandbyM1 int flag (write 1 to clear) in this register, but still
         * retain all other ctrl/status flags.
         */
        ana->LP_INT_CTRL = (ana->LP_INT_CTRL | ANAC_INT_STANDBY_M1_FLAG_Msk)
            & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
    }
}

void LP_EnterStandbyMode0(ANA_T *ana, bool enableClk32k)
{
    /* Mask all IRQs when we are on the way to enter standby mode */
    __disable_irq();

    if (enableClk32k) {
        /* Enable proper 32k clock in low power mode */
        if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
            ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
            ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
        } else {
            ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
            ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
        }
    } else {
        // Disable 32K Clock Source to save power
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
    }

    /* Set digital delay with 32k tick unit */
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= 5;

    /* Insure we are going to enter hw standby mode 1 */
    LP_SetSleepMode(ANA, LP_MODE_SEL_STANDBY_M0_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Trigger 3v sync */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;

    /* Disable SLPTMR interrupt and wakeup in lp mode (Only enable LP interrupt) */
    ANA->LP_INT_CTRL = ANAC_INT_LP_INT_EN_Msk;
    // Clear configured time of sleep timers
    ANA->LP_SPACING_TIME0 = 0;
    ANA->LP_SPACING_TIME1 = 0;
    ANA->LP_SPACING_TIME2 = 0;

    /* Clear all lowpower related int flags if any */
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;
#if 0
    /* Reset all hw peripheral modules except eFuse and GPIO */
    CLK->IPRST0 = 0x1CC;
    CLK->IPRST0 = 0x0;
    CLK->IPRST1 = 0x17FFF;
    CLK->IPRST1 = 0x0;
#endif
    /* Disable all IRQs and clear all pending IRQs on NVIC */
    NVIC->ICER[0U] = 0xFFFFFFFF;
    NVIC->ICPR[0U] = 0xFFFFFFFF;

    /* Wait until 3v sync done */
    while (ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {
        __NOP();
    }

    /* Trigger hw to enter low power mode */
    __WFI();

    /*
     * ======== (Now SoC is expected in HW Standby Mode 0 and would never return back here) =========
     */
}
