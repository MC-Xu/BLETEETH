/*
 * Copyright (c) 2020-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _OS_LP_H_
#define _OS_LP_H_

#include <stdint.h>
#include <Panseries.h>

enum {
    SOC_RST_REASON_CHIP_RESET               = 0,
    SOC_RST_REASON_PIN_RESET                = 1,
    SOC_RST_REASON_WDT_RESET                = 2,
    SOC_RST_REASON_LVR_RESET                = 3,
    SOC_RST_REASON_BOD_RESET                = 4,
    SOC_RST_REASON_SYS_RESET                = 5,
    SOC_RST_REASON_POR_RESET                = 6,
    SOC_RST_REASON_STBM0_EXTIO_WAKEUP       = 10,    /* P00/P01/P02 */
    SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP     = 11,
    SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP     = 12,
    SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP     = 13,
    SOC_RST_REASON_STBM1_GPIO_WAKEUP        = 14,    /* All GPIOs */
    SOC_RST_REASON_UNKNOWN_RESET            = 255
};

enum {
    STBM1_WAKEUP_SRC_GPIO           = BIT0,
    STBM1_WAKEUP_SRC_SLPTMR         = BIT1,
};

enum {
    STBM1_RETENTION_SRAM_NONE       = 0,
    STBM1_RETENTION_SRAM_BLOCK0     = BIT0, /* 32 KB */
    STBM1_RETENTION_SRAM_BLOCK1     = BIT1, /* 16 KB */
    STBM1_RETENTION_SRAM_DECRYPT    = BIT2, /* 256 B */
    STBM1_RETENTION_SRAM_PHY_REGS   = BIT3, /* 256 B PHY SEQ RAM + PHY Registers */
    STBM1_RETENTION_SRAM_LL         = BIT4, /* 8 KB */
    STBM1_RETENTION_SRAM_ALL        = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4),
};

/**
 * @brief Read the current counter of LP Timer.
 *
 * This routine returns the current cycle count, as measured by the SoC's
 * hardware LP TImer.
 *
 * @return Current hardware LP Timer clock up-counter (in cycles).
 */
__STATIC_FORCEINLINE uint32_t soc_lptmr_cycle_get(void)
{
    return (*(volatile uint32_t *)(0x50020014));
}

/**
 * @brief Get current 32K low speed clock freqency.
 *
 * @return Current 32K low speed clock freqency in Hz.
 */
__STATIC_FORCEINLINE uint32_t soc_32k_clock_freq_get(void)
{
    return (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v ? 32768 : 32000);
}

/**
 * @brief Read the current counter of LP Timer.
 *
 * This routine returns the current cycle count, as measured by the SoC's
 * hardware LP TImer.
 *
 * @return Current hardware LP Timer clock up-counter (in cycles).
 */
__STATIC_FORCEINLINE uint32_t soc_lptmr_uptime_get_ms(void)
{
    return soc_lptmr_cycle_get() * 1000ull / soc_32k_clock_freq_get();
}

extern void soc_busy_wait(uint32_t us);
extern uint8_t soc_reset_reason_get(void);
extern uint32_t soc_stbm1_gpio_wakeup_src_get(void);
extern void soc_enter_standby_mode_0(void);
extern void soc_enter_standby_mode_1(uint32_t wakeup_src, uint32_t retention_sram);

#endif /* _OS_LP_H_ */
