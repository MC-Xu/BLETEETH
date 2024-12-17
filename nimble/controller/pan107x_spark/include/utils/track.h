/**
 *******************************************************************************
 * @FileName  : track.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-02-18
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */

#ifndef TRACK_H_
#define TRACK_H_

#include "defs_types.h"
#include <stdarg.h>


/*! Track debug system config. (1: UART; 3:USB) */
#define TRACK_MODE_NONE               0
#define TRACK_MODE_UART               1  /** Hardware UART */
#define TRACK_MODE_SUART              2  /** Software UART: !!!!WARN - BLE timing error. */
#define TRACK_MODE_USB                3  /** USB  */

#ifndef TRACK_MODE
#define TRACK_MODE                    TRACK_MODE_NONE
#endif


/*! Timing debug system enable/disable. */
#ifndef TIMING_TRACK_EN
#define TIMING_TRACK_EN               0
#endif

/*! Custom Track Timing Enable/Disable */
#ifndef CUSTOM_TRACK_PRINT_EN
#define CUSTOM_TRACK_PRINT_EN         1
#endif

/*! Custom Track Timing Enable/Disable */
#ifndef CUSTOM_TRACK_TIMING_EN
#define CUSTOM_TRACK_TIMING_EN        1
#endif

#define TRACK_UART_PORT               UART0
#define TRACK_BAUDRATE                2000000


#if CUSTOM_TRACK_PRINT_EN
	#define TRACK_INIT()
	#define TRACK_START()
	#define g_printf                     TrackPrint
	#undef  TRACK_MODE
	#define TRACK_MODE                   TRACK_MODE_UART
#else
  #if TRACK_MODE != TRACK_MODE_NONE
	#define TRACK_INIT()                  TrackInit()
	#define TRACK_START()                 TrackStart()
	#define g_printf                      TrackPrint
  #else
	#define TRACK_INIT()
	#define TRACK_START()
	#define g_printf
  #endif
#endif

#if TRACK_MODE
	#define COM_TRACK_EN       1
    #define BB_TRACK_EN        1
	#define DRIV_TRACK_EN      0
	#define LL_TRACK_EN        1
	#define ENC_TRACK_EN       0
	#define HCI_TRACK_EN       0
	#define HCI_H5_TRACK_EN    0
	#define HCI_SLIP_TRACK_EN  0
    #define HCI_TR_TRACK_EN    0
    #define DM_TRACK_EN        0
	#define L2C_TRACK_EN       0
	#define ATT_TRACK_EN       0
	#define GATT_TRACK_EN      0
	#define SMP_TRACK_EN       0
	#define SDP_TRACK_EN       0
	#define APP_TRACK_EN       1
	#define USB_TRACK_EN       0
    #define MEM_TRACK_EN       1
#else
	#define COM_TRACK_EN       0
    #define BB_TRACK_EN        0
	#define DRIV_TRACK_EN      0
	#define LL_TRACK_EN        0
	#define ENC_TRACK_EN       0
	#define HCI_TRACK_EN       0
	#define HCI_TR_TRACK_EN    0
	#define HCI_H5_TRACK_EN    0
	#define HCI_SLIP_TRACK_EN  0
    #define HCI_TR_TRACK_EN    0
    #define DM_TRACK_EN        0
	#define L2C_TRACK_EN       0
	#define ATT_TRACK_EN       0
	#define GATT_TRACK_EN      0
	#define SMP_TRACK_EN       0
	#define SDP_TRACK_EN       0
	#define APP_TRACK_EN       0
	#define USB_TRACK_EN       0
    #define MEM_TRACK_EN       0
#endif


#define TRACK_COLOR_EN           	0

#define LOG_COLOR           "\033[0;%dm"
#define LOG_BOLD            "\033[1;%dm"
#define LOG_RESET_COLOR     "\033[0m"

#if TRACK_COLOR_EN
	/* ! The color for terminal (foreground)
	#define BLACK    30
	#define RED      31
	#define GREEN    32
	#define YELLOW   33
	#define BLUE     34
	#define PURPLE   35
	#define CYAN     36
	#define WHITE    37
	*/

	#define TRACK_COLOR_START(c)          g_printf("\033["#c"m")
	#define TRACK_COLOR_END()             g_printf("\033[0m")
#else
	#define TRACK_COLOR_START(c)
	#define TRACK_COLOR_END()
#endif

