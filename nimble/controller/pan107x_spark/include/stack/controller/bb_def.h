/**
 *******************************************************************************
 * @FileName  : bb_def.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-09-10
 * @Copyright : Copyright(C) PanChip
 *              All Rights Reserved.
 *
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * PanChip and is available under the terms of Commercial License Agreement
 * between PanChip and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */
#ifndef BB_DEF_H_
#define BB_DEF_H_

#include "utils/defs_types.h"

/* BB Register Base Address. */
#define BB_BASE              0x50020000
#define RCC_BASE             0x40040000


#define BB_SEQ_RAM_BASE      (BB_BASE + 0x1000)
#define BB_LIST_RAM_BASE     (BB_BASE + 0x8000)  /* 512B */
//#define BB_TX_RX_BUF_BASE    (BB_BASE + 0x8200)  /* 7.5K  + 8K for BB TX/RX Buffer */
//#define BB_TX_RX_BUF_BASE    (BB_BASE + 0x8100)  /* 7.5K  + 8K for BB TX/RX Buffer */

#define BB_REG32(addr)       (*((volatile uint32_t *)(BB_BASE + addr)))
#define RCC_REG32(addr)      (*((volatile uint32_t *)(RCC_BASE + addr)))


#define BB_REG_WRITE(addr, mask, pos, value) \
{\
	uint32_t v = BB_REG32(addr);\
	v &= ~(mask);\
	v |= ((value) << (pos)) & (mask);\
	BB_REG32(addr) = v;\
}


/* RCC Register */
#define RCC_RST0_ADDR         0x04


/* LL Register Address define. */
#define BB_VERSION_ADDR       0x0000
#define BB_CTRL_ADDR          0x0008
enum{
	BB_CTRL_BUS_CLK_EN_Pos    = 10,
	#define BB_CTRL_BUS_CLK_EN_Mask     (1<<BB_CTRL_BUS_CLK_EN_Pos)

	BB_CTRL_PM_PWR_STATE_Pos  = 4,
	#define BB_CTRL_PM_PWR_STATE_Mask   (0x3Ful<< BB_CTRL_PM_PWR_STATE_Pos)

	BB_CTRL_MEM_SOFT_RST_Pos  = 3,
	#define BB_CTRL_MEM_SOFT_RST_Mask   (0x1ul << BB_CTRL_MEM_SOFT_RST_Pos)

	BB_CTRL_PWR_MODE_Pos      = 0,
	#define BB_CTRL_PWR_MODE_Mask        (0x7ul << BB_CTRL_PWR_MODE_Pos)
};

#define BB_SLEEP_TIMER1_ADDR  0x000C
enum{
	BB_SLEEP_TIMER1_CMP_EN_Pos                 = 6,
	#define BB_SLEEP_TIMER1_CMP_EN_Mask         (1ul << BB_SLEEP_TIMER1_CMP_EN_Pos)

	BB_SLEEP_TIMER1_CMP_IRQ_MASK_Pos           = 5,
	#define BB_SLEEP_TIMER1_CMP_IRQ_MASK_Mask   (1ul << BB_SLEEP_TIMER1_CMP_IRQ_MASK_Pos)

	BB_SLEEP_TIMER1_CMP_IRQ_CLR_Pos            = 4,
	#define BB_SLEEP_TIMER1_CMP_IRQ_CLR_Mask    (1ul << BB_SLEEP_TIMER1_CMP_IRQ_CLR_Pos)

	BB_SLEEP_TIMER1_CMP_IRQ_STATE_Pos          = 3,
	#define BB_SLEEP_TIMER1_CMP_IRQ_STATE_Mask  (1ul << BB_SLEEP_TIMER1_CMP_IRQ_STATE_Pos)

	BB_SLEEP_TIMER1_EN_Pos                     = 1,
	#define BB_SLEEP_TIMER1_EN_Mask             (0x1ul << BB_SLEEP_TIMER1_EN_Pos)

	BB_SLEEP_TIMER1_CMP_EVENT_EN_Pos           = 0,
	#define BB_SLEEP_TIMER1_CMP_EVENT_EN_Mask   (1ul << BB_SLEEP_TIMER1_CMP_EVENT_EN_Pos)
};

#define BB_SLEEP_TIMER2_ADDR  0x0010
enum{
	BB_SLEEP_TIMER2_CMP_EVENT_DIN_Pos   = 0,
	#define BB_SLEEP_TIMER2_CMP_EVENT_DIN_Mask  0xFFFFFFFF
};

#define BB_SLEEP_TIMER3_ADDR  0x0014
enum{
	BB_SLEEP_TIMER3_CNT_Pos               = 0,
	#define BB_SLEEP_TIMER3_CNT_Mask      0xFFFFFFFF
};

