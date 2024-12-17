/**
 *******************************************************************************
 * @FileName  : ll_api.h
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

#ifndef LL_API_H_
#define LL_API_H_

#include "stack/ble_types.h"
#include "stack/hci_defs.h"
#include "stack/ble_config.h"
#include "ll_defs.h"

#include "utils/bd_addr.h"
#include "utils/mdc.h"

/*!   Link Layer sub version number. */
#define LL_SUB_VERSION_NUM       1020

enum{
    UNIT_0_625_MS = 625,        /*!< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,       /*!< Number of microseconds in 1.25 milliseconds.  */
    UNIT_10_MS    = 10000       /*!< Number of microseconds in 10 milliseconds.    */
};
#define MSEC_TO_UNITS(TIME, RESOLUTION)   (((TIME) * 1000) / (RESOLUTION))


/*!  Link configuration type */
typedef struct{
	/* Device Info */
	uint8_t   versNr;            /*!< Bluetooth controller specification version. */
	uint16_t  compId;            /*!< Company identifier. */
	uint16_t  subVersNr;         /*!< Sub-Version number. */

	/* Adv */
	uint8_t   wlNum;             /*!< White-list size.    */
	uint8_t   rlNum;             /*!< Resolving-list size.    */
	uint8_t   connCreateTo;      /*!< Create connection timeout. 0: disable timeout timer, other: timeout time, unit:s */

	/* extended Adv */
	uint8_t   numAdvSet;         /*!< The max number of ADV set. */
	uint16_t  extAdvDataLen;     /*!< ADV data length for adv set. */
	uint16_t  extScanRspDataLen; /*!< Scan response data length for adv set. */
	uint8_t   advMargin;         /*!< unit:ms */

	/* Connection */
	uint8_t   maxMst;            /*!< Number of max master connection. */
	uint8_t   maxSlv;            /*!< Number of max slave connection.  */
	uint8_t   maxAclLen;         /*!< Max ACL length. */
	uint8_t   numRxBufs;         /*!< Number of Rx Buffer. */
	uint8_t   numTxBufs;         /*!< Number of Tx Buffer. */
	uint16_t  localSCA;          /*!< local sleep clock accuracy. Unit:ppm */

	/* Schedule */
	uint16_t  mstMargin;         /*!< Master connection margin. min:4ms. Unit: ms*/
	uint8_t   numMoreDat;        /*!< Number of max more data. Range: 1-16. */

	/* BB */
	uint8_t  sleepClockSrc;      /*!< [Panchip] select sleep clock source. 0:32k RC, 1:32k XTL. */
	uint16_t bbSetupTime;        /*!< [Panchip] BB PHY setup time. Unit:us */
	uint16_t llEncTime;          /*!< [Panchip] BB CCM time. Unit:us */

	/* Other */
	uint8_t   authPyldTimeoutEn;

	/* ANT */
	uint8_t   numMaxAnt;
}LlCfg_t;
extern const LlCfg_t *pLlConfig;


/*!   The following status values are used in the LL API. */
enum
{
  LL_SUCCESS                                          = 0x00,
  LL_ERR_CODE_UNKNOWN_HCI_CMD                         = 0x01,
  LL_ERR_CODE_UNKNOWN_CONN_ID                         = 0x02,
  LL_ERR_CODE_HW_FAILURE                              = 0x03,
  LL_ERR_CODE_PAGE_TIMEOUT                            = 0x04,
  LL_ERR_CODE_AUTH_FAILURE                            = 0x05,
  LL_ERR_CODE_PIN_KEY_MISSING                         = 0x06,
  LL_ERR_CODE_MEM_CAP_EXCEEDED                        = 0x07,
  LL_ERR_CODE_CONN_TIMEOUT                            = 0x08,
  LL_ERR_CODE_CONN_LIMIT_EXCEEDED                     = 0x09,
  LL_ERR_CODE_SYNCH_CONN_LIMIT_EXCEEDED               = 0x0A,
  LL_ERR_CODE_ACL_CONN_ALREADY_EXISTS                 = 0x0B,
  LL_ERR_CODE_CMD_DISALLOWED                          = 0x0C,
  LL_ERR_CODE_CONN_REJ_LIMITED_RESOURCES              = 0x0D,
  LL_ERR_CODE_CONN_REJECTED_SECURITY_REASONS          = 0x0E,
  LL_ERR_CODE_CONN_REJECTED_UNACCEPTABLE_BDADDR       = 0x0F,
  LL_ERR_CODE_CONN_ACCEPT_TIMEOUT_EXCEEDED            = 0x10,
  LL_ERR_CODE_UNSUPPORTED_FEATURE_PARAM_VALUE         = 0x11,
  LL_ERR_CODE_INVALID_HCI_CMD_PARAMS                  = 0x12,
  LL_ERR_CODE_REMOTE_USER_TERM_CONN                   = 0x13,
  LL_ERR_CODE_REMOTE_DEVICE_TERM_CONN_LOW_RESOURCES   = 0x14,
  LL_ERR_CODE_REMOTE_DEVICE_TERM_CONN_POWER_OFF       = 0x15,
  LL_ERR_CODE_CONN_TERM_BY_LOCAL_HOST                 = 0x16,
  LL_ERR_CODE_REPEATED_ATTEMPTS                       = 0x17,
  LL_ERR_CODE_PAIRING_NOT_ALLOWED                     = 0x18,
  LL_ERR_CODE_UNKNOWN_LMP_PDU                         = 0x19,
  LL_ERR_CODE_UNSUPPORTED_REMOTE_FEATURE              = 0x1A,
  LL_ERR_CODE_SCO_OFFSET_REJ                          = 0x1B,
  LL_ERR_CODE_SCO_INTERVAL_REJ                        = 0x1C,
  LL_ERR_CODE_SCO_AIR_MODE_REJ                        = 0x1D,
  LL_ERR_CODE_INVALID_LMP_PARAMS                      = 0x1E,
  LL_ERR_CODE_UNSPECIFIED_ERROR                       = 0x1F,
  LL_ERR_CODE_UNSUPPORTED_LMP_PARAM_VAL               = 0x20,
  LL_ERR_CODE_ROLE_CHANGE_NOT_ALLOWED                 = 0x21,
  LL_ERR_CODE_LMP_LL_RESP_TIMEOUT                     = 0x22,
  LL_ERR_CODE_LMP_ERR_TRANSACTION_COLLISION           = 0x23,
  LL_ERR_CODE_LMP_PDU_NOT_ALLOWED                     = 0x24,
  LL_ERR_CODE_ENCRYPT_MODE_NOT_ACCEPTABLE             = 0x25,
  LL_ERR_CODE_LINK_KEY_CAN_NOT_BE_CHANGED             = 0x26,
  LL_ERR_CODE_REQ_QOS_NOT_SUPPORTED                   = 0x27,
  LL_ERR_CODE_INSTANT_PASSED                          = 0x28,
  LL_ERR_CODE_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED     = 0x29,
  LL_ERR_CODE_DIFFERENT_TRANSACTION_COLLISION         = 0x2A,
  LL_ERR_CODE_RESERVED1                               = 0x2B,
  LL_ERR_CODE_QOS_UNACCEPTABLE_PARAM                  = 0x2C,
  LL_ERR_CODE_QOS_REJ                                 = 0x2D,
  LL_ERR_CODE_CHAN_ASSESSMENT_NOT_SUPPORTED           = 0x2E,
  LL_ERR_CODE_INSUFFICIENT_SECURITY                   = 0x2F,
  LL_ERR_CODE_PARAM_OUT_OF_MANDATORY_RANGE            = 0x30,
  LL_ERR_CODE_RESERVED2                               = 0x31,
  LL_ERR_CODE_ROLE_SWITCH_PENDING                     = 0x32,
  LL_ERR_CODE_RESERVED3                               = 0x33,
  LL_ERR_CODE_RESERVED_SLOT_VIOLATION                 = 0x34,
  LL_ERR_CODE_ROLE_SWITCH_FAILED                      = 0x35,
  LL_ERR_CODE_EXTENDED_INQUIRY_RESP_TOO_LARGE         = 0x36,
  LL_ERR_CODE_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST    = 0x37,
  LL_ERR_CODE_HOST_BUSY_PAIRING                       = 0x38,
  LL_ERR_CODE_CONN_REJ_NO_SUITABLE_CHAN_FOUND         = 0x39,
  LL_ERR_CODE_CONTROLLER_BUSY                         = 0x3A,
  LL_ERR_CODE_UNACCEPTABLE_CONN_INTERVAL              = 0x3B,
  LL_ERR_CODE_ADV_TIMEOUT                             = 0x3C,
  LL_ERR_CODE_CONN_TERM_MIC_FAILURE                   = 0x3D,
  LL_ERR_CODE_CONN_FAILED_TO_ESTABLISH                = 0x3E,
  LL_ERR_CODE_MAC_CONN_FAILED                         = 0x3F,
  LL_ERR_CODE_COARSE_CLK_ADJ_REJ                      = 0x40,
  LL_ERR_CODE_TYPE0_SUBMAP_NOT_DEF                    = 0x41,
  LL_ERR_CODE_UNKNOWN_ADV_ID                          = 0x42,
  LL_ERR_CODE_LIMIT_REACHED                           = 0x43,
  LL_ERR_CODE_OP_CANCELLED_BY_HOST                    = 0x44,
  LL_ERR_CODE_PKT_TOO_LONG                            = 0x45,

