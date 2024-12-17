/**
 *******************************************************************************
 * @FileName  : ll_defs.h
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

#ifndef LL_DEFS_H_
#define LL_DEFS_H_

/*** Version ***/

#define LL_VER_BT_CORE_SPEC_4_0     0x06    /*!< Bluetooth core specification 4.0 */
#define LL_VER_BT_CORE_SPEC_4_1     0x07    /*!< Bluetooth core specification 4.1 */
#define LL_VER_BT_CORE_SPEC_4_2     0x08    /*!< Bluetooth core specification 4.2 */
#define LL_VER_BT_CORE_SPEC_5_0     0x09    /*!< Bluetooth core specification 5.0 */
#define LL_VER_BT_CORE_SPEC_5_1     0x0A    /*!< Bluetooth core specification 5.1 */
#define LL_VER_BT_CORE_SPEC_5_2     0x0B    /*!< Bluetooth core specification 5.2 */

#define LL_COMPANY_ID_TELINK        0x0211  /*!< Telink Semi ltd.c */
#define LL_COMPANY_ID_PANCHIP       0x07D1  /*!< Panchip Semi ltd.c */

#define LL_VER_BT_CORE_SPEC         LL_VER_BT_CORE_SPEC_5_0
#define LL_COMPANY_ID               LL_COMPANY_ID_PANCHIP

/*!   Link state machine */
enum{
	LL_STATE_STANDBY = 0x00,
	LL_STATE_ADV     = 0x01,
	LL_STATE_SCAN    = 0x02,
	LL_STATE_INIT,
	LL_STATE_CONN,
	LL_STATE_EXT_ADV,
	LL_STATE_PERIODIC_ADV,
	LL_STATE_EXT_SCAN,
	LL_STATE_EXT_INIT,

#if ANT_EN
	LL_STATE_ANT,
#endif
};

/*** Common ***/

#define LL_RSSI_MIN                 -127    /*!< Minimum RSSI value. */
#define LL_RSSI_MAX                 20      /*!< Maximum RSSI value. */
#define LL_RSSI_NOT_AVAIL           127     /*!< RSSI is not available. */

#define LL_CRC_LEN                  3       /*!< CRC length. */
#define LL_AA_LEN                   4       /*!< Access address length. */
#define LL_PREAMBLE_LEN_1M          1       /*!< Preamble length (LE 1M PHY). */
#define LL_PREAMBLE_LEN_2M          2       /*!< Preamble length (LE 2M PHY). */
#define LL_PREAMBLE_LEN_CODED_BITS  10      /*!< Preamble length (LE Coded PHY). */
#define LL_CI_LEN_BITS              2       /*!< Coding indicator length (LE Coded PHY). */
#define LL_TERM1_LEN_BITS           3       /*!< TERM1 length (LE Coded PHY). */
#define LL_TERM2_LEN_BITS           3       /*!< TERM2 length (LE Coded PHY). */

#define LL_RAND_ADDR_TYPE_MASK      (uint64_t)(0xC00000000000)    /*!< BD Random Address type mask. */
#define LL_RAND_ADDR_TYPE_STATIC    (uint64_t)(0xC00000000000)    /*!< Static Random Address type. */
#define LL_RAND_ADDR_TYPE_RPA       (uint64_t)(0x400000000000)    /*!< Resolvable Private Address type. */
#define LL_RAND_ADDR_TYPE_NRPA      (uint64_t)(0x000000000000)    /*!< Non-Resolvable Private Address type. */

/*** Advertising PDU ***/

/*!  Advertising channel PDU types. */
enum
{
  /* --- Core Spec 4.0 --- */
  LL_PDU_ADV_IND                = 0,    /*!< Connectable undirected advertising PDU. */
  LL_PDU_ADV_DIRECT_IND         = 1,    /*!< Connectable directed advertising PDU. */
  LL_PDU_ADV_NONCONN_IND        = 2,    /*!< Non-connectable undirected advertising PDU. */
  LL_PDU_SCAN_REQ               = 3,    /*!< Scan request PDU. */
  LL_PDU_SCAN_RSP               = 4,    /*!< Scan response PDU. */
  LL_PDU_CONNECT_IND            = 5,    /*!< Connect indication PDU. */
  LL_PDU_ADV_SCAN_IND           = 6,    /*!< Scannable undirected advertising PDU. */