#if COM_TRACK_EN
	#define TRACK(format, ...)             g_printf(format, ##__VA_ARGS__)
	#define TRACK_ERR(format, ...)         g_printf("[E]"format, ##__VA_ARGS__)
	#define TRACK_INFO(format, ...)        g_printf("[I]"format, ##__VA_ARGS__)
	#define TRACK_WRN(format, ...)         g_printf("[W]"format, ##__VA_ARGS__)
#else
	#define TRACK(format, ...)
	#define TRACK_ERR(format, ...)
	#define TRACK_INFO(format, ...)
	#define TRACK_WRN(format, ...)
#endif

#if DRIV_TRACK_EN
	#define DRIV_TRACK_ERR(format, ...)    g_printf("[E][DRV] "format, ##__VA_ARGS__)
	#define DRIV_TRACK_INFO(format, ...)   g_printf("[I][DRV] "format, ##__VA_ARGS__)
	#define DRIV_TRACK_WRN(format, ...)    g_printf("[W][DRV] "format, ##__VA_ARGS__)
#else
	#define DRIV_TRACK_ERR(format, ...)
	#define DRIV_TRACK_INFO(format, ...)
	#define DRIV_TRACK_WRN(format, ...)
#endif

#if BB_TRACK_EN
	#define BB_TRACK_ERR(format, ...)      g_printf("[E][BB_] "format, ##__VA_ARGS__)
	#define BB_TRACK_INFO(format, ...)     g_printf("[I][BB_] "format, ##__VA_ARGS__)
	#define BB_TRACK_WRN(format, ...)      g_printf("[W][BB_] "format, ##__VA_ARGS__)
	#define BB_TRACK_DATA(pdata, len)      TRACK_DATA(pdata, len)
#else
	#define BB_TRACK_ERR(format, ...)
	#define BB_TRACK_INFO(format, ...)
	#define BB_TRACK_WRN(format, ...)
	#define BB_TRACK_DATA(pdata, len)
#endif

#if LL_TRACK_EN
	#define LL_TRACK_ERR(format, ...)      g_printf("[E][LL_] "format, ##__VA_ARGS__)
	#define LL_TRACK_INFO(format, ...)     g_printf("[I][LL_] "format, ##__VA_ARGS__)
	#define LL_TRACK_WRN(format, ...)      g_printf("[W][LL_] "format, ##__VA_ARGS__)
	#define LL_TRACK_DATA(pdata, len)      TRACK_DATA(pdata, len)
#else
	#define LL_TRACK_ERR(format, ...)
	#define LL_TRACK_INFO(format, ...)
	#define LL_TRACK_WRN(format, ...)
	#define LL_TRACK_DATA(pdata, len)
#endif

#if ENC_TRACK_EN
	#define ENC_TRACK_ERR(format, ...)      g_printf("[E][ENC] "format, ##__VA_ARGS__)
	#define ENC_TRACK_INFO(format, ...)     g_printf("[I][ENC] "format, ##__VA_ARGS__)
	#define ENC_TRACK_WRN(format, ...)      g_printf("[W][ENC] "format, ##__VA_ARGS__)
	#define ENC_TRACK_DATA(pdata, len)      TRACK_DATA(pdata, len)
#else
	#define ENC_TRACK_ERR(format, ...)
	#define ENC_TRACK_INFO(format, ...)
	#define ENC_TRACK_WRN(format, ...)
	#define ENC_TRACK_DATA(pdata, len)
#endif

#if HCI_TRACK_EN
	#define HCI_TRACK_ERR(format, ...)     g_printf("[E][HCI] "format, ##__VA_ARGS__)
	#define HCI_TRACK_INFO(format, ...)    g_printf("[I][HCI] "format, ##__VA_ARGS__)
	#define HCI_TRACK_WRN(format, ...)     g_printf("[W][HCI] "format, ##__VA_ARGS__)
	#define HCI_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define HCI_TRACK_ERR(format, ...)
	#define HCI_TRACK_INFO(format, ...)
	#define HCI_TRACK_WRN(format, ...)
	#define HCI_TRACK_DATA(pData, len)
#endif

#if HCI_H5_TRACK_EN
	#define HCI_H5_TRACK_ERR(format, ...)     g_printf("[E][H5_] "format, ##__VA_ARGS__)
	#define HCI_H5_TRACK_INFO(format, ...)    g_printf("[I][H5_] "format, ##__VA_ARGS__)
	#define HCI_H5_TRACK_WRN(format, ...)     g_printf("[W][H5_] "format, ##__VA_ARGS__)
