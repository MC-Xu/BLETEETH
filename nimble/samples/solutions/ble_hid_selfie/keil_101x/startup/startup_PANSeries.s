
;/******************************************************************************
; * @file     startup_PANSeries.s
; * @version  V1.00
; * $Revision: 3 $
; * $Date: 20/09/23 12:13 $
; * @brief    CMSIS ARM Cortex-M0 Core Device Startup File
; *
; * @note
; * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
;*****************************************************************************/

Stack_Size      EQU     0x00000600 ;2k ram

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size


__Vectors       DCD     __initial_sp               ; -16, Top of Stack
                DCD     Reset_Handler              ; -15, Reset Handler
                DCD     NMI_Handler                ; -14, NMI Handler
                DCD     HardFault_Handler          ; -13, Hard Fault Handler
                DCD     0                          ; -12, Reserved
                DCD     0                          ; -11, Reserved
                DCD     0                          ; -10, Reserved
                DCD     0                          ; -9,  Reserved
                DCD     0                          ; -8,  Reserved
                DCD     0                          ; -7,  Reserved
                DCD     0                          ; -6,  Reserved
                DCD     SVC_Handler                ; -5,  SVCall Handler
                DCD     0                          ; -4,  Reserved
                DCD     0                          ; -3,  Reserved
                DCD     PendSV_Handler             ; -2,  PendSV Handler
                DCD     SysTick_Handler            ; -1,  SysTick Handler

                ; External Interrupts
                DCD     I2C0_IRQHandler            ; 0,
                DCD     SPI0_IRQHandler            ; 1,
                DCD     0                          ; 2,
                DCD     UART0_IRQHandler           ; 3,
                DCD     PWM_IRQHandler            ; 4,
                DCD     ADC_IRQHandler             ; 5,
                DCD     WDT_IRQHandler             ; 6,
                DCD     WWDT_IRQHandler            ; 7,
                DCD     TMR0_IRQHandler            ; 8,
                DCD     0                          ; 9,
                DCD     SPI1_IRQHandler            ; 10,
                DCD     LL_IRQHandler              ; 11,
                DCD     UART1_IRQHandler           ; 12,
                DCD     TMR1_IRQHandler            ; 13,
                DCD     TMR2_IRQHandler            ; 14,
                DCD     GPIO0_IRQHandler           ; 15,
                DCD     GPIO1_IRQHandler           ; 16,
                DCD     GPIO2_IRQHandler           ; 17,
                DCD     GPIO3_IRQHandler           ; 18,
                DCD     0                          ; 19,
                DCD     0                          ; 20,
                DCD     USBDMA_IRQHandler          ; 21,
                DCD     USB_IRQHandler             ; 22,
                DCD     TRIM_IRQHandler            ; 23,
                DCD     0                          ; 24,
                DCD     0                          ; 25,
                DCD     0                          ; 26,
                DCD     DMA_IRQHandler             ; 27,
                DCD     BOD_IRQHandler             ; 28,
                DCD     SLPTMR_IRQHandler          ; 29,
                DCD     LP_IRQHandler              ; 30,
                DCD     0                          ; 31,

__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler    PROC
                 EXPORT  Reset_Handler             [WEAK]
                 IMPORT  SystemInit
                 IMPORT  __main

                 LDR     R0, =SystemInit
                 BLX     R0
                 LDR     R0, =__main
                 BX      R0
                 ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler             [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler            [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT  I2C0_IRQHandler            [WEAK]
                EXPORT  SPI0_IRQHandler            [WEAK]
                EXPORT  UART0_IRQHandler           [WEAK]
                EXPORT  PWM_IRQHandler            [WEAK]
                EXPORT  ADC_IRQHandler             [WEAK]
                EXPORT  WDT_IRQHandler             [WEAK]
                EXPORT  WWDT_IRQHandler            [WEAK]
                EXPORT  TMR0_IRQHandler            [WEAK]
                EXPORT  SPI1_IRQHandler            [WEAK]
                EXPORT  LL_IRQHandler              [WEAK]
                EXPORT  UART1_IRQHandler           [WEAK]
                EXPORT  TMR1_IRQHandler            [WEAK]
                EXPORT  TMR2_IRQHandler            [WEAK]
                EXPORT  GPIO0_IRQHandler           [WEAK]
                EXPORT  GPIO1_IRQHandler           [WEAK]
                EXPORT  GPIO2_IRQHandler           [WEAK]
                EXPORT  GPIO3_IRQHandler           [WEAK]
                EXPORT  USBDMA_IRQHandler          [WEAK]
                EXPORT  USB_IRQHandler             [WEAK]
                EXPORT  TRIM_IRQHandler            [WEAK]
                EXPORT  DMA_IRQHandler             [WEAK]
                EXPORT  BOD_IRQHandler             [WEAK]
                EXPORT  SLPTMR_IRQHandler          [WEAK]
                EXPORT  LP_IRQHandler              [WEAK]


I2C0_IRQHandler            ; 0,
SPI0_IRQHandler            ; 1,
UART0_IRQHandler           ; 3,
PWM_IRQHandler            ; 4,
ADC_IRQHandler             ; 5,
WDT_IRQHandler             ; 6,
WWDT_IRQHandler            ; 7,
TMR0_IRQHandler            ; 8,

SPI1_IRQHandler            ; 10,
LL_IRQHandler              ; 11,
UART1_IRQHandler           ; 12,
TMR1_IRQHandler            ; 13,
TMR2_IRQHandler            ; 14,
GPIO0_IRQHandler           ; 15,
GPIO1_IRQHandler           ; 16,
GPIO2_IRQHandler           ; 17,
GPIO3_IRQHandler           ; 18,
USBDMA_IRQHandler          ; 21,
USB_IRQHandler             ; 22,
TRIM_IRQHandler            ; 23,
DMA_IRQHandler             ; 27,
BOD_IRQHandler             ; 28,
SLPTMR_IRQHandler          ; 29,
LP_IRQHandler              ; 30,

                B       .
                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB

                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit

                 ELSE

                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap

__user_initial_stackheap

                 LDR     R0, = Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

;***************** (C) COPYRIGHT Panchip Microelectronics *****END OF FILE*****
