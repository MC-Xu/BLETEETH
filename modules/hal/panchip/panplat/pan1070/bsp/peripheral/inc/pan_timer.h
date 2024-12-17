/**************************************************************************
 * @file     pan_timer.h
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 2023/11/08 15:51 $
 * @brief    Panchip series TIMER driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __PAN_TIMER_H__
#define __PAN_TIMER_H__

/**
 * @brief Timer Interface
 * @defgroup timer_interface Timer Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif



/** @defgroup TIMER_WORK_MODE_FLAG Timer work mode
 * @brief       Timer work mode definition
 * @{ */

typedef enum _TIMER_CntModeDef
{
    TIMER_ONESHOT_MODE    = (0UL << TIMER_CTL_OPMODE_Pos),  /*!< Timer working in one shot mode */
    TIMER_PERIODIC_MODE   = (1UL << TIMER_CTL_OPMODE_Pos),  /*!< Timer working in periodic mode */
    TIMER_TOGGLE_MODE     = (2UL << TIMER_CTL_OPMODE_Pos),  /*!< Timer working in toggle output mode */
    TIMER_CONTINUOUS_MODE = (3UL << TIMER_CTL_OPMODE_Pos)   /*!< Timer working in continuous mode */
} TIMER_CntModeDef;
/**@} */

/** @defgroup TIMER_COUNTING_MODE_FLAG Timer counting mode
 * @brief       Timer counting mode definition
 * @{ */
typedef enum _TIMER_CapModeDef
{
    TIMER_CAPTURE_FREE_COUNTING_MODE    = (0UL),                        /*!< Free counting mode */
    TIMER_CAPTURE_TRIGGER_COUNTING_MODE = (TIMER_EXTCTL_CAPSEL_Msk),    /*!< Trigger counting mode */
    TIMER_CAPTURE_COUNTER_RESET_MODE    = (TIMER_EXTCTL_CAPFUNCS_Msk)   /*!< Counter reset mode */
} TIMER_CapModeDef;
/**@} */

/** @defgroup TIMER_CAPUTURE_MODE_FLAG Timer capture mode
 * @brief       Timer capture edge definition
 * @{ */
typedef enum _TIMER_CapEdgeDef
{
    TIMER_CAPTURE_FALLING_EDGE              = (0UL << TIMER_EXTCTL_CAPEDGE_Pos),  /*!< Falling edge trigger timer capture */
    TIMER_CAPTURE_RISING_EDGE               = (1UL << TIMER_EXTCTL_CAPEDGE_Pos),  /*!< Rising edge trigger timer capture */
    TIMER_CAPTURE_BOTH_EDGE                 = (2UL << TIMER_EXTCTL_CAPEDGE_Pos),  /*!< Both edge trigger timer capture, valid only in Free-Cnt Mode */
    TIMER_CAPTURE_FALLING_THEN_RISING_EDGE  = (2UL << TIMER_EXTCTL_CAPEDGE_Pos),  /*!< Falling edge then rising edge trigger timer capture, valid only in Trig-Cnt Mode */
    TIMER_CAPTURE_RISING_THEN_FALLING_EDGE  = (3UL << TIMER_EXTCTL_CAPEDGE_Pos)   /*!< Rising edge then falling edge trigger timer capture, valid only in Trig-Cnt Mode */
} TIMER_CapEdgeDef;
/**@} */

/** @defgroup TIMER_COUNTER_EDGE_FLAG Timer counting edge
 * @brief       Timer counting edge definition
 * @{ */
typedef enum _TIMER_EvtCntEdgeDef
{
    TIMER_COUNTER_FALLING_EDGE  = (0UL),                        /*!< Counter increase on falling edge of external event */
    TIMER_COUNTER_RISING_EDGE   = (TIMER_EXTCTL_CNTPHASE_Msk)   /*!< Counter increase on rising edge of external event */
} TIMER_EvtCntEdgeDef;
/**@} */

// #define TIMER_TOGGLE_TMX_CNT_OUT            (0UL)                           /*!< Timer use TMx_CNT_OUT, x = 0, 1 pin as toggle output pin   */

/** @defgroup TIMER_CAPUTURE_SRC_FLAG Timer capture souce
 * @brief       Timer capture source definition
 * @{ */
