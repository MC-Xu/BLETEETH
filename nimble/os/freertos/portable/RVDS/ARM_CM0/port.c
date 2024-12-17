/*
 * FreeRTOS Kernel V10.5.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*-----------------------------------------------------------
* Implementation of functions defined in portable.h for the ARM CM0 port.
*----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_INT_CTRL_REG                 ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVSET_BIT                ( 1UL << 28UL )
#define portNVIC_PEND_SYSTICK_SET_BIT         ( 1UL << 26UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT       ( 1UL << 25UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )

#if ( configUSE_TICKLESS_IDLE == 1 )
uint32_t ulTimerCountsForOneTick = 0;  /*Used to calculate fast*/
#endif

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                      ( 0x01000000 )

/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER                 ( 0xffffffUL )

/* A fiddle factor to estimate the number of SysTick counts that would have
 * occurred while the SysTick counter is stopped during tickless idle
 * calculations. */
#ifndef portMISSED_COUNTS_FACTOR
    #define portMISSED_COUNTS_FACTOR    ( 94UL )
#endif

/* Constants used with memory barrier intrinsics. */
#define portSY_FULL_READ_WRITE    ( 15 )

/* Let the user override the default SysTick clock rate.  If defined by the
 * user, this symbol must equal the SysTick clock rate when the CLK bit is 0 in the
 * configuration register. */
#ifndef configSYSTICK_CLOCK_HZ
    #define configSYSTICK_CLOCK_HZ             ( configCPU_CLOCK_HZ )
    /* Ensure the SysTick is clocked at the same frequency as the core. */
    #define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( portNVIC_SYSTICK_CLK_BIT )
#else
    /* Select the option to clock SysTick not at the same frequency as the core. */
    #define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( 0 )
#endif

/* Legacy macro for backward compatibility only.  This macro used to be used to
 * replace the function that configures the clock used to generate the tick
 * interrupt (prvSetupTimerInterrupt()), but now the function is declared weak so
 * the application writer can override it by simply defining a function of the
 * same name (vApplicationSetupTickInterrupt()). */
#ifndef configOVERRIDE_DEFAULT_TICK_CONFIGURATION
    #define configOVERRIDE_DEFAULT_TICK_CONFIGURATION    0
#endif

/* Each task maintains its own interrupt status in the critical nesting
 * variable. */
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/* The number of SysTick increments that make up one tick period. */
#if ( configUSE_TICKLESS_IDLE == 1 )
//    static uint32_t ulTimerCountsForOneTick = 0;
    volatile TickType_t ulLPTmrLastCount = 0;
#endif /* configUSE_TICKLESS_IDLE */

/* The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer. */
#if ( configUSE_TICKLESS_IDLE == 1 )
//    static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/* Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
//    static uint32_t ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void );
void xPortSysTickHandler( void );
void vPortSVCHandler( void );

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
static void prvPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch
     * interrupt. */
    pxTopOfStack--;                                   /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
    *pxTopOfStack = portINITIAL_XPSR;                 /* xPSR */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxCode;           /* PC */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) prvTaskExitError; /* LR */
    pxTopOfStack -= 5;                                /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pvParameters;     /* R0 */
    pxTopOfStack -= 8;                                /* R11..R4. */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
     * its caller as there is nothing to return to.  If a task wants to exit it
     * should instead call vTaskDelete( NULL ).
     *
     * Artificially force an assert() to be triggered if configASSERT() is
     * defined, then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();

    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
    /* This function is no longer used, but retained for backward
     * compatibility. */
}
/*-----------------------------------------------------------*/

__asm void prvPortStartFirstTask( void )
{
    extern pxCurrentTCB;

    PRESERVE8

    /* The MSP stack is not reset as, unlike on M3/4 parts, there is no vector
     * table offset register that can be used to locate the initial stack value.
     * Not all M0 parts have the application vector table at address 0. */
/* *INDENT-OFF* */

    ldr r3, = pxCurrentTCB /* Obtain location of pxCurrentTCB. */
    ldr r1, [ r3 ]
    ldr r0, [ r1 ]         /* The first item in pxCurrentTCB is the task top of stack. */
    adds r0, # 32          /* Discard everything up to r0. */
    msr psp, r0            /* This is now the new top of stack to use in the task. */
    movs r0, # 2           /* Switch to the psp stack. */
    msr CONTROL, r0
    isb
    pop { r0 - r5 } /* Pop the registers that are saved automatically. */
    mov lr, r5 /* lr is now in r5. */
    pop { r3 } /* The return address is now in r3. */
    pop { r2 } /* Pop and discard the XPSR. */
    cpsie i /* The first task has its context and interrupts can be enabled. */
    bx r3 /* Finally, jump to the user defined task code. */

    ALIGN
/* *INDENT-ON* */
}
/*-----------------------------------------------------------*/
/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
	#if ( configUSE_TICKLESS_IDLE == 1 )
    vTaskTickSet(lp_get_curr_tmr_cnt());
    
    ulTimerCountsForOneTick = configSYSTICK_CLOCK_HZ / configTICK_ON_WAKING_RATE_HZ;
	#endif
    
    /* Make PendSV, CallSV and SysTick the same priority as the kernel. */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;

    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;

    /* Start the timer that generates the tick ISR.  Interrupts are disabled
     * here already. */
    vPortSetupTimerInterrupt();

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

    /* Start the first task. */
    prvPortStartFirstTask();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented in ports where there is nothing to return to.
     * Artificially force an assert. */
    configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
    /* Set a PendSV to request a context switch. */
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;

    /* Barriers are normally not required but do ensure the code is completely
     * within the specified behaviour for the architecture. */
    __dsb( portSY_FULL_READ_WRITE );
    __isb( portSY_FULL_READ_WRITE );
}
/*-----------------------------------------------------------*/