#define BB_SLEEP_TIMER4_ADDR  0x0018
enum{
	BB_SLEEP_TIMER4_CMP_DIN_Pos            = 0,
	#define BB_SLEEP_TIMER4_CMP_DIN_Mask   0xFFFFFFFF
};

#define BB_IRQ_POLARITY_ADDR  0x001C
enum{
#ifdef IP_107
	BB_IRQ_POLARITY_PHY_RST_Pos            = 1,
	#define BB_IRQ_POLARITY_PHY_RST_Mask   (1ul << BB_IRQ_POLARITY_PHY_RST_Pos) //only 107 support
#endif

	BB_IRQ_POLARITY_IRQ_POL_Pos            = 0,
	#define BB_IRQ_POLARITY_IRQ_POL_Mask   (1ul << BB_IRQ_POLARITY_IRQ_POL_Pos)
};


/* BB FSM Register Address define. */
#define BB_FSM5_ADDR           0x404 // BB FSM Stop for scan/init
enum{
	BB_FSM5_FSM_STOP_Pos      = 6,  // stop LL scan/initiator initiating immediately
	#define BB_FSM5_FSM_STOP_Mask        (1<<BB_FSM5_FSM_STOP_Pos)

	BB_FSM5_FSM_TX_LATENCY_Pos = 0,
	#define BB_FSM5_FSM_TX_LATENCY_Mask   (0x3F << BB_FSM5_FSM_TX_LATENCY_Pos)
};

#define BB_FSM6_ADDR           0x408
enum{
	BB_FSM6_FSM_RX_LATENCY_Pos = 0,
	#define BB_FSM6_FSM_RX_LATENCY_Mask   (0x0F << BB_FSM5_FSM_TX_LATENCY_Pos)
};

#define BB_FSM13_ADDR          0x40C // Set RF Rx/TX Buffer address 0 [31:16]:RX; [15:0]:TX
#define BB_FSM14_ADDR          0x410 // Set RF Rx/TX Buffer address 1 [31:16]:RX; [15:0]:TX
#define BB_FSM15_ADDR          0x414 // Set RF Rx/TX Buffer address 2 [31:16]:RX; [15:0]:TX
#define BB_FSM16_ADDR          0x418 // Set RF Rx/TX Buffer address 3 [31:16]:RX; [15:0]:TX
#define BB_FSM17_ADDR          0x41C // Set RF Rx/TX Buffer address 4 [31:16]:RX; [15:0]:TX
#define BB_FSM18_ADDR          0x420 // Set RF Rx/TX Buffer address 5 [31:16]:RX; [15:0]:TX
#define BB_FSM19_ADDR          0x424 // Set RF Rx/TX Buffer address 6 [31:16]:RX; [15:0]:TX
enum{
	BB_FSM_RX_BUF_ADDR_Pos          = 16,
	#define BB_FSM_RX_BUF_ADDR_Mask   (0xfffful << BB_FSM_RX_BUF_ADDR_Pos)

	BB_FSM_TX_BUF_ADDR_Pos          = 0,
	#define BB_FSM_TX_BUF_ADDR_Mask   (0xfffful << BB_FSM_TX_BUF_ADDR_Pos)
};


/* BB PHY Register Address define. */
#define BB_PHY1_ADDR           0x428
enum{
	BB_PHY1_RSP_BUF_FULL_Pos           = 22,
	#define BB_PHY1_RSP_BUF_FULL_Mask    (1ul << BB_PHY1_RSP_BUF_FULL_Pos)

	BB_PHY1_RSP_BUF_EMPTY_Pos          = 21,
	#define BB_PHY1_RSP_BUF_EMPTY_Mask   (1ul << BB_PHY1_RSP_BUF_EMPTY_Pos)

	BB_PHY1_CFG_BUF_ERR_Pos            = 20,
	#define BB_PHY1_CFG_BUF_ERR_Mask     (1ul << BB_PHY1_CFG_BUF_ERR_Pos)

	BB_PHY1_CFG_BUF_FULL_Pos           = 19,
	#define BB_PHY1_CFG_BUF_FULL_Mask    (1ul << BB_PHY1_CFG_BUF_FULL_Pos)

	BB_PHY1_CFG_BUF_EMPTY_Pos          = 18,
	#define BB_PHY1_CFG_BUF_EMPTY_Mask   (1ul << BB_PHY1_CFG_BUF_EMPTY_Pos)

	BB_PHY1_CLK_PRESCALE_Pos           = 13,
	#define BB_PHY1_CLK_PRESCALE_Mask    (0x1Ful << BB_PHY1_CLK_PRESCALE_Pos)

	BB_PHY1_RSSI_ADDR_Pos              = 5,
	#define BB_PHY1_RSSI_ADDR_Mask       (0xFFul << BB_PHY1_RSSI_ADDR_Pos)

