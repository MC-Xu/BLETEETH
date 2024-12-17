#include "FreeRTOS.h"
#include "task.h"
#include "ble_config.h"
#include "pan_clktrim.h"
#include "nimble/nimble_port.h"
#include "pan_svc_call.h"
#ifdef NIMBLE_SPARK_SUP
#include "nimble/pan107x/nimble_glue_spark.h"
#endif
#if CONFIG_RTT_LOG_ENABLE
#include "SEGGER_RTT.h"
#endif
#include "pan_power.h"

/* For RCL Clock test */
//#define PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE

#define CLK_ACT32K_TMR_EN_Pos   			(0)                                         
#define CLK_ACT32K_TMR_EN_Msk   			(0x1ul << CLK_ACT32K_TMR_EN_Pos)
#define CLK_ACT32K_LL_32KCLK_SEL_Pos   		(1)                                         
#define CLK_ACT32K_LL_32KCLK_SEL_Msk   		(0x1ul << CLK_ACT32K_LL_32KCLK_SEL_Pos)

/* Do not modify this definition */
#define LPTMR_CURR_CNT_ENA_REG  (0x5002000C)

extern uint32_t lp_int_ctrl_reg;
extern uint32_t rst_status_reg;
extern void app_init(void);

static uint8_t m_chip_mac[6];

#if (CONFIG_LOW_SPEED_CLOCK_SRC == 1)
uint32_t clk_32k_count = 32768;	
#else
uint32_t clk_32k_count = 32000;
#endif

#if CONFIG_AUTO_OPTIMIZE_POWER_PARAM

#define TEMP_CHECK_INTERVAL_MS      (CONFIG_TEMP_SAMPLE_INTERVAL_S*1000)

#ifdef NIMBLE_SPARK_SUP
static struct ble_npl_callout temp_check_timer;
static ADC_HandleTypeDef adc_temperature = {
	.id = ADC_CH_TEMP,
};
#endif

void temp_check_start(uint32_t ms)
{
#ifdef NIMBLE_SPARK_SUP
    ble_npl_callout_reset(&temp_check_timer, pdMS_TO_TICKS(ms));
#endif
}

void temp_check_cb(struct ble_npl_event *e)
{
#ifdef NIMBLE_SPARK_SUP
	float temp;
	
    temp_check_start(TEMP_CHECK_INTERVAL_MS);

	if (HAL_ADC_GetValue(&adc_temperature, &temp) == PAN_HAL_SUCCESS) {
		PW_AutoOptimizeParams((int)temp);
	
		/* Update SCA */
		static uint8_t config = 0xFF;
		uint16_t SCA = CONFIG_BT_CTLR_SCA;
		if(temp > 15 && temp < 30){
			if(config == 0)
				return;
			config = 0;
			SCA = CONFIG_BT_CTLR_SCA;
		}
		else{
			if(config == 1)
				return;
			config = 1;
			SCA = (CONFIG_LOW_SPEED_CLOCK_SRC==0 ? 1000:500);
		}
		pan_misc_upd_local_sca(SCA);
		
		//printf("update SCA:%d\n", SCA);
	}
#endif
}

void temp_check_init(void)
{
#ifdef NIMBLE_SPARK_SUP
	//register timer
	ble_npl_callout_init(&temp_check_timer, (struct ble_npl_eventq *)nimble_port_get_dflt_eventq(), temp_check_cb, NULL);

	//start timer right after os shedulingk
	temp_check_start(0);
#endif
}

void temp_check_stop(void)
{
#ifdef NIMBLE_SPARK_SUP
    ble_npl_callout_stop(&temp_check_timer);
#endif
}

#endif /* CONFIG_AUTO_OPTIMIZE_POWER_PARAM */


#define DST_CLK_CNT     100     /* measure 100 rcl clock period, time spend about 50ms+ */

#if ((CONFIG_LOW_SPEED_CLOCK_SRC == 0) || (CONFIG_LOW_SPEED_CLOCK_SRC == 1))
static uint32_t clktrim_measure_32k_clk(uint32_t dst_clk_cnt)
{
    uint32_t counter_result;
    uint32_t calib_freq;

    CLK_EnableClkTrim(ENABLE);
    CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);

    TRIM_SetCalCnt(TRIM, dst_clk_cnt);

    TRIM_StartTuning(TRIM, TRIM_MEASURE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_MEASURE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_MEASURE_STOP_Msk);
    counter_result = TRIM_GetRealCnt(TRIM);

    calib_freq = ((uint64_t)FREQ_32MHZ * (uint64_t)dst_clk_cnt) / counter_result;
	clk_32k_count = calib_freq;
    CLK_EnableClkTrim(DISABLE);

    return calib_freq;
}
#endif