#else
	#define HCI_H5_TRACK_ERR(format, ...)
	#define HCI_H5_TRACK_INFO(format, ...)
	#define HCI_H5_TRACK_WRN(format, ...)
#endif

#if HCI_SLIP_TRACK_EN
	#define HCI_SLIP_TRACK_ERR(format, ...)   g_printf("[E][SLIP] "format, ##__VA_ARGS__)
	#define HCI_SLIP_TRACK_INFO(format, ...)  g_printf("[I][SLIP] "format, ##__VA_ARGS__)
	#define HCI_SLIP_TRACK_WRN(format, ...)   g_printf("[W][SLIP] "format, ##__VA_ARGS__)
#else
	#define HCI_SLIP_TRACK_ERR(format, ...)
	#define HCI_SLIP_TRACK_INFO(format, ...)
	#define HCI_SLIP_TRACK_WRN(format, ...)
#endif

#if HCI_TR_TRACK_EN
	#define HCI_TR_TRACK_ERR(format, ...)     g_printf("[E][TR_] "format, ##__VA_ARGS__)
	#define HCI_TR_TRACK_INFO(format, ...)    g_printf("[I][TR_] "format, ##__VA_ARGS__)
	#define HCI_TR_TRACK_WRN(format, ...)     g_printf("[W][TR_] "format, ##__VA_ARGS__)
	#define HCI_TR_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define HCI_TR_TRACK_ERR(format, ...)
	#define HCI_TR_TRACK_INFO(format, ...)
	#define HCI_TR_TRACK_WRN(format, ...)
#endif

#if DM_TRACK_EN
	#define DM_TRACK_ERR(format, ...)     g_printf("[E][DM_] "format, ##__VA_ARGS__)
	#define DM_TRACK_INFO(format, ...)    g_printf("[I][DM_] "format, ##__VA_ARGS__)
	#define DM_TRACK_WRN(format, ...)     g_printf("[W][DM_] "format, ##__VA_ARGS__)
#else
	#define DM_TRACK_ERR(format, ...)
	#define DM_TRACK_INFO(format, ...)
	#define DM_TRACK_WRN(format, ...)
#endif

#if L2C_TRACK_EN
	#define L2C_TRACK_ERR(format, ...)     g_printf("[E][L2C] "format, ##__VA_ARGS__)
	#define L2C_TRACK_INFO(format, ...)    g_printf("[I][L2C] "format, ##__VA_ARGS__)
	#define L2C_TRACK_WRN(format, ...)     g_printf("[W][L2C] "format, ##__VA_ARGS__)
	#define L2C_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define L2C_TRACK_ERR(format, ...)
	#define L2C_TRACK_INFO(format, ...)
	#define L2C_TRACK_WRN(format, ...)
	#define L2C_TRACK_DATA(pData, len)
#endif

#if ATT_TRACK_EN
	#define ATT_TRACK_ERR(format, ...)     g_printf("[E][ATT] "format, ##__VA_ARGS__)
	#define ATT_TRACK_INFO(format, ...)    g_printf("[I][ATT] "format, ##__VA_ARGS__)
	#define ATT_TRACK_WRN(format, ...)     g_printf("[W][ATT] "format, ##__VA_ARGS__)
	#define ATT_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define ATT_TRACK_ERR(format, ...)
	#define ATT_TRACK_INFO(format, ...)
	#define ATT_TRACK_WRN(format, ...)
    #define ATT_TRACK_DATA(pData, len)
#endif

#if GATT_TRACK_EN
	#define GATT_TRACK_ERR(format, ...)    g_printf("[E][GATT] "format, ##__VA_ARGS__)
	#define GATT_TRACK_INFO(format, ...)   g_printf("[I][GATT] "format, ##__VA_ARGS__)
	#define GATT_TRACK_WRN(format, ...)    g_printf("[W][GATT] "format, ##__VA_ARGS__)
#else
	#define GATT_TRACK_ERR(format, ...)
	#define GATT_TRACK_INFO(format, ...)
	#define GATT_TRACK_WRN(format, ...)
#endif

#if SMP_TRACK_EN
	#define SMP_TRACK_ERR(format, ...)     g_printf("[E][SMP] "format, ##__VA_ARGS__)
	#define SMP_TRACK_INFO(format, ...)    g_printf("[I][SMP] "format, ##__VA_ARGS__)
	#define SMP_TRACK_WRN(format, ...)     g_printf("[W][SMP] "format, ##__VA_ARGS__)
	#define SMP_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define SMP_TRACK_ERR(format, ...)
	#define SMP_TRACK_INFO(format, ...)
	#define SMP_TRACK_WRN(format, ...)
	#define SMP_TRACK_DATA(pData, len)