    BB_PHY1_CFG_TRIG_Pos               = 4,
	#define BB_PHY1_CFG_TRIG_Mask        (0x1ul << BB_PHY1_CFG_TRIG_Pos)

	BB_PHY1_CFG_REG_Pos                = 2,
	#define BB_PHY1_CFG_REG_Mask         (0x3ul << BB_PHY1_CFG_REG_Pos)

	BB_PHY1_RSP_BUF_FLUSH_Pos          = 1,
	#define BB_PHY1_RSP_BUF_FLUSH_Mask   (0x1ul << BB_PHY1_RSP_BUF_FLUSH_Pos)

	BB_PHY1_CFG_BUF_FLUSH_Pos          = 0,
	#define BB_PHY1_CFG_BUF_FLUSH_Mask   (0x1ul << BB_PHY1_CFG_BUF_FLUSH_Pos)
};

//
#define BB_PHY2_ADDR           0x42C
enum{
	BB_PHY2_RSP_BUF_ERR_Pos               = 28,
	#define BB_PHY2_RSP_BUF_ERR_Mask      (1ul << BB_PHY2_RSP_BUF_ERR_Pos)

	BB_PHY2_SEQ_RX_END_ADDR_Pos           = 21,
	#define BB_PHY2_SEQ_RX_END_ADDR_Mask  (0x7Ful << BB_PHY2_SEQ_RX_END_ADDR_Pos)

	BB_PHY2_SEQ_TX_END_ADDR_Pos           = 14,
	#define BB_PHY2_SEQ_TX_END_ADDR_Mask  (0x7Ful << BB_PHY2_SEQ_TX_END_ADDR_Pos)

	BB_PHY2_SEQ_RX_START_ADDR_Pos         = 7,
	#define BB_PHY2_SEQ_RX_START_ADDR_Mask (0x7Ful << BB_PHY2_SEQ_RX_START_ADDR_Pos)

	BB_PHY2_SEQ_TX_START_ADDR_Pos         = 0,
	#define BB_PHY2_SEQ_TX_START_ADDR_Mask (0x7Ful << BB_PHY2_SEQ_TX_START_ADDR_Pos)
};

//
#define BB_PHY3_ADDR           0x430
enum{
	BB_PHY3_CFG_BUF_DIN_Pos              = 0,
	#define BB_PHY3_CFG_BUF_DIN_Mask     (0xFFFFFFFF)
};

//
#define BB_PHY4_ADDR           0x434
enum{
	BB_PHY4_RSP_BUF_DOUT_Pos             = 0,
	#define BB_PHY4_RSP_BUF_DOUT_Mask    (0xFFFFFFFF)
};

//
#define BB_PHY5_ADDR           0x438
enum{
	BB_PHY5_CHN_REG_ADDR_Pos              = 0,
	#define BB_PHY5_CHN_REG_ADDR_Mask     (0xFFul << BB_PHY5_CHN_REG_ADDR_Pos)
};


/* BB Interrupt Register Address define. */
#define BB_INT_STAT_ADDR       0x444
enum{
	BB_INT_STAT_RNG_DONE_Pos               = 3,
	#define BB_INT_STAT_RNG_DONE_Mask      (0x1ul << BB_INT_STAT_RNG_DONE_Pos)

	BB_INT_STAT_SEC_DONE_Pos               = 2,
	BB_INT_STAT_TIMER_CAP_START_Pos        = 1,

    BB_INT_STAT_PHY_CFG_DONE_Pos           = 0,
	#define BB_INT_STAT_PHY_CFG_DONE_Mask  (0x1ul << BB_INT_STAT_PHY_CFG_DONE_Pos)
};

#define BB_INT_STAT_CLR_ADDR   0x448
enum{
	BB_INT_STAT_CLR_RNG_DONE_Pos = 3,
	#define BB_INT_STAT_CLR_RNG_DONE_Mask             (0x1ul << BB_INT_STAT_CLR_RNG_DONE_Pos)

	BB_INT_STAT_CLR_SEC_DONE_Pos = 2,
	#define BB_INT_STAT_CLR_SEC_DONE_Mask             (0x1ul << BB_INT_STAT_CLR_SEC_DONE_Pos)

	BB_INT_STAT_CLR_TIMER_CMP_START_EVT_Pos = 1,
	#define BB_INT_STAT_CLR_TIMER_CMP_START_EVT_Mask  (0x1ul << BB_INT_STAT_CLR_TIMER_CMP_START_EVT_Pos)

	BB_INT_STAT_CLR_PHY_CFG_DONE_Pos = 0,
	#define BB_INT_STAT_CLR_PHY_CFG_DONE_Mask         (0x1ul << BB_INT_STAT_CLR_PHY_CFG_DONE_Pos)
};

