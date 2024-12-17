/**
 *******************************************************************************
 * @FileName  : bb_api.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-09-10
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *
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
#ifndef BB_API_H_
#define BB_API_H_

#include "bb_def.h"

/*!  Base-band Power level. */
typedef enum {
	BB_POWER_N45_dB  = -45,
	BB_POWER_N44_dB  = -44,

	BB_POWER_N3_dB   = -3,
	BB_POWER_N2_dB   = -2,
	BB_POWER_N1_dB   = -1,
	BB_POWER_0_dB    = 0,
	BB_POWER_1_dB    = 1,
	BB_POWER_2_dB    = 2,
	BB_POWER_3_dB    = 3,
	BB_POWER_4_dB    = 4,
	BB_POWER_5_dB    = 5,
	BB_POWER_6_dB    = 6,
	BB_POWER_7_dB    = 7,
}BB_Power_Type_t;

/*! Adv PDU Filter Mask */
enum{
    BB_ADV_PDU_FILT_ADV_IND                = (1 << 0),
	BB_ADV_PDU_FILT_ADV_NONCONN_IND        = (1 << 2),
	BB_ADV_PDU_FILT_SCAN_RSP               = (1 << 4),
	BB_ADV_PDU_FILT_ADV_SCAN_IND           = (1 << 6),
	BB_ADV_PDU_FILT_ADV_EXT_IND_OR_AUX_ADV = (1 << 7),
};

/**
 * @brief : Set RF TX power.
 * @param : pwr      can be BB_POWER_0_dB ...
 * @return: none.
 */
void BB_SetTxPower(uint8_t level);

/**
 * @brief : Set BLE Channel
 * @param : bleChn      ble channel
 * @return: none.
 */
void BB_SetBleChannel(uint8_t bleChn);
/*! BB time */
static inline uint32_t BB_GetCurTick(void)
{
	return BB_REG32(BB_SLEEP_TIMER3_ADDR); // 32KHz tick
}
bool_t BB_IsTimeExpired(uint32_t refTick, uint32_t us);
void BB_Delay(uint32_t tick);

uint32_t BB_UsToTick(uint32_t us);
uint32_t BB_TickToUs(uint32_t t);

/*! 32k RC calibrate and measure */
void BB_Start32kRCMeasure(uint32_t dst_clk_cnt);
void BB_Get32kRCMeasureValue(void);
void BB_Calibrate32kRC(uint32_t expected_freq);
bool_t BB_Is32kClkTrackBusy(void);

/*! ADV Type/PDU Filter API */
// Only use for pan108
void BB_SetAdvTypeFilterEnable(uint8_t en);

/**
 * @brief : Set ADV Type to Filter.
 * @param : id               rang 0-6
 * @param : advType
 * @param : advTypeDataLen
 * @return: -1 if error or 0
 */
int BB_SetAdvTypeToFilter(uint8_t id, uint8_t advType, uint8_t advTypeDataLen);

// use for pan108/pan107
void BB_SetAdvPduFilterEnable(uint8_t en);
void BB_SetAdvPduFilter(uint8_t en, uint16_t advPduMsk);

// Only use for pan107
/**
 * @brief : Set ADV Packet data to Filter.
 * @param : id        rang 0-4
 * @param : offset    data start position in adv packet
 * @param : data      pointer point to data buffer.
 * @param : len       the length of data
 * @return: -1 if error or 0
 */
int BB_SetAdvPktDataToFilter(uint8_t id, uint8_t offset, uint8_t *data, uint32_t len);

/**
 * @brief : Enable ADV Packet Filter function.
 * @param : id        rang 0-4
 * @param : en        ENABLE/DISABLE
 * @return: -1 if error or 0
 */
int BB_SetAdvPktFilterEnable(uint8_t id, uint8_t en);

/*! BB Debug */
void BB_DebugInit(void);
void BB_DebugReset(void);
void BB_SetDebugChn(uint8_t chnId, uint8_t sigCode);

#endif /* BB_API_H_ */
