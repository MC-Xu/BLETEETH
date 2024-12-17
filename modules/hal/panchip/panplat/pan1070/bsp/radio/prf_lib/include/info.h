#ifndef __INFO_H__
#define __INFO_H__

#include "PANSeries.h"

#define CONFIG_SIMULATE_WRITE_INFO                               0
#define CONFIG_UART_OUTPUT_INFO                                  0

#define INFO_VERSION_16_BITS                                0x0004

#define INFO_TYPE_TX_PWR                                         0
#define INFO_TYPE_PHY_INIT                                       1
#define INFO_TYPE_POST_TX_CONFIG                                 2
#define INFO_TYPE_POST_RX_CONFIG                                 3
#define INFO_TYPE_PRE_TX_CONFIG_PRF                              4
#define INFO_TYPE_PRE_RX_CONFIG_PRF                              5
#define INFO_TYPE_PRE_TX_CONFIG_2M_PRF                           6
#define INFO_TYPE_PRE_RX_CONFIG_2M_PRF                           7
#define INFO_TYPE_PRE_TX_CONFIG_LR                               8
#define INFO_TYPE_PRE_RX_CONFIG_LR                               9
#define INFO_TYPE_PRE_TX_CONFIG_250K_PRF                         10
#define INFO_TYPE_PRE_RX_CONFIG_250K_PRF                         11
#define INFO_TYPE_PRE_EVNT_CONFIG                                12
#define INFO_TYPE_POST_EVNT_CONFIG                               13
#define INFO_TYPE_DCOC_LIMIT                                     14
#define INFO_TYPE_CONSTANT_TONE                                  15
#define INFO_TYPE_MAX                                            16

#define INFO_HEADER_LEN                                           4
#define TLV_HEAD_LEN                                              3
#define TLV_TYPE_LEN                                              1
#define TLV_LEN_LEN                                               2

#define INFO_TOTAL_LEN_MAX                                        (3*1024 - 4)

#define INFO_READ_BUFFER_LEN                                      512
#define INFO_ITER_READ_BUFFER_LEN                                 32

#define CRC_LEN                                                   4

#define INFO_READ_OFFSET                                          0x400
#define INFO_READ_SIZE                                            0xC00

extern uint16_t info_value_size(void);

extern void info_value_tlv_value_output(void);

extern void send_raw_value(uint8_t* ptr, uint32_t len);

extern uint16_t get_info_crc(void);

extern void total_tlv_data_output(void);

extern void send_uart_raw_value(uint8_t* ptr, uint32_t len);

extern void info_value_tlv_value_uart_output(void);

extern void total_tlv_uart_data_output(void);

bool check_info_tlv_data_prf(void);

extern void start_overwrite_info(void);

extern void start_read_info(void);

extern void start_read_1k_info(void);

extern uint16_t info_read_from_type_prf(uint8_t type, void* data, uint16_t len, uint16_t offset);

extern bool info_data_is_valid_prf(void);

extern bool check_info_is_empyt_prf(void);

extern uint16_t phy_value_init_from_code_prf(void);
extern uint16_t phy_value_init_from_info_prf(void);

struct tlv_item {
    uint16_t addr;
    uint16_t len;
};

extern const uint8_t dcoc_init_limit_prf[];
extern struct tlv_item tlv_items_prf[INFO_TYPE_MAX];

extern uint32_t* g_phy_pre_tx_1M_ptr_prf;
extern uint32_t* g_phy_pre_tx_2M_ptr_prf;
extern uint32_t* g_phy_pre_tx_LR_ptr_prf;
extern uint32_t* g_phy_pre_tx_250K_ptr_prf;
extern uint32_t* g_phy_pre_rx_1M_ptr_prf;
extern uint32_t* g_phy_pre_rx_2M_ptr_prf;
extern uint32_t* g_phy_pre_rx_LR_ptr_prf;
extern uint32_t* g_phy_pre_rx_250K_ptr_prf;
extern uint32_t* g_phy_post_tx_ptr_prf;
extern uint32_t* g_phy_post_rx_ptr_prf;
extern uint32_t* g_phy_pre_event_ptr_prf;
extern uint32_t* g_phy_post_event_ptr_prf;
extern uint32_t* g_phy_constant_tone_ptr_prf;

#endif