#define BB_INT_MASK_ADDR       0x44C
enum{
	BB_INT_MSK_RNG_DONE_Pos                  = 3,
	#define BB_INT_MSK_RNG_DONE_Mask         (1ul << BB_INT_MSK_RNG_DONE_Pos)

	BB_INT_MSK_SEC_DONE_Pos                  = 2,
	#define BB_INT_MSK_SEC_DONE_Mask         (1ul << BB_INT_MSK_SEC_DONE_Pos)

	BB_INT_MSK_TIMER_CAP_START_Pos           = 1,
    #define BB_INT_MSK_TIMER_CAP_START_Mask  (1ul << BB_INT_MSK_TIMER_CAP_START_Pos)

	BB_INT_MSK_PHY_CFG_DONE_Pos              = 0,
    #define BB_INT_MSK_PHY_CFG_DONE_Mask     (1ul << BB_INT_MSK_PHY_CFG_DONE_Pos)
};

#define BB_INT4_ADDR       0x450
enum{
	BB_INT4_IC_MODE_IRQ_Pos           = 4,
	#define BB_INT4_IC_MODE_IRQ_Mask  (1ul << BB_INT4_IC_MODE_IRQ_Pos)
};

/* BB 32M Timer Register Address define. */
#define BB_ACTTMR1_MAX_CMP_ADDR    0x458  //????   ACTTIMER1
#define BB_ACTTMR2_CTRL_ADDR       0x45C  //       ACTTIMER2
#define BB_ACTTMR6_START_CMP_ADDR  0x460  //???    ACTTIMER6 -- DIN
#define BB_ACTTMR7_CMP_ADDR        0x464  //???    ACTTIMER7 -- DOUT -- RX timestamp: unit:tick ??


/* BB Secure Register Address define. */
#define BB_SEC_CTRL_ADDR         0x46C  // SECURE1
enum{
	BB_SEC_CTRL_AES_EN_Pos           = 0,
    #define BB_SEC_CTRL_AES_EN_Mask    (1ul << BB_SEC_CTRL_AES_EN_Pos)

	BB_SEC_CTRL_MODE_Pos             = 1,
	#define BB_SEC_CTRL_MODE_Mask      (0x3ul << BB_SEC_CTRL_MODE_Pos)

	BB_SEC_CTRL_STATE_Pos            = 3,
    #define BB_SEC_CTRL_STATE_Mask     (0x7Ful << BB_SEC_CTRL_STATE_Pos)

	BB_SEC_CTRL_VALID_MIC_Pos        = 10,
    #define BB_SEC_CTRL_VALID_MIC_Mask (0x7Ful << BB_SEC_CTRL_VALID_MIC_Pos)

	BB_SEC_CTRL_MIC_LEN_Pos          = 17,
    #define BB_SEC_CTRL_MIC_LEN_Mask   (0x3ul << BB_SEC_CTRL_MIC_LEN_Pos)

	BB_SEC_CTRL_AD_LEN_Pos           = 19,
    #define BB_SEC_CTRL_AD_LEN_Mask    (0x7Ful << BB_SEC_CTRL_AD_LEN_Pos)

	BB_SEC_CTRL_DECRYPT_EN_Pos       = 26,
    #define BB_SEC_CTRL_DECRYPT_EN_Mask (1ul << BB_SEC_CTRL_DECRYPT_EN_Pos)
};

#define BB_SEC_IV_LO_ADDR        0x470  // SECURE2 - IV3 - IV0
#define BB_SEC_IV_HI_ADDR        0x474  // SECURE3 - IV7 - IV4
#define BB_SEC_KEY0_BUF_ADDR     0x478  // SECURE4 - key address for packet0-1
enum{
	BB_SEC_KEY0_BUF_ADDR_Pos   = 0,
	#define BB_SEC_KEY0_BUF_ADDR_Mask (0xFFFFul << BB_SEC_KEY0_BUF_ADDR_Pos)

	BB_SEC_KEY1_BUF_ADDR_Pos   = 16,
	#define BB_SEC_KEY1_BUF_ADDR_Mask  (0xFFFFul << BB_SEC_KEY1_BUF_ADDR_Pos)
};

#define BB_SEC_KEY1_BUF_ADDR     0x47C  // SECURE5  - key address for packet2-3
#define BB_SEC_KEY2_BUF_ADDR     0x480  // SECURE6  - key address for packet4-5
#define BB_SEC_KEY3_BUF_ADDR     0x484  // SECURE7  - key address for packet6 + Tx packet counter MSB [7:0]
enum{
	BB_SEC_KEY6_BUF_ADDR_Pos   = 8,
	#define BB_SEC_KEY6_BUF_ADDR_Mask (0xFFFFul << BB_SEC_KEY6_BUF_ADDR_Pos)
};
#define BB_SEC_TX_PKT_CNT1_ADDR  0x484  // SECURE7  - key address for packet6 + Tx packet counter MSB [7:0]
#define BB_SEC_TX_PKT_CNT2_ADDR  0x488  // SECURE8  - Tx packet counter LSB [31:0]
#define BB_SEC_RX_PKT_CNT1_ADDR  0x48C  // SECURE9  - Rx packet counter MSB [7:0]
#define BB_SEC_RX_PKT_CNT2_ADDR  0x490  // SECURE10 - Rx packet counter LSB [31:0]

