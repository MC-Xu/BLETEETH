#include "FreeRTOS.h"
#include "task.h"
#include "os_lp.h"

#ifdef NIMBLE_SPARK_SUP
#include "sdk_config.h"
#endif

#ifdef NIMBLE_SPARK_SUP
#include "nimble/pan107x/nimble_glue_spark.h"
#endif

uint32_t lp_int_ctrl_reg;
uint32_t rst_status_reg;

uint8_t soc_reset_reason_get(void)
{
#if 0
    printf("ANA->LP_INT_CTRL: 0x%08x\n", lp_int_ctrl_reg);
    printf("CLK->RSTSTS: 0x%08x\n", rst_status_reg);
#endif
    /* Check standby mode int flags to detect standby mode wakeup */
    if (lp_int_ctrl_reg & ANAC_INT_STANDBY_M1_FLAG_Msk) {
        /* Check lptmr wakeup flag */
        if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR0_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP;
        } else if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR1_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP;
        } else if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR2_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP;
        } else {
            return SOC_RST_REASON_STBM1_GPIO_WAKEUP;
        }
    } else if (lp_int_ctrl_reg & ANAC_INT_STANDBY_M0_FLAG_Msk) {
        return SOC_RST_REASON_STBM0_EXTIO_WAKEUP;
    }

    /* Check common reset status flags */
    if (rst_status_reg & BIT0) {
        return SOC_RST_REASON_CHIP_RESET;
    } else if (rst_status_reg & BIT1) {
        return SOC_RST_REASON_PIN_RESET;
    } else if (rst_status_reg & BIT2) {
        return SOC_RST_REASON_WDT_RESET;
    } else if (rst_status_reg & BIT3) {
        return SOC_RST_REASON_LVR_RESET;
    } else if (rst_status_reg & BIT4) {
        return SOC_RST_REASON_BOD_RESET;
    } else if (!(ANA->LP_FL_CTRL_3V & BIT6)) {
        /* (Workaround) Re-set the additional reserved indication flag after use */
        ANA->LP_FL_CTRL_3V |= BIT6;
        return SOC_RST_REASON_SYS_RESET;
    } else if (rst_status_reg & BIT6) {
        return SOC_RST_REASON_POR_RESET;
    }

    return SOC_RST_REASON_UNKNOWN_RESET;
}

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
extern void WDT_Stop(void);
extern void WDT_Start(void);
void system_watch_dog_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT | CLK_APB1Periph_MILI_CLK, ENABLE);
	// Select Clock Source
    CLK_SetWdtClkSrc(CLK_APB1_WDTSEL_MILLI_PULSE);

    // Enable WDT, disable Reset Mode, disable Wakeup Signal
    WDT_Open(WDT_TIMEOUT_2POW16, WDT_RESET_DELAY_2CLK, TRUE, FALSE);
	WDT_Start();
}
#endif

#if configUSE_TICKLESS_IDLE
#include "pan_svc_call.h"

#define DBG_PM_PIN              P15   // P30
#define FLASH_RDP_WT_CNT        0x400 // zhongfeng sync 0x400;  old_val:0xA8
//#define LP_DLY_TICK             5     // unit: 32k tick  

#if CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
	#define LP_DLY_TICK             4     // unit: 32k tick
#else
	#define LP_DLY_TICK             5     // unit: 32k tick
#endif

/*************** Do not modify these definition ***************/

#define SETUP_TIME              14
#ifdef IP_107x
#define WAKEUP_CNT              (2 + LP_DLY_TICK + 7)
#elif defined(IP_101x)
#define WAKEUP_CNT              (2 + LP_DLY_TICK + 7 + 4)
#endif
#define REG_CNT                 30
#define MINIEST_SLP_CNT         25
#define LP_TIMER_FOREVER        0xFFFFFFFF

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_INT_CTRL_REG                 ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVSET_BIT                ( 1UL << 28UL )
#define portNVIC_PEND_SYSTICK_SET_BIT         ( 1UL << 26UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT       ( 1UL << 25UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )
#define configSYSTICK_CLOCK_HZ             ( configCPU_CLOCK_HZ )
#define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( portNVIC_SYSTICK_CLK_BIT )

