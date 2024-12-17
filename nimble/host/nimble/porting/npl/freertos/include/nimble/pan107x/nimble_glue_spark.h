#ifndef _NIMBLE_GLUE_H_
#define _NIMBLE_GLUE_H_

#include "PanSeries.h"
#include "stack/ble_types.h"
#include "pan_ble_stack.h"

/* Here we borrow NVIC ADC IRQ as BLE event for LL interrupt offloading */
#define BLE_EVENT_PROC_IRQ    ADC_IRQHandler
#define BLE_EVENT_PROC_IRQn   ADC_IRQn

// Priority used for thread control.
typedef enum {
    os_priority_high,
    os_priority_normal,
    os_priority_low,
} os_priority;


extern void pan_ble_init(const pan_ble_cfg *cfg);
extern void pan_ble_handle(void);
extern void pan_ble_irq(void);

extern uint32_t pan_ble_hci_acl_nimble_handle(uint8_t *p_data, uint16_t data_len);
typedef int (*host_copydata_t)(void *from, void *dst, uint16_t len);
extern void pan_ll_register_hostcopy_cb(host_copydata_t func);


/* Misc API */
extern uint32_t db_set_bd_address(uint8_t *bd_addr);
extern uint8_t rf_check_sleep_state(void);

#endif  /* _NIMBLE_GLUE_H_ */