/* BB RNG Register Address define. */
#define BB_RNG_CTRL_ADDR         0x49C
#define BB_RNG_VALUE_ADDR        0x4A0


/* BB LL MAC CTRL Register Address define. */
#define BB_LL_EN_ADDR            0x4A8  // Enable LL
enum{
	BB_LL_LL_EN_Pos = 0,
	#define BB_LL_LL_EN_Mask     (1ul << BB_LL_LL_EN_Pos)
};

#define BB_PRI0_ADDR         0x4C0  //PRI_R00  - AA length | CRC Enable  | RF Mode(BLE, Nordic) |
enum{
	BB_PRI0_CRC_INCLUDE_Pos = 15,
	#define BB_PRI0_CRC_INCLUDE_Mask  (1ul << BB_PRI0_CRC_INCLUDE_Pos)

	BB_PRI0_CRC24_EN_Pos    = 14,
	#define BB_PRI0_CRC24_EN_Mask     (1ul << BB_PRI0_CRC24_EN_Pos)

	BB_PRI0_AA_LEN_Pos      = 12,
	#define BB_PRI0_AA_LEN_Mask       (0x3ul << BB_PRI0_AA_LEN_Pos)

	BB_PRI0_AUTO_RX_LEN_Pos = 11,

	BB_PRI0_CRC_EN_Pos      = 10,
	#define BB_PRI0_CRC_EN_Mask       (1ul << BB_PRI0_CRC_EN_Pos)

	BB_PRI0_CRC16_EN_Pos    = 9, // CRC24_EN = 0, CRC16_EN = 1 -> CRC16 Or not CRC8

	BB_PRI0_PHY_MODE_Pos    = 5, // 1Mbps / 2Mbps
	#define BB_PRI0_PHY_MODE_Mask     (1ul << BB_PRI0_PHY_MODE_Pos)

	BB_PRI0_RF_MODE_Pos     = 3,  //BLE mode / nordic mode / 297 mode
	#define BB_PRI0_RF_MODE_Mask      (3ul << BB_PRI0_RF_MODE_Pos)

	BB_PRI0_RX_TX_CTRL_Pos  = 0,
};

#define BB_PRI1_ADDR         0x4C4  //PRI_R01  - Tx done irq | Rx done irq | Crc Error | Rx timeout |
enum{
	BB_PRI1_RX_PID_Pos  = 29,
	#define BB_PRI1_RX_PID_Mask    (0x3UL<<BB_PRI1_RX_PID_Pos)

	BB_PRI1_TX_PID_Pos  = 27,
	#define BB_PRI1_TX_PID_Mask    (0x3UL<<BB_PRI1_TX_PID_Pos)
};

#define BB_PRI2_ADDR         0x4C8  //PRI_R02  - Rx wait time | Tx wait time
#define BB_PRI3_ADDR         0x4CC  //PRI_R03  - Rx address
#define BB_PRI4_ADDR         0x4D0  //PRI_R04  - Rx header
#define BB_PRI5_ADDR         0x4D4  //PRI_R05  - Tx address
#define BB_PRI6_ADDR         0x4D8  //PRI_R06  - Tx header | whitening init(def:0x7F)
#define BB_PRI7_ADDR         0x4DC  //PRI_R07  - Rx/Tx RAM address

#define BB_FSM1_ADDR              0x80C // Get TX/RX PKT CNT | NESN | SN | PKT_PER_CONN | ADV CHN MAP | BLE FSM
enum{
	BB_FSM1_TX_PKT_NUM_Pos                = 29,
	#define BB_FSM1_TX_PKT_NUM_Mask       (0x07ul << BB_FSM1_TX_PKT_NUM_Pos) // the number of acknowledged transmitted packets in the current event.

	BB_FSM1_RX_PKT_NUM_Pos                = 14,
	#define BB_FSM1_RX_PKT_NUM_Mask       (0x7FFFul << BB_FSM1_RX_PKT_NUM_Pos) //  the number of unique correctly received packets in the current event.

	BB_FSM1_RX_NESN_Pos                   = 13,
	BB_FSM1_RX_SN_Pos                     = 12,
	BB_FSM1_RX_PKT_PER_CONN_Pos           = 9,
	#define BB_FSM1_RX_PKT_PER_CONN_Mask  (0x7ul << BB_FSM1_RX_PKT_PER_CONN_Pos)