  LL_ERR_CODE_MAX
};

/*! The supported state bitmask indicates the LE states supported by the LL. */
#define LL_SUP_STATE_NON_CONN_ADV                  (UINT64(1) <<  0)    /*!< Non-connectable Advertising State supported. */
#define LL_SUP_STATE_SCAN_ADV                      (UINT64(1) <<  1)    /*!< Scannable Advertising State supported. */
#define LL_SUP_STATE_CONN_ADV                      (UINT64(1) <<  2)    /*!< Connectable Advertising State supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV               (UINT64(1) <<  3)    /*!< High Duty Cycle Directed Advertising State supported. */
#define LL_SUP_STATE_PASS_SCAN                     (UINT64(1) <<  4)    /*!< Passive Scanning State supported. */
#define LL_SUP_STATE_ACT_SCAN                      (UINT64(1) <<  5)    /*!< Active Scanning State supported. */
#define LL_SUP_STATE_INIT                          (UINT64(1) <<  6)    /*!< Initiating State supported. Connection State in the Master Role supported is also supported. */
#define LL_SUP_STATE_CONN_SLV                      (UINT64(1) <<  7)    /*!< Connection State in the Slave Role supported. */
#define LL_SUP_STATE_NON_CONN_ADV_AND_PASS_SCAN    (UINT64(1) <<  8)    /*!< Non-connectable Advertising State and Passive Scanning State combination supported. */
#define LL_SUP_STATE_SCAN_ADV_AND_PASS_SCAN        (UINT64(1) <<  9)    /*!< Scannable Advertising State and Passive Scanning State combination supported. */
#define LL_SUP_STATE_CONN_ADV_AND_PASS_SCAN        (UINT64(1) << 10)    /*!< Connectable Advertising State and Passive Scanning State combination supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV_AND_PASS_SCAN (UINT64(1) << 11)    /*!< Directed Advertising State and Passive Scanning State combination supported. */
#define LL_SUP_STATE_NON_CONN_ADV_AND_ACT_SCAN     (UINT64(1) << 12)    /*!< Non-connectable Advertising State and Active Scanning State combination supported. */
#define LL_SUP_STATE_SCAN_ADV_AND_ACT_SCAN         (UINT64(1) << 13)    /*!< Scannable Advertising State and Active Scanning State combination supported. */
#define LL_SUP_STATE_CONN_ADV_AND_ACT_SCAN         (UINT64(1) << 14)    /*!< Connectable Advertising State and Active Scanning State combination supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV_ACT_SCAN      (UINT64(1) << 15)    /*!< Directed Advertising State and Active Scanning State combination supported. */
#define LL_SUP_STATE_NON_CONN_ADV_AND_INIT         (UINT64(1) << 16)    /*!< Non-connectable Advertising State and Initiating State combination supported. */
#define LL_SUP_STATE_SCAN_ADV_AND_INIT             (UINT64(1) << 17)    /*!< Scannable Advertising State and Initiating State combination supported */
#define LL_SUP_STATE_NON_CONN_ADV_MST              (UINT64(1) << 18)    /*!< Non-connectable Advertising State and Master Role combination supported. */
#define LL_SUP_STATE_SCAN_ADV_MST                  (UINT64(1) << 19)    /*!< Scannable Advertising State and Master Role combination supported. */
#define LL_SUP_STATE_NON_CONN_ADV_SLV              (UINT64(1) << 20)    /*!< Non-connectable Advertising State and Slave Role combination supported. */
#define LL_SUP_STATE_SCAN_ADV_SLV                  (UINT64(1) << 21)    /*!< Scannable Advertising State and Slave Role combination supported. */
#define LL_SUP_STATE_PASS_SCAN_AND_INIT            (UINT64(1) << 22)    /*!< Passive Scanning State and Initiating State combination supported. */
#define LL_SUP_STATE_ACT_SCAN_AND_INIT             (UINT64(1) << 23)    /*!< Active Scanning State and Initiating State combination supported. */
#define LL_SUP_STATE_PASS_SCAN_MST                 (UINT64(1) << 24)    /*!< Passive Scanning State and Master Role combination supported. */
#define LL_SUP_STATE_ACT_SCAN_MST                  (UINT64(1) << 25)    /*!< Active Scanning State and Master Role combination supported. */
#define LL_SUP_STATE_PASS_SCAN_SLV                 (UINT64(1) << 26)    /*!< Passive Scanning state and Slave Role combination supported. */
#define LL_SUP_STATE_ACT_SCAN_SLV                  (UINT64(1) << 27)    /*!< Active Scanning state and Slave Role combination supported. */
#define LL_SUP_STATE_INIT_MST                      (UINT64(1) << 28)    /*!< Initiating State and Master Role combination supported. Master Role and Master Role combination is also supported. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV               (UINT64(1) << 29)    /*!< Low Duty Cycle Directed Advertising State. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV_AND_PASS_SCAN (UINT64(1) << 30)    /*!< Low Duty Cycle Directed Advertising and Passive Scanning State combination supported. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV_AND_ACT_SCAN  (UINT64(1) << 31)    /*!< Low Duty Cycle Directed Advertising and Active Scanning State combination supported. */
#define LL_SUP_STATE_CONN_ADV_AND_INIT             (UINT64(1) << 32)    /*!< Connectable Advertising State and Initiating State combination supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV_AND_INIT      (UINT64(1) << 33)    /*!< High Duty Cycle Directed Advertising and Initiating combination supported. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV_AND_INIT      (UINT64(1) << 34)    /*!< Low Duty Cycle Directed Advertising and Initiating combination supported. */
#define LL_SUP_STATE_CONN_ADV_MST                  (UINT64(1) << 35)    /*!< Connectable Advertising State and Master Role combination supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV_MST           (UINT64(1) << 36)    /*!< High Duty Cycle Directed Advertising and Master Role combination supported. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV_MST           (UINT64(1) << 37)    /*!< Low Duty Cycle Directed Advertising and Master Role combination supported. */
#define LL_SUP_STATE_CONN_ADV_SLV                  (UINT64(1) << 38)    /*!< Connectable Advertising State and Slave Role combination supported. */
#define LL_SUP_STATE_HI_DUTY_DIR_ADV_SLV           (UINT64(1) << 39)    /*!< High Duty Cycle Directed Advertising and Slave Role combination supported. */
#define LL_SUP_STATE_LO_DUTY_DIR_ADV_SLV           (UINT64(1) << 40)    /*!< Low Duty Cycle Directed Advertising and Slave Role combination supported. */
#define LL_SUP_STATE_INIT_SLV                      (UINT64(1) << 41)    /*!< Initiating State and Slave Role combination. */


