/**
 *******************************************************************************
 * @FileName  : stimer.h
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
#ifndef STIMER_H_
#define STIMER_H_

#include "mdc.h"

#ifndef MDC_MS_PER_TICK
	#define MDC_MS_PER_TICK  1
#endif

/*! Timer Tick data type */
typedef uint32_t MdcTimerTicks_t;

/*! Timer type */
typedef struct MdcTimer_s{
	struct MdcTimer_s  *pNext;

	bool_t              isStarted;    /*!< it is maintained by the Timer system itself, users are not allowed to operate directly. */

	MdcMsgHdr_t         msg;
	MdcTimerTicks_t     ticks;
	MdcHandlerId_t      handlerId;
}MdcTimer_t;

void MDC_TimerInit(void);
void MDC_TimerConfig(MdcTimer_t *pTimer, MdcHandlerId_t handlerId, MdcMsgHdr_t *msg);
void MDC_TimerConfigEx(MdcTimer_t *pTimer, MdcHandlerId_t handlerId, uint8_t event, uint8_t status, uint16_t userData);

void MDC_TimerStartSec(MdcTimer_t *pTimer, MdcTimerTicks_t sec);
void MDC_TimerStartMs(MdcTimer_t *pTimer, MdcTimerTicks_t ms);

void MDC_TimerStop(MdcTimer_t *pTimer);

void MDC_TimerSleepUpdate(void);
MdcTimer_t *MDC_TimerExpired(void);

uint32_t MDC_TimerIdleTime(void);

#endif /* STIMER_H_ */
