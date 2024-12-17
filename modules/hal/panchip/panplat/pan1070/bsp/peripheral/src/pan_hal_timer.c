/**************************************************************************//**
* @file     pan_hal_timer.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series Timer HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#include "pan_hal.h"



TIMER_HandleTypeDef TIMER_Handle_Array[3] =
{
    // TIMER0 Configuration
    {
        .TIMERx = TIMER0,  // Assuming TIMER0 is defined somewhere in your code
        .initObj = {0},  // Initialize with default values or specify them
        .interruptObj = {0},  // Initialize with default values or specify them
        .IRQn = TMR0_IRQn,  // Assuming TIMER0_IRQn is defined as an IRQn_Type for TIMER0
        .callback = NULL,  // Using the dummy callBack
    },
    // TIMER1 Configuration
    {
        .TIMERx = TIMER1,  // Assuming TIMER1 is defined somewhere in your code
        .initObj = {0},  // Initialize with default values or specify them
        .interruptObj = {0},  // Initialize with default values or specify them
        .IRQn = TMR1_IRQn,  // Assuming TIMER1_IRQn is defined as an IRQn_Type for TIMER1
        .callback = NULL,  // Using the dummy callBack
    },
    // TIMER2 Configuration
    {
        .TIMERx = TIMER2,  // Assuming TIMER2 is defined somewhere in your code
        .initObj = {0},  // Initialize with default values or specify them
        .interruptObj = {0},  // Initialize with default values or specify them
        .IRQn = TMR2_IRQn,  // Assuming TIMER2_IRQn is defined as an IRQn_Type for TIMER2
        .callback = NULL,  // Using the dummy callBack
    }
};

void HAL_TIMER_Init(TIMER_HandleTypeDef *pTimer)
{
	if(pTimer->TIMERx == TIMER0)
	{
		CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);
		CLK_SetTmrClkSrc(TIMER0, CLK_APB1_TMR0SEL_APB1CLK);
	}
	else if(pTimer->TIMERx == TIMER1)
	{
		CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR1, ENABLE);
		CLK_SetTmrClkSrc(TIMER1, CLK_APB2_TMR1SEL_APB2CLK);
	}
	else if(pTimer->TIMERx == TIMER2)
	{
		CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR2, ENABLE);
		CLK_SetTmrClkSrc(TIMER2, CLK_APB2_TMR2SEL_APB2CLK);
	}
	
    TIMER_SetCmpValue(pTimer->TIMERx, pTimer->initObj.tmr0CmpSel, pTimer->initObj.cmpValue);
	
    if(pTimer->initObj.mode == TIMER_MODE_BASECNT)
    {
        TIMER_Open(pTimer->TIMERx, pTimer->initObj.cntMode, pTimer->initObj.freq);
    }  
    else if(pTimer->initObj.mode == TIMER_MODE_EVENTCNT)
	{
        // Counter increase on falling edge
        // Set Timer Mode
        TIMER_SetCountingMode(pTimer->TIMERx, pTimer->initObj.cntMode);
        // Set prescale
        TIMER_SetPrescaleValue(pTimer->TIMERx, pTimer->initObj.prescale);
        TIMER_EnableEventCounter(pTimer->TIMERx,pTimer->initObj.evtCntEdge);
    }
    else if(pTimer->initObj.mode == TIMER_MODE_INCAP)
	{
        TIMER_Open(pTimer->TIMERx, pTimer->initObj.cntMode, pTimer->initObj.freq);
        // Set capture source to ext PIN
        TIMER_SetCaptureSource(pTimer->TIMERx, pTimer->initObj.capSrc);
        // Configure Timer capture mode and capture edge
        TIMER_EnableCapture(pTimer->TIMERx, pTimer->initObj.capMode, pTimer->initObj.capEdge);
    }
    else if(pTimer->initObj.mode == TIMER_MODE_WAKEUP)
	{
        // Set Timer Mode to One Shot Mode
        TIMER_SetCountingMode(pTimer->TIMERx, TIMER_ONESHOT_MODE);
        // Set prescale
        TIMER_SetPrescaleValue(pTimer->TIMERx, pTimer->initObj.prescale);
        // Enable wakeup function
        TIMER_EnableWakeup(pTimer->TIMERx);
    }
	
	HAL_TIMER_Init_INT(pTimer);

    NVIC_EnableIRQ(pTimer->IRQn);
}

void HAL_TIMER_DeInit(TIMER_HandleTypeDef *pTimer)
{
    if (pTimer->initObj.mode == TIMER_MODE_EVENTCNT) 
	{
        // Disable event counter mode
        TIMER_DisableEventCounter(pTimer->TIMERx);
    }
    else if(pTimer->initObj.mode == TIMER_MODE_WAKEUP) 
	{
        // Disable wakeup function
        TIMER_DisableWakeup(pTimer->TIMERx);    
    }
    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(pTimer->TIMERx);
}

void HAL_TIMER_Init_INT(TIMER_HandleTypeDef *pTimer)
{
    if (pTimer->initObj.mode == TIMER_MODE_INCAP)
    {
        // Enable Timer capture interrupt
        TIMER_EnableCaptureInt(pTimer->TIMERx);
    }
    else 
    {
        // Enable interrupt
        TIMER_EnableInt(pTimer->TIMERx);
    }
}

void HAL_TIMER_DeInit_INT(TIMER_HandleTypeDef *pTimer)
{
    if (pTimer->initObj.mode == TIMER_MODE_INCAP)
    {
        // Disable Timer capture interrupt
        TIMER_DisableCaptureInt(pTimer->TIMERx);
    }
    else
    {
        // Disable interrupt
        TIMER_DisableInt(pTimer->TIMERx);
    }
}

void HAL_TIMER_Start(TIMER_HandleTypeDef *pTimer)
{
    TIMER_Start(pTimer->TIMERx);
}

void HAL_TIMER_Stop(TIMER_HandleTypeDef *pTimer)
{
    TIMER_Stop(pTimer->TIMERx);
}

uint32_t HAL_TIMER_GetRealFreq(TIMER_HandleTypeDef *pTimer)
{
    uint32_t u32Clk = CLK_GetPeripheralFreq(pTimer->TIMERx);
    uint32_t u32Prescale = 0;
    uint32_t u32RealFreq = pTimer->initObj.freq;

    if (u32RealFreq > u32Clk)
        u32RealFreq = u32Clk;
    else if (u32RealFreq == 0)
        u32RealFreq = 1;

    u32Prescale = u32Clk / u32RealFreq - 1;

    if (u32Prescale > 0xFF)
	{
		//Prescale is 8bit in reg
        u32Prescale = 0xFF;
	}

    // Calc real frequency and return
    u32RealFreq = u32Clk / (u32Prescale + 1);

    return u32RealFreq;    
}

static void TIMER_HandleProc(TIMER_HandleTypeDef *pTimer)
{
    if (TIMER_GetIntFlag(pTimer->TIMERx))
    {
        // Clear Timer interrupt flag
        TIMER_ClearIntFlag(pTimer->TIMERx);
        pTimer->callback(TIMER_CB_FLAG_CNT);

    }

    if (TIMER_GetCaptureIntFlag(pTimer->TIMERx))
    {
        // Clear Timer capture interrupt flag
        TIMER_ClearCaptureIntFlag(pTimer->TIMERx);
        pTimer->callback(TIMER_CB_FLAG_CAP);
    }

    if (TIMER_GetWakeupFlag(pTimer->TIMERx))
    {
        // Clear Timer interrupt flag
		TIMER_ClearWakeupFlag(pTimer->TIMERx, TIMER_GetWakeupFlag(pTimer->TIMERx));
		TIMER_ClearTFFlag(pTimer->TIMERx, TIMER_GetTFFlag(pTimer->TIMERx));
		TIMER_ClearIntFlag(pTimer->TIMERx);		
        pTimer->callback(TIMER_CB_FLAG_WK);
    }
}

void TMR0_IRQHandler(void)
{
    TIMER_HandleProc(&TIMER_Handle_Array[0]);
}

void TMR1_IRQHandler(void)
{
    TIMER_HandleProc(&TIMER_Handle_Array[1]);
}

void TMR2_IRQHandler(void)
{
    TIMER_HandleProc(&TIMER_Handle_Array[2]);
}

