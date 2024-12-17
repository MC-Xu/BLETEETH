/**
 *******************************************************************************
 * @FileName  : ble_types.h
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
#ifndef BLE_TYPES_H
#define BLE_TYPES_H

#include "utils/defs_types.h"

#include "utils/assert.h"
#include "utils/track.h"

typedef unsigned char      HciState_t;
typedef unsigned char      L2cState_t;
typedef unsigned char      AttState_t;
typedef unsigned char      SmpState_t;

typedef unsigned char      LlState_t;
typedef unsigned char      BleState_t;

typedef unsigned short     MtuSize_t;


/* Connection interval define. unit: 1.25ms */
enum{
	CONN_INTR_7P5_MS   = 6,
	CONN_INTR_8P75_MS  = 7,
	CONN_INTR_10_MS    = 8,
	CONN_INTR_11P25_MS = 9,
	CONN_INTR_12P5_MS  = 10,
	CONN_INTR_13P75_MS = 11,
	CONN_INTR_15_MS    = 12,
	CONN_INTR_16P25_MS = 13,
	CONN_INTR_17P5_MS  = 14,
	CONN_INTR_18P75_MS = 15,
	CONN_INTR_20_MS    = 16,
	CONN_INTR_21P25_MS = 17,
	CONN_INTR_22P5_MS  = 18,
	CONN_INTR_23P75_MS = 19,
	CONN_INTR_25_MS    = 20,
	CONN_INTR_30_MS    = 24,
	CONN_INTR_35_MS    = 28,
	CONN_INTR_40_MS    = 32,
	CONN_INTR_45_MS    = 36,
	CONN_INTR_48P75_MS = 39,
	CONN_INTR_50_MS    = 40,
	CONN_INTR_60_MS    = 48,
	CONN_INTR_70_MS    = 56,
	CONN_INTR_80_MS    = 64,
	CONN_INTR_90_MS    = 72,
	CONN_INTR_91P25_MS = 73,
	CONN_INTR_100_MS   = 80,
	CONN_INTR_200_MS   = 160,
	CONN_INTR_300_MS   = 240,
	CONN_INTR_400_MS   = 320,
	CONN_INTR_500_MS   = 400,
	CONN_INTR_1S       = 800,
	CONN_INTR_2S       = 1600,
	CONN_INTR_3S       = 2400,
	CONN_INTR_4S       = 3200,
	CONN_INTR_5S       = 4000,
	CONN_INTR_6S       = 4800,
};

/* ADV interval define. unit: 0.625ms */
enum{
	ADV_INTR_3P75_MS   = 6,
	ADV_INTR_20_MS     = 32,
	ADV_INTR_25_MS     = 40,
	ADV_INTR_30_MS     = 48,
	ADV_INTR_35_MS     = 56,
	ADV_INTR_40_MS     = 64,
	ADV_INTR_45_MS     = 72,
	ADV_INTR_50_MS     = 80,
	ADV_INTR_60_MS     = 96,
	ADV_INTR_70_MS     = 112,
	ADV_INTR_80_MS     = 128,
	ADV_INTR_90_MS     = 144,
	ADV_INTR_100_MS    = 160,
	ADV_INTR_200_MS    = 320,
	ADV_INTR_300_MS    = 480,
	ADV_INTR_1S        = 1600,
};

/* Scan interval define. unit: 0.625ms */
enum{
	SCAN_INTR_20_MS     = 32,
	SCAN_INTR_25_MS     = 40,
	SCAN_INTR_30_MS     = 48,
	SCAN_INTR_35_MS     = 56,
	SCAN_INTR_40_MS     = 64,
	SCAN_INTR_45_MS     = 72,
	SCAN_INTR_50_MS     = 80,
	SCAN_INTR_60_MS     = 96,
	SCAN_INTR_70_MS     = 112,
	SCAN_INTR_80_MS     = 128,
	SCAN_INTR_90_MS     = 144,
	SCAN_INTR_100_MS    = 160,
	SCAN_INTR_125_MS    = 200,
	SCAN_INTR_200_MS    = 320,
	SCAN_INTR_300_MS    = 480,
	SCAN_INTR_1S        = 1600,
};


#endif // BLE_TYPES_H
