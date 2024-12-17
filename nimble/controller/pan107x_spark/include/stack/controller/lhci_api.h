/**
 *******************************************************************************
 * @FileName  : lhci_api.h
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

#ifndef LHCI_API_H_
#define LHCI_API_H_

#include "stack/ble_types.h"
#include "utils/fifo.h"

//#define LHCI_CMD_PLD_MAX     65
//#define LHCI_EVT_PLD_MAX     65

#define LHCI_BUF_SIZE_ALIGN4(payload)       MEM_ALIGNED_SIZE(HCI_ACL_HDR_LEN + payload, 4)

/*! LHCI Tx call-back */
typedef void (*LHci_TxHciPktCback_t)(uint8_t *pBuf, uint32_t len);

/*! LHCI Config type. */
typedef struct{
	uint16_t  maxAclLen;  /*!< max acl parameter length. */
	uint16_t  maxCmdLen;  /*!< max cmd parameter length. */
	uint16_t  maxEvtLen;  /*!< max evt parameter length. */

	uint8_t   numCmdBufs; /*!< max cmd buffer number. */
	uint8_t   numAclBufs; /*!< max acl buffer number. */
	uint8_t   numEvtBufs; /*!< max evt buffer number. */
}LhciConfig_t;

/*! LHCI config control block. */
extern const LhciConfig_t *pLhciConfig;

/**
 * @brief  : LHCI initialization.
 * @param  : pLhciCfg      Pointer point to LhciConfig_t struct;
 * @return : none.
 */
void LHCI_Init(const LhciConfig_t *pLhciCfg);

/**
 * @brief  : Notify LHCI to Handle Received HCI Packet.
 * @param  : none.
 * @return : none.
 */
void LHCI_NotifyRxHciPkt(void);

/**
 * @brief  : Register ACL/EVT Tx cback.
 * @param  : aclCback
 * @param  : evtCback
 * @return : none.
 */
void LHCI_RegisterHciTxCback(LHci_TxHciPktCback_t aclCback, LHci_TxHciPktCback_t evtCback);

/**
 * @brief  : LHCI Rx handler.
 * @param  : none.
 * @return : none.
 */
void LHCI_RxHandler(void);

/**
 * @brief  : Get HCI RX Cmd FIFO
 * @param  : none.
 * @return : return pointer point to HCI RX CMD FIFO
 */
fifo_t *LHCI_GetRxCmdFifo(void);

/**
 * @brief  : Get HCI RX ACL FIFO
 * @param  : none.
 * @return : return pointer point to HCI RX ACL FIFO
 */
fifo_t *LHCI_GetRxAclFifo(void);

/**
 * @brief  : Get HCI TX ACL FIFO
 * @param  : none.
 * @return : return pointer point to HCI TX ACL FIFO
 */
fifo_t *LHCI_GetTxAclFifo(void);

/**
 * @brief  : Get HCI TX EVT FIFO
 * @param  : none.
 * @return : return pointer point to HCI TX ACL/EVT FIFO
 */
fifo_t *LHCI_GetTxFifo(void);

/**
 * @brief  : Get HCI TX low priority EVT FIFO
 * @param  : none.
 * @return : return pointer point to HCI TX EVT FIFO
 */
fifo_t *LHCI_GetLowPrioTxFifo(void);

#endif /* LHCI_API_H_ */