	BB_FSM1_TX_NESN_Pos                   = 8,
	#define BB_FSM1_TX_NESN_Mask          (1ul << BB_FSM1_TX_NESN_Pos)

	BB_FSM1_TX_SN_Pos                     = 7,
	#define BB_FSM1_TX_SN_Mask            (1ul << BB_FSM1_TX_SN_Pos)

	BB_FSM1_ADV_CHN_MAP_Pos               = 4,
	#define BB_FSM1_ADV_CHN_MAP_Mask      (0x7ul << BB_FSM1_ADV_CHN_MAP_Pos)

	BB_FSM1_FSM_MODE_Pos                  = 0,  //!!!important.
	#define BB_FSM1_FSM_MODE_Mask         (0xFul << BB_FSM1_FSM_MODE_Pos)
};

#define BB_FSM2_ADDR              0x810
enum{
	BB_FSM2_PHY_TX_CFG_Pos             = 0,
	#define BB_FSM2_PHY_TX_CFG_Mask    (0xfful << BB_FSM2_PHY_TX_CFG_Pos)
};

#define BB_FSM4_ADDR              0x814
enum{
	BB_FSM4_PHY_RX_CFG_Pos            = 0,
	#define BB_FSM4_PHY_RX_CFG_Mask   (0xfful << BB_FSM4_PHY_RX_CFG_Pos)
};

#define BB_FSM8_ADDR              0x818 // DTM mode
#define BB_FSM9_ADDR              0x81C // Set First RX Timeout
enum{
	BB_FSM9_FISRT_RX_TIMEOUT_Pos           = 0,
	#define BB_FSM9_FISRT_RX_TIMEOUT_Mask  0xFFFFFFFF
};

#define BB_FSM11_ADDR             0x824 // Set RX Timeout
enum{
	BB_FSM11_RX_TIMEOUT_Pos = 0,
	#define BB_FSM11_RX_TIMEOUT_Mask   (0x1FFUL << BB_FSM11_RX_TIMEOUT_Pos)
};

#define BB_FSM12_ADDR             0x828 // Set BLE Whitening | CRC error 2 | PHY opt | BLE RX PHY | BLE Tx PHY | BLE data channel
enum{
	BB_FSM12_BLE_WHITENING_VAL_Pos             = 24,
	#define BB_FSM12_BLE_WHITENING_VAL_Mask    (0x7Ful << BB_FSM12_BLE_WHITENING_VAL_Pos)

	BB_FSM12_BLE_WHITENING_EN_Pos              = 23,
	#define BB_FSM12_BLE_WHITENING_EN_Mask     (0x1ul << BB_FSM12_BLE_WHITENING_EN_Pos)

	BB_FSM12_BLE_CRC_ERR_NUM_Pos               = 22,
	#define BB_FSM12_BLE_CRC_ERR_NUM_Mask      (0x1ul << BB_FSM12_BLE_CRC_ERR_NUM_Pos)

	BB_FSM12_BLE_CI_Pos                        = 14,
	#define BB_FSM12_BLE_CI_Mask               (0x7Ful << BB_FSM12_BLE_CI_Pos)

	BB_FSM12_BLE_RX_PHY_Pos                    = 12,
	#define BB_FSM12_BLE_RX_PHY_Mask           (0x3ul << BB_FSM12_BLE_RX_PHY_Pos)

	BB_FSM12_BLE_TX_PHY_Pos                    = 10,
	#define BB_FSM12_BLE_TX_PHY_Mask           (0x3ul << BB_FSM12_BLE_TX_PHY_Pos)

	BB_FSM12_PAUSE_NESN_UPD_Pos                = 6,
	#define BB_FSM12_PAUSE_NESN_UPD_Mask       (0x1ul << BB_FSM12_PAUSE_NESN_UPD_Pos)

	BB_FSM12_BLE_DATA_CHN_ID_Pos               = 0,
	#define BB_FSM12_BLE_DATA_CHN_ID_Mask      (0x3Ful << BB_FSM12_BLE_DATA_CHN_ID_Pos)
};

#define BB_PP1_ADDR               0x830 // PADV SYNC | RX CI | max Rx length |
#define BB_PP2_ADDR               0x834 // Access code
#define BB_PP3_ADDR               0x838 // CRC init
#define BB_PP4_ADDR               0x83C // ADV PDU Tx addr type + adv address(MSB)
#define BB_PP5_ADDR               0x840 // ADV address LSB
#define BB_PP6_ADDR               0x844 // Peer ADV address MSB //for direct adv
#define BB_PP7_ADDR               0x848 // Peer ADV address LSB //for direct adv
enum{
	BB_PP1_PERIOD_SYNC_EN_Pos             = 22,
	#define BB_PP1_PERIOD_SYNC_EN_Mask    (1ul << BB_PP1_PERIOD_SYNC_EN_Pos)

