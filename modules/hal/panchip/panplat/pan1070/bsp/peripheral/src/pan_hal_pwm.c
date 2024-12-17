
/**************************************************************************//**
* @file     pan_hal_pwm.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series PWM (Pulse Width Modulation) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#include "pan_hal.h"

#define PWM_MAX_WORKING_NUMBER      4

typedef struct {
	uint8_t id;
	uint8_t operateType;
} PWM_ConfDef;

static PWM_ConfDef deviceConfigures[PWM_MAX_WORKING_NUMBER] = {
	{0xff, 0xff},
	{0xff, 0xff},
	{0xff, 0xff},
	{0xff, 0xff},
};

static uint8_t SearchFreeHandle(void)
{
	for (uint8_t i = 0; i < PWM_MAX_WORKING_NUMBER; i++) {
		if (deviceConfigures[i].id == 0xff) {
			return i;
		}
	}
	return PWM_MAX_WORKING_NUMBER;
}

static uint32_t ClockBitGet(PWM_ChId chID)
{
	if (chID <= 1) {
		return 0x10;
	} else if (chID > 1 && chID <= 3) {
		return 0x20;
	} else if (chID > 3 && chID <= 5) {
		return 0x40;
	} else if (chID > 6 && chID <= 7) {
		return 0x80;
	}
	return 0;
}


int HAL_PWM_Init(PWM_ChId chId, PWM_InitOpt *pInitObj)
{
	uint32_t clockBit = ClockBitGet(chId);
	
	if (clockBit == 0) {
		return -PAN_HAL_INVALID_PARAMS;
	}
	
	uint8_t handle = SearchFreeHandle();
	
	if (handle == PWM_MAX_WORKING_NUMBER) {
		return -PAN_HAL_NO_HARDWARE_SOURCE;
	}
	/* store inforamtion to device configurations, and it will be used by handle */ 
	deviceConfigures[handle].id = chId;
	deviceConfigures[handle].operateType = pInitObj->operateType;
	
	/* If the low power pwm model is Enabled */
	if (pInitObj->lowPowerEn)
		CLK_SetPwmClkSrc(chId, (clockBit<<16));
	
	CLK_APB1PeriphClockCmd(clockBit | CLK_APB1Periph_PWM0_EN, ENABLE);
	
	PWM_ResetPrescaler(PWM, chId);
	
    PWM_ConfigOutputChannel(PWM, chId, pInitObj->frequency, pInitObj->dutyCycle, pInitObj->operateType);
	
	if(pInitObj->inverter == true){
        PWM_EnableOutputInverter(PWM, chId);        
    }
	
    switch (pInitObj->mode)
    {
		case PWM_MODE_INDEPENDENT:
			PWM_EnableIndependentMode(PWM);
			break;
		case PWM_MODE_COMPLEMENTARY:
			PWM_EnableComplementaryMode(PWM);
			break;
		case PWM_MODE_SYNCHRONIZED:
			PWM_EnableSyncMode(PWM);
			break;
		case PWM_MODE_GROUP:
			PWM_EnableGroupMode(PWM);
			break;
		default:
			break;
    }
	return handle;
}

void HAL_PWM_DeInit(uint32_t handle)
{
	if (handle >= PWM_MAX_WORKING_NUMBER) {
		assert_param(0);
	}
	HAL_PWM_Stop(handle, true);
	deviceConfigures[handle].id = 0xff;
	
	return;
}

void HAL_PWM_Start(uint32_t handle)
{
	if (handle >= PWM_MAX_WORKING_NUMBER) {
		assert_param(0);
	}
    PWM_EnableOutput(PWM, BIT(deviceConfigures[handle].id));
    PWM_Start(PWM, BIT(deviceConfigures[handle].id));
}

void HAL_PWM_Stop(uint32_t handle, bool force)
{
	if (handle >= PWM_MAX_WORKING_NUMBER) {
		assert_param(0);
	}
	
    if(force == true)
        PWM_ForceStop(PWM, deviceConfigures[handle].id);
    else{
        PWM_Stop(PWM, deviceConfigures[handle].id);
    }
	
    PWM_DisableOutputInverter(PWM, deviceConfigures[handle].id);
}

void HAL_PWM_SetFreqAndDuty(uint32_t handle, uint32_t frequency, uint32_t dutyCycle)
{
	if (handle >= PWM_MAX_WORKING_NUMBER) {
		assert_param(0);
	}
    PWM_ConfigOutputChannel(PWM, deviceConfigures[handle].id, frequency, dutyCycle, (PWM_OperateTypeDef)deviceConfigures[handle].operateType);
}