  /* --- Core Spec 5.0 --- */
  LL_PDU_AUX_SCAN_REQ           = 3,    /*!< Auxiliary scan request PDU. */
  LL_PDU_AUX_CONNECT_REQ        = 5,    /*!< Auxiliary connect request PDU. */
  LL_PDU_ADV_EXT_IND            = 7,    /*!< Extended advertising PDU. */
  LL_PDU_AUX_ADV_IND            = 7,    /*!< Auxiliary advertising PDU. */
  LL_PDU_AUX_SCAN_RSP           = 7,    /*!< Auxiliary scan response PDU. */
  LL_PDU_AUX_SYNC_IND           = 7,    /*!< Auxiliary synchronize PDU. */
  LL_PDU_AUX_CHAIN_IND          = 7,    /*!< Auxiliary chain PDU. */
  LL_PDU_AUX_CONNECT_RSP        = 8,    /*!< Auxiliary connect response PDU. */
};

#define LL_SCAN_REQ_PLD_LEN     12      /*!< Size of a scan request payload. */
#define LL_CONN_IND_PLD_LEN     34      /*!< Size of a connect indication payload. */

#define LL_ADV_DATA_MAX_LEN     31      /*!< Maximum advertising channel host data length. */
#define LL_ADV_PLD_MAX_LEN      37      /*!< Maximum advertising channel Payload length. */
#define LL_ADV_PDU_MAX_LEN      39      /*!< Maximum advertising channel PDU length. */

#define LL_CHAN_ADV_MIN_IDX     37      /*!< Minimum advertising channel index. */
#define LL_CHAN_ADV_MAX_IDX     39      /*!< Maximum advertising channel index. */
#define LL_NUM_CHAN_ADV         3       /*!< Total number of advertising channels. */


#define LL_ADV_ACCESS_ADDR      (0x8E89BED6)    /*!< Advertising channel access address. */
#define LL_ADV_CRC_INIT         (0x555555)      /*!< Advertising CRC initial value. */

#define LL_DIR_ADV_INTER_TICKS  6       /*!< Advertising interval between directed advertising events (3.75 ms). */
#define LL_DIR_ADV_DUR_TICKS    2048    /*!< Maximum high duty cycle directed advertising duration (1.28 seconds). */

/* Extended Header field length */
#define LL_EXT_HDR_FLAGS_LEN    1
#define LL_CTE_INFO_LEN         1
#define LL_ADI_LEN              2
#define LL_AUX_PTR_LEN          3
#define LL_SYNC_INFO_LEN        18
#define LL_TX_PWR_LEN           1

/*** Data PDU ***/

/*!  Data channel LL Control PDU types. */
enum
{
  /* --- Core Spec 4.0 --- */
  LL_PDU_CONN_UPDATE_IND        = 0x00, /*!< Connection update indication PDU. */
  LL_PDU_CHANNEL_MAP_IND        = 0x01, /*!< Channel map indication PDU. */
  LL_PDU_TERMINATE_IND          = 0x02, /*!< Terminate indication PDU. */
  LL_PDU_ENC_REQ                = 0x03, /*!< Encryption request PDU. */
  LL_PDU_ENC_RSP                = 0x04, /*!< Encryption response PDU. */
  LL_PDU_START_ENC_REQ          = 0x05, /*!< Start encryption request PDU. */
  LL_PDU_START_ENC_RSP          = 0x06, /*!< Start encryption response PDU. */
  LL_PDU_UNKNOWN_RSP            = 0x07, /*!< Unknown response PDU. */
  LL_PDU_FEATURE_REQ            = 0x08, /*!< Feature request PDU. */
  LL_PDU_FEATURE_RSP            = 0x09, /*!< Feature response PDU. */
  LL_PDU_PAUSE_ENC_REQ          = 0x0A, /*!< Pause encryption request PDU. */
  LL_PDU_PAUSE_ENC_RSP          = 0x0B, /*!< Pause encryption response PDU. */
  LL_PDU_VERSION_IND            = 0x0C, /*!< Version indication PDU. */
  LL_PDU_REJECT_IND             = 0x0D, /*!< Reject indication PDU. */
  /* --- Core Spec 4.2 --- */
  LL_PDU_SLV_FEATURE_REQ        = 0x0E, /*!< Slave feature request PDU. */
  LL_PDU_CONN_PARAM_REQ         = 0x0F, /*!< Connection parameter request PDU. */
  LL_PDU_CONN_PARAM_RSP         = 0x10, /*!< Connection parameter response PDU. */
  LL_PDU_REJECT_EXT_IND         = 0x11, /*!< Reject extended indication PDU. */
  LL_PDU_PING_REQ               = 0x12, /*!< Ping request PDU. */
  LL_PDU_PING_RSP               = 0x13, /*!< Ping response PDU. */
  LL_PDU_LENGTH_REQ             = 0x14, /*!< Data length request PDU. */
  LL_PDU_LENGTH_RSP             = 0x15, /*!< Data length response PDU. */