#define portSY_FULL_READ_WRITE    ( 15 )

extern uint32_t ulTimerCountsForOneTick;
extern void UpdateTickAndSch(void);
extern const uint32_t PanFlashLineMode;
extern const bool PanFlashEnhanceEnable;

volatile static bool ble_more_closer = false;
static uint32_t stbm1_gpio_int_flag = 0;

/*************************************************************/

uint32_t soc_stbm1_gpio_wakeup_src_get(void)
{
    return stbm1_gpio_int_flag;
}

CONFIG_RAM_CODE void LP_IRQHandler()
{
    /*
	 * Clear DeepSleep int flag (write 1 to clear) in this register, but still
	 * retain all other ctrl/status flags.
	 */
	ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_DP_FLAG_Msk)
		& ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
		| ANAC_INT_STANDBY_M1_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);

	/* Re-disable LP IRQ after use */
	NVIC_DisableIRQ(LP_IRQn);
}

__WEAK void sleep_timer1_handler(void)
{
	/* This function can be overridden in application */
    printf("%s in..\n", __func__);
}

__WEAK void sleep_timer2_handler(void)
{
	/* This function can be overridden in application */
    printf("%s in..\n", __func__);
}

CONFIG_RAM_CODE void SLPTMR_IRQHandler(void)
{
	/* Handle os clock timeout */
	if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR0_Msk) {
		/*
		 * Clear sleep timer 0 interrupt flags (write 1 to clear) in this register,
		 * and retain other settings / flags.
		 */
		ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR0_Msk)
			& ~(ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
			| ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
			| ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
	}

	/* Handle custom sleep timer1 event */
	if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR1_Msk) {
		/*
		 * Clear sleep timer 1 interrupt flags (write 1 to clear) in this register,
		 * and retain other settings / flags.
		 */
		ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR1_Msk)
			& ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR2_Msk
			| ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
			| ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
		/* Execute slptmr1 handler */
		sleep_timer1_handler();
	}

	/* Handle custom sleep timer2 event */
	if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR2_Msk) {
		/*
		 * Clear sleep timer 2 interrupt flags (write 1 to clear) in this register,
		 * and retain other settings / flags.
		 */
		ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR2_Msk)
			& ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk
			| ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
			| ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
		/* Execute slptmr2 handler */
		sleep_timer2_handler();
	}
}

CONFIG_RAM_CODE void deepsleep_init(void)
{
    uint32_t RamPowerCtrl = 0x1F;   // Enable power of all sram in Deepsleep mode

    // Enable SLPTMR interrupt and wakeup
    ANA->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR_INT_EN_Msk | ANAC_INT_SLEEP_TMR_WK_EN_Msk;

#if CONFIG_DEEPSLEEP_MODE_2
    // Configure power of deepsleep to mode 2 (Only LPLDOH in use)
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V &= ~ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_LDO_ISOLATE_EN_Msk;
#else
    // Configure power of deepsleep to mode 1 (Both LPLDOH & LPLDOL in use)
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_LDO_POWER_CTL_Msk | ANAC_FL_LDO_ISOLATE_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_LDOL_POWER_CTL_Msk;
#endif /* CONFIG_DEEPSLEEP_MODE_2 */

    // Enable LPDOH mode 2 to save power
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;

#if CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Keep flash power in Deepsleep mode
#if CONFIG_FLASH_LDO_EN
    ANA->LP_FL_CTRL_3V |= ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#else
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_FLASH_LDO_EN */
    // Configure rdp wait cnt for auto dp use
    FMC_SetRdpWaitCount(FLCTL, FLASH_RDP_WT_CNT);
#else
    // Power down flash in Deepsleep mode
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_LP_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_FLASH_BP_EN_Msk;
#endif /* CONFIG_KEEP_FLASH_POWER_IN_LP_MODE */

    // Configure power of SRAM
    ANA->LP_FL_CTRL_3V = ((RamPowerCtrl & 0x1f) << 24u) | (ANA->LP_FL_CTRL_3V & 0xe0FFFFFF);

    // Configure LP delay
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= LP_DLY_TICK;

    // Clear configured time of sleep timers
    ANA->LP_SPACING_TIME0 = 0;
    ANA->LP_SPACING_TIME1 = 0;
    ANA->LP_SPACING_TIME2 = 0;

    // Configure SLPTMR and LP irqs
    NVIC_EnableIRQ(SLPTMR_IRQn);
    NVIC_SetPriority(SLPTMR_IRQn, 3);   // Lowest prio
    NVIC_SetPriority(LP_IRQn, 3);       // Lowest prio

    /* Store gpio int flags if any in case of waking up from standby mode 1 by gpio interrupt */
    *((uint8_t *)&stbm1_gpio_int_flag + 0) = P0->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 1) = P1->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 2) = P2->INTSRC;
    *((uint8_t *)&stbm1_gpio_int_flag + 3) = P3->INTSRC;

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
    system_watch_dog_init();
