/**************************************************************************//**
* @file     pan_hal_gpio.h
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series GPIO (General Purpose Input/Output) HAL header file.
* @note     Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __PAN_HAL_GPIO_H__
#define __PAN_HAL_GPIO_H__

#include "pan_hal_def.h"

/**
 * @brief GPIO HAL Interface
 * @defgroup gpio_hal_interface GPIO HAL Interface
 * @{
 */

#define GPIO_P0_PIN_NUM     8
#define GPIO_P1_PIN_NUM     8
#define GPIO_P2_PIN_NUM     8
#define GPIO_P3_PIN_NUM     2

#define GPIO_INTEN_MASK_BASE                0x00010001UL
#define GPIO_INTTYPE_MASK_BASE              0x00000001UL

/* PIN ID Definitions */
typedef enum
{
    P0_0 = 0x00,
    P0_1 = 0x01,
    P0_2 = 0x02,
    P0_3 = 0x03,
    P0_4 = 0x04,
    P0_5 = 0x05,
    P0_6 = 0x06,
    P0_7 = 0x07,
    P1_0 = 0x10,
    P1_1 = 0x11,
    P1_2 = 0x12,
    P1_3 = 0x13,
    P1_4 = 0x14,
    P1_5 = 0x15,
    P1_6 = 0x16,
    P1_7 = 0x17,
    P2_0 = 0x20,
    P2_1 = 0x21,
    P2_2 = 0x22,
    P2_3 = 0x23,
    P2_4 = 0x24,
    P2_5 = 0x25,
    P2_6 = 0x26,
    P2_7 = 0x27,
    P3_0 = 0x30,
    P3_1 = 0x31
} HAL_GPIO_PinId;

/**
  * @brief  MODE Constant Definitions
  */
typedef enum
{
    HAL_GPIO_MODE_INPUT_DIGITAL         = 0x00000,  /*!< Digital Input Mode */
    HAL_GPIO_MODE_INPUT_ANALOG          = 0x10000,  /*!< Analog Input Mode */
    HAL_GPIO_MODE_OUTPUT_PUSHPULL       = 0x00001,  /*!< Push-pull Output Mode */
    HAL_GPIO_MODE_OUTPUT_OPENDRAIN      = 0x00002,  /*!< Open-Drain Output Mode */
    HAL_GPIO_MODE_QUASI_BIDIRECTIONAL   = 0x00003   /*!< Quasi-bidirectional Mode */
} HAL_GPIO_Mode;

/**
  * @brief  MODE Constant Definitions for GPIO Pull Options.
  */
typedef enum
{
    HAL_GPIO_PULL_DISABLE,  /**< No pull-up or pull-down. */
    HAL_GPIO_PULL_UP,       /**< Enable pull-up. */
    HAL_GPIO_PULL_DOWN      /**< Enable pull-down. */
} HAL_GPIO_Pull;

/**
  * @brief  GPIO Bit SET and Bit RESET enumeration
  */
typedef enum
{
    HAL_GPIO_LEVEL_LOW   = 0,   /**< Pin is in reset state. */
    HAL_GPIO_LEVEL_HIGH  = 1    /**< Pin is in set. */
} HAL_GPIO_Level;

/**
  * @brief  GPIO Interrupt Type Constant Definitions
  */
typedef enum
{
    HAL_GPIO_INT_RISING         = 0x00010000UL, /*!< Interrupt enable by Input Rising Edge */
    HAL_GPIO_INT_FALLING        = 0x00000001UL, /*!< Interrupt enable by Input Falling Edge */
    HAL_GPIO_INT_BOTH_EDGE      = 0x00010001UL, /*!< Interrupt enable by both Rising Edge and Falling Edge */
    HAL_GPIO_INT_HIGH           = 0x01010000UL, /*!< Interrupt enable by Level-High */
    HAL_GPIO_INT_LOW            = 0x01000001UL, /*!< Interrupt enable by Level-Low */
    HAL_GPIO_INT_DISABLE        = 0x00000000UL  /*!< Interrupt disable */
} HAL_GPIO_IntMode;

/**
  * @brief   Typedef for GPIO callback function pointer.
  */
