/**
 *******************************************************************************
 * @FileName  : assert.h
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
#ifndef ASSERT_H
#define ASSERT_H

#include "track.h"

/*! Assert enable/disable */
#define ASSERT_EN                     1

/**
 * Error Code define.
 *
 * Error code field define:
 *   bit: 0 -15   error parameter
 *   bit: 16-23   error Id
 *   bit: 24-31   error type
 */

/*** FIFO ***/
#define FIFO_ERR_INIT_FAILED             0x01010000

/*** BB ***/
#define BB_ERR_PWR_MODE                  0x02810000
#define BB_ERR_RNG                       0x02820000
#define BB_ERR_AES_ALLOC_BUF_FAILED      0x02830000

/*** Link ***/
#define LL_ERR_TX_BUF_ALLOC_FAILED        0x02010000
#define LL_ERR_RX_BUF_ALLOC_FAILED        0x02020000
#define LL_ERR_CONN_CTX_BUF_ALLOC_FAILED  0x02030000
#define LL_ERR_MAX_CONN_NUM               0x02040000
#define LL_ERR_CFG_PARAM_NULL             0x02050000
#define LL_ERR_CONN_CTX_NULL              0x02060000
#define LL_ERR_SCH_TBL_BUF_ALLOC_FAILED   0x02070000
#define LL_ERR_WL_ALLOC_FAILED            0x02080000
#define LL_ERR_RL_ALLOC_FAILED            0x02090000
#define LL_ERR_LL_CONFIG_NULL             0x020A0000
#define LL_ERR_CONN_CTX_TBL_NULL          0x020B0000
#define LL_ERR_SCAN_BUF_ALLOC_FAILED      0x020C0000
#define LL_ERR_EXT_ADV_SET_MEM_ALLOC_FAILED  0x020D0000
#define LL_ERR_ADV_FIFO_ALLOC_FAILED       0x020E0000
#define LL_ERR_LL_CONFIG_PARAM_INVALID    0x020F0000
#define LL_ERR_INIT_BUF_ALLOC_FAILED      0x02100000
#define LL_ERR_KEY_BUF_ALLOC_FAILED       0x02110000

/*** L2CAP ***/
#define L2CAP_ERR_BUF_ALLOC_FAILED         0x03010000
#define L2CAP_ERR_CONN_CB_MEM_ALLOC_FAILED 0x03020000

/*** ATT ***/
#define ATT_ERR_BUF_ALLOC_FAILED          0x04010000
#define ATT_ERR_CONN_ID                   0x04020000
#define ATT_ERR_CONN_CB_MEM_ALLOC_FAILED  0x04030000

/*** SMP ***/
#define SMP_ERR_BUF_ALLOC_FAILED          0x05010000
#define SMP_ERR_RECORD_CREATE_FAILED      0x05020000
#define SMP_ERR_CONN_ID                   0x05030000
#define SMP_ERR_CONN_CB_MEM_ALLOC_FAILED  0x05040000

/*** Device Manager(DM) ***/
#define DM_ERR_CONN_ID                    0x06010000
#define DM_ERR_CONN_CB_MEM_ALLOC_FAILED   0x06020000

/*** Utils ***/
#define UTILS_ERR                         0x07000000
#define UTILS_ERR_POOL_BUF_ALLOC_FAILED   0x07010000
#define UTILS_ERR_QUEUE_NULL              0x07020000
#define UTILS_ERR_QUEUE_ELEM_NULL         0x07030000
#define UTILS_ERR_NO_MEM_FOR_POOL         0x07040000
#define UTILS_ERR_INVALID_PARAM           0x07050000
#define UTILS_ERR_RINGBUF_INIT_FAILED     0x07060000
#define UTILS_ERR_POOL_BUF_FREE_FAILED    0x07070000
#define UTILS_ERR_STIMER_IS_NULL          0x07080000
#define UTILS_ERR_MSG_IS_NULL             0x07090000
#define UTILS_ERR_HEAP_IS_NULL            0x070A0000
#define UTILS_ERR_MDC_HANDLER_FULL        0x070B0000

/*** LHCI ***/
#define LHCI_ERR_CMD_BUF_ALLOC_FAILED     0x08010000
#define LHCI_ERR_ACL_BUF_ALLOC_FAILED     0x08020000
#define LHCI_ERR_TX_BUF_ALLOC_FAILED      0x08030000
#define LHCI_ERR_HCI_CONFIG_NULL          0x08040000
#define LHCI_ERR_LL_CONFIG_NULL           0x08050000
#define LHCI_ERR_BUF_FREE_FAILED          0x08060000

/*** Schedule ****/
#define SCH_ERR_SCH_NONE                  0x09010000
#define SCH_ERR_LL_CFG_IS_NULL            0x09020000

/*** HCI Transport ***/
#define HCI_TR_ERR_RX_BUF_ALLOC_FAILED    0x0A010000
#define HCI_TR_ERR_BKUP_BUF_ALLOC_FAILED  0x0A020000
#define HCI_TR_ERR_SLIP_BUF_ALLOC_FAILED  0x0A030000
#define HCI_TR_ERR_H5_TX_BUF_ALLOC_FAILED 0x0A040000

/*** Host HCI ***/
#define HCI_ERR_ACL_BUF_ALLOC_FAILED      0x0A810000
#define HCI_ERR_CONN_CB_MEM_ALLOC_FAILED  0x0A820000

/*** USB ***/
#define USB_ERR_DBG_FIFO_ALLOC_FAILED     0x0B010000

/*** SDP ***/
#define SDP_ERR_ALLOC_ATT_BUF_FAILED      0x0C010000

/*** ANT ***/
#define ANT_ERR_ALLOC_CTX_BUF_FAILED      0x0D010000
#define ANT_ERR_CTX_IS_NULL               0x0D020000

/*** Driver ***/
#define DRIV_ERR                          0x0F000000
#define DRIV_ERR_INVALID_PARAM            0x0F010000


#if ASSERT_EN
	#define ASSERT(exp, errCode) assert_failed(exp, errCode, __FILE__, __LINE__);
#else
	#define ASSERT(exp, errCode)
#endif

//0xFFFF1111  0xFFFF2222 0xFFFF3333 0xFFFF4444 0xFFFF5555 0xFFFF9999 0xFFFFBBBB

#define bsp_assert(exp)

#endif