	BB_PP1_MAX_RX_LEN_Pos                 = 12,
	#define BB_PP1_MAX_RX_LEN_Mask        (0xFFul << BB_PP1_MAX_RX_LEN_Pos)

	BB_PP1_DTM_TX_LEN_Pos                 = 4,
	#define BB_PP1_DTM_TX_LEN_Mask        (0xFFul << BB_PP1_DTM_TX_LEN_Pos)

	BB_PP1_DTM_PKT_TYPE_Pos               = 0,
	#define BB_PP1_DTM_PKT_TYPE_Mask      (0xFul)


	BB_PP2_AA_Pos                         = 0,
	#define BB_PP2_AA_Mask                (0xFFFFFFFF)

	BB_PP3_CRC_INIT_Pos                   = 0,
	#define BB_PP3_CRC_INIT_Mask          (0xFFFFFF)

	BB_PP4_ADV_TX_ADDR_TYPE_Pos           = 16,
    #define BB_PP4_ADV_TX_ADDR_TYPE_Mask  (1ul << BB_PP4_ADV_TX_ADDR_TYPE_Pos)

	BB_PP4_ADV_TX_ADDR_MSB_Pos            = 0,
    #define BB_PP4_ADV_TX_ADDR_MSB_Mask   (0xFFFF)

    BB_PP5_ADV_TX_ADDR_LSB_Pos            = 0,
    #define BB_PP5_ADV_TX_ADDR_LSB_Mask   (0xFFFFFFFF)

	BB_PP6_ADV_RX_ADDR_MSB_Pos            = 0,
	#define BB_PP6_ADV_RX_ADDR_MSB_Mask   (0xFFFF)

	BB_PP7_ADV_RX_ADDR_LSB_Pos            = 0,
	#define BB_PP7_ADV_RX_ADDR_LSB_Mask   (0xFFFFFFFF)
};

// For BLE FSM Intterupt
#define BB_BLE_INT_STAT_ADDR      0x890 // Rx timeout | crc error | cmd done IRQ
#define BB_BLE_INT_CLR_ADDR       0x894
#define BB_BLE_INT_MASK_ADDR      0x898

#define BB_BLE_RX_TX_SETTLE_TIME  0x8AC // 150 + n

// For BLE ADV PDU filter module
#define BB_ADV_PDU_FLTR_CTRL_ADDR 0x940
enum{
	BB_ADV_TYPE_FLTR_EN_Pos = 17,
	#define BB_ADV_TYPE_FLTR_EN_Mask    (1ul << BB_ADV_TYPE_FLTR_EN_Pos)

	BB_ADV_PDU_FLTR_EN_Pos  = 16,
	#define BB_ADV_PDU_FLTR_EN_Mask     (1ul << BB_ADV_PDU_FLTR_EN_Pos)

	BB_ADV_PDU_FLTR_MSK_Pos = 0,
	#define BB_ADV_PDU_FLTR_MSK_Mask  	(0xFFFF)
};
#define BB_ADV_PDU_FLTR0_ADDR     0x944 // Format: ADV Data Type | AD Length
#define BB_ADV_PDU_FLTR1_ADDR     0x948 // Format: ADV Data Type | AD Length
#define BB_ADV_PDU_FLTR2_ADDR     0x94C // Format: ADV Data Type | AD Length
#define BB_ADV_PDU_FLTR3_ADDR     0x950 // Format: ADV Data Type | AD Length

// For whitlist / resolvelist / blacklist module
#define BB_LST1_ADDR           0x850
#define BB_LST2_ADDR           0x854
enum{
	BB_LST2_PERIODIC_SID_Pos                   = 11,
	#define BB_LST2_PERIODIC_SID_Mask          (0xFul << BB_LST1_PERIODIC_SID_Pos)

	BB_LST2_PERIODIC_FILT_POLICY_Pos           = 10,
	#define BB_LST2_PERIODIC_FILT_POLICY_Mask  (0x1ul << BB_LST1_PERIODIC_FILT_POLICY_Pos)

	BB_LST2_PERIODIC_FILT_ADDR_Pos             = 3,
	#define BB_LST2_PERIODIC_FILT_ADDR_Mask     (0x7Ful << BB_LST1_PERIODIC_FILT_ADDR_Pos)

	BB_LST2_FILT_POLICY_Pos                    = 0,
	#define BB_LST2_FILT_POLICY_Mask            (0x7ul << BB_LST2_FILT_POLICY_Pos)
};

#define BB_LST3_ADDR           0x858
#define BB_LST4_ADDR           0x85C
enum{
	BB_LST4_INITATOR_SYNC_ADDR_TYPE_Pos = 16,
	#define BB_LST4_INITATOR_SYNC_ADDR_TYPE_Mask  (1ul << BB_LST4_INITATOR_SYNC_ADDR_TYPE_Pos)