/*!   Link layer feature define */
/* --- Core Spec 4.0 --- */
#define LL_FEAT_ENCRYPTION                          (UINT64(1) <<  0)   /*!< Encryption supported. */
/* --- Core Spec 4.2 --- */
#define LL_FEAT_CONN_PARAM_REQ_PROC                 (UINT64(1) <<  1)   /*!< Connection Parameters Request Procedure supported. */
#define LL_FEAT_EXT_REJECT_IND                      (UINT64(1) <<  2)   /*!< Extended Reject Indication supported. */
#define LL_FEAT_SLV_INIT_FEAT_EXCH                  (UINT64(1) <<  3)   /*!< Slave-Initiated Features Exchange supported. */
#define LL_FEAT_LE_PING                             (UINT64(1) <<  4)   /*!< LE Ping supported. */
#define LL_FEAT_DATA_LEN_EXT                        (UINT64(1) <<  5)   /*!< Data Length Extension supported. */
#define LL_FEAT_PRIVACY                             (UINT64(1) <<  6)   /*!< LL Privacy supported. */
#define LL_FEAT_EXT_SCAN_FILT_POLICY                (UINT64(1) <<  7)   /*!< Extended Scan Filter Policy supported. */
/* --- Core Spec 5.0 --- */
#define LL_FEAT_LE_2M_PHY                           (UINT64(1) <<  8)   /*!< LE 2M PHY supported. */
#define LL_FEAT_STABLE_MOD_IDX_TRANSMITTER          (UINT64(1) <<  9)   /*!< Stable Modulation Index - Transmitter supported. */
#define LL_FEAT_STABLE_MOD_IDX_RECEIVER             (UINT64(1) << 10)   /*!< Stable Modulation Index - Receiver supported. */
#define LL_FEAT_LE_CODED_PHY                        (UINT64(1) << 11)   /*!< LE Coded PHY supported. */
#define LL_FEAT_LE_EXT_ADV                          (UINT64(1) << 12)   /*!< LE Extended Advertising supported. */
#define LL_FEAT_LE_PER_ADV                          (UINT64(1) << 13)   /*!< LE Periodic Advertising supported. */
#define LL_FEAT_CH_SEL_2                            (UINT64(1) << 14)   /*!< Channel Selection Algorithm #2 supported. */
#define LL_FEAT_LE_POWER_CLASS_1                    (UINT64(1) << 15)   /*!< LE Power Class 1 supported. */
#define LL_FEAT_MIN_NUM_USED_CHAN                   (UINT64(1) << 16)   /*!< Minimum Number of Used Channels supported. */

#define LL_FEAT_ALL_MASK                            UINT64(0x00000000FF01FFFF) /*!< All feature mask, need to be updated when new features are added. */


/*!   Link Role */
#define LL_ROLE_MASTER                 0            /*!< Role is master. */
#define LL_ROLE_SLAVE                  1            /*!< Role is slave. */