#endif
}

CONFIG_RAM_CODE void deepsleep_pre_process( uint32_t xLP_SleepTime )
{
#if 1//CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Enable flash auto dp
    FLCTL->X_FL_DP_CTL |= 1u;
#endif
    /* Set timeout of slptmr0 (for os tick use) */
    ANA->LP_SPACING_TIME0 = (xLP_SleepTime - WAKEUP_CNT);

    /* Insure we are going to enter hw-deep-sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk) | (LP_MODE_SEL_DEEPSLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

    /* Do AON register (3v) sync manually */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;
    while(ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk));
}

CONFIG_RAM_CODE void deepsleep_post_process( uint32_t xExpectedIdleTime )
{
    /* Clear CPU core deepsleep ctrl flag */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    /* Reset SoC lp mode to sleep mode (1.2v area, do not need 3v sync) */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk) | (LP_MODE_SEL_SLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

#if 1//CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    // Re-disable flash auto dp
    FLCTL->X_FL_DP_CTL &= ~1u;
#endif

#if 0
	/* Wait for 3v sync ready in case that cpu may be waked up too quick  */
	ANA->LP_REG_SYNC |= (ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk);
	while (ANA->LP_REG_SYNC & ANAC_LP_REG_SYNC_3V_TRG_Msk) {}
#endif

#ifdef NIMBLE_SPARK_SUP
    //TODO:
    //pan_ll_pm_post_handler();
#endif
}

#ifdef NIMBLE_SPARK_SUP
CONFIG_RAM_CODE uint32_t pan_deep_sleep_flow(TickType_t xExpectedIdleTime)
{
    uint32_t ll_remain_sleep_cyc = pan_get_ll_idle_time();
    
    uint32_t os_decided_sleep_cyc;
    
    if(xExpectedIdleTime == portMAX_DELAY)
    {    
        os_decided_sleep_cyc = LP_TIMER_FOREVER;
    } else {
        os_decided_sleep_cyc = (uint32_t)xExpectedIdleTime;
    }

	/* Check if the active 32K is used to LL timer */
	if (ANA->ACT_32K_CTRL & ANAC_ACT_32K_SEL_Msk) {
		return 0;
	}
    
	/*
	 * Update final sleep time to meet the need of BLE Controller if it needs
	 * an earlier wakeup than other threads.
	 */
	if (ll_remain_sleep_cyc < os_decided_sleep_cyc) {
        ble_more_closer = true;
		if ((ll_remain_sleep_cyc > (SETUP_TIME  + WAKEUP_CNT + REG_CNT)) && (ll_remain_sleep_cyc < 400000)) {
			if (rf_check_sleep_state()) {
				return ll_remain_sleep_cyc;
			} else {
				/* Fall back to Sleep Flow */
				return 0;
			}
		} else {
			/* Fall back to Sleep Flow */
			return 0;
		}
	} else {
        ble_more_closer = false;
		if (rf_check_sleep_state()) {
			if (os_decided_sleep_cyc > (MINIEST_SLP_CNT)) {
				return os_decided_sleep_cyc;
			} else {
				/* Fall back to Sleep Flow */
				return 0;
			}
		} else {
			/* Fall back to Sleep Flow */
			return 0;
		}
	}
}
#else /*deep sleep check without ble event*/
CONFIG_RAM_CODE uint32_t pan_deep_sleep_flow(TickType_t xExpectedIdleTime)
{    
    uint32_t os_decided_sleep_cyc;
    
    if(xExpectedIdleTime == portMAX_DELAY)
    {    
        os_decided_sleep_cyc = LP_TIMER_FOREVER;
    } else {
        os_decided_sleep_cyc = (uint32_t)xExpectedIdleTime;
    }

    /* Check if the active 32K is used to LL timer */
    if (ANA->ACT_32K_CTRL & ANAC_ACT_32K_SEL_Msk) {
        return 0;
    }
    
    if (os_decided_sleep_cyc > (MINIEST_SLP_CNT)) {
        return os_decided_sleep_cyc;
    } else {
        /* Fall back to Sleep Flow */
        return 0;
    }
}
#endif