typedef enum _TIMER_CapSrcDef
{
    TIMER_CAPTURE_SOURCE_EXT_PIN    = (0UL),                  /*!< Capture source is external GPIO pin */
    TIMER_CAPTURE_SOURCE_32K_OUTPUT = (TIMER_CTL_CAPSRC_Msk)  /*!< Capture source is internal 32KHz RCL Clock */
} TIMER_CapSrcDef;
/**@} */

/**
 * @brief Timer 0 comparator selection: Peripheral timer0 comparator select cmp0
 */
typedef enum _TIMER0_CmpSelDef
{
    TMR0_COMPARATOR_SEL_CMP    = (0),    /*!< Peripheral timer0 comparator select cmp0 */
    TMR0_COMPARATOR_SEL_CMP1   = (1),    /*!< Peripheral timer0 comparator select cmp1 */
    TMR0_COMPARATOR_SEL_CMP2   = (2)     /*!< Peripheral timer0 comparator select cmp2 */
} TIMER0_CmpSelDef;
/**@} */

#define	FIXED_DEVIATION			(12)	/*!< deviation from hw timer counter count to compare value to interrupt*/

/**@} */


/**
  * @brief This API is used to enable timer capture function with specified mode and capture edge
  * @param[in] timer The base address of Timer module
  * @param[in] capMode Timer capture mode. Could be
  *                 - \ref TIMER_CAPTURE_FREE_COUNTING_MODE
  *                 - \ref TIMER_CAPTURE_TRIGGER_COUNTING_MODE
  *                 - \ref TIMER_CAPTURE_COUNTER_RESET_MODE
  * @param[in] capEdge Timer capture edge. Possible values are
  *                 - \ref TIMER_CAPTURE_FALLING_EDGE
  *                 - \ref TIMER_CAPTURE_RISING_EDGE
  *                 - \ref TIMER_CAPTURE_BOTH_EDGE
  * @return None
  * @note Timer frequency should be configured separately by using \ref TIMER_Open API, or program registers directly
  */
__STATIC_INLINE void TIMER_EnableCapture(TIMER_T *timer, TIMER_CapModeDef capMode, TIMER_CapEdgeDef capEdge)
{

    timer->EXTCTL = (timer->EXTCTL & ~(TIMER_EXTCTL_CAPSEL_Msk |
                                       TIMER_EXTCTL_CAPFUNCS_Msk |
                                       TIMER_EXTCTL_CAPEDGE_Msk)) |
                    capMode | capEdge | TIMER_EXTCTL_CAPEN_Msk;
}

/**
  * @brief This API is used to disable the Timer capture function
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableCapture(TIMER_T *timer)
{
    timer->EXTCTL &= ~TIMER_EXTCTL_CAPEN_Msk;
}

/**
  * @brief This function is used to enable the Timer event counter function with specify detection edge
  * @param[in] timer The base address of Timer module
  * @param[in] evtCntEdge Detection edge of counter pin. Could be ether
  *             - \ref TIMER_COUNTER_RISING_EDGE, or
  *             - \ref TIMER_COUNTER_FALLING_EDGE
  * @return None
  * @note Timer compare value should be configured separately by using \ref TIMER_SetCmpValue function or program registers directly
  */
__STATIC_INLINE void TIMER_EnableEventCounter(TIMER_T *timer, TIMER_EvtCntEdgeDef evtCntEdge)
{
    timer->EXTCTL = (timer->EXTCTL & ~TIMER_EXTCTL_CNTPHASE_Msk) | evtCntEdge;
    timer->CTL |= TIMER_CTL_EXTCNTEN_Msk;
}

/**
  * @brief This API is used to disable the Timer event counter function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableEventCounter(TIMER_T *timer)
{
    timer->CTL &= ~TIMER_CTL_EXTCNTEN_Msk;
}

/**
  * @brief This function is used to set new Timer prescale value
  * @param[in] timer The base address of Timer module
  * @param[in] u32Value  Timer prescale value. Valid values are between 0 to 0xFF
  * @return None
  * @note Clock input is divided by (prescale + 1) before it is fed into timer
  * \hideinitializer
  */
