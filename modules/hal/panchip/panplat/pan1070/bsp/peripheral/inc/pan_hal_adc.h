/*********************************************************
 * @file     pan_hal_adc.h
 * @version  V0.0.0
 * $Revision: 1 $
 * $Date:    23/09/10 $
 * @brief    Panchip series ADC (Analog-to-Digital Converter) HAL (Hardware Abstraction Layer) header file.
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __PAN_HAL_ADC_H
#define __PAN_HAL_ADC_H

#include "pan_hal_def.h"

/**
 * @brief ADC HAL Interface
 * @defgroup adc_hal_interface Adc HAL Interface
 * @{
 */

/**
 * @brief ADC operation mode options.
 *
 * This enumeration defines the possible operation modes for the ADC.
 */
typedef enum
{
    ADC_MODE_CONV,       /*!< Regular ADC conversion mode. */
    ADC_MODE_EXTRIG,     /*!< External trigger mode for ADC. */
    ADC_MODE_COMPARE,    /*!< ADC comparison mode. */
    ADC_MODE_PWMSEQ,     /*!< ADC PWM sequencing mode. */
    ADC_MODE_SGLCONVSEQ, /*!< Single ADC conversion in a sequence. */
} ADC_ModeOpt;

/**
 * @brief ADC conversion mode options.
 *
 * This enumeration defines the possible conversion modes for the ADC.
 */
typedef enum
{
    ADC_CONV_MODE_BASE, /*!< Basic ADC conversion mode. */
    ADC_CONV_MODE_TEMP, /*!< Temperature sensor conversion mode. */
    ADC_CONV_MODE_VBG,  /*!< Voltage BandGap conversion mode. */
    ADC_CONV_MODE_VDD4, /*!< Quarter VDD conversion mode. */
} ADC_ConvertModeOpt;

/**
 * @brief ADC channel identifiers.
 *
 * This enumeration defines the identifiers for ADC channels.
 */
typedef enum
{
    ADC_CH0 = 0x0, /*!< ADC channel 0. */
    ADC_CH1 = 0x1, /*!< ADC channel 1. */
    ADC_CH2 = 0x2, /*!< ADC channel 2. */
    ADC_CH3 = 0x3, /*!< ADC channel 3. */
    ADC_CH4 = 0x4, /*!< ADC channel 4. */
    ADC_CH5 = 0x5, /*!< ADC channel 5. */
    ADC_CH6 = 0x6, /*!< ADC channel 6. */
    ADC_CH7 = 0x7, /*!< ADC channel 7. */

    ADC_CH_TEMP = 0x9,    /*!< Temperature sensor channel. Note: Duplicates the channel number of ADC_CH9. */
    ADC_CH_BATTERY = 0xA, /*!< Quarter VDD channel. Note: it is used to get battery voltage by inner via. */
	ADC_IDLE = 0xff,
} ADC_ChId;


/**
 * @brief ADC input range options.
 *
 * This enumeration defines the input voltage range options available for ADC
 * conversion.
 */
typedef enum
{
    ADC_INPUT_RANGE_LOW = 0UL, /*!< ADC input range 0.4V~1.4V. */
    ADC_INPUT_RANGE_HIGH = 1UL /*!< ADC input range 0.4V~2.4V. */
} ADC_InputRangeOpt;


/**
 * @brief ADC initialization options structure.
 *
 * This structure defines the parameters required for the initialization of
 * the ADC. It includes configuration options like sample clock, input range,
 * mode of operation, and hardware trigger source.
 */
typedef struct
{
    ADC_InputRangeOpt InputRange;   /*!< Defines the input voltage range for ADC conversion. */
    ADC_ModeOpt Mode;               /*!< Defines the ADC mode of operation. */
} ADC_InitOpt;


/**
 * @brief ADC Handle Structure Definition
 *
 * This structure defines the configuration and management parameters for operating ADC.
 */

typedef struct {
	ADC_InitOpt initOpt;
	ADC_ChId id;
} ADC_HandleTypeDef;

/**
 * @brief Initialize the ADC for a specific channel with the given configuration.
 *
 * @param ChID     The ADC channel ID to initialize.
 * @param InitObj  ADC initialization options.
 */
void HAL_ADC_Init(ADC_HandleTypeDef *pADC);

/**
 * @brief DeInitialize the ADC.
 *
 * @note To be implemented: Handle deinitialization if necessary.
 *
 * @note This function powers down the ADC.
 */
void HAL_ADC_DeInit(ADC_HandleTypeDef *pADC);

/**
 * @brief Start DMA-based ADC data transfer to a memory buffer.
 *
 * @param Buf       Pointer to the destination memory buffer.
 * @param Size      The size of the data transfer.
 * @param Callback  Callback function to be executed upon DMA completion.
 *
 * @return The DMA channel number used for the transfer.
 *
 * @note This function initializes and starts DMA transfer for ADC data (Ref: DMA_Init).
 */
int32_t HAL_ADC_GetValue(ADC_HandleTypeDef *pADC, void *value);


/** @} */ // end of group
#endif    /* __PAN_HAL_ADC_H */