/*!  The advertising type indicates the connectable and discoverable nature of the advertising packets transmitted by a device. */
#define LL_ADV_CONN_UNDIRECT           0            /*!< Connectable undirected advertising.  Peer devices can scan and connect to this device. */
#define LL_ADV_CONN_DIRECT_HIGH_DUTY   1            /*!< Connectable directed advertising, high duty cycle.  Only a specified peer device can connect to this device. */
#define LL_ADV_SCAN_UNDIRECT           2            /*!< Scannable undirected advertising.  Peer devices can scan this device but cannot connect. */
#define LL_ADV_NONCONN_UNDIRECT        3            /*!< Non-connectable undirected advertising.  Peer devices cannot scan or connect to this device. */
#define LL_ADV_CONN_DIRECT_LOW_DUTY    4            /*!< Connectable directed advertising, low duty cycle.  Only a specified peer device can connect to this device. */


#define IS_STATIC_RANDOM_ADDR(addr)             ( ((addr)[5] & 0xC0) == 0xC0 )
#define IS_RESOLVABLE_PRIVATE_ADDR(addr)  		( ((addr)[5] & 0xC0) == 0x40 )
#define IS_NON_RESOLVABLE_PRIVATE_ADDR(addr)  	( ((addr)[5] & 0xC0) == 0x00 )


/*! The advertising event property */
#define LL_ADV_PROP_CONN_ADV_BIT                           ((uint16_t)(1 << 0))
#define LL_ADV_PROP_SCAN_ADV_BIT                           ((uint16_t)(1 << 1))
#define LL_ADV_PROP_DIRECT_ADV_BIT                         ((uint16_t)(1 << 2))
#define LL_ADV_PROP_HIGH_DUTY_ADV_BIT                      ((uint16_t)(1 << 3))
#define LL_ADV_PROP_LEGACY_ADV_BIT                         ((uint16_t)(1 << 4))
#define LL_ADV_PROP_OMIT_AA_BIT                            ((uint16_t)(1 << 5))
#define LL_ADV_PROP_TX_PWR_EN_BIT                          ((uint16_t)(1 << 6))
#define LL_ADV_PROP_CONN_SCAN_ADV                          (LL_ADV_PROP_CONN_ADV_BIT | LL_ADV_PROP_SCAN_ADV_BIT)     /*!< only legacy mode. */
#define LL_ADV_PROP_CONN_NON_SCAN_ADV                      LL_ADV_PROP_CONN_ADV_BIT                                  /*!< Only extend mode. */
#define LL_ADV_PROP_CONN_NON_SCAN_DIRECT_ADV               (LL_ADV_PROP_CONN_ADV_BIT | LL_ADV_PROP_DIRECT_ADV_BIT)   /*!< Low duty.  */
#define LL_ADV_PROP_CONN_NON_SCAN_DIRECT_HIGH_DUTY_ADV     (LL_ADV_PROP_HIGH_DUTY_ADV_BIT | LL_ADV_PROP_CONN_ADV_BIT | LL_ADV_PROP_DIRECT_ADV_BIT)   /*!< high duty.  */
#define LL_ADV_PROP_SCAN_NON_CONN_ADV                      LL_ADV_PROP_SCAN_ADV_BIT
#define LL_ADV_PROP_NON_CONN_NON_SCAN_ADV                  0   /*!< Non-connectable and non-scannable advertising. */

/*! ADV Mode */
#define LL_ADV_MODE_NON_CONN_NON_SCAN                      0x00
#define LL_ADV_MODE_CONN_NON_SCAN                          0x01
#define LL_ADV_MODE_SCAN_NON_CONN                          0x02

/*! Advertising data operation. */
enum
{
	LL_ADV_DATA_OP_FRAG_INTER     = 0,            /*!< Intermediate fragment. */
	LL_ADV_DATA_OP_FRAG_FIRST     = 1,            /*!< First fragment. */
	LL_ADV_DATA_OP_FRAG_LAST      = 2,            /*!< Last fragment. */
	LL_ADV_DATA_OP_COMP           = 3,            /*!< Complete extended advertising data. */
	LL_ADV_DATA_OP_UNCHANGED      = 4             /*!< Unchanged data (just update the Advertising DID). */
};

/*! Advertising data fragment preference. */
enum
{
	LL_ADV_DATA_FRAG_ALLOW        = 0,            /*!< Controller may fragment all Host advertising data. */
	LL_ADV_DATA_FRAG_DISALLOW     = 1             /*!< Controller should not fragment nor minimize fragmentation of Host advertising data. */
};

/*!  The address type indicates whether an address is public or random. */
#define LL_ADDR_PUBLIC                 0            /*!< Public address. */
#define LL_ADDR_RANDOM                 1            /*!< Random address. */
#define LL_ADDR_PUBLIC_IDENTITY        2            /*!< Public identity address. */
#define LL_ADDR_RANDOM_IDENTITY        3            /*!< Random (static) identity address. */
#define LL_ADDR_RANDOM_UNRESOLVABLE    0xFE         /*!< Un-resolvable random address. */
#define LL_ADDR_ANONYMOUS              0xFF         /*!< Anonymous advertiser. */

/*!   Advertising channel bit. */
#define LL_ADV_CHAN_37_BIT            (1 << 0)      /*!< Advertising channel 37. */
#define LL_ADV_CHAN_38_BIT            (1 << 1)      /*!< Advertising channel 38. */
#define LL_ADV_CHAN_39_BIT            (1 << 2)      /*!< Advertising channel 39. */
#define LL_ADV_CHAN_ALL               0x7           /*!< All advertising channels. */

/*!   ADV Filter Policy type */
#define LL_ADV_FILTER_NONE                0x00
#define LL_ADV_FILTER_ONLY_SCAN_REQ       0x01
#define LL_ADV_FILTER_ONLY_CONN_REQ       0x02
#define LL_ADV_FILTER_SCAN_AND_CONN_REQ   0x03


/*!  Initiator filter policy. */
#define LL_INIT_FILTER_PEER           0             /*!< Filter to only Peer addr and addr type. */
#define LL_INIT_FILTER_WL             1             /*!< Filter to only whitelist. */


/*! Scan type. */
#define LL_PASSIVE_SCAN               0
#define LL_ACTIVE_SCAN                1

/*!   Scan filter policy. */
#define LL_SCAN_FILTER_NONE            0            /*!< Accept all advertising packets. */
#define LL_SCAN_FILTER_WL_BIT          1            /*!< Accept from While List only. */
#define LL_SCAN_FILTER_RES_INIT_BIT    2            /*!< Accept directed advertisements with RPAs. */
#define LL_SCAN_FILTER_WL_OR_RES_INIT  3            /*!< Accept from White List or directed advertisements with RPAs. */