	BB_LST4_INITATOR_SYNC_ADDR_MSB_Pos = 0,
	#define BB_LST4_INITATOR_SYNC_ADDR_MSB_Mask   (0xFFFFul << BB_LST4_INITATOR_SYNC_ADDR_MSB_Pos)
};

#define BB_LST5_ADDR           0x860
enum{
	BB_LST5_INITATOR_SYNC_ADDR_LSB_Pos = 0,
	#define BB_LST5_INITATOR_SYNC_ADDR_LSB_Mask 0xFFFFFFFF
};

#define BB_LST6_ADDR           0x864
#define BB_LST7_ADDR           0x868

//For channel select module
#define BB_CS1_ADDR            0x870
#define BB_CS2_ADDR            0x874
#define BB_CS3_ADDR            0x878
#define BB_CS4_ADDR            0x87C
#define BB_CM1_ADDR            0x888

#ifdef IP_108
//AOA
#define BB_AOA_RECOGNIZE0_ADDR  0x954
enum{
	BB_SCAN_MD_SCAN_GO_ON_EN_Pos  = 13,
	#define BB_SCAN_MD_SCAN_GO_ON_EN_Mask  (1ul << BB_SCAN_MD_SCAN_GO_ON_EN_Pos)
};
#define BB_AOA_RECOGNIZE1_ADDR  0x958
#define BB_AOA_RECOGNIZE2_ADDR  0x95C
#define BB_AOA_RECOGNIZE3_ADDR  0x960

#elif defined(IP_107)
#define BB_SCAN_MORE_DATA_ADDR  0x954
enum{
	BB_SCAN_MD_SCAN_GO_ON_EN_Pos  = 4,
	#define BB_SCAN_MD_SCAN_GO_ON_EN_Mask  (1ul << BB_SCAN_MD_SCAN_GO_ON_EN_Pos)

	BB_SCAN_MD_SCAN_MD_EN_Pos     = 3,
	#define BB_SCAN_MD_SCAN_MD_EN_Mask     (1ul << BB_SCAN_MD_SCAN_MD_EN_Pos)

	BB_SCAN_MD_SCAN_MD_LEN_Pos    = 0,
	#define BB_SCAN_MD_SCAN_MD_LEN_Mask    (0x07ul << BB_SCAN_MD_SCAN_MD_LEN_Pos)
};
#endif


//RND
#define BB_RNG1_ADDR            0x49C
#define BB_RNG2_ADDR            0x4A0



//BB Debug
/* BB Test Mux Register Address define. */
#define BB_DBG_BASE_ADDR        (BB_BASE + 0x498)

#define BB_TEST_MUX0_ADDR        0x498
#define BB_TEST_MUX1_ADDR        0x4AC
#define BB_TEST_MUX2_ADDR        0x4B0
#define BB_TEST_MUX3_ADDR        0x4B4

enum{
	//MUX0
	BB_TEST_MUX_CHN0_Pos      = 0,
	BB_TEST_MUX_CHN1_Pos      = 6,
	BB_TEST_MUX_CHN2_Pos      = 12,
	BB_TEST_MUX_CHN3_Pos      = 18,

	//MUX1
	BB_TEST_MUX_CHN4_Pos      = 0,
	BB_TEST_MUX_CHN5_Pos      = 6,
	BB_TEST_MUX_CHN6_Pos      = 12,
	BB_TEST_MUX_CHN7_Pos      = 18,

	//MUX2
	BB_TEST_MUX_CHN8_Pos      = 0,
	BB_TEST_MUX_CHN9_Pos      = 6,
	BB_TEST_MUX_CHN10_Pos     = 12,
	BB_TEST_MUX_CHN11_Pos     = 18,

	//MUX3
	BB_TEST_MUX_CHN12_Pos     = 0,
	BB_TEST_MUX_CHN13_Pos     = 6,
	BB_TEST_MUX_CHN14_Pos     = 12,
	BB_TEST_MUX_CHN15_Pos     = 18,
};

#ifdef IP_108
	#define LL_DBG_TX_DATA_VAL          0x30
	#define LL_DBG_TX_ON_VAL            0x31
	#define LL_DBG_RX_ON_VAL            0x32
	#define LL_DBG_ACCESS_MATCH_VAL     0x33
	#define LL_DBG_RX_DATA_VAL          0x34
	#define LL_DBG_RX_CLK_VAL           0x35
#elif defined(IP_107)
	#define LL_DBG_TX_DATA_VAL          0x13
	#define LL_DBG_TX_ON_VAL            0x14
	#define LL_DBG_RX_ON_VAL            0x15
	#define LL_DBG_ACCESS_MATCH_VAL     0x16
	#define LL_DBG_RX_DATA_VAL          0x17
	#define LL_DBG_RX_CLK_VAL           0x18
#endif

#endif /* BB_DEF_H_ */