#endif

#if SDP_TRACK_EN
	#define SDP_TRACK_ERR(format, ...)     g_printf("[E][SDP] "format, ##__VA_ARGS__)
	#define SDP_TRACK_INFO(format, ...)    g_printf("[I][SDP] "format, ##__VA_ARGS__)
	#define SDP_TRACK_WRN(format, ...)     g_printf("[W][SDP] "format, ##__VA_ARGS__)
	#define SDP_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define SDP_TRACK_ERR(format, ...)
	#define SDP_TRACK_INFO(format, ...)
	#define SDP_TRACK_WRN(format, ...)
	#define SDP_TRACK_DATA(pData, len)
#endif

#if APP_TRACK_EN
	#define APP_TRACK_ERR(format, ...)     g_printf("[E][APP] "format, ##__VA_ARGS__)
	#define APP_TRACK_INFO(format, ...)    g_printf("[I][APP] "format, ##__VA_ARGS__)
	#define APP_TRACK_WRN(format, ...)     g_printf("[W][APP] "format, ##__VA_ARGS__)
	#define APP_TRACK_DATA(pData, len)      TRACK_DATA(pData, len)
#else
	#define APP_TRACK_ERR(format, ...)
	#define APP_TRACK_INFO(format, ...)
	#define APP_TRACK_WRN(format, ...)
	#define APP_TRACK_DATA(pData, len)
#endif

#if USB_TRACK_EN
	#define USB_TRACK_ERR(format, ...)     g_printf("[E][USB] "format, ##__VA_ARGS__)
	#define USB_TRACK_INFO(format, ...)    g_printf("[I][USB] "format, ##__VA_ARGS__)
	#define USB_TRACK_WRN(format, ...)     g_printf("[W][USB] "format, ##__VA_ARGS__)
	#define USB_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define USB_TRACK_ERR(format, ...)
	#define USB_TRACK_INFO(format, ...)
	#define USB_TRACK_WRN(format, ...)
	#define USB_TRACK_DATA(pData, len)
#endif

#if MEM_TRACK_EN
	#define MEM_TRACK_ERR(format, ...)     g_printf("[E][MEM] "format, ##__VA_ARGS__)
	#define MEM_TRACK_INFO(format, ...)    g_printf("[I][MEM] "format, ##__VA_ARGS__)
	#define MEM_TRACK_WRN(format, ...)     g_printf("[W][MEM] "format, ##__VA_ARGS__)
	#define MEM_TRACK_DATA(pData, len)     TRACK_DATA(pData, len)
#else
	#define MEM_TRACK_ERR(format, ...)
	#define MEM_TRACK_INFO(format, ...)
	#define MEM_TRACK_WRN(format, ...)
	#define MEM_TRACK_DATA(pData, len)
#endif

#if TRACK_MODE
	#define TRACK_DATA(pData, len)             TrackData(pData, len)
	#define TRACK_STR_DATA(pDesc, pData, len)  TrackStringData(pDesc, pData, len)
#else
	#define TRACK_DATA(pData, len)
	#define TRACK_STR_DATA(pDesc, pData, len)
#endif

