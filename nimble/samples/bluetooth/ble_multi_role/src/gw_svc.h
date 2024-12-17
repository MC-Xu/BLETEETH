/*
 ********************************************************************************
 * @FileName  : gw_svc.h
 * @CreateDate: 2021-2-19
 * @Author    : GaoQiu
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 ********************************************************************************
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
#ifndef GW_SVC_H_
#define GW_SVC_H_

#include "utils/defs_types.h"

extern uint16_t gwRxValHandle;
extern uint16_t gwTxValHandle;

int  ble_svc_gw_init(void);

#endif /* PROFILES_GW_SVC_H_ */