  /* --- Core Spec 5.0 --- */
  LL_PDU_PHY_REQ                = 0x16, /*!< PHY request PDU. */
  LL_PDU_PHY_RSP                = 0x17, /*!< PHY response PDU. */
  LL_PDU_PHY_UPDATE_IND         = 0x18, /*!< PHY update indication PDU. */
  LL_PDU_MIN_USED_CHAN_IND      = 0x19, /*!< Minimum used channels indication PDU. */

  LL_PDU_UNSPECIFIED            = 0xFF  /*!< Unspecified PDU. */
};

/* Data Payload length */
/* --- Core Spec 4.0 --- */
#define LL_CONN_UPD_IND_PLD_LEN       12      /*!< Connection update indication PAYLAOD length. */
#define LL_CHAN_MAP_IND_PLD_LEN       8       /*!< Channel map indication PAYLAOD length. */
#define LL_TERMINATE_IND_PLD_LEN      2       /*!< Terminate indication PAYLAOD length. */
#define LL_ENC_REQ_LEN                23      /*!< Encryption request PAYLAOD length. */
#define LL_ENC_RSP_LEN                13      /*!< Encryption response PAYLAOD length. */
#define LL_START_ENC_LEN              1       /*!< Start encryption request/response PAYLAOD length. */
#define LL_UNKNOWN_RSP_LEN            2       /*!< Unknown response PAYLAOD length. */
#define LL_FEATURE_PLD_LEN            9       /*!< Feature request/response PAYLAOD length. */
#define LL_PAUSE_ENC_LEN              1       /*!< Pause encryption request/response PAYLAOD length. */
#define LL_VERSION_IND_PLD_LEN        6       /*!< Version indication PAYLAOD length. */
#define LL_REJECT_IND_PLD_LEN         2       /*!< Reject indication PAYLAOD length. */
/* --- Core Spec 4.2 --- */
#define LL_CONN_PARAM_PLD_LEN         24      /*!< Connection parameter request or response PAYLAOD length. */
#define LL_REJECT_EXT_IND_PLD_LEN     3       /*!< Reject extended indication PAYLAOD length. */
#define LL_PING_PLD_LEN               1       /*!< Ping request/response PAYLAOD length. */
#define LL_DATA_LEN_PLD_LEN           9       /*!< Data length request or response PAYLAOD length. */
/* --- Core Spec 5.0 --- */
#define LL_PHY_PLD_LEN                3       /*!< PHY request/response PAYLAOD length. */
#define LL_PHY_UPD_IND_PLD_LEN        5       /*!< PHY update indication PAYLAOD length. */
#define LL_MIN_USED_CHAN_PLD_LEN      3       /*!< Minimum used channels indication PAYLAOD length. */


#define LL_EMPTY_PDU_LEN              2       /*!< Length of an empty data PDU. */

#define LL_DATA_HDR_LEN               2       /*!< Data channel header length. */
#define LL_DATA_MIC_LEN               4       /*!< Data channel PDU MIC length. */

#define LL_MAX_DATA_CHN_NUM           37      /*!< Maximum number of used data channels. */
#define LL_MIN_DATA_CHN_NUM           2       /*!< Minimum number of used data channels. */


/*!  Data PDU LLID types. */
enum
{
  LL_LLID_VS_PDU                = 0x00, /*!< Vendor specific PDU. */

  /* N.B. next two enumerations intentionally use identical values. */
  LL_LLID_EMPTY_PDU             = 0x01, /*!< Empty PDU. */
  LL_LLID_CONT_PDU              = 0x01, /*!< Data PDU: continuation fragment of an L2CAP message. */
  LL_LLID_START_PDU             = 0x02, /*!< Data PDU: start of an L2CAP message or a complete L2CAP message with no fragmentation. */
  LL_LLID_CTRL_PDU              = 0x03, /*!< Control PDU. */
};