/*!   Master SCA(Sleep Clock Accuracy). */
#define LL_MST_SCA_251_TO_500_PPM    0   /*<! 251 ppm to 500 ppm */
#define LL_MST_SCA_151_TO_250_PPM    1   /*<! 151 ppm to 250 ppm */
#define LL_MST_SCA_101_TO_150_PPM    2   /*<! 101 ppm to 150 ppm */
#define LL_MST_SCA_76_TO_100_PPM     3   /*<! 76  ppm to 100 ppm */
#define LL_MST_SCA_51_TO_75_PPM      4   /*<! 51  ppm to 75  ppm */
#define LL_MST_SCA_31_TO_50_PPM      5   /*<! 31  ppm to 50  ppm */
#define LL_MST_SCA_21_TO_30_PPM      6   /*<! 21  ppm to 30  ppm */
#define LL_MST_SCA_0_TO_20_PPM       7   /*<! 0   ppm to 20  ppm */

/*! LL Privacy Mode. */
#define LL_NETWORK_PRIVACY           0  /*!< Network privacy mode. */
#define LL_DEVICE_PRIVACY            1  /*!< device privacy mode.  */

/*! PHYs specification. */
enum
{
  LL_PHYS_NONE                  = 0,            /*!< No selected PHY. */
  LL_PHYS_LE_1M_BIT             = (1 << 0),     /*!< LE 1M PHY. */
  LL_PHYS_LE_2M_BIT             = (1 << 1),     /*!< LE 2M PHY. */
  LL_PHYS_LE_CODED_BIT          = (1 << 2),     /*!< LE Coded PHY. */
};

/*!  All PHYs preference. */
enum
{
  LL_ALL_PHY_ALL_PREFERENCES    = 0,            /*!< All PHY preferences. */
  LL_ALL_PHY_TX_PREFERENCE_BIT  = (1 << 0),     /*!< Set if no Tx PHY preference. */
  LL_ALL_PHY_RX_PREFERENCE_BIT  = (1 << 1)      /*!< Set if no Rx PHY preference. */
};

/*!  PHY options. */
enum
{
  LL_PHY_OPTIONS_NONE           = 0,            /*!< No preferences. */
  LL_PHY_OPTIONS_S2_PREFERRED   = 1,            /*!< S=2 coding preferred when transmitting on LE Coded PHY. */
  LL_PHY_OPTIONS_S8_PREFERRED   = 2,            /*!< S=8 coding preferred when transmitting on LE Coded PHY. */
};

/*! PHY types. HCI evt */
enum
{
  LL_PHY_NONE                   = 0,            /*!< PHY not selected. */
  LL_PHY_LE_1M                  = 1,            /*!< LE 1M PHY. */
  LL_PHY_LE_2M                  = 2,            /*!< LE 2M PHY. */
  LL_PHY_LE_CODED               = 3             /*!< LE Coded PHY. */
};

/*! Channel selection algorithm methods. */
enum
{
  LL_CH_SEL_1                   = 0,            /*!< Channel selection #1. */
  LL_CH_SEL_2                   = 1             /*!< Channel selection #2. */
};

/*! Initiator parameter */
typedef struct{
  uint16_t      scanInterval;                   /*!< Scan interval. */
  uint16_t      scanWindow;                     /*!< Scan window. */
  uint8_t       filterPolicy;                   /*!< Scan filter policy. */
  uint8_t       ownAddrType;                    /*!< Address type used by this device. */
  uint8_t       peerAddrType;                   /*!< Address type used for peer device. */
  uint8_t       *pPeerAddr;                     /*!< Address of peer device. */
} LlInitParam_t;

/*! Connection indicator parameter */
typedef struct{
  uint16_t      connIntervalMin;                /*!< Minimum connection interval. */
  uint16_t      connIntervalMax;                /*!< Maximum connection interval. */
  uint16_t      connLatency;                    /*!< Connection latency. */
  uint16_t      supTimeout;                     /*!< Supervision timeout. */
  uint16_t      minCeLen;                       /*!< Minimum CE length. */
  uint16_t      maxCeLen;                       /*!< Maximum CE length. */
} LlConnSpec_t;

/*!  Scan parameters. */
typedef struct
{
  uint16_t      scanInterval;                   /*!< Scan interval. */
  uint16_t      scanWindow;                     /*!< Scan window. */
  uint8_t       scanType;                       /*!< Scan type. */
  uint8_t       ownAddrType;                    /*!< Address type used by this device. */
  uint8_t       scanFiltPolicy;                 /*!< Scan filter policy. */
} LlScanParam_t;

/*! Extended advertising parameters. */
typedef struct{
	uint16_t      advEventProp;       /*!< Advertising Event Properties. */
	uint32_t      primAdvInterMin;    /*!< Primary Advertising Interval Minimum. */
	uint32_t      primAdvInterMax;    /*!< Primary Advertising Interval Maximum. */
	uint64_t      peerAddr;           /*!< Peer Address. */
	uint8_t       primAdvChanMap;     /*!< Primary Advertising Channel Map. */
	uint8_t       ownAddrType;        /*!< Own Address Type. */
	uint8_t       peerAddrType;       /*!< Peer Address Type. */
	uint8_t       advFiltPolicy;      /*!< Advertising Filter Policy. */
	int8_t        advTxPwr;           /*!< Advertising Tx Power. */
	uint8_t       primAdvPhy;         /*!< Primary Advertising PHY. */
	uint8_t       secAdvMaxSkip;      /*!< Secondary Advertising Maximum Skip. */
	uint8_t       secAdvPhy;          /*!< Secondary Advertising PHY. */
	uint16_t      advDID;             /*!< Advertising Data ID. */
	uint8_t       advSID;             /*!< Advertising SID. */
	uint8_t       scanReqNotifEna;    /*!< Scan Request Notification Enable. */
}LlExtAdvParam_t;

/*!  Disconnect complete event */
typedef struct{
  MdcMsgHdr_t       hdr;           /*!<  Event header. */
  uint8_t           status;        /*!<  Disconnect complete status. */
  uint16_t          handle;        /*!<  Connect handle. */
  uint8_t           reason;        /*!<  Reason. */
} LlDisconnectCmplEvt_t;

/*!  Encryption change event */
typedef struct{
  MdcMsgHdr_t       hdr;     /*!<  Event header. */
  uint8_t           status;  /*!<  Status. */
  uint16_t          handle;  /*!<  Connection handle. */
  uint8_t           enabled; /*!<  Encryption enabled flag. */
} LlEncChangeEvt_t;