__STATIC_INLINE void TIMER_SetPrescaleValue(TIMER_T *timer, uint32_t u32Value)
{
    timer->CTL = (timer->CTL & ~TIMER_CTL_PSC_Msk) | u32Value;
}

/**
  * @brief This function is used to Set Timer counting mode
  * @param[in] timer    The base address of Timer module
  * @param[in] cntMode  Operation mode. Possible options are
  *                     - \ref TIMER_ONESHOT_MODE
  *                     - \ref TIMER_PERIODIC_MODE
  *                     - \ref TIMER_TOGGLE_MODE
  *                     - \ref TIMER_CONTINUOUS_MODE
  * @return None
  */
__STATIC_INLINE void TIMER_SetCountingMode(TIMER_T *timer, TIMER_CntModeDef cntMode)
{
    timer->CTL = (timer->CTL & ~TIMER_CTL_OPMODE_Msk) | cntMode;
}

/**
  * @brief This function is used to check if specify Timer is inactive or active
  * @param[in] timer The base address of Timer module
  * @return timer is activate or inactivate
  * @retval 0 Timer 24-bit up counter is inactive
  * @retval 1 Timer 24-bit up counter is active
  * \hideinitializer
  */
__STATIC_INLINE bool TIMER_IsActive(TIMER_T *timer)
{
    return timer->CTL & TIMER_CTL_ACTSTS_Msk ? true : false;
}


/**
  * @brief This function is used to start Timer counting
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_Start(TIMER_T *timer)
{
    timer->CTL |= TIMER_CTL_CNTEN_Msk;
}

/**
  * @brief This function is used to stop Timer counting
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_Stop(TIMER_T *timer)
{
    timer->CTL &= ~TIMER_CTL_CNTEN_Msk;
}

/**
  * @brief This function is used to reset Timer counting, prescale counter and CNTEN
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_Reset(TIMER_T *timer)
{
    timer->CTL |= TIMER_CTL_RSTCNT_Msk;
}

/**
  * @brief This function is used to enable the Timer wake-up function
  * @param[in] timer The base address of Timer module
  * @return None
  * @note  To wake the system from power down mode, timer clock source must be ether LXT or LIRC
  */
__STATIC_INLINE void TIMER_EnableWakeup(TIMER_T *timer)
{
    timer->CTL |= TIMER_CTL_WKEN_Msk;
}

/**
  * @brief This function is used to disable the Timer wake-up function
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableWakeup(TIMER_T *timer)
{
    timer->CTL &= ~TIMER_CTL_WKEN_Msk;
}


/**
  * @brief This function is used to enable the capture pin detection de-bounce function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_EnableCaptureDebounce(TIMER_T *timer)
{
    timer->EXTCTL |= TIMER_EXTCTL_CAPDBEN_Msk;
}

/**
  * @brief This function is used to disable the capture pin detection de-bounce function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableCaptureDebounce(TIMER_T *timer)
{
    timer->EXTCTL &= ~TIMER_EXTCTL_CAPDBEN_Msk;
}


/**
  * @brief This function is used to enable the counter pin detection de-bounce function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_EnableEventCounterDebounce(TIMER_T *timer)
{
    timer->EXTCTL |= TIMER_EXTCTL_CNTDBEN_Msk;
}

/**
  * @brief This function is used to disable the counter pin detection de-bounce function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableEventCounterDebounce(TIMER_T *timer)
{
    timer->EXTCTL &= ~TIMER_EXTCTL_CNTDBEN_Msk;
}

/**
  * @brief This function is used to enable the Timer time-out interrupt function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_EnableInt(TIMER_T *timer)
{
    timer->CTL |= TIMER_CTL_INTEN_Msk;
}

/**
  * @brief This function is used to disable the Timer time-out interrupt function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableInt(TIMER_T *timer)
{
    timer->CTL &= ~TIMER_CTL_INTEN_Msk;
}

/**
  * @brief This function is used to enable the Timer capture trigger interrupt function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_EnableCaptureInt(TIMER_T *timer)
{
    timer->EXTCTL |= TIMER_EXTCTL_CAPIEN_Msk;
}

/**
  * @brief This function is used to disable the Timer capture trigger interrupt function.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_DisableCaptureInt(TIMER_T *timer)
{
    timer->EXTCTL &= ~TIMER_EXTCTL_CAPIEN_Msk;
}

/**
  * @brief This function indicates Timer time-out interrupt occurred or not.
  * @param[in] timer The base address of Timer module
  * @return Timer time-out interrupt occurred or not
  * @retval 0 Timer time-out interrupt did not occur
  * @retval 1 Timer time-out interrupt occurred
  */
