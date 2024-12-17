/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMM_PRF_H
#define COMM_PRF_H

#include "pan_pri_rf.h"
#include "pan_phy.h"

#ifndef __ramfunc
#define __ramfunc __attribute__((section(".ramfunc")))
#endif

#define PRF_LL_IRQ_PRIORITY     0
#define PRF_DATA_MAX_SIZE     255

#define TX_WINDOW               (5)
#define RX_WINDOW               (1)
#define RX_LEGENCY_1M   		(11)
#define RX_LEGENCY_2M   		(5)


typedef enum {
	PRF_CHIP_MODE_SEL_BLE          = 1,
	PRF_CHIP_MODE_SEL_XN297        = 2,
	PRF_CHIP_MODE_SEL_NRF		   = 3,
}prf_chip_mode_sel_t;

typedef enum PRF_ADDR_LENGTH_SEL {
	PRF_ADDR_LENGTH_SEL_3   = 3,
	PRF_ADDR_LENGTH_SEL_4   = 4,
	PRF_ADDR_LENGTH_SEL_5   = 5,
}prf_addr_length_sel_t;

typedef enum PRF_CRC_SEL {
	PRF_CRC_SEL_NOCRC       = 0,
	PRF_CRC_SEL_CRC8        = 1,
	PRF_CRC_SEL_CRC16       = 2,
	PRF_CRC_SEL_CRC24       = 3,
}prf_crc_sel_t;

typedef enum PRF_SCRAMBLE_SEL {
	PRF_SRC_SEL_NOSRC       = 0,
	PRF_SRC_SEL_EN          = 1,
}prf_scramble_sel_t;

typedef enum PRF_MODE {
	PRF_MODE_NORMAL       = 0,
	PRF_MODE_ENHANCE          = 1,
	PRF_MODE_NORMAL_M1	= 2,
}prf_mode_t;

typedef enum PRF_PHY {
	PRF_PHY_1M          = 1,
	PRF_PHY_2M          = 2,
    PRF_PHY_CODED_S8    = 3,
    PRF_PHY_CODED       = 4,
	PRF_PHY_250K        = 5,
}prf_phy_t;

typedef enum PRF_TRX_MODE {
	PRF_TX_MODE          = 0,
	PRF_RX_MODE          = 1,
}prf_trx_mode_t;

typedef enum PRF_MODE_CONF_SEL {
	PRF_BLE_CONF          = 1,
	PRF_NRF_CONF          = 2,
	PRF_G_250k_CONF       = 3,
	PRF_B_250K_CONF       = 4,
}prf_mode_conf_sel_t;

typedef enum PRF_PIPE {
	PRF_PIPE0        = (0x1<<0),
	PRF_PIPE1        = (0x1<<1),
	PRF_PIPE2        = (0x1<<2),
	PRF_PIPE3        = (0x1<<3),
	PRF_PIPE4        = (0x1<<4),
	PRF_PIPE5        = (0x1<<5),
	PRF_PIPE6        = (0x1<<6),
	PRF_PIPE7        = (0x1<<7),
}prf_pipe_t;

typedef enum PRF_TRANSFER_TYPE {
	PRF_TRF_NORMAL     = (0),
	PRF_TRF_NRF52      = (1),
	PRF_TRF_B250K      = (2),
}prf_trf_t;

typedef enum PRF_ENDIAN {
	PRF_BIG_ENDIAN          = 0,
	PRF_LITTLE_ENDIAN       = 1,
}prf_endian_t;

typedef enum PRF_ENC_DEC_MODE {
	PRF_ENCRYPT_MODE          = 0,
	PRF_DECRYPT_MODE          = 1,
}prf_enc_dec_mode_t;

typedef enum PRF_SPEED_SEL {
	PRF_TRANSFER_SPEED_LOW  = 0,
	PRF_TRANSFER_SPEED_4K   = 1,
	PRF_TRANSFER_SPEED_8K   = 2,
} prf_speed_sel_t;


typedef struct {
	uint32_t data_length;
	uint8_t data[PRF_DATA_MAX_SIZE];
}panchip_prf_payload_t;

