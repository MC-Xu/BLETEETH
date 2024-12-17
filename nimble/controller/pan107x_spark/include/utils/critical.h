/**
 *******************************************************************************
 * @FileName  : critical.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-05-18
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

#ifndef CRITICAL_H_
#define CRITICAL_H_

#include "defs_types.h"

/**
 * @Name: critical section define.
 */
#define ENTER_CRITICAL(cpu_state)     cpu_state = EnterCritical()
#define EXIT_CRITICAL(cpu_state)      ExitCritical(cpu_state)


uint32_t EnterCritical(void);
void ExitCritical(uint32_t state);

#endif /* CRITICAL_H_ */