#if (CONFIG_FORCE_CALIB_RCL_CLK && (CONFIG_LOW_SPEED_CLOCK_SRC == 0))
static void calibrate_rcl_clk(uint32_t expected_freq)
{
    uint32_t AHB_clk_reg;

    const uint8_t coarse_tune_bitwidth = 3;
    const uint8_t fine_tune_bitwidth = 7;

    AHB_clk_reg = CLK->AHB_CLK_CTRL;
    CLK->AHB_CLK_CTRL |= CLK_AHBPeriph_APB2;

    TRIM_EnableInt(TRIM, ENABLE);

    /* Start calibration */
    /* Disable hardware calculate. */
    ANA->RCL_HW_CAL_CTRL &= ~ANAC_RCL_HW_CAL_EN_Msk;
    /* Enable Clktrim */
    CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);
    CLK_EnableClkTrim(ENABLE);

    TRIM_SetCalCnt(TRIM, DST_CLK_CNT);
    TRIM_SetIdealCnt(TRIM, FREQ_32MHZ / expected_freq * DST_CLK_CNT - 1);

    /* a. Coarse tuning proc */
    TRIM_SetBitWidth(TRIM, coarse_tune_bitwidth);
    TRIM_SetCoarseCode(TRIM, 1 << coarse_tune_bitwidth);
    TRIM_StartTuning(TRIM, TRIM_COARSE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_CTUNE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_CTUNE_STOP_Msk);

    /* b. Fine tuning proc */
    TRIM_SetBitWidth(TRIM, fine_tune_bitwidth);
    TRIM_SetFineCode(TRIM, 1 << fine_tune_bitwidth);
    TRIM_StartTuning(TRIM, TRIM_FINE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_FTUNE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_FTUNE_STOP_Msk);

    /* Disable Clktrim */
    CLK_EnableClkTrim(DISABLE);

    CLK->AHB_CLK_CTRL = AHB_clk_reg;
}
#endif /* CONFIG_FORCE_CALIB_RCL_CLK */

#if CONFIG_UART_LOG_ENABLE
void debug_uart_init(void)
{
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = CONFIG_LOG_UART_BAUDRATE,
        .UART_LineCtrl = Uart_Line_8n1,
    };

#if (CONFIG_LOG_UART_PIN == 0)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P0, 5, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 1)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 1, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 2)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 6, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_PIN == 3)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P0, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 4)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 0, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 5)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 2, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 6)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P2, 5, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_PIN == 7)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P3, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
}
#endif //CONFIG_UART_LOG_ENABLE

void track_pin_init(void)
{
#ifdef NIMBLE_SPARK_SUP
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
	
	debug_io_map debug_io;

  #if 1 //32 pin
	GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT); P15 = 0;
	GPIO_SetMode(P1, BIT3, GPIO_MODE_OUTPUT); P13 = 0;
	GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT); P14 = 0;
	GPIO_SetMode(P1, BIT2, GPIO_MODE_OUTPUT); P12 = 0;
	GPIO_SetMode(P0, BIT5, GPIO_MODE_OUTPUT); P05 = 0;
	GPIO_SetMode(P0, BIT3, GPIO_MODE_OUTPUT); P03 = 0;
	GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT); P23 = 0;
	
	debug_io.scan_pin      = (uint32_t)&P15;
	debug_io.adv_pin       = (uint32_t)&P13;
	debug_io.timer_irq_pin = (uint32_t)&P14;
	debug_io.ll_irq_pin    = (uint32_t)&P12;
	debug_io.conn_pin[0]   = (uint32_t)&P05;
	debug_io.conn_pin[1]   = (uint32_t)&P03;
	debug_io.sch_pin       = (uint32_t)&P23;
	
  #else
	GPIO_SetMode(P2, BIT2, GPIO_MODE_OUTPUT); P22 = 0;
	GPIO_SetMode(P2, BIT4, GPIO_MODE_OUTPUT); P24 = 0;
	GPIO_SetMode(P2, BIT5, GPIO_MODE_OUTPUT); P25 = 0;
	GPIO_SetMode(P0, BIT3, GPIO_MODE_OUTPUT); P03 = 0;
	GPIO_SetMode(P2, BIT6, GPIO_MODE_OUTPUT); P26 = 0;
	GPIO_SetMode(P1, BIT2, GPIO_MODE_OUTPUT); P12 = 0;
	GPIO_SetMode(P1, BIT1, GPIO_MODE_OUTPUT); P11 = 0;
	GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT); P15 = 0;
	
	debug_io.scan_pin      = (uint32_t)&P22;
	debug_io.adv_pin       = (uint32_t)&P24;
	debug_io.timer_irq_pin = (uint32_t)&P25;
	debug_io.ll_irq_pin    = (uint32_t)&P03;
	debug_io.conn_pin[0]   = (uint32_t)&P26;
	debug_io.conn_pin[1]   = (uint32_t)&P12;
	debug_io.sch_pin       = (uint32_t)&P15;
  #endif

	pan_svc_interface(SVC_LL_DEBUG, &debug_io);
