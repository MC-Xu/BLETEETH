#ifndef _SOC_CONFIG_H_
#define _SOC_CONFIG_H_

/* sdk_cofig.h can overide the macro defined in this file */
#include "sdk_config.h"

/*
 * Here defines macro related to the SoC platform,
 * user can redefine the macro using sdk_config.h.
 * ---> Warning: do not change macro in this file <----
 */

/* Enable DCDC or not */
#ifndef CONFIG_SOC_DCDC_PAN1070
#define CONFIG_SOC_DCDC_PAN1070                             0
#endif

/* Configure sysem main clock in MHz, can be 32 or 48 */
#ifndef CONFIG_SYSTEM_CLOCK
#define CONFIG_SYSTEM_CLOCK                                 32
#endif

/* Divisor of peripheral clocks on APB1, It can only be even numbers b/w 0~30 */
#ifndef CONFIG_APB1_CLOCK_DIVISOR
#define CONFIG_APB1_CLOCK_DIVISOR                           2
#endif

/* Divisor of peripheral clocks on APB2, It can only be even numbers b/w 0~30 */
#ifndef CONFIG_APB2_CLOCK_DIVISOR
#define CONFIG_APB2_CLOCK_DIVISOR                           2
#endif

/*
 * Select a low-speed clock source:
 * 0 means RCL (32000 Hz)
 * 1 means XTL (32768 Hz)
 * 2 means ACT32K (32000 Hz)
 */
 #ifndef CONFIG_LOW_SPEED_CLOCK_SRC
#define CONFIG_LOW_SPEED_CLOCK_SRC                          0
#endif

/* Force Calib RCL Clock */
#ifndef CONFIG_FORCE_CALIB_RCL_CLK
#define CONFIG_FORCE_CALIB_RCL_CLK                          0
#endif

/* Enable UART Log or not */
#ifndef CONFIG_UART_LOG_ENABLE
#define CONFIG_UART_LOG_ENABLE                              1
#endif

/* Select Log UART Tx Pin, 2 means UART0 P16 */
#ifndef CONFIG_LOG_UART_PIN
#define CONFIG_LOG_UART_PIN                                 2
#endif

#if !((CONFIG_LOG_UART_PIN == 0) || (CONFIG_LOG_UART_PIN == 1) || (CONFIG_LOG_UART_PIN == 2))
// This take effect in retarget.c
#define DEBUG_ENABLE_UART1
#endif

/* Set Log UART baudrate */
#ifndef CONFIG_LOG_UART_BAUDRATE
#define CONFIG_LOG_UART_BAUDRATE                            921600
#endif

/* Enable RTT Log or not */
#ifndef CONFIG_RTT_LOG_ENABLE
#define CONFIG_RTT_LOG_ENABLE                               0
#endif

/* Set RTT Log buffer size in bytes (channel 0) */
#ifndef CONFIG_LOG_RTT_UP_BUFFER_SIZE
#define CONFIG_LOG_RTT_UP_BUFFER_SIZE                       512
#endif

/* Enable RAM Function */
#ifndef CONFIG_RAM_FUNCTION
#define CONFIG_RAM_FUNCTION                                 1
#endif

#if CONFIG_RAM_FUNCTION
#define CONFIG_RAM_CODE __attribute__((section(".ramfunc")))
#else
#define CONFIG_RAM_CODE
#endif

/* Enable Flash LDO or not */
#ifndef CONFIG_FLASH_LDO_EN
#define CONFIG_FLASH_LDO_EN                                 1
#endif

/* Remap Vector Table to SRAM */
#ifndef CONFIG_VECTOR_REMAP_TO_RAM
#define CONFIG_VECTOR_REMAP_TO_RAM                          1
#endif

/* Enable Auto Temperature Detecting Flow */
#ifndef CONFIG_AUTO_OPTIMIZE_POWER_PARAM
#define CONFIG_AUTO_OPTIMIZE_POWER_PARAM                    0
#endif

/* ADC temperature sample interval(s) */
#ifndef CONFIG_TEMP_SAMPLE_INTERVAL_S
#define CONFIG_TEMP_SAMPLE_INTERVAL_S                       300
#endif

/* Enable DVDD Voltage Optimization */
#ifndef CONFIG_DVDD_VOL_OPTIMIZE_EN
#define CONFIG_DVDD_VOL_OPTIMIZE_EN                         0
#endif

/* Enable Low Power Mode */
#ifndef CONFIG_PM
#define CONFIG_PM                                           0
#endif

/* Enable System Watchdog or not */
#ifndef CONFIG_SYSTEM_WATCH_DOG_ENABLE
#define CONFIG_SYSTEM_WATCH_DOG_ENABLE                      0
#endif

/* Keep Flash Power in Low Power Mode or not */
#ifndef CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
#define CONFIG_KEEP_FLASH_POWER_IN_LP_MODE                  0
#endif

/* Enable DeepSleep Mode 2 */
#ifndef CONFIG_DEEPSLEEP_MODE_2
#define CONFIG_DEEPSLEEP_MODE_2                             0
#endif

/* Increase LPLDOH trim value, value can be 0~8 */
#ifndef CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE
#define CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE               0
#endif

/* Continue Run After Standby M1 Wakeup */
#ifndef CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
#define CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET           0
#endif

/* Enable AHB Clock Optimization or not */
#ifndef CONFIG_HCLK_OPTIMIZE_EN
#define CONFIG_HCLK_OPTIMIZE_EN                             0
#endif

#endif /* End of _SOC_CONFIG_H_ */