/*
 * This function can be override by APP to add customized procedure
 * BEFORE SoC enter DeepSleep State
 */
__WEAK void vSocDeepSleepEnterHook(void)
{
    /* Do nothing here */
}

/*
 * This function can be override by APP to add customized procedure
 * AFTER SoC exit DeepSleep State
 */
__WEAK void vSocDeepSleepExitHook(void)
{
    /* Do nothing here */
}

CONFIG_RAM_CODE void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    uint32_t xLPSleepTime;
    eSleepModeStatus eSleepStatus;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __set_PRIMASK(1);

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    eSleepStatus = eTaskConfirmSleepModeStatus();
    
    if( eSleepStatus == eAbortSleep )
    {
        vTaskTickSet(lp_get_curr_tmr_cnt());
        vTaskStepTick(0);
        /* Re-enable interrupts - see comments above the __disable_irq()
         * call above. */
        __set_PRIMASK(0);
        return;
    }

     /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
     * set its parameter to 0 to indicate that its implementation contains
     * its own wait for interrupt or wait for event instruction, and so wfi
     * should not be executed again.  However, the original expected idle
     * time variable must remain unmodified, so a copy is taken. */        
    xLPSleepTime = pan_deep_sleep_flow(xExpectedIdleTime);
    if(xLPSleepTime)
    {
        /* Stop the SysTick momentarily.  The time the SysTick is stopped for
         * is accounted for as best it can be, but using the tickless mode will
         * inevitably result in some tiny drift of the time maintained by the
         * kernel with respect to calendar time. */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        
        /*
         * This function can be override by APP to add customized procedure
         * BEFORE SoC enter DeepSleep State
         */
        vSocDeepSleepEnterHook();
        
        configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Stop();
#endif

		/* Store and Clear NVIC Interrupt Enable Register */
		uint32_t nvicInt = NVIC->ISER[0U];
		NVIC->ICER[0U] = 0xFFFFFFFF;
        /* Enable LP IRQ to make sure related ISR would trigger after wake up */
		NVIC_EnableIRQ(LP_IRQn);

		/* Deepsleep Pre handler */
		deepsleep_pre_process( xLPSleepTime );

        /* Records the time the current timestamp is used to calculate the sleep
         * value after the system wakes up */
        vTaskTickSet(lp_get_curr_tmr_cnt());

#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
        DBG_PM_PIN = 0;
#endif
        __WFI();    // Try to enter LowPower mode
#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
        DBG_PM_PIN = 1;
#endif

		/* Restore NVIC Interrupt Enable Register [xxx:107 must]*/
		NVIC->ISER[0U] = nvicInt;

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

#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
        WDT_Start();
#endif
		/* Deepsleep Post handler */
        deepsleep_post_process( xExpectedIdleTime );

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

        UpdateTickAndSch();

        /* restart systick, use configTICK_ON_WAKING_RATE_HZ */
		portNVIC_SYSTICK_CTRL_REG = 0UL;
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
        /* Configure SysTick to interrupt at the requested rate. */
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
        portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );

        /*
         * This function can be override by APP to add customized procedure
         * AFTER SoC exit DeepSleep State
         */
        vSocDeepSleepExitHook();

        /* Exit with interrupts enabled. */
        __set_PRIMASK(0);

    } else { /*enter mcu sleep, not deep sleep*/
        if(ble_more_closer) {   /*system will wakeup from ble interrupt*/
		#if !CONFIG_HCLK_OPTIMIZE_EN
            __set_PRIMASK(0);
		#endif

#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
            DBG_PM_PIN = 0;
#endif
#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
            WDT_Stop();
#endif
		#if CONFIG_HCLK_OPTIMIZE_EN
			uint32_t tmp = CLK->CLK_TOP_CTRL_3V;
			//CLK_HCLKConfig(15);  // 4MHz 
			CLK_HCLKConfig(7);     // 8MHz 
			//CLK_HCLKConfig(3);   // 16MHz
            __WFI();
			CLK->CLK_TOP_CTRL_3V = tmp;
			
			__set_PRIMASK(0);
		#else
			__WFI();
		#endif
			
#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
            WDT_Start();
#endif
#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
            DBG_PM_PIN = 1;
#endif
        } else {
            portNVIC_SYSTICK_CTRL_REG = 0UL;
            portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
            
            uint32_t lp_count = xExpectedIdleTime;
            /* Configure SysTick to interrupt at the requested rate. */
            portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) * lp_count;
            portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
            
            __set_PRIMASK(0);
#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
            DBG_PM_PIN = 0;
#endif
#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
            WDT_Stop();
#endif
            __WFI();
#if CONFIG_SYSTEM_WATCH_DOG_ENABLE
            WDT_Start();
#endif
#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
            DBG_PM_PIN = 1;
#endif
        }
    }
}

