/**
 *******************************************************************************
 * @FileName  : mdc.h  Message Distribution Center(MDC) implementation
 * @Author    : GaoQiu
 * @CreateDate: 2020-05-18
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *******************************************************************************
 */

#ifndef MDC_H_
#define MDC_H_

#include "defs_types.h"
#include "xqueue.h"

/*! Max message handler define. */
#define MDC_MAX_MSG_HANDLER   8


/*! MDC Message Type define. */
#define MDC_MSG_TYPE_QUEUE   0x01
#define MDC_MSG_TYPE_EVENT   0x02
#define MDC_MSG_TYPE_TIMER   0x04


/*! SMP event type. */
#define SMP_EVT_PAIR_TIMEOUT  0


/*! LL event type. */
#define LL_EVT_RSP_TIMEOUT    0
#define LL_EVT_TX_HCI_EVT     1
#define LL_EVT_TX_HCI_ACL     2


typedef uint16_t MdcEventMask_t;
typedef uint8_t  MdcMsgId_t;
typedef uint8_t  MdcHandlerId_t;

/*! MDC messgage header. */
typedef struct{
	uint8_t        event;
	uint8_t        status;
	uint16_t       param;
}MdcMsgHdr_t;


/*! Message Handler call-back type. */
typedef void (*MsgHandler_t)(MdcEventMask_t eventMask, MdcMsgHdr_t *msg);

/*! Semaphore Call-back for supporting OS */
typedef int (*MdcSemphrGiveCback_t)(void);

/**
 * @brief : Message Distribution Center(MDC) initialization.
 * @param : None.
 * @return: None.
 */
void MDC_Init(void);

/**
 * @brief : Register Semaphore for supporting OS.
 * @param : cback
 * @return: None.
 */
void MDC_RegisterSemphrCback(MdcSemphrGiveCback_t cback);

/**
 * @brief : Give Semaphore for supporting OS.
 * @param : None.
 * @return: None.
 */
void MDC_GiveSemphr(void);

/**
 * @brief : Send event to MDC.
 * @param : handlerId       handler id.
 * @param : event           event mask.
 * @return: None.
 */
void MDC_SendEvent(MdcHandlerId_t handlerId, MdcEventMask_t event);

/**
 * @brief : Send Timer ready event tp MDC.
 * @param : None.
 * @return: None.
 */
void MDC_TimerReady(void);

/**
 * @brief : Send Queue event to MDC.
 * @param : None.
 * @return: None.
 */
void MDC_SendQueueMsg(MdcHandlerId_t handlerId, void *pMsg);

/**
 * @brief : Get MDC's message queue.
 * @param : None.
 * @return: return MDC's message queue.
 */
queue_t *MDC_GetMsgQueue(void);

/**
 * @brief : Add message handler.
 * @param : MsgHandler_t   Pointer point to message handler.
 * @return: true if successfully or false.
 */
MdcHandlerId_t MDC_AddMsgHandler(MsgHandler_t handler);

/**
 * @brief : start MDC
 * @param : None
 * @return: None.
 */
void MDC_Start(void);

#endif /* MDC_H_ */