#if CUSTOM_TRACK_TIMING_EN
	#define TIMING_TRACK_INIT()

	#define DBG_CHN0_HI        do{ if(dbg_scan_pin) {REG_32(dbg_scan_pin) = 1;} }while(0);
	#define DBG_CHN0_LO        do{ if(dbg_scan_pin) {REG_32(dbg_scan_pin) = 0;} }while(0);
	#define DBG_CHN0_TOGGLE

	#define DBG_CHN1_HI        do{ if(dbg_adv_pin) {REG_32(dbg_adv_pin) = 1;} }while(0);
	#define DBG_CHN1_LO        do{ if(dbg_adv_pin) {REG_32(dbg_adv_pin) = 0;} }while(0);
	#define DBG_CHN1_TOGGLE

	#define DBG_CHN2_HI        do{ if(dbg_timer_irq_pin) {REG_32(dbg_timer_irq_pin) = 1;} }while(0);
	#define DBG_CHN2_LO        do{ if(dbg_timer_irq_pin) {REG_32(dbg_timer_irq_pin) = 0;} }while(0);
	#define DBG_CHN2_TOGGLE

	#define DBG_CHN3_HI        do{ if(dbg_ll_irq_pin) {REG_32(dbg_ll_irq_pin) = 1;} }while(0);
	#define DBG_CHN3_LO        do{ if(dbg_ll_irq_pin) {REG_32(dbg_ll_irq_pin) = 0;} }while(0);
	#define DBG_CHN3_TOGGLE

	#define DBG_CHN4_HI        do{ if(dbg_conn_pin[0]) {REG_32(dbg_conn_pin[0]) = 1;} }while(0);
	#define DBG_CHN4_LO        do{ if(dbg_conn_pin[0]) {REG_32(dbg_conn_pin[0]) = 0;} }while(0);
	#define DBG_CHN4_TOGGLE

	#define DBG_CHN5_HI        do{ if(dbg_conn_pin[1]) {REG_32(dbg_conn_pin[1]) = 1;} }while(0);
	#define DBG_CHN5_LO        do{ if(dbg_conn_pin[1]) {REG_32(dbg_conn_pin[1]) = 0;} }while(0);
	#define DBG_CHN5_TOGGLE

	#define DBG_CHN6_HI	       do{ if(dbg_conn_pin[2]) {REG_32(dbg_conn_pin[2]) = 1;} }while(0);
	#define DBG_CHN6_LO        do{ if(dbg_conn_pin[2]) {REG_32(dbg_conn_pin[2]) = 0;} }while(0);
	#define DBG_CHN6_TOGGLE

	#define DBG_CHN7_HI        do{ if(dbg_sch_pin) {REG_32(dbg_sch_pin) = 1;} }while(0);
	#define DBG_CHN7_LO        do{ if(dbg_sch_pin) {REG_32(dbg_sch_pin) = 0;} }while(0);
	#define DBG_CHN7_TOGGLE


	#define DBG_CHN8_HI
	#define DBG_CHN8_LO
	#define DBG_CHN8_TOGGLE

	#define DBG_CHN9_HI
	#define DBG_CHN9_LO
	#define DBG_CHN9_TOGGLE

	#define DBG_CHN10_HI
	#define DBG_CHN10_LO
	#define DBG_CHN10_TOGGLE

	#define DBG_CHN11_HI
	#define DBG_CHN11_LO
	#define DBG_CHN11_TOGGLE

	#define DBG_CHN12_HI
	#define DBG_CHN12_LO
	#define DBG_CHN12_TOGGLE

	#define DBG_CHN13_HI
	#define DBG_CHN13_LO
	#define DBG_CHN13_TOGGLE

	#define DBG_CHN14_HI
	#define DBG_CHN14_LO
	#define DBG_CHN14_TOGGLE

	#define DBG_CHN15_HI
	#define DBG_CHN15_LO
	#define DBG_CHN15_TOGGLE