#define LL_PB_START_H2C          0x00  /*!< Packet boundary flag, start, host-to-controller */
#define LL_PB_CONTINUE           0x01  /*!< Packet boundary flag, continue */
#define LL_PB_START_C2H          0x02  /*!< Packet boundary flag, start, controller-to-host */


/*** Feature ***/

#define LL_FEAT_LEN                   8       /*!< Length of features byte array */

/*** Encryption ***/

#define LL_ECC_KEY_LEN                32      /*!< ECC key length. */
#define LL_RAND_LEN                   8       /*!< Length of random number */
#define LL_KEY_LEN                    16      /*!< Encryption key length. */
#define LL_SKD_LEN                    LL_KEY_LEN      /*!< Session key diversifier length. */
#define LL_IV_LEN                     8       /*!< Initialization vector length. */
#define LL_NONCE_LEN                  13      /*!< Nonce length. */

#define LL_DEF_RES_ADDR_TO_SEC        900     /*!< Default resolvable address timeout in seconds. */
#define LL_DEF_AUTH_TO_MS             30000   /*!< Default authentication timeout in milliseconds. */

/*** LLCP ***/

/*!   Data Channel PDU format = Header + Payload + MIC (1M PHY = 1us/per bit) */
#define LL_MAX_TX_RX_TIME_1M(payload)              (uint16_t)((10 + (payload) + 4)<<3)
#define LL_MAX_TX_RX_TIME_2M(payload)              (uint16_t)((10 + (payload) + 4)<<2)
#define LL_MAX_TX_RX_TIME_CODED(payload, phyOpt)   (uint16_t)(80+256+16+24 + (2+payload+4+3)*8*(phyOpt<<phyOpt) + 3*(phyOpt<<phyOpt))
#define LL_MAX_TX_RX_TIME_CODED_S2(payload)        (uint16_t)(80+256+16+24 + (2+payload+4+3)*8*2 + 3*2)
#define LL_MAX_TX_RX_TIME_CODED_S8(payload)        (uint16_t)(80+256+16+24 + (2+payload+4+3)*8*8 + 3*8)

#define LL_TIME_TO_OCTETS_1M(time)                 (uint16_t)(((time) - 112)>>3)
#define LL_TIME_TO_OCTETS_2M(time)                 (uint16_t)(((time) - 56)>>2)
#define LL_TIME_TO_OCTETS_CODED_S2(time)           (uint16_t)((((time) - 382)>>4) - 9)
#define LL_TIME_TO_OCTETS_CODED_S8(time)           (uint16_t)((((time) - 400)>>6) - 9)

/*!   Link default MAX Tx Octets and time. PHY = 1MHz */
#define LL_DEF_MAX_TX_OCTETS          27
#define LL_DEF_MAX_TX_TIME_1M         LL_MAX_TX_RX_TIME_1M(LL_DEF_MAX_TX_OCTETS)

/*!  Link default MAX rx Octets and time. PHY = 1MHz */
#define LL_DEF_MAX_RX_OCTETS          27
#define LL_DEF_MAX_RX_TIME_1M         LL_MAX_TX_RX_TIME_1M(LL_DEF_MAX_RX_OCTETS)

#define LL_SUP_MIN_TX_RX_OCTETS       27  /*!< Link supported Min RX/TX payload size */
#define LL_SUP_MAX_TX_RX_OCTETS       251 /*!< Link supported Max RX/TX payload size */
#define LL_SUP_MAX_TX_OCTETS          251 /*!< Link supported Max TX payload size */
#define LL_SUP_MAX_RX_OCTETS          251 /*!< Link supported Max TX payload size */

#define LL_MIN_TX_WIN_SIZE            1       /*!< Minimum value for transmit window size. */
#define LL_MAX_TX_WIN_SIZE            8       /*!< Maximum value for transmit window size. */

#define LL_MIN_CONN_INTERVAL          6       /*!< Minimum value for connection interval. */
#define LL_MAX_CONN_INTERVAL          3200    /*!< Maximum value for connection interval. */

#define LL_MAX_CONN_LATENCY           499     /*!< Maximum value for connection slave latency. */

#define LL_MIN_SUP_TIMEOUT            10      /*!< Minimum value for connection supervision timeout. */
#define LL_MAX_SUP_TIMEOUT            3200    /*!< Maximum value for connection supervision timeout. */

#define LL_CHAN_MAP_LEN               5       /*!< Channel map length */
#define LL_CHAN_MAP_MASK              0x1FFFFFFFFF

#endif /* BLC_DEFS_H_ */
