/**
 *******************************************************************************
 * @FileName  : ant_api.h
 * @Author    : GaoQiu
 * @CreateDate: 2023-12-23
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

#ifndef ANT_API_H_
#define ANT_API_H_

#include "utils/defs_types.h"

typedef void (*AntStartCback_t)(void);

typedef struct{
	uint32_t           interval;       /* unit: tick (32us) */
	uint32_t           slot_duration;  /* scan window  unit: 1.25ms */
	uint32_t           before_point;   /* unit: tick (32us) */
	uint32_t           anchor_point;
	void               *event;         /* ab_event_come_t    event;*/
	uint32_t           reserve;
	uint8_t            priority;
	uint8_t            dy_scan;

	AntStartCback_t    AntStartCback;
	AntStartCback_t    AntStopCback;
}ab_event_node_t;

/**
 * @brief   ANT Module Initialization
 * @param
 * @return
 */
void ANT_Init(void);

/**
 * @brief   ANT task create.
 * @param   pParam   - Pointer point to ant configuration parameter
 * @return  return ANT instant id
 */
int ANT_Create(ab_event_node_t *pParam);

/**
 * @brief   ANT task delete.
 * @param   id   - ANT instant id for delete
 * @return
 */
int ANT_Delete(int id);


#endif /* ANT_API_H_ */