#endif
}

__weak void vPortSystemClockEnable()
{
    /* User overload, self - realization clock enable and disable */
}

void sys_clock_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
    
    ANA->LP_FSYN_LDO |= 0X1;

	CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
	
#if (CONFIG_SYSTEM_CLOCK == 32)
	CLK_HCLKConfig(1);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_64M); 
#elif (CONFIG_SYSTEM_CLOCK == 48)
	CLK_HCLKConfig(0);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_48M); 
#endif
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

	CLK->RCH_CTRL &= ~BIT(0);

    CLK_PCLK1Config(CONFIG_APB1_CLOCK_DIVISOR >> 1);
    CLK_PCLK2Config(CONFIG_APB2_CLOCK_DIVISOR >> 1);
    /*
	 * Note that all clocks on APB are disabled by default after SoC power up, and
	 * the default AHB clocks enabled after SoC power up are:
	 * ACC/eFuse/ROM/RCC_AHB/Systick/GPIO.
	 * Here we disable ACC, eFuse and ROM clock as these modules are not commonly
	 * used. We should re-enabled them once we use later.
	 * Here We also enable APB1 and APB2 clock path to make sure modules on APB can
	 * be easily enabled by each peripheral driver by enabling their clock enable
	 * bits on APB bus.
	 */
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_ROM, DISABLE);
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1 | CLK_AHBPeriph_APB2, ENABLE);
	  
    /*Basic clock for BLE*/
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_BLE_32M | CLK_AHBPeriph_BLE_32K, ENABLE);
	
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB|CLK_AHBPeriph_USB_48M, DISABLE);

    vPortSystemClockEnable();
}

void ble_misc_init(void)
{
#ifdef NIMBLE_SPARK_SUP
    NVIC_EnableIRQ(LL_IRQn);
    NVIC_SetPriority(LL_IRQn, 0);

    NVIC_EnableIRQ(BLE_EVENT_PROC_IRQn);
    NVIC_SetPriority(BLE_EVENT_PROC_IRQn, 1);
#endif
}

#define APP_POWER_TEST_EN    0
#if APP_POWER_TEST_EN
void app_power_test(OTP_STRUCT_T *otp)
{
	//buck out(DCDC):default:8;   FT-2
	uint32_t tmp = ANA->LP_BUCK_3V; 
	tmp &= ~(0xFul<<2);
	tmp |= (((otp->m.buck_out_trim >> 1) - 2) << 2);
	//tmp |= (((otp->m.buck_out_trim >> 1) - 3) << 2);
	ANA->LP_BUCK_3V = tmp;

	//HPLDO(DVDD) default - 1/2; default:8
	tmp = ANA->LP_HP_LDO;
	tmp &= ~(0xFul <<3);
	tmp |= ((otp->m.hp_ldo_trim - 2)<<3); //~1.12V
	ANA->LP_HP_LDO = tmp;
}
#endif

