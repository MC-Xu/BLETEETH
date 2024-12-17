/**************************************************************************//**
* @file     pan_hal_wdt.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series WDT (Watchdog Timer) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"

WDT_CallbackFunc wdt_callback[1];

void HAL_WDT_Init(WDT_Init_Opt *wdt)
{
    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    //Enable WDT Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT, ENABLE);

    // Select Clock Source
    CLK_SetWdtClkSrc(wdt->ClockSource);

    // Enable WDT, ENABLE Reset Mode, disable Wakeup Signal
    WDT_Open(wdt->Timeout, wdt->ResetDelay, wdt->ResetSwitch, wdt->WakeupSwitch);

    // Re-lock Regs
    SYS_LockReg();
}

void HAL_WDT_DeInit(void)
{
    SYS_UnlockReg();
    // Disable WDT, need to ensure Regs are unlocked
    WDT_Close();
    
    //Disable WDT Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT, DISABLE);

    // Re-lock Regs
    SYS_LockReg();
}

void HAL_WDT_Feed(WDT_Init_Opt *wdt)
{
    SYS_UnlockReg();
    WDT_ResetCounter();
    SYS_LockReg();
}

void HAL_WDT_Init_INT(WDT_Interrupt_Opt *opt)
{    
    wdt_callback[0] = opt->CallbackFunc;
    // Unlock Regs
    SYS_UnlockReg();
    WDT_EnableInt();
    NVIC_EnableIRQ(WDT_IRQn);
    // Re-Lock Regs
    SYS_LockReg();
}

void HAL_WDT_DeInit_INT(void)
{
    // Unlock Regs
    SYS_UnlockReg();
    wdt_callback[0] = NULL;
    WDT_DisableInt();
    // Re-Lock Regs
    SYS_LockReg();
}

void WDT_IRQHandler(void)
{
    // Clear Timeout Int Flag
    WDT_ClearTimeoutIntFlag();

    if (wdt_callback[0] != NULL)
    {
        wdt_callback[0]();
    }


}