#elif TIMING_TRACK_EN
	#define TIMING_TRACK_INIT()             TrackTimingInit()

  #if defined(PAN10x_DK)
	#define DBG_CHN0_PIN      GPIO_P50
	#define DBG_CHN1_PIN      GPIO_P23
	#define DBG_CHN2_PIN      GPIO_P43
	#define DBG_CHN3_PIN      GPIO_P42
	#define DBG_CHN4_PIN      GPIO_P40
	#define DBG_CHN5_PIN      GPIO_P24
	#define DBG_CHN6_PIN      GPIO_P27
	#define DBG_CHN7_PIN      GPIO_P02

	#define DBG_CHN8_PIN      GPIO_P52
	#define DBG_CHN9_PIN      GPIO_P14
	#define DBG_CHN10_PIN     GPIO_P51
	#define DBG_CHN11_PIN     GPIO_P22
	#define DBG_CHN12_PIN     GPIO_P44
	#define DBG_CHN13_PIN     GPIO_P41
	#define DBG_CHN14_PIN     GPIO_P25
	#define DBG_CHN15_PIN     GPIO_P31

  #elif defined(PAN10x_DK_32)
	#define DBG_CHN0_PIN      GPIO_P03
	#define DBG_CHN1_PIN      GPIO_P02
	#define DBG_CHN2_PIN      GPIO_P31
	#define DBG_CHN3_PIN      GPIO_P30
	#define DBG_CHN4_PIN      GPIO_P24
	#define DBG_CHN5_PIN      GPIO_P10
	#define DBG_CHN6_PIN      GPIO_P11
	#define DBG_CHN7_PIN      GPIO_P06

	#define DBG_CHN8_PIN      GPIO_Pin_None
	#define DBG_CHN9_PIN      GPIO_Pin_None
	#define DBG_CHN10_PIN     GPIO_Pin_None
	#define DBG_CHN11_PIN     GPIO_Pin_None
	#define DBG_CHN12_PIN     GPIO_Pin_None
	#define DBG_CHN13_PIN     GPIO_Pin_None
	#define DBG_CHN14_PIN     GPIO_Pin_None
	#define DBG_CHN15_PIN     GPIO_Pin_None

  #elif defined(PAN10x_DG)
	#define DBG_CHN0_PIN      GPIO_P02
	#define DBG_CHN1_PIN      GPIO_P03
	#define DBG_CHN2_PIN      GPIO_P30
	#define DBG_CHN3_PIN      GPIO_P31
	#define DBG_CHN4_PIN      GPIO_P24
	#define DBG_CHN5_PIN      GPIO_P07
	#define DBG_CHN6_PIN      GPIO_P06
	#define DBG_CHN7_PIN      GPIO_P16

	#define DBG_CHN8_PIN      GPIO_Pin_None
	#define DBG_CHN9_PIN      GPIO_Pin_None
	#define DBG_CHN10_PIN     GPIO_Pin_None
	#define DBG_CHN11_PIN     GPIO_Pin_None
	#define DBG_CHN12_PIN     GPIO_Pin_None
	#define DBG_CHN13_PIN     GPIO_Pin_None
	#define DBG_CHN14_PIN     GPIO_Pin_None
	#define DBG_CHN15_PIN     GPIO_Pin_None

  #elif defined(PAN107x_DK)
  	#define DBG_CHN0_PIN      GPIO_P22
	#define DBG_CHN1_PIN      GPIO_P24
	#define DBG_CHN2_PIN      GPIO_P25
	#define DBG_CHN3_PIN      GPIO_P03
	#define DBG_CHN4_PIN      GPIO_P26
	#define DBG_CHN5_PIN      GPIO_P12
	#define DBG_CHN6_PIN      GPIO_P11
	#define DBG_CHN7_PIN      GPIO_P15

	#define DBG_CHN8_PIN      GPIO_P23
	#define DBG_CHN9_PIN      GPIO_P27
	#define DBG_CHN10_PIN     GPIO_P05
	#define DBG_CHN11_PIN     GPIO_P02
	#define DBG_CHN12_PIN     GPIO_P06
	#define DBG_CHN13_PIN     GPIO_P13
	#define DBG_CHN14_PIN     GPIO_P14
	#define DBG_CHN15_PIN     GPIO_P14

  #elif defined(PAN107x_DK_32)
  	#define DBG_CHN0_PIN      GPIO_P15
	#define DBG_CHN1_PIN      GPIO_P13
	#define DBG_CHN2_PIN      GPIO_P14
	#define DBG_CHN3_PIN      GPIO_P12//GPIO_P11
	#define DBG_CHN4_PIN      GPIO_P05
	#define DBG_CHN5_PIN      GPIO_P03
	#define DBG_CHN6_PIN      GPIO_P24
	#define DBG_CHN7_PIN      GPIO_P23

	#define DBG_CHN8_PIN      GPIO_Pin_None
	#define DBG_CHN9_PIN      GPIO_Pin_None
	#define DBG_CHN10_PIN     GPIO_Pin_None
	#define DBG_CHN11_PIN     GPIO_Pin_None
	#define DBG_CHN12_PIN     GPIO_Pin_None
	#define DBG_CHN13_PIN     GPIO_Pin_None
	#define DBG_CHN14_PIN     GPIO_Pin_None
	#define DBG_CHN15_PIN     GPIO_Pin_None
  #endif

	#define DBG_CHN0_HI       GPIO_SetBit(DBG_CHN0_PIN);
	#define DBG_CHN0_LO       GPIO_ResetBit(DBG_CHN0_PIN);
	#define DBG_CHN0_TOGGLE   GPIO_ToggleEx(DBG_CHN0_PIN);

	#define DBG_CHN1_HI       GPIO_SetBit(DBG_CHN1_PIN);
	#define DBG_CHN1_LO       GPIO_ResetBit(DBG_CHN1_PIN);
	#define DBG_CHN1_TOGGLE   GPIO_ToggleEx(DBG_CHN1_PIN);

	#define DBG_CHN2_HI       GPIO_SetBit(DBG_CHN2_PIN);
	#define DBG_CHN2_LO       GPIO_ResetBit(DBG_CHN2_PIN);
	#define DBG_CHN2_TOGGLE   GPIO_ToggleEx(DBG_CHN2_PIN);

	#define DBG_CHN3_HI       GPIO_SetBit(DBG_CHN3_PIN);
	#define DBG_CHN3_LO       GPIO_ResetBit(DBG_CHN3_PIN);
	#define DBG_CHN3_TOGGLE   GPIO_ToggleEx(DBG_CHN3_PIN);

	#define DBG_CHN4_HI       GPIO_SetBit(DBG_CHN4_PIN);
	#define DBG_CHN4_LO       GPIO_ResetBit(DBG_CHN4_PIN);
	#define DBG_CHN4_TOGGLE   GPIO_ToggleEx(DBG_CHN4_PIN);

	#define DBG_CHN5_HI       GPIO_SetBit(DBG_CHN5_PIN);
	#define DBG_CHN5_LO       GPIO_ResetBit(DBG_CHN5_PIN);
	#define DBG_CHN5_TOGGLE   GPIO_ToggleEx(DBG_CHN5_PIN);

	#define DBG_CHN6_HI       GPIO_SetBit(DBG_CHN6_PIN);
	#define DBG_CHN6_LO       GPIO_ResetBit(DBG_CHN6_PIN);
	#define DBG_CHN6_TOGGLE   GPIO_ToggleEx(DBG_CHN6_PIN);

	#define DBG_CHN7_HI       GPIO_SetBit(DBG_CHN7_PIN);
	#define DBG_CHN7_LO       GPIO_ResetBit(DBG_CHN7_PIN);
	#define DBG_CHN7_TOGGLE   GPIO_ToggleEx(DBG_CHN7_PIN);


	#define DBG_CHN8_HI       GPIO_SetBit(DBG_CHN8_PIN);
	#define DBG_CHN8_LO       GPIO_ResetBit(DBG_CHN8_PIN);
	#define DBG_CHN8_TOGGLE   GPIO_ToggleEx(DBG_CHN8_PIN);

	#define DBG_CHN9_HI       GPIO_SetBit(DBG_CHN9_PIN);
	#define DBG_CHN9_LO       GPIO_ResetBit(DBG_CHN9_PIN);
	#define DBG_CHN9_TOGGLE   GPIO_ToggleEx(DBG_CHN9_PIN);

	#define DBG_CHN10_HI      GPIO_SetBit(DBG_CHN10_PIN);
	#define DBG_CHN10_LO      GPIO_ResetBit(DBG_CHN10_PIN);
	#define DBG_CHN10_TOGGLE  GPIO_ToggleEx(DBG_CHN10_PIN);

	#define DBG_CHN11_HI      GPIO_SetBit(DBG_CHN11_PIN);
	#define DBG_CHN11_LO      GPIO_ResetBit(DBG_CHN11_PIN);
	#define DBG_CHN11_TOGGLE  GPIO_ToggleEx(DBG_CHN11_PIN);

	#define DBG_CHN12_HI      GPIO_SetBit(DBG_CHN12_PIN);
	#define DBG_CHN12_LO      GPIO_ResetBit(DBG_CHN12_PIN);
	#define DBG_CHN12_TOGGLE  GPIO_ToggleEx(DBG_CHN12_PIN);

	#define DBG_CHN13_HI      GPIO_SetBit(DBG_CHN13_PIN);
	#define DBG_CHN13_LO      GPIO_ResetBit(DBG_CHN13_PIN);
	#define DBG_CHN13_TOGGLE  GPIO_ToggleEx(DBG_CHN13_PIN);

	#define DBG_CHN14_HI      GPIO_SetBit(DBG_CHN14_PIN);
	#define DBG_CHN14_LO      GPIO_ResetBit(DBG_CHN14_PIN);
	#define DBG_CHN14_TOGGLE  GPIO_ToggleEx(DBG_CHN14_PIN);

	#define DBG_CHN15_HI      GPIO_SetBit(DBG_CHN15_PIN);
	#define DBG_CHN15_LO      GPIO_ResetBit(DBG_CHN15_PIN);
	#define DBG_CHN15_TOGGLE  GPIO_ToggleEx(DBG_CHN15_PIN);