/*!  Read remote version info complete event */
typedef struct{
  MdcMsgHdr_t         hdr;          /*!<  Event header. */
  uint8_t             status;       /*!<  Status. */
  uint16_t            handle;       /*!<  Connection handle. */
  uint8_t             version;      /*!<  Version. */
  uint16_t            mfrName;      /*!<  Manufacturer name. */
  uint16_t            subversion;   /*!<  Sub-version. */
} LlReadRemoteVerInfoCmplEvt_t;

/*!  Hardware error event */
typedef struct{
  MdcMsgHdr_t         hdr;  /*!<  Event header. */
  uint8_t             code; /*!<  Error code. */
} LlHwErrorEvt_t;

/*!  Encryption key refresh complete event */
typedef struct{
  MdcMsgHdr_t         hdr;    /*!<  Event header. */
  uint8_t             status; /*!<  Status. */
  uint16_t            handle; /*!<  Connection handle. */
} LlEncKeyRefreshCmpl_t;


/*!  LE connection complete event */
typedef struct{
  MdcMsgHdr_t        hdr;           /*!<  Event header */
  uint8_t            status;        /*!<  Status. */
  uint16_t           handle;        /*!<  Connection handle. */
  uint8_t            role;          /*!<  Local connection role. */
  uint8_t            peerAddrType;  /*!<  Peer address type. */
  uint64_t           peerAddr;      /*!<  Peer address. */
  uint16_t           connInterval;  /*!<  Connection interval */
  uint16_t           connLatency;   /*!<  Connection latency. */
  uint16_t           supTimeout;    /*!<  Supervision timeout. */
  uint8_t            clockAccuracy; /*!<  Clock accuracy. */

  /*  enhanced fields */
  bdAddr_t            localRpa;      /*!<  Local RPA. */
  bdAddr_t            peerRpa;       /*!<  Peer RPA. */
} LlLeConnCmplEvt_t;

/*!  LE advertising report event */
typedef struct{
  MdcMsgHdr_t         hdr;            /*!<  Event header. */
  uint8_t             *pData;         /*!<  advertising or scan response data. */
  uint8_t             len;            /*!<  length of advertising or scan response data. */
  int8_t              rssi;           /*!<  RSSI. */
  uint8_t             eventType;      /*!<  Advertising event type. */
  uint8_t             addrType;       /*!<  Address type. */
  bdAddr_t            addr;           /*!<  Device address. */

  /*  direct fields */
  bdAddr_t           directAddr;     /*!<  Direct advertising address. */
  uint8_t            directAddrType; /*!<  Direct advertising address type. */
} LlLeAdvReportEvt_t;

/*!  LE connection update complete event */
typedef struct{
  MdcMsgHdr_t       hdr;          /*!<  Event header. */
  uint8_t           status;       /*!<  Status. */
  uint16_t          handle;       /*!<  Connection handle. */
  uint16_t          connInterval; /*!<  Connection interval. */
  uint16_t          connLatency;  /*!<  Connection latency. */
  uint16_t          supTimeout;   /*!<  Supervision timeout. */
} LlLeConnUpdateCmplEvt_t;

/*!  LE read remote features complete event */
typedef struct{
  MdcMsgHdr_t       hdr;                    /*!<  Event header. */
  uint8_t           status;                 /*!<  Status. */
  uint16_t          handle;                 /*!<  Connection handle. */
  uint8_t           features[LL_FEAT_LEN];  /*!<  Remote features buffer. */
} LlLeReadRemoteFeatCmplEvt_t;

/*!  LE LTK request event */
typedef struct{
  MdcMsgHdr_t         hdr;                   /*!<  Event header. */
  uint16_t            handle;                /*!<  Connection handle. */
  uint8_t             randNum[LL_RAND_LEN];  /*!<  LTK random number. Rand[8]*/
  uint16_t            encDiversifier;        /*!<  LTK encryption diversifier.  EDIV*/
} LlLeLtkReqEvt_t;

/*!  LE remote connection parameter request event */
typedef struct{
  MdcMsgHdr_t         hdr;         /*!<  Event header. */
  uint16_t            handle;      /*!<  Connection handle. */
  uint16_t            intervalMin; /*!<  Interval minimum. */
  uint16_t            intervalMax; /*!<  Interval maximum. */
  uint16_t            latency;     /*!<  Connection latency. */
  uint16_t            timeout;     /*!<  Connection timeout. */
} LlLeRemConnParamReqEvt_t;

/*!  LE data length change event */
typedef struct{
  MdcMsgHdr_t        hdr;         /*!<  Event header. */
  uint16_t           handle;      /*!<  Connection handle. */
  uint16_t           maxTxOctets; /*!<  Maximum Tx octets. */
  uint16_t           maxTxTime;   /*!<  Maximum Tx time. */
  uint16_t           maxRxOctets; /*!<  Maximum Rx octets. */
  uint16_t           maxRxTime;   /*!<  Maximum Rx time. */
} LlLeDataLenChangeEvt_t;

/*!  LE Read local  P256 ECC key command complete event */
typedef struct{
  MdcMsgHdr_t        hdr;                      /*!<  Event header. */
  uint8_t            status;                   /*!<  Status. */
  uint8_t            pubKey_x[LL_ECC_KEY_LEN]; /*!< Public key x-coordinate. */
  uint8_t            pubKey_y[LL_ECC_KEY_LEN]; /*!< Public key y-coordinate. */
} LlLeP256CmplEvt_t;

/*!  LE generate DH key command complete event */
typedef struct{
  MdcMsgHdr_t        hdr;                 /*!<  Event header. */
  uint8_t            status;              /*!<  Status. */
  uint8_t            key[LL_ECC_KEY_LEN]; /*!<  Diffie-Hellman (Share Secret) key. */
} LlLeGenDhKeyEvt_t;

/*!  LE number of Completed packets.  */
typedef struct{
	MdcMsgHdr_t        hdr;               /*!<  Event header. */
	uint8_t            numHandles;        /*!<  Number of handles. */
	uint16_t           connHandle;        /*!<  connection Handle. */
	uint16_t           numCmplPkt;        /*!<  Number of complete packet. */
} LlNumCmplPktEvt_t;

typedef struct{
	MdcMsgHdr_t        hdr;               /*!<  Event header. */
	uint16_t           handle;
}LlAuthPyldTimeoutExpiredEvt_t;

typedef struct{
	MdcMsgHdr_t        hdr;               /*!<  Event header. */
    uint16_t           handle;            /*!<  Connection handle. */
	uint8_t            chSelAlg;          /*!<  Channel Selection Algorithm. */
}LlLeChSelEvt_t;

