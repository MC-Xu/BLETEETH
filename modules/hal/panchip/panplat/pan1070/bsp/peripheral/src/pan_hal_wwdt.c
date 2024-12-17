/**************************************************************************//**
* @file     pan_hal_wwdt.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series WWDT (Windowed Watchdog Timer) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"


WWDT_CallbackFunc wwdt_callback[1];

void HAL_WWDT_Init(WWDT_Init_Opt *wwdt){

    // Unlock Regs
    SYS_UnlockReg();
    
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WWDT, ENABLE);

    // Select Clock Source
    CLK_SetWwdtClkSrc(wwdt->ClockSource);

    // Delay some nops before Re-lock Regs
    SYS_delay_10nop(1);
    
    // Enable WWDT, disable Int
    WWDT_Open(wwdt->Prescaler, wwdt->CmpValue, FALSE);
    
    // Re-Lock Regs
    SYS_LockReg();

}

void HAL_WWDT_DeInit(void){
    SYS_UnlockReg();
    WWDT_Close();
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WWDT, DISABLE);
    SYS_LockReg();
}

void HAL_WWDT_Feed(WWDT_Init_Opt *wwdt){
    WWDT_ReloadCounter();
}

void HAL_WWDT_Init_INT(WWDT_Interrupt_Opt *wwdt){
    wwdt_callback[0] = wwdt->CallbackFunc;
    
    SYS_UnlockReg();
    WWDT->CTL |= WWDT_CTL_INTEN_Msk;
    
    // Enable NVIC WWDT IRQ
    NVIC_EnableIRQ(WWDT_IRQn);
    
    SYS_LockReg();
}

void HAL_WWDT_DeInit_INT(void){
    wwdt_callback[0] = NULL;
    
    WWDT->CTL &= ~WWDT_CTL_INTEN_Msk;

    // Disable NVIC WWDT IRQ, thus WWDT will stop reloading counter
    NVIC_DisableIRQ(WWDT_IRQn);
}

void WWDT_IRQHandler(void)
{
    // Clear Timeout Int Flag
    WWDT_ClearIntFlag();

    if (wwdt_callback[0] != NULL)
        wwdt_callback[0]();
}
