// *** <<< Use Configuration Wizard in Context Menu >>> ***

#ifndef SAMPLE_CONFIG_H
#define SAMPLE_CONFIG_H


// <h> PAN_PRF Config
// <o> prf work mode select   <0=> PRF_MODE_NORMAL    <1=> PRF_MODE_ENHANCE    <2=> PRF_MODE_NORMAL_M1
#define PRF_WORK_MODE                   0
// <i> prf work mode select

// <o> prf chip mode select   <2=> PRF_CHIP_MODE_SEL_XN297    <3=> PRF_CHIP_MODE_SEL_NRF
#define PRF_CHIP_MODE                  3
// <i> prf chip mode select

// <o> prf trx mode select   <0=> PRF_TX_MODE    <1=> PRF_RX_MODE
#define PRF_TRX_SELECT                  0
// <i> prf trx mode select

// <o> prf phy select   <1=> PRF_PHY_1M    <2=> PRF_PHY_2M    <3=> PRF_PHY_CODED_S8    <4=> PRF_PHY_CODED    <5=> PRF_PHY_250K
#define PRF_PHY_SELECT                  5
// <i> prf phy select

// <o> prf crc select   <0=> PRF_CRC_SEL_NOCRC    <1=> PRF_CRC_SEL_CRC8    <2=> PRF_CRC_SEL_CRC16    <3=> PRF_CRC_SEL_CRC24
#define PRF_CRC                  2
// <i> prf crc select

// <o> prf scramble select   <0=> PRF_SRC_SEL_NOSRC    <1=> PRF_SRC_SEL_EN
#define PRF_SCR                  0
// <i> prf scramble select

// <o> prf mode config select   <1=> PRF_BLE_CONF    <2=> PRF_NRF_CONF    <3=> PRF_G_250k_CONF    <4=> PRF_B_250K_CONF
#define PRF_MODE_CONFIG                  4
// <i> prf mode config select

// <o> prf transfer type select   <0=> PRF_TRF_NORMAL    <1=> PRF_TRF_NRF52    <2=> PRF_TRF_B250K
#define PRF_TRANS_TYPE                  2
// <i> prf transfer type select

// </h>

#endif // SAMPLE_CONFIG_H

// *** <<< end of configuration section >>>    ***
