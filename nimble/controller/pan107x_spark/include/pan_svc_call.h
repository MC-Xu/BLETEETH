#ifndef __PAN_SVC_CALL_H
#define __PAN_SVC_CALL_H

#include "defs_types.h"
#include "utils/track.h"

typedef int (*printk_handle)(char *fmt, ...);
typedef uint32_t (*measure_rcl_32k_handle)(uint32_t dst);
typedef uint32_t (*ll_clock_changed_cb)(uint32_t source);

extern printk_handle printk;
extern measure_rcl_32k_handle measure_rcl_32k_start;
extern measure_rcl_32k_handle measure_rcl_32k_stop;
extern volatile ll_clock_changed_cb m_ll_clk_cb;

extern uint32_t maxACLLen;

typedef enum {
	SVC_ANA_RF_LDO_DIS,
	SVC_LL_HW_CHECK_SLEEP_STATE,
	SVC_LL_REAMIN_GET,
	SVC_LL_SET_TX_POWER,
	SVC_LL_INTF_RESET,
	SVC_LL_INTF_LE_TEST_END,
	SVC_LL_INTF_LE_RESEIVE_V2,
	SVC_LL_INTF_LE_TRANSMITTER_TEST_V2,
	SVC_LL_INTF_CONSTANT_TONE_TEST_V1,
	SVC_LL_SCAN,
	SVC_LL_ADV_DATA_FILTER,
	SVC_LL_ADV_TYPE_FILTER,
	SVC_LL_DEBUG,
	SVC_SYS_PRK,
	SVC_LL_PHY_DEBUG,
	SVC_LL_DONE,
	SVC_LL_EVENT_COUNT_GET,
	SVC_RC_FRE_FUNCTION,
	SVC_MESH_OBSERVER_ENABLE,
	SVC_LL_SCAN_GOON_SWITCH,
	SVC_BLE_FEATURE_SET_MASK,
	SVC_BLE_EVT_DONE_CB,
	SVC_CLOCK_SOURCE_SWITCH,
	SVC_LL_CONFIG_SET,
	SVC_LL_DATA_EXTENSION_LENGTH_VALUE,
    SVC_CONN_RSSI_READ,
	SVC_TX_POWER_CHANGE_SUBSCRIBE,
    SVC_LATENCY_USER_SET,
    SVC_LATENCY_ENABLE_SCH_SOON,
	SVC_LL_ENABLE_RX_TIME,
	SVC_UPD_STIMER,
	SVC_GET_LL_IDLE_TIME,
	SVC_LL_PM_POST_HANDLER,
	SVC_REGISTER_SEMPHR_CBACK,
}PAN_SVC_OPCODE;

enum {
	CMD_SIGNAL_GET,
	CMD_RESP_GET,
	ACL_SIGNAL_GET,
	ACL_RESP_GET,
};

typedef struct {
	uint8_t rx_channel;
	uint8_t phy;
	uint8_t modulation_index;
} rx_test_parameters;

typedef struct {
	uint8_t tx_channel;
	uint8_t length_of_test_data;
	uint8_t packet_payload;
	uint8_t phy;
} tx_test_parameters;

typedef struct {
	uint8_t index_1;
	uint8_t index_2;
	uint8_t index_3;
} scan_channel_map;

/*
 * | bit    | 0      | 8    | 16     | 24   | 32     | 40   | 48     | 56   | 64     | 72   | 80     | 88   | 96     | 104  | 112    | 120  |
 * | ------ | ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- |
 * | index  | 0      | 0    | 1      | 1    | 2      | 2    | 3      | 3    | 4      | 4    | 5      | 5    | 6      | 6    | 7      | 7    |
 * | params | length | type | length | type | length | type | length | type | length | type | length | type | length | type | length | type |
 */
typedef struct {
	uint8_t enable;
	uint8_t index;
	uint8_t len;
	uint8_t type;
} adv_date_filter_t;

#define FIL_ADV_IND_TYPE        			(1 << 0)
#define FIL_ADV_NONCONN_IND_TYPE			(1 << 2)
#define FIL_SCAN_RSP_TYPE        			(1 << 4)
#define FIL_ADV_SCAN_IND_TYPE    			(1 << 6)
#define FIL_ADV_EXT_IND_OR_AUX_ADV_TYPE    	(1 << 7)

typedef struct {
	uint8_t enable;
	uint8_t type;
} adv_type_fi_t;

typedef struct {
	uint8_t type;
	uint8_t page;
	uint8_t address;
	uint8_t *data;
} ll_phy_debug;

typedef struct {
	uint32_t sch_pin;
	uint32_t scan_pin;
	uint32_t adv_pin;
	uint32_t conn_pin[DBG_PIN_NUM];
	uint32_t timer_irq_pin;
	uint32_t ll_irq_pin;
} debug_io_map;

typedef struct {
	uint32_t (*p_fun1)(uint32_t counts);
	uint32_t (*p_fun2)(uint32_t counts);
}rc_measure_t;

typedef struct {
	uint32_t dst_source;
	ll_clock_changed_cb cb;
}clock_switch_t;

uint32_t pan_svc_interface(PAN_SVC_OPCODE opcode, const void *p_data);

#endif
