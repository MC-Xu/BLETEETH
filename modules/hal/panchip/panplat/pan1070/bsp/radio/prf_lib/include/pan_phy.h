/*
 * Copyright (C) 2021 Panchip Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 
 */
 
/**
 * @file
 * @brief    Panchip series phy driver header file 
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 21/11/19 18:33 $ 
 */
#ifndef __PRI_RHY_H__
#define __PRI_RHY_H__

#include <stdint.h>

/**
 * @brief Phy Interface
 * @defgroup phy_interface Phy Interface
 * @{
 */
#ifdef __cplusplus
extern "C"
{
#endif

/**@defgroup PHY_SELECT_FLAG Phy type select
 * @brief       Phy type select definitions
 * @{ */
enum tx_rx_phy
{
    LE_NO_CHANGE    = 0x00,
    LE_1M           = 0x01,
    LE_2M           = 0x02,
    LE_CODED_S8     = 0x03,
    LE_CODED        = 0x04,
	LE_250K			= 0x05,
};
/**@} */

/**@defgroup PHY_ACTION_FLAG Phy action select
 * @brief       Phy action select definitions
 * @{ */
typedef enum llhw_phy_strt_stp_phy
{
    LLHWC_PHY_START,
    LLHWC_PHY_STOP
} llhw_phy_strt_stp_phy_e;
/**@} */

/**
  * @brief  Reset phy.
  * @retval None
  */
void PHY_reset(void);

/**
  * @brief  Configures the channel.
  * @param  chan: channel num
  * @retval None
  */
void PHY_SetChConfig(uint8_t chan);

/**
  * @brief  Phy init.
  * @retval None
  */
void PHY_init(void);

/**
  * @brief  Gonfigures the send and receive sequence.
  * @param  phy: select phy
  * @retval None
  */
void PHY_SetTrxSeq_prf_LR(enum tx_rx_phy phy);
void PHY_SetTrxSeq_prf_250K(enum tx_rx_phy phy);

/**
  * @brief  Gonfigures the send and receive time.
  * @param  phy: select phy
  * @param  tx1rx0: select tx or rx mode
  * @retval None
  */
uint8_t phy_time_cfg(uint8_t phy, uint8_t tx1rx0);

/**
  * @brief  Reset channel config.
  * @retval None
  */
void PHY_ResetChannel(void);

/**
  * @brief  Start or stop phy action.
  * @param  phy_state:	phy action select  
  * @retval None
  */
void PHY_strt_stp(llhw_phy_strt_stp_phy_e phy_state);
/**
  * @brief  Write fifo batch config .
  * @param  command: command select 
  * @param  cmd_len: command lenth 
  * @param  ptr_rsp_value: point to response value
  * @retval None
  */
uint32_t PHY_WriteCfgFifoBatch(const uint32_t *command, const uint32_t cmd_len, uint32_t *ptr_rsp_value);

void PHY_set_tx_pwr(int8_t tx_pwr);
uint32_t PHY_SeqRamInit(void);

extern const uint32_t init_slp[];
extern const uint32_t init_stdby[];
extern const uint32_t init_slp_to_stdby[];
extern const uint32_t reset[];
extern const uint32_t phy_init[];
extern const uint32_t pre_tx_interpacket[];
extern const uint32_t pre_tx_interpacket_LR[];
extern const uint32_t pre_tx_interpacket_2M[];
extern const uint32_t pre_tx_interpacket_250K[];
extern const uint32_t post_tx_interpacket[];
extern const uint32_t pre_rx_interpacket[];
extern const uint32_t pre_rx_interpacket_LR[];
extern const uint32_t pre_rx_interpacket_2M[];
extern const uint32_t pre_rx_interpacket_250K[];
extern const uint32_t post_rx_interpacket[];
extern const uint32_t pre_evnt[];
extern const uint32_t post_evnt[];
extern       uint32_t channel_init[];
extern const uint32_t init_constant_tone[];
extern const uint32_t init_pll_bw_cal[];

extern const uint32_t init_slp_size;
extern const uint32_t init_stdby_size;
extern const uint32_t init_slp_to_stdby_size;
extern const uint32_t reset_size;
extern uint32_t phy_init_size;
extern uint32_t pre_tx_interpacket_size;
extern uint32_t pre_tx_interpacket_LR_size;
extern uint32_t pre_tx_interpacket_2M_size;
extern uint32_t post_tx_interpacket_size;
extern uint32_t pre_rx_interpacket_size;
extern uint32_t pre_rx_interpacket_LR_size;
extern uint32_t pre_rx_interpacket_2M_size;
extern uint32_t post_rx_interpacket_size;
extern uint32_t pre_evnt_size;
extern uint32_t post_evnt_size;
extern const uint32_t channel_init_size;
extern uint32_t pre_tx_interpacket_250K_size;
extern uint32_t pre_rx_interpacket_250K_size;

extern uint32_t init_constant_tone_size;
extern const uint32_t init_pll_bw_cal_size;

extern const uint8_t phy_ch_cfg_reg;
extern const uint8_t tx_pwr_cfg_reg;
extern const uint8_t rssi_out_phy_reg_addrs;

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* __PRI_RHY_H__ */