CONFIG_RAM_CODE void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;
    __dsb( portSY_FULL_READ_WRITE );
    __isb( portSY_FULL_READ_WRITE );
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;

    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
/*-----------------------------------------------------------*/

__asm uint32_t ulSetInterruptMaskFromISR( void )
{
/* *INDENT-OFF* */
    mrs r0, PRIMASK
    cpsid i
    bx lr
/* *INDENT-ON* */
}
/*-----------------------------------------------------------*/

__asm void vClearInterruptMaskFromISR( uint32_t ulMask )
{
/* *INDENT-OFF* */
    msr PRIMASK, r0
    bx lr
/* *INDENT-ON* */
}
/*-----------------------------------------------------------*/

CONFIG_RAM_CODE __asm void xPortPendSVHandler( void )
{
    extern vTaskSwitchContext
    extern pxCurrentTCB

/* *INDENT-OFF* */
    PRESERVE8

    mrs r0, psp

    ldr r3, = pxCurrentTCB /* Get the location of the current TCB. */
    ldr r2, [ r3 ]

    subs r0, # 32  /* Make space for the remaining low registers. */
    str r0, [ r2 ] /* Save the new top of stack. */
    stmia r0 !, { r4 - r7 } /* Store the low registers that are not saved automatically. */
    mov r4, r8 /* Store the high registers. */
    mov r5, r9
    mov r6, r10
    mov r7, r11
    stmia r0 !, { r4 - r7 }

    push { r3, r14 }
    cpsid i
    bl vTaskSwitchContext
    cpsie i
    pop { r2, r3 } /* lr goes in r3. r2 now holds tcb pointer. */

    ldr r1, [ r2 ]
    ldr r0, [ r1 ] /* The first item in pxCurrentTCB is the task top of stack. */
    adds r0, # 16  /* Move to the high registers. */
    ldmia r0 !, { r4 - r7 } /* Pop the high registers. */
    mov r8, r4
    mov r9, r5
    mov r10, r6
    mov r11, r7

    msr psp, r0   /* Remember the new top of stack for the task. */

    subs r0, # 32 /* Go back for the low registers that are not automatically restored. */
    ldmia r0 !, { r4 - r7 } /* Pop low registers.  */

    bx r3
    ALIGN
/* *INDENT-ON* */
}
/*-----------------------------------------------------------*/

#if ( configUSE_TICKLESS_IDLE == 1 )
CONFIG_RAM_CODE void UpdateTickAndSch(void)
{
    uint32_t ulPreviousMask;
	
    vTaskTickSet(lp_get_curr_tmr_cnt());

    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            /* Pend a context switch. */
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
}
#endif

CONFIG_RAM_CODE void xPortSysTickHandler( void )
{
	  #if ( configUSE_TICKLESS_IDLE == 1 )
        UpdateTickAndSch();
        
        if(portNVIC_SYSTICK_LOAD_REG != (ulTimerCountsForOneTick - 1UL))
        {
            portNVIC_SYSTICK_CTRL_REG = 0UL;
            portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
				
            /* Configure SysTick to interrupt at the requested rate. */
            portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
            portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
        }
		#else
        uint32_t ulPreviousMask;

        ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
        {
            /* Increment the RTOS tick. */
            if( xTaskIncrementTick() != pdFALSE )
            {
                /* Pend a context switch. */
                portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
            }
        }
        portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
		#endif
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
#if ( configOVERRIDE_DEFAULT_TICK_CONFIGURATION == 0 )

    __weak void vPortSetupTimerInterrupt( void )
    {

        /* Stop and clear the SysTick. */
        portNVIC_SYSTICK_CTRL_REG = 0UL;
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

        /* Configure SysTick to interrupt at the requested rate. */				
        #if ( configUSE_TICKLESS_IDLE == 1 )
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
        portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
        #else
        portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
        portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
        #endif
    }

#endif /* configOVERRIDE_DEFAULT_TICK_CONFIGURATION */

