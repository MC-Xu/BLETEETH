/*****************************************************************************
 * @file     pan_ble_stack.h
 * @version  V1.00
 * $Revision: 1$
 * $Date: 20/04/14 14:34 $ 
 * @brief    PN108 BLE Stack header file
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __PAN_BLE_STACK_H__
#define __PAN_BLE_STACK_H__

/* Includes -----------------------------------------------------------------*/
#include <stdint.h>
#ifndef NIMBLE_SPARK_SUP
#include "stack/controller/lhci_api.h"
#endif

#include "stack/ant/ant_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported  Defines --------------------------------------------------------*/
enum sleep_clock_source_t {
    SLEEP_CLOCK_SOURCE_RCL      = 0,
    SLEEP_CLOCK_SOURCE_XTL      = 1,
    SLEEP_CLOCK_SOURCE_ACT32K   = 2, // sleep disabled
};

enum sleep_clock_accuracy_t {
    SLEEP_CLOCK_ACCUARCY_500PPM = 0,
    SLEEP_CLOCK_ACCUARCY_250PPM = 1,
    SLEEP_CLOCK_ACCUARCY_150PPM = 2,
    SLEEP_CLOCK_ACCUARCY_100PPM = 3,
    SLEEP_CLOCK_ACCUARCY_75PPM  = 4,
    SLEEP_CLOCK_ACCUARCY_50PPM  = 5,
    SLEEP_CLOCK_ACCUARCY_30PPM  = 6,
    SLEEP_CLOCK_ACCUARCY_20PPM  = 7,
};

typedef struct {
    uint8_t    sleep_clock_source;
    uint16_t   sleep_clock_accuracy;
    uint8_t    max_num_of_states;
    int8_t     tx_power;

    void *     (*pf_mem_init)(uint32_t size);
    
    // NOT Release
    uint8_t     link_layer_debug;
    struct {
        uint8_t os_drift_time;
        uint8_t reg_time;
    } timings;

    uint8_t     agc_cfg_mode;

    uint8_t     wlNum;
    uint8_t     rlNum;
    uint8_t     mstMargin;
    uint8_t     numMoreData;
    uint8_t     maxMst;
    uint8_t     maxSlv;
    uint8_t     maxAclLen;
    uint8_t     numRxBufs;
    uint8_t     numTxBufs;
    uint16_t    llEncTime;

    uint8_t     pmEnable;
	uint8_t     connCreateTo;
	uint8_t     authPyldTimeoutEn;

	uint8_t     numAdvSet;
	uint16_t    extAdvDataLen;
	uint16_t    extScanRspDataLen;
	uint8_t     advMargin;
} pan_ble_cfg;

typedef struct {
    uint8_t   major;
    uint8_t   minor;
    uint16_t  patch;
    uint32_t  commit_id;
} pan_ble_version_t;

typedef int (*PanMdcSemphrGiveCback_t)(void);

#define PAN_BLE_STACK_API __attribute__((visibility ("default")))

typedef void (*pan_ble_hci_evt_cbk)(uint8_t *p_evt, uint16_t evt_len);
typedef void (*pan_ble_hci_acl_cbk)(uint8_t *p_acl, uint16_t acl_len);

/* Exported functions -------------------------------------------------------*/


/**
 * @brief PAN BLE Stack Initialization.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_init(const pan_ble_cfg *cfg);

/**
 * @brief PAN BLE Stack De-initialize.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_uninit(void);

/**
 * @brief PAN BLE CFG Update.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_cfg_set(uint8_t phy_cfg);

/**
 * @brief Process the PAN BLE before sleep.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_sleep(uint32_t level);

/**
 * @brief Process the PAN BLE after wakeup.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_wakeup(uint32_t source);

/**
 * @brief Process the PAN BLE interrupt.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_irq(void);

/**
 * @brief Process the PAN BLE Stack.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_handle(void);

/**
 * @brief HCI Initialization.
 *
 * @param p_evt_cbk Pointer to the callback which process the 
 *      HCI event (exclude command status or command complete) 
 *      packet to the host.
 * @param p_acl_cbk Pointer to the callback which process the 
 *      HCI ACL packet to the host.
 * 
 * @return N/A
 */
PAN_BLE_STACK_API void pan_ble_hci_init(pan_ble_hci_evt_cbk p_evt_cbk,
                              pan_ble_hci_acl_cbk p_acl_cbk);

/**
 * @brief Process the HCI command packet from the host.
 *
 * @param p_cmd Pointer to the HCI command packet.
 * @param cmd_len Length of the HCI command packet.
 * @param p_evt Pointer to the HCI event (command status or command 
 *      complete) packet to host.
 * @param p_evt_len Pointer to the Length of the HCI event (command 
 *      status or command complete).
 *
 * Sample HCI_Reset_Command:
 *      input p_cmd "\x03\x0C\x00", cmd_len 3, 
 *      output p_evt "\x0E\x04\x01\x03\x0C\x00", *p_evt_len 6
 * 
 * @return Command status to be sent to the Host, 
 *      see @ble_error_codes.h.
 */
PAN_BLE_STACK_API uint32_t pan_ble_hci_cmd_handle(uint8_t  *p_cmd,
                                        uint16_t  cmd_len,
                                        uint8_t  *p_evt,
                                        uint16_t *p_evt_len);
/**
 * @brief Process the HCI ACL packet from the host.
 *
 * @param p_data Pointer to the HCI ACL packet.
 * @param data_len Length of the HCI ACL packet.
 * 
 * Sample HCI_ACL_Data:
 *      input p_data "00 00 01 00 00", data_len 5
 *      means LLID: Start, Payload Length: 1, Payload: "\x00"
 * 
 * @return Command status to be sent to the Host, 
 *      see @ble_error_codes.h.
 */
PAN_BLE_STACK_API uint32_t pan_ble_hci_acl_handle(uint8_t *p_data,
                                        uint16_t data_len);

//[GQ] add
PAN_BLE_STACK_API uint32_t pan_get_ll_idle_time(void);
PAN_BLE_STACK_API void pan_ll_pm_post_handler(void);
PAN_BLE_STACK_API void pan_update_stimer(void);
PAN_BLE_STACK_API void pan_ll_register_semphr_cback(PanMdcSemphrGiveCback_t func);

//ANT API
void pan_ant_init(void);
int  pan_ant_create(ab_event_node_t *pParam);
void pan_ant_delete(int id);

//misc API
extern void  pan_req_new_tx_power(int8_t txPower);
extern void  pan_misc_register_print(void *print);
extern void *pan_misc_malloc_ll_data_ram(uint32_t size);
extern void  pan_misc_upd_local_sca(uint16_t localSca);
extern void  pan_misc_set_bd_addr(uint8_t addr[6]);
extern void  pan_misc_get_bd_addr(uint8_t addr[6]);
extern uint32_t pan_misc_get_cur_sys_tick(void);


PAN_BLE_STACK_API const pan_ble_version_t *pan_ble_get_version(void);

uint8_t event_can_handle(uint8_t hw_done);
void abstract_ll_event_register(void *event_node);
uint32_t abstract_ll_event_unregister(uint32_t event_handle);
void abstract_hw_completed(void *ptr_env_done);
uint8_t ab_event_on_air(void);

void event_time_init(void *p_event_time);

#ifdef __cplusplus
}
#endif

#endif /* __PAN_BLE_STACK_H__ */