__STATIC_INLINE uint32_t TIMER_GetIntFlag(TIMER_T *timer)
{
    return(timer->INTSTS & TIMER_INTSTS_TIF_Msk ? 1 : 0);
}

/**
  * @brief This function clears the Timer time-out interrupt flag.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_ClearIntFlag(TIMER_T *timer)
{
    timer->INTSTS = TIMER_INTSTS_TIF_Msk;
}

/**
  * @brief This function indicates Timer time-out event occurred or not.
  * @param[in] timer The base address of Timer module
  * @return Timer time-out event occurred or not
  * @retval 0 Timer time-out event did not occur
  * @retval Timer time-out event occurred
  */
__STATIC_INLINE uint32_t TIMER_GetTFFlag(TIMER_T *timer)
{
	if (timer == TIMER0) {
		return (timer->INTSTS & (TIMER_INTSTS_TF0_Msk | TIMER_INTSTS_TF1_Msk | TIMER_INTSTS_TF2_Msk)); 
	} else{
		return (timer->INTSTS & (0x1 << 2));
	}
}

/**
  * @brief This function clears the Timer time-out evnet flag.
  * @param[in] timer The base address of Timer module
  * @param[in] cmp_seq The Timer0 comparator sequence (e.g., TMR0_COMPARATOR_SEL_CMP0).
  * @return None
  */
__STATIC_INLINE void TIMER_ClearTFFlag(TIMER_T *timer, uint32_t timer_flag)
{
	timer->INTSTS = timer_flag; 
}

/**
  * @brief This function indicates Timer has waked up system or not.
  * @param[in] timer The base address of Timer module
  * @return Timer has waked up system or not
  * @retval 0 Timer did not wake up system
  * @retval Timer wake up system
  */
__STATIC_INLINE uint32_t TIMER_GetWakeupFlag(TIMER_T *timer)
{
	if (timer == TIMER0) {
		return (timer->INTSTS & (TIMER_INTSTS_TWKF0_Msk | TIMER_INTSTS_TWKF1_Msk | TIMER_INTSTS_TWKF2_Msk)); 
	} else{
		return (timer->INTSTS & TIMER_INTSTS_TWKF0_Msk);
	}
}

/**
  * @brief This function clears the Timer wakeup interrupt flag.
  * @param[in] timer The base address of Timer module
  * @param[in] cmp_seq The Timer0 comparator sequence (e.g., TMR0_COMPARATOR_SEL_CMP0).
  * @return None
  */
__STATIC_INLINE void TIMER_ClearWakeupFlag(TIMER_T *timer, uint32_t wake_flag)
{
	timer->INTSTS = wake_flag; 
}

/**
  * @brief This function indicates Timer capture interrupt occurred or not.
  * @param[in] timer The base address of Timer module
  * @return Timer capture interrupt occurred or not
  * @retval 0 Timer capture interrupt did not occur
  * @retval 1 Timer capture interrupt occurred
  */
__STATIC_INLINE uint32_t TIMER_GetCaptureIntFlag(TIMER_T *timer)
{
    return timer->EINTSTS & TIMER_EINTSTS_CAPIF_Msk;
}

/**
  * @brief This function clears the Timer capture interrupt flag.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_ClearCaptureIntFlag(TIMER_T *timer)
{
    timer->EINTSTS = TIMER_EINTSTS_CAPIF_Msk;
}

/**
  * @brief This function indicates Timer capture event occurred or not.
  * @param[in] timer The base address of Timer module
  * @return Timer capture event occurred or not
  * @retval 0 Timer capture event did not occur
  * @retval 1 Timer capture event occurred
  */
__STATIC_INLINE uint32_t TIMER_GetCaptureFlag(TIMER_T *timer)
{
    return ((timer->EINTSTS & TIMER_EINTSTS_CAPF_Msk)? 1:0);
}