typedef struct{
	MdcMsgHdr_t        hdr;               /*!<  Event header. */
	uint8_t            status;            /*!<  Status. */
	uint16_t           connHandle;        /*!<  Connection handle. */
	uint8_t            txPhy;             /*!<  Tx PHY. */
	uint8_t            rxPhy;             /*!<  Rx PHY. */
}LlLePhyUpdCmplEvt_t;

/*! LL message event. */
enum{
	LL_EVT_DISCONNECT_IND,            /*!< Disconnection complete. */
	LL_EVT_ENC_CHANGE_IND,            /*!< Encrypt changed. */
	LL_EVT_READ_REMOTE_VER_INFO_CNF,  /*!< Read remote version info complete. */
	LL_EVT_HW_ERR_IND,                /*!< Hardware error, radio error. */
	LL_EVT_ENC_KEY_REFRESH_IND,       /*!< Encrypt key refresh complete. */
	LL_EVT_NUM_OF_CMPL_PKTS,
	LL_EVT_AUTH_TIMEOUT_EXPIRED_IND,  /*!< Authenticated Payload Timeout Expired indicator. */

	LL_EVT_CONN_IND,                  /*!< Connection complete. */
	LL_EVT_ADV_REPORT_IND,            /*!< Received adv report. */
	LL_EVT_CONN_UPD_IND,              /*!< Connection Update complete. */
	LL_EVT_READ_REMOTE_FEAT_CNF,      /*!< Read remote features complete. */
	LL_EVT_LTK_REQ_IND,               /*!< LTK request. */
	LL_EVT_REM_CONN_PARAM_IND,        /*!< Remote connection parameter change. */
	LL_EVT_DATA_LEN_CHANGE_IND,       /*!< Data length changed. */
	LL_EVT_READ_LOCAL_P256_CMPL_IND,  /*!< Read local P-256 public key complete. */
	LL_EVT_GENERATE_DHKEY_CMPL_IND,   /*!< Generate Diffie-Hellman key complete. */
	LL_EVT_CHN_SEL_IND,               /*!< Channel Selectiong algorithm. */
	LL_EVT_PHY_UPD_IND,               /*!< Phy update complete indicator. */

	LL_EVT_MAX,
};

/*!   Union for all event types */
typedef union{
	MdcMsgHdr_t                   hdr;                    /* Common event header */

	LlDisconnectCmplEvt_t         disconnectCmpl;
	LlEncChangeEvt_t              encChange;
	LlReadRemoteVerInfoCmplEvt_t  readRemoteVerInfoCmpl;
	LlHwErrorEvt_t                hwError;
	LlEncKeyRefreshCmpl_t         encKeyRefreshCmpl;
	LlNumCmplPktEvt_t             numCmplPkts;
	LlAuthPyldTimeoutExpiredEvt_t authTimeout;

	/* HCI LE event */
	LlLeConnCmplEvt_t             connCmpl;               /*!<  LE connection complete/enchance connection complete. */
	LlLeAdvReportEvt_t            advReport;              /*!<  LE advertising/direct advertising report. */
	LlLeConnUpdateCmplEvt_t       connUpdateCmpl;
	LlLeReadRemoteFeatCmplEvt_t   readRemoteFeatCmpl;
	LlLeLtkReqEvt_t               ltkReq;
	LlLeRemConnParamReqEvt_t      remConnParamReq;
	LlLeDataLenChangeEvt_t        dataLenChange;
	LlLeP256CmplEvt_t             p256Cmpl;
	LlLeGenDhKeyEvt_t             genDhKeyCmpl;
	LlLeChSelEvt_t                chSelInd;
	LlLePhyUpdCmplEvt_t           phyUpdCmpl;
}LlEvt_t;


/*! Link layer event call-back */
typedef void (*LlEvtCback_t)(LlEvt_t *pEvt);
typedef void (*LlAclCback_t)(uint16_t connHandle, uint8_t pktBound, uint8_t *pPacket, uint32_t len);
typedef void (*LlTxFifoAvailCback_t)(void);

/**
 * @brief:  LL initialization.
 * @param:  none.
 * @return: none.
 *          !!!Note:This function can only be called once during initialization.
 */
void LL_Init(const LlCfg_t *pLlCfg);

/**
 * @brief:  Slave initialization.
 * @param:  none.
 * @return: none.
 *          !!!Note:This function can only be called once during initialization.
 */
void LL_MstInit(void);

/**
 * @brief:  Slave initialization.
 * @param:  none.
 * @return: none.
 *          !!!Note:This function can only be called once during initialization.
 */
void LL_SlvInit(void);

/**
 * @brief:  LL Task.
 * @param:  none.
 * @return: none.
 */
void LL_TaskStart(void);

/**
 * @brief:  BLE IRQ handler.
 * @param:  none.
 * @return: none.
 */
void BLE_IRQHandler(void);

/**
 * @brief:  Get the number of available Tx buffers in LL.
 * @param:  none.
 * @return: none.
 */
uint32_t LL_GetFreeTxBufNum(uint16_t connHandle);

/**
 * @brief:  Enable/Disable Rx time
 * @param:  en   enable rx time.
 * @return: none.
 */
void LL_ConnEnableRxTime(uint8_t en);

/**
 * @brief :  Send Acl by RF
 * @param :  connHandle        connection handle.
 * @param :  llid              LL LLID
 * @param :  pPayload          Pointer point to payload.
 * @param :  len               the length of payload.
 * @return: none.
 */
LlState_t LL_SendAcl(uint16_t connHandle, uint8_t llid, uint8_t *pPayload, uint32_t len);

void LL_EvtRegister(LlEvtCback_t evtCback);
void LL_AclRegister(LlAclCback_t aclCback);
void LL_TxFifoAvailCbackRegister(LlTxFifoAvailCback_t func);
void LL_SendEvt(LlEvt_t *pEvt);

void LL_Reset(void);

uint16_t LL_GetAclLen(void);
void LL_GetVersion(uint8_t *pVer, uint16_t *pCompId, uint16_t *pSubVer);

LlState_t LL_ReadRssi(uint16_t connHandle, int8_t *pRSSI);
LlState_t LL_Disconnect(uint16_t connHandle, uint8_t reason);
LlState_t LL_ReadRemoteVerInfo(uint16_t connHandle);
LlState_t LL_SetBdAddr(uint8_t *pAddr, uint32_t len);
uint64_t  LL_GetBdAddr(void);

