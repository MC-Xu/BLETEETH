/**
 *******************************************************************************
 * @FileName  : rf_phy_spi.c
 * @Author    : GaoQiu
 * @CreateDate: 2024-04-18
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

#ifndef RF_PHY_H_
#define RF_PHY_H_

#include "utils/defs_types.h"

#define PHY_CLK           32 // unit:MHz
#define PHY_CLK_PRESCALE  2  // XXX: stable value = 4

#define REG_WRITE_BITS(reg, mask, value)    (reg) = (((reg) & ~(mask)) | (value))

enum{
	RF_RW_MODE_READ      = 0x03,
	RF_RW_MODE_READ_EXE  = 0x13,

	RF_RW_MODE_WRITE     = 0x02,
	RF_RW_MODE_WRITE_EXE = 0x12,
};

typedef uint32_t (*RF_TxPowerPreHandlerCback_t)(int8_t txPwr);
void RF_RegisterTxPwrPreHandler(RF_TxPowerPreHandlerCback_t func);

void RF_SetPhyAgcMode(uint8_t agcMode);

void RF_PhyInit(void);

void RF_EnableLDO(void);
void RF_DisableLDO(void);

void RF_PhyResetSeq(void);

void RF_SetPowerLevel(int8_t tx_pwr);

void RF_SetChannel(uint8_t chan);
void RF_SetBlePhy(uint8_t rxPhy, uint8_t txPhy, uint8_t phyOpt);
void RF_EnablePhy(uint8_t enable);

void RF_SetPhyCfgTime(uint8_t op_mode);
void RF_PhyResetEx(void);
uint16_t RF_GetTxSettleTime(void);
uint16_t RF_GetRxSettleTime(void);

int8_t RF_CalcRssi(int32_t rssiInit);


/* RF PHY SPI API */
void RF_SetPhySpiClk(uint8_t div);
uint32_t RF_WritePhyCfg(const uint32_t *cmd, const uint32_t cmdNum, uint32_t *pRspBuf);

uint8_t RF_ReadReg(uint8_t addr, uint32_t resultBuf[1]);
void RF_WriteReg(uint32_t *cmd, uint8_t cmdLen);
void RF_WriteRegEnd(void);

uint32_t RF_ReadWritePhyReg(uint8_t rwMode, uint8_t addr, uint8_t data);
void RF_SetPhyRegPage(uint8_t pageId);

/* RF Msic API */
extern void dcoc_calibration_process(void);
extern bool_t check_dcoc_calib_ok(void);

#endif /* RF_PHY_H_ */