/**
  * @brief This function clears the Timer capture event flag.
  * @param[in] timer The base address of Timer module
  * @return None
  */
__STATIC_INLINE void TIMER_ClearCaptureFlag(TIMER_T *timer)
{
    timer->EINTSTS = TIMER_EINTSTS_CAPF_Msk;
}


/**
  * @brief This function set the Timer capture source.
  * @param[in] timer The base address of Timer module
  * @param[in] capSrc	The capture source for timer capture functions
	*					\ref TIMER_CAPTURE_SOURCE_EXT_PIN
	*					\ref TIMER_CAPTURE_SOURCE_32K_OUTPUT
  * @return None
  */
__STATIC_INLINE void TIMER_SetCaptureSource(TIMER_T *timer, TIMER_CapSrcDef capSrc)
{
    timer->CTL = (timer->CTL & ~TIMER_CTL_CAPSRC_Msk) | capSrc;
}

/**
  * @brief This function gets the Timer capture data.
  * @param[in] timer The base address of Timer module
  * @return Timer capture data value
  */
__STATIC_INLINE uint32_t TIMER_GetCaptureData(TIMER_T *timer)
{
    return timer->CAP;
}

/**
  * @brief This function reports the current timer counter value.
  * @param[in] timer The base address of Timer module
  * @return Timer counter value
  */
__STATIC_INLINE uint32_t TIMER_GetCounter(TIMER_T *timer)
{
    return timer->CNT;
}

/**
  * @brief This function set the timer counter mode.
  * @param[in] timer The base address of Timer module
  * @param[in] u32CntMode count mode select, \ref TIMER_CntModeDef
  * @return Timer counter value 
  */
__STATIC_INLINE void TIMER_SetTmrCounterMode(TIMER_T *timer,uint32_t u32CntMode)
{
    timer->CTL &= ~(TIMER_CTL_OPMODE_Msk);
    timer->CTL |= u32CntMode;
}

/**
  * @brief This API is used to configure timer to operate in specified mode
  *        and frequency. If timer cannot work in target frequency, a closest
  *        frequency will be chose and returned.
  * @param[in] timer The base address of Timer module
  * @param[in] idx comparator sequence number. Could be
  *             - \ref TMR0_COMPARATOR_SEL_CMP 
  *             - \ref TMR0_COMPARATOR_SEL_CMP1
  *             - \ref TMR0_COMPARATOR_SEL_CMP2
  * @param[in] u32Value  Timer compare value.
  * 			timer0--> Valid values are between 2 to 0xFFFFFFFF
  * 			timer1--> Valid values are between 2 to 0xFFFFFF
  * 			timer2--> Valid values are between 2 to 0xFFFFFF
  * @return None
  * \hideinitializer
  */



uint32_t TIMER_Open(TIMER_T *timer, TIMER_CntModeDef cntMode, uint32_t u32Freq);

/**
  * @brief This API stops Timer counting and disable the Timer interrupt function
  * @param[in] timer The base address of Timer module
  * @return None
  */
void TIMER_Close(TIMER_T *timer);

/**
  * @brief This API is used to create a delay loop for u32usec micro seconds
  * @param[in] timer The base address of Timer module
  * @param[in] u32Usec Delay period in micro seconds with 10 usec every step. Valid values are between 10~1000000 (10 micro second ~ 1 second)
  * @return None
  * @note This API overwrites the register setting of the timer used to count the delay time.
  * @note This API use polling mode. So there is no need to enable interrupt for the timer module used to generate delay
  */
void TIMER_Delay(TIMER_T *timer, uint32_t u32Usec);


/**
  * @brief This function is used to set new Timer compared value
  * @param[in] timer The base address of Timer module
  * @param[in] cmp_seq The Timer comparator sequence (e.g., TMR0_COMPARATOR_SEL_CMP0).
  * @param[in] u32Value  Timer compare value. Valid values are between 2 to 0xFFFFFF
  * @return None
  */
void TIMER_SetCmpValue(TIMER_T *timer, TIMER0_CmpSelDef cmp_seq, uint32_t u32Value);

/**@} */


#ifdef __cplusplus
}
#endif

#endif //__PAN_TIMER_H__

/*** (C) COPYRIGHT 2023 Panchip Technology Corp. ***/
