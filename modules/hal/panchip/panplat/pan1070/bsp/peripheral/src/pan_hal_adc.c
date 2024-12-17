/**
 * @file     pan_hal_adc.c
 * @version  V0.0.0
 * $Revision: 1 $
 * $Date:    23/09/10 $
 * @brief    Panchip series ADC (Analog-to-Digital Converter) HAL (Hardware Abstraction Layer) source file.
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#include "pan_hal.h"
#include "freertos.h"
#include "semphr.h"

SemaphoreHandle_t xSemaphore = NULL;

void HAL_ADC_Init(ADC_HandleTypeDef *pADC)
{
	if (xSemaphore == NULL) {
		xSemaphore = xSemaphoreCreateMutex();
	} else {
		return;
	}
	
	/* Setting input range (0 ~ 1.2V) as default 
	 *  Note: if the input voltage over 1.2V, should
	 *  adopt r1/r1+r2 mothod
	 */
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, ENABLE);
	ADC_Init(ADC, true);
	ADC_SelInputRange(ADC,ADC_INPUTRANGE_LOW);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, DISABLE);
}


void HAL_ADC_DeInit(ADC_HandleTypeDef *pADC)
{
    pADC->id = ADC_IDLE;
}

int32_t HAL_ADC_GetValue(ADC_HandleTypeDef *pADC, void *value)
{
	uint32_t ch_bit = BIT(pADC->id);
	uint32_t result_code;
	
	if ((pADC->id > ADC_CH_BATTERY) || ((pADC->id > ADC_CH7) && (pADC->id < ADC_CH_TEMP))) {
		return PAN_HAL_IO_ERROR;
	}
	
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	
	/* Open clock related adc */
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, ENABLE);
	
	/* Normal external adc channel sample */
	if (pADC->id <= ADC_CH7) {
		
		uint16_t temp_buf[32];
		
		ADC_Open(ADC,ch_bit);
		/* Prepare ADC vbg calib paramter because currnet soc battery volatage has changed in a lot of application*/
		ADC_PrepareVbgCalibData(ADC);
		ADC_PowerOn(ADC);
		
		/* Delay a while to wait adc stable */
		if (m_adc_opt.chip_info >= 0x20) {
			SYS_delay_10nop(500);   // Delay 100us+
		} else {
			SYS_delay_10nop(50000); // Delay 15ms+
		}
		
		/* It can be optimized by user */
		result_code = ADC_SamplingCodeTrimMean(ADC, temp_buf, 32, 8);
		
		/* Convert ADC sampling code to voltage */
		*(float *)value = ADC_OutputVoltage(ADC, result_code);
		
		ADC_PowerDown(ADC);

	} else if (pADC->id == ADC_CH_BATTERY) {
		*(uint16_t *)value = ADC_MeasureSocVbat(ADC);
	} else if (pADC->id == ADC_CH_TEMP) {
		*(float *)value = ADC_MeasureSocTemperature(ADC);
	}
	
	/* Open clock related adc */
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, DISABLE);
	xSemaphoreGive(xSemaphore);
	
	return PAN_HAL_SUCCESS;
}