#else
	#define TIMING_TRACK_INIT()


	#define DBG_CHN0_HI
	#define DBG_CHN0_LO
	#define DBG_CHN0_TOGGLE

	#define DBG_CHN1_HI
	#define DBG_CHN1_LO
	#define DBG_CHN1_TOGGLE

	#define DBG_CHN2_HI
	#define DBG_CHN2_LO
	#define DBG_CHN2_TOGGLE

	#define DBG_CHN3_HI
	#define DBG_CHN3_LO
	#define DBG_CHN3_TOGGLE

	#define DBG_CHN4_HI
	#define DBG_CHN4_LO
	#define DBG_CHN4_TOGGLE

	#define DBG_CHN5_HI
	#define DBG_CHN5_LO
	#define DBG_CHN5_TOGGLE

	#define DBG_CHN6_HI
	#define DBG_CHN6_LO
	#define DBG_CHN6_TOGGLE

	#define DBG_CHN7_HI
	#define DBG_CHN7_LO
	#define DBG_CHN7_TOGGLE


	#define DBG_CHN8_HI
	#define DBG_CHN8_LO
	#define DBG_CHN8_TOGGLE

	#define DBG_CHN9_HI
	#define DBG_CHN9_LO
	#define DBG_CHN9_TOGGLE

	#define DBG_CHN10_HI
	#define DBG_CHN10_LO
	#define DBG_CHN10_TOGGLE

	#define DBG_CHN11_HI
	#define DBG_CHN11_LO
	#define DBG_CHN11_TOGGLE

	#define DBG_CHN12_HI
	#define DBG_CHN12_LO
	#define DBG_CHN12_TOGGLE

	#define DBG_CHN13_HI
	#define DBG_CHN13_LO
	#define DBG_CHN13_TOGGLE

	#define DBG_CHN14_HI
	#define DBG_CHN14_LO
	#define DBG_CHN14_TOGGLE

	#define DBG_CHN15_HI
	#define DBG_CHN15_LO
	#define DBG_CHN15_TOGGLE