typedef struct
{
    prf_mode_t          		work_mode;
	prf_chip_mode_sel_t		 	chip_mode;
	prf_trx_mode_t				trx_mode;

    prf_phy_t       			phy;
    prf_crc_sel_t           	crc;
	prf_scramble_sel_t			src;
	prf_mode_conf_sel_t			mode_conf;

	uint16_t 					rx_timeout;
	uint16_t 					rf_channel;
    uint8_t						tx_no_ack;
	prf_trf_t					trf_type;
	uint8_t						rx_length;

	uint8_t						sync_length;
	uint8_t						sync[5];

	int8_t						tx_power;
	uint8_t						pid_manual_flag;
	uint8_t						crc_include_sync;
	uint8_t						src_include_sync;

	prf_pipe_t					pipe;
} pan_prf_config_t;

typedef void (*RRF_CallbackFunc)(void);

typedef struct
{
    RRF_CallbackFunc			tx_cb;
	RRF_CallbackFunc			rx_timeout_cb;
	RRF_CallbackFunc			rx_cb;
	RRF_CallbackFunc			rx_crc_err_cb;
	RRF_CallbackFunc			rx_len_err_cb;
	RRF_CallbackFunc			rx_acc_adr_err_cb;
	RRF_CallbackFunc			rx_pid_err_cb;
} pan_prf_callback_t;

typedef struct {
	uint32_t major;
	uint32_t minor;
	uint32_t patch;
	uint32_t commit_id;
} pan_prf_version_t;


#define SLPTMR1_SLPTMR_CMPR_BUS_ENA             (3 << 2)
#define SLPTMR1_SLPTMR_CMPR_BUS_ENA_MSK         0x40   /* 1'b0: disable  - 1'b1: Enable */
#define SLPTMR1_SLPTMR_CMPR_BUS_ENA_SHFT        6
#define SLPTMR2_SLPTMR_CMPR_STRT_EVENT_DIN         (4<<2)
#define SLPTMR4_SLPTMR_CMPR_BUS_DIN                (6<<2)

#define CTRL_PM_PWR_STATE                       (2<<2)
#define CTRL_PM_PWR_STATE_MSK                   0x3f0
#define CTRL_PM_PWR_STATE_SHFT                  4

#define CTRL_PWR_MOD                            (2<<2)
#define CTRL_PWR_MOD_MSK                        0x7             
#define CTRL_PWR_MOD_SHFT                       0

#define CTRL_MEM_SOFT_RST_N                     (2<<2)
#define CTRL_MEM_SOFT_RST_N_MSK                 0x8
#define CTRL_MEM_SOFT_RST_N_SHFT                3

#define SLPTMR1_SLPTMR_REST_N                      (3<<2)
#define SLPTMR1_SLPTMR_REST_N_MSK                  0x4             /* active low sleep timer reset signal */
#define SLPTMR1_SLPTMR_REST_N_SHFT                 2

#define SLPTMR1_SLPTMR_ENA                         (3<<2)
#define SLPTMR1_SLPTMR_ENA_MSK                     0x2             /* This field specifies the enable for sleep timer */
#define SLPTMR1_SLPTMR_ENA_SHFT                    1

#define DSLP1_LL_PHY_DRIVER_NRST             	   (7<<2)
#define DSLP1_LL_PHY_DRIVER_NRST_MSK         	   0x2
#define DSLP1_LL_PHY_DRIVER_NRST_SHFT        	   1


#define SLPTMR1_SLPTMR_CMPR_STRT_EVENT_ENA         (3<<2)
#define SLPTMR1_SLPTMR_CMPR_STRT_EVENT_ENA_MSK     0x1             /* This field specifies the control compare units status function.  - 1'b0: disable  - 1'b1: Enable */
#define SLPTMR1_SLPTMR_CMPR_STRT_EVENT_ENA_SHFT    0


#define SLPTMR2_SLPTMR_CMPR_STRT_EVENT_DIN         (4<<2)
#define SLPTMR2_SLPTMR_CMPR_STRT_EVENT_DIN_MSK     0xffffffff      /* This field specifies the compare unit inputs (compare values). */
#define SLPTMR2_SLPTMR_CMPR_STRT_EVENT_DIN_SHFT    0


#define SLPTMR3_SLPTMR_CURRENT                     (5<<2)
#define SLPTMR3_SLPTMR_CURRENT_MSK                 0xffffffff      /* This field specifies the sleep timer current value. */
#define SLPTMR3_SLPTMR_CURRENT_SHFT                0


#define LLHWC_WRITE_MASK32_REG(base_addr, reg_ofst, mask, shift, data)		   \
	LLHWC_WRITE32_REG(base_addr, reg_ofst,					   \
			  ((LLHWC_READ32_REG((base_addr), (reg_ofst)) & ~(mask)) | \
			   ((mask) & ((data) << (shift)))))