static int pan10xx_hw_calib_init(void)
{
	OTP_STRUCT_T otp;
	
    printf("Try to load HW calibration data..");
    if (!SystemHwParamLoader(&otp)) {
        printf("\nWARNING: Cannot find valid calib data in current chip!\n");
    } else {
        printf(" DONE.\n");
        printf("- Chip Info         : 0x%x\n", otp.m.chip_info);
        printf("- Chip CP Version   : %d\n", otp.m.cp_version);
        printf("- Chip FT Version   : %d\n", otp.m.ft_version);
        if (otp.m.ft_version >= 2) {
			memcpy(m_chip_mac, otp.m_v2.mac_addr, 6);
			printf("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m_v2.mac_addr[0], otp.m_v2.mac_addr[1],
				otp.m_v2.mac_addr[2], otp.m_v2.mac_addr[3], otp.m_v2.mac_addr[4], otp.m_v2.mac_addr[5]);
		} else {
			memcpy(m_chip_mac, otp.m.mac_addr, 6);
			printf("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
				otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
		}
        printf("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
            otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);
		
	#if APP_POWER_TEST_EN
		app_power_test(&otp);
	#endif
    }
    printf("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        printf("%02X", flash_ids.uid[i]);
    }
    printf("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

    return 0;
}

uint8_t pan10x_mac_addr_get(uint8_t *mac)
{
	uint8_t addr_null[6] = {0, 0, 0, 0, 0, 0};
	
	if (memcmp(m_chip_mac, addr_null, 6) == 0) {
		printf("Warnning: No chip mac addr\n");
		return 1;
	} else {
		memcpy(mac, m_chip_mac, 6);
		return 0;
	}
}


uint8_t pan10x_roll_mac_addr_get(uint8_t *mac)
{
	uint8_t addr_temp[6];
	uint8_t addr_null[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
	FMC_ReadInfoArea(FLCTL, 0x100, CMD_DREAD, addr_temp, sizeof(addr_temp));
	
	if (memcmp(addr_temp, addr_null, 6) == 0) {
		printf("Warnning: No chip roll mac addr\n");
		return 1;
	} else {
		memcpy(mac, addr_temp, 6);
		return 0;
	}
}


void pan10x_init()
{

#if CONFIG_VECTOR_REMAP_TO_RAM
    static ALIGN(256) uint32_t ram_vector[64] = {0};
    extern uint32_t __Vectors;
    memcpy((void*)ram_vector, &__Vectors, 256);
    ANA->CPU_ADDR_REMAP_CTRL = ((uint32_t)ram_vector >> 8) | BIT(31);
#endif /* CONFIG_VECTOR_REMAP_TO_RAM */

    pan10xx_hw_calib_init();

#if CONFIG_FLASH_LDO_EN
    ANA->LP_HP_LDO &= ~ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#else
    ANA->LP_HP_LDO |= ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#endif /* CONFIG_FLASH_LDO_EN */

    // Ensure SoC LP configure is sleep mode (default state)
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    LP_SetSleepMode(ANA, LP_MODE_SEL_SLEEP_MODE);

#if (CONFIG_LOW_SPEED_CLOCK_SRC == 0)  /*LP Clock from RCL, 32000 HZ default*/
    /* Enable RCL */
    CLK->RCL_CTRL_3V |= CLK_RCLCTL_RC32K_EN_Msk_3v;
    /* Wait for stable */
    while (!(CLK->RCL_CTRL_3V & CLK_STABLE_STATUS_Msk));
    /* Select RCL as SoC 32K clock source */
    CLK->CLK_TOP_CTRL_3V &= ~CLK_TOPCTL_32K_CLK_SEL_Msk_3v;
    /* Disable lp xtl src */
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    /* Delay more than two 32K clock cycles */
    SYS_delay_10nop(250);
    /* Disable XTL */
    CLK->XTL_CTRL_3V &= ~CLK_XTLCTL_XTL_EN_Msk_3v;
#if CONFIG_FORCE_CALIB_RCL_CLK
#ifdef PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE
    printf("RCL Freq before calib: %d Hz\n", clktrim_measure_32k_clk(DST_CLK_CNT));
#endif
    /* Calibrate RCL to 32K Hz */
    calibrate_rcl_clk(32000);
#ifdef PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE
    printf("RCL Freq after calib: %d Hz\n", clktrim_measure_32k_clk(DST_CLK_CNT));
#endif
#endif /* CONFIG_FORCE_CALIB_RCL_CLK */
#elif ( CONFIG_LOW_SPEED_CLOCK_SRC == 2)  /*LP Clock from ACT32K,32000 HZ default*/
	ANA->ACT_32K_CTRL |= (CLK_ACT32K_TMR_EN_Msk | CLK_ACT32K_LL_32KCLK_SEL_Msk);  /*LP Clock from XTL,32768 HZ default*/
#else /*LP Clock from XTL, 32768 HZ */
#ifdef XTL_SLOW_SETUP
    /* Enable XTL clock */
    CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_EN_Msk_3v;
    while(!(CLK->XTL_CTRL_3V & CLK_XTLCTL_STABLE_Msk));
#else // Quick Setup
    CLK->MEAS_CLK_CTRL = (CLK->MEAS_CLK_CTRL & ~(CLK_MEASCLK_XTH_DIV_Msk)) | (0x1e8 << CLK_MEASCLK_XTH_DIV_Pos);
    CLK->XTL_CTRL_3V = (CLK->XTL_CTRL_3V & ~(CLK_XTLCTL_DELAY_Msk_3v)) | (3 << CLK_XTLCTL_DELAY_Pos_3v);
    /* Enable quick startup */
    CLK->MEAS_CLK_CTRL |= CLK_MEASCLK_XTL_QUICK_EN_Msk;
    SYS_SET_MFP(P2, 0, XTL_C1_CLK);
    SYS_SET_MFP(P2, 1, XTL_C2_CLK);
    /* Enable XTL clock */
    CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_EN_Msk_3v;
    /* Delay a while */
    SYS_delay_10nop(5000);
    /* Disable quick startup */
    CLK->MEAS_CLK_CTRL &= ~CLK_MEASCLK_XTL_QUICK_EN_Msk;
    SYS_SET_MFP(P2, 0, GPIO);
    SYS_SET_MFP(P2, 1, GPIO);
    /* Wait for stable */
    while (!(CLK->XTL_CTRL_3V & CLK_STABLE_STATUS_Msk)) {
        /* Busy wait */
        __NOP();
    }
#endif /* XTL_SLOW_SETUP */
    /* Select XTL as current 32K clock */
    CLK->CLK_TOP_CTRL_3V |= CLK_TOPCTL_32K_CLK_SEL_Msk_3v;
    /* Delay more than two 32K clock cycles */
    SYS_delay_10nop(250);
    /* Disable lp rcl src */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    /* Disable RCL clock */
    CLK->RCL_CTRL_3V &= ~BIT(0);
#endif /* CONFIG_LOW_SPEED_CLOCK_SRC */

    /* Store value in rst status reg and lp int ctrl reg for later possible soc_reset_reason_get() use */
    rst_status_reg = CLK->RSTSTS;
    lp_int_ctrl_reg = ANA->LP_INT_CTRL;
    /* Clear status registers for next time detecting */
    CLK->RSTSTS = CLK->RSTSTS;
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;

    /* Enable lp timer counter */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_SLEEP_CNT_EN_Msk;
    (*(volatile uint32_t *)LPTMR_CURR_CNT_ENA_REG) |= BIT1;

    /* Enable ble related irqs, etc. */
    ble_misc_init();
}

void setup_thread(void *param)
{
    app_init();
}

CONFIG_RAM_CODE void soc_busy_wait(uint32_t us)
{
    while (us--) {
        for (int32_t cnt = SystemCoreClock; cnt > 0; cnt-= 16000000u) {
            __NOP();
        }
    }
}

void __WEAK app_init_early(void)
{
	return;
}

int main()
{
    /* system clock initialize */
    sys_clock_Init();

	/* add a weak function for application initializing early */
	extern void app_init_early(void);
	app_init_early();

#if CONFIG_UART_LOG_ENABLE
    debug_uart_init();
#endif

#if CONFIG_RTT_LOG_ENABLE
    SEGGER_RTT_Init();
#endif

#if NIMBLE_CFG_CONTROLLER
#if (CONFIG_UART_LOG_ENABLE || CONFIG_RTT_LOG_ENABLE)
    pan_misc_register_print(vprintf);
#endif
#endif /* NIMBLE_CFG_CONTROLLER */

	/* Track Init */
#if CONFIG_BT_CTLR_LINK_LAYER_DEBUG
	track_pin_init();
#endif

    /* system configure */
    pan10x_init();

#if CONFIG_PM
    /* Init low power flow */
    extern void deepsleep_init(void);
    deepsleep_init();
#endif

#ifdef NIMBLE_SPARK_SUP
    /* ble stack init*/
    nimble_port_init();
#endif

#if CONFIG_AUTO_OPTIMIZE_POWER_PARAM
#ifdef NIMBLE_SPARK_SUP	
	if(PW_ParamIsHas()){
		HAL_ADC_Init(&adc_temperature);
		temp_check_init();
	}
#endif
#endif

#if ((CONFIG_LOW_SPEED_CLOCK_SRC == 0) || (CONFIG_LOW_SPEED_CLOCK_SRC == 1))
	/* get clock frequence, no matter rcl or xtl */
	clktrim_measure_32k_clk(1000);
#endif

    setup_thread(NULL);

    /* The operating system starts scheduling. */
    vTaskStartScheduler();

    printf("\033[31;1m[ERR] This is not where the program should go\n\033[0m");
    while(1);
}

#if configUSE_MALLOC_FAILED_HOOK
void vApplicationMallocFailedHook(void)
{
    printf("pvPortMalloc failed\n");
}
#endif

__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line) {
  char str[12], *p;

  fputs("*** assertion failed: ", stderr);
  fputs(expr, stderr);
  fputs(", file ", stderr);
  fputs(file, stderr);
  fputs(", line ", stderr);

  p = str + sizeof(str);
  *--p = '\0';
  *--p = '\n';
  while (line > 0) {
    *--p = '0' + (line % 10);
    line /= 10;
  }
  fputs(p, stderr);

  abort();
}

__attribute__((weak))
void abort(void) {
  for (;;);
}
