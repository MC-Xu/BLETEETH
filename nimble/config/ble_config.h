#ifndef BLE_CONFIG_H_
#define BLE_CONFIG_H_


/* sdk_cofig.h can overide the macro defined in this file */
#include "sdk_config.h"
#include "soc_config.h"

/*
 * Here defines macro related to ble controller and parts of host,
 * user can redefine the macro using sdk_config.h.
 * ---> warnning do not change macro in this file <---- 
 */

/* if enable, ble use chip unique mac addr */
#ifndef CONFIG_USER_CHIP_MAC_ADDR
#define CONFIG_USER_CHIP_MAC_ADDR               0
#endif

/* select the max number of central roles which the device support */
#ifndef CONFIG_BT_MAX_NUM_OF_CENTRAL
#define CONFIG_BT_MAX_NUM_OF_CENTRAL            0
#endif

/* select the max number of peripheral roles which the device support */
#ifndef CONFIG_BT_MAX_NUM_OF_PERIPHERAL
#define CONFIG_BT_MAX_NUM_OF_PERIPHERAL         1
#endif

/* define bluetooth tx power {-40->9dbm} */
#ifndef CONFIG_BT_CTLR_TX_POWER_DFT
#define CONFIG_BT_CTLR_TX_POWER_DFT         	0
#endif

/* force calibration of the rcl clock */
#ifndef CONFIG_FORCE_CALIB_RCL_CLK
#define CONFIG_FORCE_CALIB_RCL_CLK         		0
#endif

/* select agc mode */
#ifndef CONFIG_BT_CTLR_AGC_MODE
#define CONFIG_BT_CTLR_AGC_MODE         		0
#endif

/* select bluetooth controller ppm depended on 32K clock source
 * 0 means rcl
 * 1 means xtl
 * 2 means act32
 */
#ifndef CONFIG_BT_CTLR_SCA
#if (CONFIG_LOW_SPEED_CLOCK_SRC == 0)
#define CONFIG_BT_CTLR_SCA                     700
#else
#define CONFIG_BT_CTLR_SCA                     50
#endif
#endif

/* this is used to debug link layer time from GPIO,
 * and gpio are defined in feature
 */
#ifndef CONFIG_BT_CTLR_LINK_LAYER_DEBUG
#define CONFIG_BT_CTLR_LINK_LAYER_DEBUG         0
#endif

/* this is used to debug the controller memory consuming */
#ifndef CONFIG_BT_CTLR_MEM_POOL_PRINT
#define CONFIG_BT_CTLR_MEM_POOL_PRINT         	0
#endif


/* BLE Controller RF RX Buffer Number (must be a power of 2) */
#ifndef CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM
#define CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM    (16)
#endif

/* BLE Controller RF TX Buffer Number (must be a power of 2) */
#ifndef CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM
#define CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM    (16)
#endif

/* BLE Controller Packet Encrypt Time(unit:us) */
#ifndef CONFIG_BLE_CONTROLLER_LL_ENC_TIME
#define CONFIG_BLE_CONTROLLER_LL_ENC_TIME		(100)
#endif

/* BLE Controller More Data Number */
#ifndef CONFIG_BLE_CONTROLLER_MORE_DATA_NUM
#define CONFIG_BLE_CONTROLLER_MORE_DATA_NUM		(6)
#endif
 
/* BLE Controller WhiteList Number */
#ifndef CONFIG_BLE_CONTROLLER_WIHTELIST_NUM
#define CONFIG_BLE_CONTROLLER_WIHTELIST_NUM     (1)
#endif

/* BLE Controller Resolving List Number */
#ifndef CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM
#define CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM    (0)
#endif

/* BLE Controller Master Link Margin(unit:0.625ms) */
#ifndef CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN
#define CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN  (6)
#endif

#endif /* End of BLE_CONFIG_H_ */