#define LLHWC_READ_MASK32_REG(base_addr, reg_ofst, mask, shift) \
		((LLHWC_READ32_REG((base_addr), (reg_ofst)) & (mask)) >> (shift))
#define LLHWC_READ_MASK32_REG_EXT(base_addr, reg)           LLHWC_READ_MASK32_REG(base_addr, reg, reg##_MSK, reg##_SHFT)



extern uint8_t panchip_prf_data_rec(panchip_prf_payload_t *p_payload);
extern void panchip_prf_set_data(panchip_prf_payload_t *p_payload);
extern void panchip_prf_set_ack_data(panchip_prf_payload_t *p_payload);
extern void panchip_prf_init(pan_prf_config_t *p_config);
extern void panchip_prf_trx_start(void);
extern void panchip_prf_module_init(void);
extern void panchip_prf_ble_resume(void);
extern void panchip_switch_prf(pan_prf_config_t *p_config);
extern void panchip_prf_set_chn(uint16_t rf_channel);
extern void panchip_prf_trx_trans_time(pan_prf_config_t *p_config);
extern uint8_t panchip_prf_isr_resume_ble_cb(void);
extern void panchip_prf_dual_mode_start(pan_prf_config_t *prf_config);
extern uint8_t panchip_prf_ble_handler(void);
extern void panchip_prf_isr_proc(void);
extern void panchip_prf_irq_enable(void);
extern void panchip_prf_set_tx_pwr(int8_t tx_pwr);
extern void panchip_prf_module_enable(pan_prf_config_t *p_config);
extern pan_prf_version_t *panchip_prf_get_version(void);
extern void panchip_prf_mode_conf_set(prf_mode_conf_sel_t conf);
extern void panchip_prf_set_phy(prf_phy_t phy);
extern void panchip_prf_rx_length_irq_cfg(uint8_t value);

extern void panchip_prf_set_trx_mode(prf_trx_mode_t trx_mode);
extern void panchip_prf_set_work_mode(prf_mode_t work_mode);
extern void panchip_prf_set_addr(uint8_t *addr, uint8_t len, prf_pipe_t pipe, uint8_t trx_addr);
extern void panchip_prf_set_tx_noack(bool flag);
extern void panchip_prf_rx_timeout(uint16_t time);
extern void panchip_prf_rx_stop(void);
extern void panchip_prf_reset(void);
extern void panchip_prf_pid_cfg(pan_prf_config_t *prf_config, uint8_t pid);
extern void panchip_white_init_value(uint8_t value);
extern void panchip_prf_carrier_start(uint16_t  tx_channel);
extern void panchip_prf_carrier_stop(void);
extern int16_t panchip_prf_read_rssi(void);
extern void panchip_prf_enable_carrier_rssi(void);
extern void panchip_prf_disable_carrier_rssi(void);
extern int16_t panchip_prf_read_carrier_rssi(void);
extern void panchip_prf_b250k_init(void);
extern void panchip_prf_isr_init(void);
extern void panchip_prf_b250k_set_channel(uint8_t idx);
extern uint32_t panchip_prf_encrypt_decrypt(uint8_t *plain_text,
							uint16_t plain_text_len,
							uint8_t *key,
							uint8_t *iv,
							uint8_t *encry_data,
							prf_enc_dec_mode_t enc_mode);
extern void panchip_prf_rx_patch_handler(void);
extern void panchip_prf_get_current_stamp(void);
extern uint8_t panchip_prf_get_pipe(void);
extern void panchip_prf_set_crc(prf_crc_sel_t crc, uint8_t crc_include_sync);
extern void panchip_prf_set_whitening(prf_scramble_sel_t src, uint8_t src_include_sync);
extern void panchip_prf_set_endian(prf_endian_t endian);
extern void panchip_prf_reduce_trx_pre_post_delay_time(pan_prf_config_t *p_config, bool restore);
extern int16_t panchip_prf_set_trx_trans_time(pan_prf_config_t *p_config,
					      prf_speed_sel_t speed,
					      uint16_t transfer_time);

extern uint32_t data_addr_tx;
extern uint32_t data_addr_rx;
extern int8_t prf_rssi;

extern pan_prf_config_t rf_config;
extern pan_prf_callback_t isr_cb;
extern volatile bool wake_from_lp_mode;

#endif

#ifdef __cplusplus
}
#endif
