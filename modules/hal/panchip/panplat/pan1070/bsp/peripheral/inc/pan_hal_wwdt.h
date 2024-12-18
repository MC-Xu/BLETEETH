/**************************************************************************//**
* @file     pan_hal_wwdt.h
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series WWDT (Window Watchdog Timer) HAL (Hardware Abstraction Layer) header file.
*
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __PAN_HAL_WWDT_H
#define __PAN_HAL_WWDT_H

/**
 * @brief WWDT HAL Interface
 * @defgroup wwdt_hal_interface Wwdt HAL Interface
 * @{
 */

#include "pan_hal_def.h"

/** 
 * @brief Type definition for WWDT callback function.
 */
typedef void (*WWDT_CallbackFunc)(void);

/** 
 * @brief WWDT initialization options structure.
 */
typedef struct 
{
    WWDT_PrescaleDef Prescaler;   /**< WWDT prescaler definition. */
    uint32_t  ClockSource;        /**< WWDT clock source. */
    uint32_t  CmpValue;           /**< WWDT comparison value. */
} WWDT_Init_Opt;

/**
 * @struct WWDT_Interrupt_Opt
 * @brief  WWDT interrupt configuration structure definition.
 */
typedef struct
{   
    WWDT_CallbackFunc CallbackFunc;
}WWDT_Interrupt_Opt;

/**
 * @brief Initialize the WWDT.
 * @param wwdt Pointer to WWDT_HandleTypeDef. Represents the WWDT instance to initialize.
 */
void HAL_WWDT_Init(WWDT_Init_Opt *wwdt);

/**
 * @brief DeInitialize the WWDT.
 * @param wwdt Pointer to WWDT_HandleTypeDef. Represents the WWDT instance to deinitialize.
 */
void HAL_WWDT_DeInit(void);

/**
 * @brief Feed the WWDT.
 * @param wwdt Pointer to WWDT_HandleTypeDef. Represents the WWDT instance to feed.
 */
void HAL_WWDT_Feed(WWDT_Init_Opt *wwdt);

/**
 * @brief Initialize WWDT interrupt.
 * @param wwdt Pointer to WWDT_HandleTypeDef. Represents the WWDT instance whose interrupt is to be initialized.
 */
void HAL_WWDT_Init_INT(WWDT_Interrupt_Opt *wwdt);

/**
 * @brief DeInitialize WWDT interrupt.
 * @param wwdt Pointer to WWDT_HandleTypeDef. Represents the WWDT instance whose interrupt is to be deinitialized.
 */
void HAL_WWDT_DeInit_INT(void);

/**
 * @brief Handler for WWDT interrupts.
 */
void WWDT_IRQHandler(void);

/** @} */ // end of group wwdt

#endif // __PAN_HAL_WWDT_H

