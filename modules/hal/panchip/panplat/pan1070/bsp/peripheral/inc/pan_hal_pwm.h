/**************************************************************************//**
* @file     pan_hal_pwm.h
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series PWM (Pulse Width Modulation) HAL header file.
* @note     Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef __PAN_HAL_PWM_H
#define __PAN_HAL_PWM_H
#include "pan_hal_def.h"

/**
 * @brief PWM HAL Interface
 * @defgroup pwm_hal_interface Pwm HAL Interface
 * @{
 */

/**
 * @brief Enumeration of PWM channel identifiers.
 *
 * These values represent different PWM channels available.
 */
typedef enum
{
	PWM0_CH0,
	PWM0_CH1,
	PWM0_CH2,
	PWM0_CH3,
	PWM0_CH4,
	PWM0_CH5,
	PWM0_CH6,
	PWM0_CH7,
} PWM_ChId;

/**
 * @brief Enumeration of PWM operating modes.
 *
 * These values define different PWM operating modes.
 */
typedef enum
{
    PWM_MODE_INDEPENDENT   = 0x00,  /*!< Independent mode */
    PWM_MODE_COMPLEMENTARY = 0x01,  /*!< Complementary mode */
    PWM_MODE_SYNCHRONIZED  = 0x10,  /*!< Synchronized mode */
    PWM_MODE_GROUP = 0X100          /*!< Group mode */
} PWM_ModeOpt;

/**
 * @brief Structure defining PWM initialization parameters.
 *
 * This structure holds the parameters required to initialize a PWM channel.
 */
typedef struct
{
    uint32_t frequency;        		/*!< PWM frequency in Hz */
    uint32_t dutyCycle;        		/*!< Initial duty cycle value */
    PWM_OperateTypeDef operateType; /*!< PWM operating type */
    PWM_ModeOpt mode;         		/*!< PWM operating mode */
    bool inverter;             		/*!< Inverter control */
	bool lowPowerEn;   				/*!< PWM clk source select */
} PWM_InitOpt;



/**
 * @brief it is a macro maping gpio to pwm
 * @param port  @example P0
 * @param bit   @example 3
 * @param chId  {PWM_CH0, PWM_CH1, PWM_CH2, PWM_CH3, PWM_CH4, PWM_CH5, PWM_CH6, PWM_CH7}
 * @example HAL_PWM_PinConfiguration(P0, 3, PWM_CH2), means configurating P03 as PWM0 Channel 2
 */
#define HAL_PWM_PinConfiguration(port, bit, chId)   \
            (SYS->port##_MFP = (SYS->port##_MFP & ~SYS_MFP_##port##bit##_Msk) | SYS_MFP_##port##bit##_##chId)

/**
 * @brief Initialize a PWM channel with the specified configuration.
 *
 * @param ChID     PWM channel ID to initialize.
 * @param InitObj  Configuration parameters for PWM initialization.
 *
 * This function initializes a PWM channel with the provided configuration.
 */
int HAL_PWM_Init(PWM_ChId chId, PWM_InitOpt *pInitObj);

/**
 * @brief DeInitialize a PWM channel.
 *
 * @param ChID   PWM channel ID to deinitialize.
 */
void HAL_PWM_DeInit(uint32_t handle);

/**
 * @brief Start a PWM channel.
 *
 * @param ChID   PWM channel ID to start.
 *
 * This function enables the output of the specified PWM channel and starts its operation.
 */
void HAL_PWM_Start(uint32_t handle);

/**
 * @brief Stop a PWM channel.
 *
 * @param ChID   PWM channel ID to stop.
 * @param force  Flag to force stop the channel.
 *
 * This function stops the specified PWM channel's operation. If 'force' is true, it forcefully stops the channel.
 * Otherwise, it stops the channel gracefully.
 */
void HAL_PWM_Stop(uint32_t handle, bool force);

/**
 * @brief Stop a PWM channel.
 *
 * @param ChID   PWM channel ID to stop.
 * @param force  Flag to force stop the channel.
 *
 * This function stops the specified PWM channel's operation. If 'force' is true, it forcefully stops the channel.
 * Otherwise, it stops the channel gracefully.
 */
void HAL_PWM_SetFreqAndDuty(uint32_t handle, uint32_t frequency, uint32_t dutyCycle);

/** @} */ // end of group
#endif /* __PAN_HAL_PWM_H */

