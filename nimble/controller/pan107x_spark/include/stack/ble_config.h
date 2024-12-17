/**
 *******************************************************************************
 * @FileName  : ble_config.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-09-10
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms desc ribed
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */

#ifndef BLE_CONFIG_H_
#define BLE_CONFIG_H_

/*--------------------------------- Private ----------------------------------*/

/*! Handle connect parameter request/channel map update/PHY update in IRQ.(unused) */
#define HANDLE_UPDATE_REQ_IN_IRQ_EN   0

/*! RAM optimization enable/disable. */
#define RAM_OPTIMIZATION_EN           0
#define RAM_OPTIMIZATION2_EN          1 //save 264B

/*! Flash optimization enable/disable. */
#define FLASH_OPTIMIZATION_EN         1

/*! Extended Adv enable/disable. */
#define LE_EXT_ADV_EN                 0

/*! New schedule scheme. */
#define SCH_NEW_SCHEME_EN             1
#define SCH_NEW_SCHEME_2_EN           1

/*! ANT Enable */
#define ANT_EN                        1 //increase RAM 816B, Flash 1K

/*------------------------------ Link Layer ----------------------------------*/
/* --- Core Spec 4.0 --- */
#define LE_FEAT_ENCRYPTION_EN                          (1)   /*!< Encryption supported. */
/* --- Core Spec 4.2 --- */
#define LE_FEAT_CONN_PARAM_REQ_PROC_EN                 (0)   /*!< Connection Parameters Request Procedure supported. */
#define LE_FEAT_EXT_REJECT_IND_EN                      (1)   /*!< Extended Reject Indication supported. */
#define LE_FEAT_SLV_INIT_FEAT_EXCH_EN                  (1)   /*!< Slave-Initiated Features Exchange supported. */
#define LE_FEAT_LE_PING_EN                             (1)   /*!< LE Ping supported. */
#define LE_FEAT_DATA_LEN_EXT_EN                        (1)   /*!< Data Length Extension supported. */
#define LE_FEAT_PRIVACY_EN                             (0)   /*!< LL Privacy supported. */
#define LE_FEAT_EXT_SCAN_FILT_POLICY_EN                (0)   /*!< Extended Scan Filter Policy supported. */
/* --- Core Spec 5.0 --- */
#define LE_FEAT_LE_2M_PHY_EN                           (1)   /*!< LE 2M PHY supported. */
#define LE_FEAT_STABLE_MOD_IDX_TRANSMITTER_EN          (0)   /*!< Stable Modulation Index - Transmitter supported. */
#define LE_FEAT_STABLE_MOD_IDX_RECEIVER_EN             (0)   /*!< Stable Modulation Index - Receiver supported. */
#define LE_FEAT_LE_CODED_PHY_EN                        (1)   /*!< LE Coded PHY supported. */
#define LE_FEAT_LE_EXT_ADV_EN                          (0)   /*!< LE Extended Advertising supported. */
#define LE_FEAT_LE_PER_ADV_EN                          (0)   /*!< LE Periodic Advertising supported. */
#define LE_FEAT_CH_SEL_2_EN                            (1)   /*!< Channel Selection Algorithm #2 supported. */
#define LE_FEAT_LE_POWER_CLASS_1_EN                    (0)   /*!< LE Power Class 1 supported. */
#define LE_FEAT_MIN_NUM_USED_CHAN_EN                   (0)   /*!< Minimum Number of Used Channels supported. */
/* --- Private Feature --- */
#define LE_FEAT_DTM_EN                                 (1)   /*!< RF Direct Test Mode supported. */

/*! Scan dup filter. */
#define SCAN_DUP_FILT_NUM             8

#define SCAN_FIFO_NUM                 8

/*! Scan can be preempted. */
#define SCAN_CAN_BE_PREEMPTED         1

/*! Hardware CCM enable/disable */
#define HW_CCM_EN                     1
#define LL_32K_TRACK_EN               0

#define LL_NEW_DLE_PHY_EN             1
#define LL_BQB_EN                     0

/*! Enable/Disable dynamic update local SCA function. */
#define LL_DYNAMIC_UPD_SCA_EN         1

/*! HCI VS Command Support [Specific for PanChip] */
#define HCI_VS_CMD_PRF_EN             0
#define HCI_VS_CMD_MODEM_EN           0
#define HCI_VS_CMD_PCTE_EN            0
#define HCI_VS_CMD_DEBUG_EN           1

/*------------------------------- ble common ------------------------*/
#include "utils/utils.h"
#define ble_memcpy                     xmemcpy
#define ble_memcmp                     xmemcmp
#define ble_memset                     xmemset

/*------------------------------ BB ----------------------------------*/
//#include "stack/controller/bb_api.h"
#define GetCurSysTick()                 BB_GetCurTick()   //SysTime_GetCurTick()
#define SetSysCapture(x)                BB_SetTimerCmp(x) //SysTime_SetCurCmp(x)
#define GetSysCapture()                 BB_GetTimerCmp()  //SysTime_GetCurCmp()

#define EnableSysTickIrq()              BB_EnableTimerIrq(BB_TIMER_IT_CMP) //SysTime_EnableIrq()
#define DisableSysTickIrq()             {BB_DisableTimerIrq(BB_TIMER_IT_CMP);BB_SetTimerCmpEnable(0);}  //SysTime_DisableIrq()

#define ClearSysTimerITFlag()           BB_ClearTimerIrqFlag(BB_TIMER_IT_Clr_CMP); //SysTime_ClearIrqState()

#define IsTimeExpired(refTick, timeUs)  BB_IsTimeExpired(refTick, timeUs) //SysTimer_IsTimeExpired(refTick, timeUs)


#endif /* BLE_CONFIG_H_ */