#if CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
__ASM void wfi_with_core_regs_backup_and_resume(void)
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
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */

/*
 * Several hw modules can be selectable to retain or lose power in this mode
 */
void soc_enter_standby_mode_1(uint32_t wakeup_src, uint32_t retention_sram)
{
    /* Mask all IRQs when we are on the way to enter standby mode */
    __disable_irq();

    /* Disable Systick clock */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* Clear pending flag of systick (PENDSTCLR) if any */
    SCB->ICSR = (SCB->ICSR & (~BIT26)) | BIT25;

    /* StandbyM1 Power Mode 1, use both LPLDOH and LPLDOL */
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_LDO_ISOLATE_EN_Msk;
    ANA->LP_FL_CTRL_3V &= ~(ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk);

    /* LPDOH switch to mode 2 for better power consumption performance */
    ANA->LP_LP_LDO_3V |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;

    /* Power down Flash in lp mode discard the dedicated Flash LDO enabled or not */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~(0x3 << 12u)) | (0x0 << 12u);

    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }

    /* Configure retention SRAM modules in low power mode */
    ANA->LP_FL_CTRL_3V = ((retention_sram & 0x1f) << 24u) | (ANA->LP_FL_CTRL_3V & 0xe0FFFFFF);

    /* Set digital delay with 32k tick unit */
    ANA->LP_DLY_CTRL_3V &= ~0x3ff;
    ANA->LP_DLY_CTRL_3V |= 5;

    /* Insure we are going to enter hw standby mode 1 */
    LP_SetSleepMode(ANA, LP_MODE_SEL_STANDBY_M1_MODE);
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /* Trigger 3v sync */
    ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;