LlState_t LL_GetAuthPayloadTimeout(uint16_t connHandle, uint16_t *timeout);
LlState_t LL_SetAuthPayloadTimeout(uint16_t connHandle, uint16_t timeout);

LlState_t LL_LE_SetRandomAddr(uint8_t *pAddr, uint32_t len);

// Adv
LlState_t LL_LE_SetAdvParam(uint16_t intervalMin, uint16_t intervalMax, uint8_t advType, uint8_t ownAddrType,
		                    uint8_t peerAddrType, uint8_t *pPeerAddr, uint8_t advChnMap, uint8_t advFilter);
LlState_t LL_LE_SetAdvData(uint8_t *pData, uint32_t len);
LlState_t LL_LE_SetScanRspData(uint8_t *pData, uint32_t len);
LlState_t LL_LE_SetAdvEnable(bool_t enable);

// Scan
LlState_t LL_LE_SetScanParam(uint8_t scanType, uint16_t scanInterv, uint16_t scanWin, uint8_t ownAddrType, uint8_t scanFilt);
LlState_t LL_LE_SetScanEnable(bool_t enable, bool_t filtDupEnable);

// Connection Create
LlState_t LL_LE_CreateConn(LlInitParam_t *pInitParam,LlConnSpec_t *pConnSpec);
LlState_t LL_LE_CreateConnCancel(void);

// White-List
uint8_t   LL_LE_GetWhiteListSize(void);
LlState_t LL_LE_ClearWhiteList(void);
LlState_t LL_LE_AddDeviceToWhiteList(uint8_t addrType, uint8_t *pAddr);
LlState_t LL_LE_RemoveDeviceFromWhiteList(uint8_t addrType, uint8_t *pAddr);

LlState_t LL_LE_ConnUpd(uint16_t connHandle, LlConnSpec_t *pConnSpec);
LlState_t LL_LE_SetHostChnClass(uint8_t *pChnMap);
LlState_t LL_LE_ReadChnMap(uint16_t connHandle, uint8_t *pChnMap);
LlState_t LL_LE_ReadRemoteFeat(uint16_t connHandle);

// LE Encryption
LlState_t LL_LE_Encrypt(uint8_t *pOut, uint8_t *pIn, uint8_t *pKey);
LlState_t LL_LE_StartEncrypt(uint16_t connHandle, uint8_t *pRand, uint16_t ediv, uint8_t *pLtk);

LlState_t LL_LE_LtkReqReplyHandler(uint16_t connHandle, uint8_t *pLtk);
LlState_t LL_LE_LtkReqNegativeReplyHandler(uint16_t connHandle);

// Date Length Exchange(DLE)
LlState_t LL_LE_SetDataLen(uint16_t connHandle, uint16_t txOctets, uint16_t txTime);
LlState_t LL_LE_GetDefDataLen(uint16_t *pTxOctets, uint16_t *pTxTime);
LlState_t LL_LE_SetDefDataLen(uint16_t txOctets, uint16_t txTime);
LlState_t LL_LE_ReadMaximumDataLen(uint16_t *pMaxTxLen, uint16_t *pMaxTxTime, uint16_t *pMaxRxLen, uint16_t *pMaxRxTime);

// LL Privacy
LlState_t LL_LE_AddDeviceToResolveList(uint8_t peerIdAddrType, uint8_t peerIdAddr[6], uint8_t peerIrk[16], uint8_t localIrk[16]);
LlState_t LL_LE_RemoveDeviceFromResolveList(uint8_t peerIdAddrType, uint8_t peerIdAddr[6]);
LlState_t LL_LE_CLearResolveList(void);
uint8_t   LL_LE_GetResolveListSize(void);
LlState_t LL_LE_GetPeerResolvableAddr(uint8_t *pPeerRpa, uint8_t peerIdAddrType, uint8_t peerIdAddr[6]);
LlState_t LL_LE_GetLocalResolvableAddr(uint8_t *pLocalRpa, uint8_t peerIdAddrType, uint8_t peerIdAddr[6]);
LlState_t LL_LE_SetAddrResEnable(bool_t enable);
LlState_t LL_LE_SetAddrResTimeout(uint16_t timeout);
LlState_t LL_LE_SetPrivacyMode(uint8_t peerIdAddrType, uint8_t peerIdAddr[6], uint8_t privacyMode);

// PHY
bool_t    LL_ValidateSupportedPhy(uint8_t txPhys, uint8_t rxPhys);
LlState_t LL_LE_ReadPhy(uint16_t handle, uint8_t *pTxPhy, uint8_t *pRxPhy);
LlState_t LL_LE_SetDefaultPhy(uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys);
LlState_t LL_LE_SetPhy(uint16_t handle, uint8_t allPhys, uint8_t txPhys, uint8_t rxPhys, uint16_t phyOptions);

LlState_t LL_LE_SetLocalMinUsedChan(uint8_t phys, int8_t pwrThres, uint8_t minUsedCh);

// Extended Adv
LlState_t LL_LE_SetAdvSetRandomAddr(uint8_t advHandle, uint8_t rndAddr[6]);
LlState_t LL_LE_SetExtAdvParam(uint8_t advHandle, LlExtAdvParam_t *pExtAdvParam);
LlState_t LL_LE_SetExtAdvData(uint8_t advHandle, uint8_t operation, uint8_t fragPref, uint8_t *pData, uint32_t len);
LlState_t LL_LE_SetExtAdvScanRspData(uint8_t advHandle, uint8_t operation, uint8_t fragPref, uint8_t *pData, uint32_t len);
LlState_t LL_LE_SetExtAdvEnable(uint8_t enable, uint8_t advHandle, uint16_t duration, uint8_t maxExtAdvEvt);
uint16_t  LL_LE_ReadMaxAdvDataLen(void);
uint8_t   LL_LE_ReadNumOfSupportedAdvSets(void);
LlState_t LL_LE_RemoveAdvSet(uint8_t advHandle);
LlState_t LL_LE_ClearAdvSets(void);

// LL PM
typedef int (*LL_PmSleepStart_t)(uint8_t sleepMode, uint32_t sleepTick);
void LL_PmInit(LL_PmSleepStart_t sleepFunc, uint8_t en);
LL_PmSleepStart_t LL_PmSleepStart(void);
uint32_t LL_PmGetIdleTime(void);
void LL_PmPostHandler(void);

// LL Misc
bool_t LL_IsRfBusy(void);
void LL_ReqSetNewTxPower(int8_t txPower);
void LL_SetLocalSCA(uint16_t localSCA);

#endif /* LL_API_H_ */