typedef void (*HAL_GPIO_CallbackFunc)(HAL_GPIO_IntMode intMode);

/**
  * @brief   Global array to store GPIO callback functions.
  *          Supports up to 48 GPIO pins.
  */
extern HAL_GPIO_CallbackFunc HAL_GPIO_CallbackArray[GPIO_P0_PIN_NUM + GPIO_P1_PIN_NUM + GPIO_P2_PIN_NUM + GPIO_P3_PIN_NUM];

/**
  * @brief  GPIO Init structure definition
  */
typedef struct
{
    HAL_GPIO_Mode mode;         /*!< Specifies the operating mode for the selected pin.
                                     This parameter can be a value of @ref HAL_GPIO_Mode */
    HAL_GPIO_Pull pull;         /*!< Specifies the Pull-up or Pull-Down activation for the selected pin.
                                     This parameter can be a value of @ref HAL_GPIO_Pull */
    HAL_GPIO_Level level;       /*!< Specifies the Initial IO output level for the selected pin.
                                     This parameter can be a value of @ref HAL_GPIO_Level */
} HAL_GPIO_InitTypeDef;

/**
  * @struct HAL_GPIO_IntInitTypeDef
  * @brief  GPIO interrupt configuration structure definition.
  */
typedef struct
{
    HAL_GPIO_IntMode intMode;
    HAL_GPIO_CallbackFunc callbackFunc;
    FunctionalState debounce;
} HAL_GPIO_IntInitTypeDef;

/**
  * @brief  Initializes a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to be initialized.
  * @param  initStruct: Pointer to a structure containing GPIO initialization options.
  * @retval None
  */
extern void HAL_GPIO_Init(HAL_GPIO_PinId pinID, HAL_GPIO_InitTypeDef *initStruct);

/**
  * @brief  Set output level to a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to write to.
  * @param  level: The IO level to write to the GPIO pin (HAL_GPIO_LEVEL_LOW or HAL_GPIO_LEVEL_HIGH).
  * @retval None
  */
extern void HAL_GPIO_WritePin(HAL_GPIO_PinId pinID, HAL_GPIO_Level level);

/**
  * @brief  Get output level of a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to read.
  * @retval HAL_GPIO_Level: The IO level of the GPIO pin (HAL_GPIO_LEVEL_LOW or HAL_GPIO_LEVEL_HIGH).
  */
extern HAL_GPIO_Level HAL_GPIO_ReadPin(HAL_GPIO_PinId pinID);

/**
  * @brief  Toggle output level of a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to write to.
  * @retval None
  */
extern void HAL_GPIO_TogglePin(HAL_GPIO_PinId pinID);

/**
  * @brief  Set internal pull-up or pull-down resistor path of a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to write to.
  * @param  pull: The Pull-up or Pull-Down activation for the selected pin.
  * @retval None
  */
extern void HAL_GPIO_SetPull(HAL_GPIO_PinId pinID, HAL_GPIO_Pull pull);

/**
  * @brief  Deinitializes a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to be deinitialized.
  */
extern void HAL_GPIO_DeInit(HAL_GPIO_PinId pinID);

/**
  * @brief  Initializes a GPIO pin for interrupt handling.
  * @param  pinID: Identifier of the GPIO pin to be initialized for interrupts.
  * @param  intInitStruct: Pointer to a structure containing GPIO interrupt configuration.
  * @retval None
  */
extern void HAL_GPIO_InterruptInit(HAL_GPIO_PinId pinID, HAL_GPIO_IntInitTypeDef *intInitStruct);

/**
  * @brief  Set interrupt mode for a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to write to.
  * @param  intMode: The interrupt mode for the selected pin.
  * @retval None
  */
extern void HAL_GPIO_SetIntMode(uint32_t pinID, HAL_GPIO_IntMode intMode);

/**
  * @brief  Set interrupt debounce control for a GPIO pin.
  * @param  pinID: Identifier of the GPIO pin to write to.
  * @param  debounce: ENABLE or DISABLE interrupt debounce function for the selected pin.
  * @retval None
  */
extern void HAL_GPIO_SetIntDebounce(uint32_t pinID, FunctionalState debounce);

/** @} */ // end of group
#endif // __PAN_HAL_GPIO_H__