//    if (wakeup_src & STBM1_WAKEUP_SRC_GPIO) {
//        /* Do nothing here */
//    } else {
//        /* Reset GPIO module to default state */
//        CLK->IPRST1 = CLK_IPRST1_GPIORST_Msk;
//        CLK->IPRST1 = 0x0;
//    }

    /* Set specific slptmr timeout cnt if needed */
    if (wakeup_src & STBM1_WAKEUP_SRC_SLPTMR) {
        /* Do nothing here */
    } else {
        /* Disable SLPTMR interrupt and wakeup in lp mode (Only enable LP interrupt) */
        ANA->LP_INT_CTRL = ANAC_INT_LP_INT_EN_Msk;
        // Clear configured time of sleep timers
        ANA->LP_SPACING_TIME0 = 0;
        ANA->LP_SPACING_TIME1 = 0;
        ANA->LP_SPACING_TIME2 = 0;
    }

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

#if !CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
#if CONFIG_VECTOR_REMAP_TO_RAM
    /* Reset CPU Vector Remap register to avoid issue after waking up */
    ANA->CPU_ADDR_REMAP_CTRL = 0;
#endif

    /* Trigger hw to enter low power mode */
    __WFI();

    /*
     * ======== (Now SoC is expected in HW Standby Mode 1 and would never return back here) =========
     */
#else
    /*
     * Enable CPU core regs retention in standby mode 1
     * The CPU core registers PC, MSP, PSP and CONTROL would automatically
     * be saved and restored by hardware in SoC standby mode 1.
     */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_CPU_RETENTION_EN_Msk;

    /* Backup the FMC remap register in case we configure it in bootloader */
    uint32_t fmc_remap_bkp = FLCTL->X_FL_REMAP_ADDR;

    /* Records the time the current timestamp is used to calculate the sleep
     * value after the system wakes up */
    vTaskTickSet(lp_get_curr_tmr_cnt());

    /*
     * 1. Backup CPU core registers which are not auto-saved by hardware
     * 2. Enter SoC Standby Mode 1 (WFI)
     * 3. Restore previous backup CPU core registers
     */
    wfi_with_core_regs_backup_and_resume();

    /* Restore FMC remap register after waking up */
    FLCTL->X_FL_REMAP_ADDR = fmc_remap_bkp;

    /* Reset SoC lp mode to sleep mode (1.2v area, do not need 3v sync) */
    ANA->LP_FL_CTRL_3V = (ANA->LP_FL_CTRL_3V & ~ANAC_FL_SLEEP_MODE_SEL_Msk)
        | (LP_MODE_SEL_SLEEP_MODE << ANAC_FL_SLEEP_MODE_SEL_Pos);

    /* Restore fmc and flash status */
    FMC_SetFlashMode(FLCTL, PanFlashEnhanceEnable, PanFlashEnhanceEnable);
    /* Reinit I-Cache */
    InitIcache(FLCTL, PanFlashEnhanceEnable);
    /*
     * Clear StandbyM1 int flag (write 1 to clear) in this register, but still
     * retain all other ctrl/status flags.
     */
    ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_STANDBY_M1_FLAG_Msk)
        & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
        | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);

    /* Restore 32K current counter register LPTMR_CURR_CNT_ENA_REG */
    (*(volatile uint32_t *)0x5002000C) |= BIT1;

    /* Update OS tick and scheduler */
    UpdateTickAndSch();

    /* Restart systick, use configTICK_ON_WAKING_RATE_HZ */
    portNVIC_SYSTICK_CTRL_REG = 0UL;
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
    /* Configure SysTick to interrupt at the requested rate. */
    portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
    portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );

    /* Exit with interrupts enabled. */
    __enable_irq();
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */
}


/*
 * The most power saving mode in which can only be waked up by GPIO P02/P01/P00 pin
 */
void soc_enter_standby_mode_0(void)
{
    /* Mask all IRQs when we are on the way to enter standby mode */
    __disable_irq();

    /* Disable Systick clock */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* Clear pending flag of systick (PENDSTCLR) if any */
    SCB->ICSR = (SCB->ICSR & (~BIT26)) | BIT25;
#if 0
    /* Enable proper 32k clock in low power mode */
    if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
        ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    } else {
        ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
        ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    }
#else
    // Disable 32K Clock Source to save power
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
#endif
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

#endif   /* configUSE_TICKLESS_IDLE */