#endif

typedef int (*print_func_t)(const char *fmt, va_list ap);
void TrackRegisterPrint(print_func_t func);

/* Track Format Type */
enum{
	TRACK_DATA_TYPE_SCH  = 0,
	TRACK_DATA_TYPE_SLV  = 1,
	TRACK_DATA_TYPE_MST  = 2,
	TRACK_DATA_TYPE_ENC  = 3,
};

/* Timing Format */
typedef struct{
	uint32_t  state;

	uint32_t  anchorPoint1;
	uint32_t  anchorPoint2;

	uint32_t  endAnchorPoint1;
	uint32_t  endAnchorPoint2;

	uint32_t  windowWiden1;
	uint32_t  windowWiden2;

	uint32_t err1;
	uint32_t err2;
}Timing_t;

/* for encrypt */
typedef struct{
	uint32_t  w;
	uint32_t  r;
	uint32_t  len;
	uint8_t   sk[16];
	uint8_t   nonce[16];
	uint8_t   data[2 + 27 + 3];
}EncFormat_t;

/* for scan */
typedef struct{
	uint32_t  id;
	uint32_t  curTick;
	uint32_t  endAnchorPoint;
	uint32_t  winTimeUs;
}ScanFormat_t;


typedef struct{
	uint32_t t0;
	uint32_t t1;
	uint32_t cmp;
	uint32_t busIRQ;
}BBFormat_t;

/* Debug Data structure */
typedef struct{
	uint32_t  type;
	union{
		Timing_t     timing;
		EncFormat_t  enc;
	};
}DebugFrame_t;

void TrackPushData(DebugFrame_t pBuf);
#define TRACK_PUSH_DATA(debug)     TrackPushData(debug)

// Print Track
void TrackInit(void);
int TrackPrint(char *fmt, ...);
void TrackData(uint8_t *pdata, uint32_t len);
void TrackStart(void);
void TrackPrintSystemInfo(void);
void assert_failed(uint8_t exp, uint32_t errCode, char *fileName, int line);

// Timing Track
void TrackTimingInit(void);

#define DBG_PIN_NUM  3

#if CUSTOM_TRACK_TIMING_EN
typedef struct{
	uint32_t  sch_pin;
	uint32_t  scan_pin;
	uint32_t  adv_pin;
	uint32_t  conn_pin[DBG_PIN_NUM];
	uint32_t  timer_irq_pin;
	uint32_t  ll_irq_pin;
}DbgPin_t;

extern volatile uint32_t dbg_sch_pin;
extern volatile uint32_t dbg_scan_pin;
extern volatile uint32_t dbg_adv_pin;
extern volatile uint32_t dbg_conn_pin[];
extern volatile uint32_t dbg_timer_irq_pin;
extern volatile uint32_t dbg_ll_irq_pin;

void TrackTimingInitEx(DbgPin_t *debugPins);
#endif

#endif /* TRACK_H_ */
