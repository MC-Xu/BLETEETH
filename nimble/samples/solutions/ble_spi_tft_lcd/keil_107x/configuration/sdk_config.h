#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

//*** <<< Use Configuration Wizard in Context Menu >>> ***

// ==========================================================
// <h> SoC Platform

// <o> Chip Power Mode
// <0=> LDO
// <1=> DCDC
#define CONFIG_SOC_DCDC_PAN1070                             1

// <o> System Clock
// <48=> 48 MHz (DPLL)
// <32=> 32 MHz (DPLL)
// <i> System main frequency, Unit MHz
#define CONFIG_SYSTEM_CLOCK                                 32

// <o> APB1 Clock Divisor
// <0=> No Divider
// <2=> 2
// <4=> 4
// <6=> 6
// <8=> 8
// <10=> 10
// <12=> 12
// <14=> 14
// <16=> 16
// <i> Divisor of peripheral clocks on APB1, It can only be even numbers.
#define CONFIG_APB1_CLOCK_DIVISOR                           2

// <o> APB2 Clock Divisor
// <0=> No Divider
// <2=> 2
// <4=> 4
// <6=> 6
// <8=> 8
// <10=> 10
// <12=> 12
// <14=> 14
// <16=> 16
// <i> Divisor of peripheral clocks on APB2, It can only be even numbers.
#define CONFIG_APB2_CLOCK_DIVISOR                           2

// <o> 32K Low-Speed Clock Source
// <0=> RCL (32000 Hz)
// <1=> XTL (32768 Hz)
// <2=> ACT32K (32000 Hz)
// <i> Select a low-speed clock source
#define CONFIG_LOW_SPEED_CLOCK_SRC                          2

// <q> Force Calib RCL Clock
// <i> Force calibrate the 32K RCL clock at system init stage.
// <i> NOTE this only take effect when the Low-Speed Clock Source is seleted to RCL.
#define CONFIG_FORCE_CALIB_RCL_CLK                          0

// <e> Enable UART Log
#define CONFIG_UART_LOG_ENABLE                              1

// <o> Log UART Tx Pin
// <0=> P05 (UART0)
// <1=> P11 (UART0)
// <2=> P16 (UART0)
// <3=> P01 (UART1)
// <4=> P10 (UART1)
// <5=> P12 (UART1)
// <6=> P25 (UART1)
// <7=> P31 (UART1)
// <i> Select a UART Tx pin for logging output.
#define CONFIG_LOG_UART_PIN                                 2

// <o> Log UART Baudrate
// <115200=> 115200
// <230400=> 230400
// <460800=> 460800
// <921600=> 921600
// <1000000=> 1M
// <2000000=> 2M
#define CONFIG_LOG_UART_BAUDRATE                            921600
// </e>

// <e> Enable RTT Log
// <i> Note that the Low Power Mode (CONFIG_PM) should be disabled
// <i> while using RTT log, since the Jlink SWD connnection would be lost
// <i> at SoC DeepSleep or Standby Mode.
#define CONFIG_RTT_LOG_ENABLE                               0

// <o> RTT Log Buffer Size (Bytes)
// <i> Configure Log RTT Up Buffer Size in Bytes (Channel 0).
#define CONFIG_LOG_RTT_UP_BUFFER_SIZE                       512
// </e>

// <q> Enable RAM Function
// <i> Adding essential code to SRAM could improve running performance.
#define CONFIG_RAM_FUNCTION                                 1

// <q> Enable Flash LDO
// <i> Enable the internal 1.8v flash LDO for flash power supply
// <i> instead of the default flash power from SoC VBAT.
#define CONFIG_FLASH_LDO_EN                                 1

// <q> Remap Vector Table to SRAM
#define CONFIG_VECTOR_REMAP_TO_RAM                          1

// <e> Enable Auto Power Optimization
// <i> Several power configurations could be updated due to temperature change.
#define CONFIG_AUTO_OPTIMIZE_POWER_PARAM                    0

// <o> Temperature Sample Interval (in Seconds)
#define CONFIG_TEMP_SAMPLE_INTERVAL_S                       300

// <q> Enable DVDD Voltage Optimization
#define CONFIG_DVDD_VOL_OPTIMIZE_EN                         1
// </e>

// <e> Enable Low Power Mode
#define CONFIG_PM                                           0

// <q> Enable System Watchdog
#define CONFIG_SYSTEM_WATCH_DOG_ENABLE                      0

// <q> Keep Flash Power in Low Power Mode
// <i> Select this means flash power would be retained in Low Power Mode, and
// <i> there would be a little avg-current increase (about 1uA). The benefit is that
// <i> the large peak current (>15mA) would not occur.
#define CONFIG_KEEP_FLASH_POWER_IN_LP_MODE                  1

// <q> Enable DeepSleep Mode 2
// <i> Enable DeepSleep Mode 2 (Only LPLDOH in use), and the HW APB Timer Wakeup
// <i> and PWM waveform output can be use in this mode.
#define CONFIG_DEEPSLEEP_MODE_2                             0

// <o> Increase LPLDOH trim value
// <0=> +0
// <1=> +1
// <2=> +2
// <3=> +3
// <4=> +4
// <5=> +5
// <6=> +6
// <7=> +7
// <8=> +8
// <i> Increase LPLDOH voltage for specific LowPower scenario use.
#define CONFIG_SOC_INCREASE_LPLDOH_CALIB_CODE               0

// <q> Continue Run After Standby M1 Wakeup
// <i> Check this configuration to let CPU continue run after standby M1 waking up,
// <i> or CPU would reset after waking up from standby M1.
#define CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET           0

// <q> Enable AHB Clock Optimization
#define CONFIG_HCLK_OPTIMIZE_EN                             1
// </e>


// </h>
// ========================================================== SoC Platform


// ==========================================================
// <h> RTOS

// ******************************
// <h> Timer Thread

// <q> Use freeretos software timer
#define configUSE_TIMERS                                    1

// <o> Timer thread priority
#define configTIMER_TASK_PRIORITY                           2

// <o> Support maximun number of timer queue length
#define configTIMER_QUEUE_LENGTH                            12

// <o> Timer thread stack size
#define configTIMER_TASK_STACK_DEPTH                        128

// </h>
// ****************************** Timer Thread

// ******************************
// <h> BLE Host Thread

// <o> Host thread stack size
#define MYNEWT_VAL_BLE_HOST_THREAD_STACK_SIZE               256

// <o> Host thread prioirty
#define MYNEWT_VAL_BLE_HOST_THREAD_PRIORITY                 6

// </h>
// ****************************** BLE Host Thread

// <o> Total Heap Size
#define configTOTAL_HEAP_SIZE                               4096

// <q> Print Current Heap Usage
#define CONFIG_FREERTOS_HEAP_PRINT                          0

// <q> Enable OS Idle Hook
#define configUSE_IDLE_HOOK                                 0

// <q> Enable OS Tick Hook
#define configUSE_TICK_HOOK                                 0

// <q> Enable OS Malloc Fail Hook
#define configUSE_MALLOC_FAILED_HOOK                        1

// </h>
// ========================================================== RTOS


// ==========================================================
// <h> BLE Resource

// <o> Support maximun number of BLE Master Link
#define CONFIG_BT_MAX_NUM_OF_CENTRAL                        0

// <o> Support maximun number of BLE Slave Link
#define CONFIG_BT_MAX_NUM_OF_PERIPHERAL                     1

// <q> Support gap broadcaster role
#define MYNEWT_VAL_BLE_ROLE_BROADCASTER                     1

// <q> Support gap central role
#define MYNEWT_VAL_BLE_ROLE_CENTRAL                         0

// <q> Support gap observser role
#define MYNEWT_VAL_BLE_ROLE_OBSERVER                        0

// <q> Support gap peripheral role
#define MYNEWT_VAL_BLE_ROLE_PERIPHERAL                      1

// <o>  Support acl buffer counts receiving data from controller
#define MYNEWT_VAL_BLE_TRANSPORT_ACL_FROM_LL_COUNT          5

// <o> Support acl buffer receiving data from controller
#define MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE                   27

// <o> Support hci events counts
#define MYNEWT_VAL_BLE_TRANSPORT_EVT_COUNT                  4

// <o> Support hci discardable events counts
#define MYNEWT_VAL_BLE_TRANSPORT_EVT_DISCARDABLE_COUNT      0

// <o> Support l2cap buffer counts
#define MYNEWT_VAL_MSYS_1_BLOCK_COUNT                       4

// <o> Support l2cap buffer size
#define MYNEWT_VAL_MSYS_1_BLOCK_SIZE                        120

// <o> BLE Controller RF RX Buffer Number (must be a power of 2)
#define CONFIG_BLE_CONTROLLER_RF_RX_BUF_NUM                 16

// <o> BLE Controller RF TX Buffer Number (must be a power of 2)
#define CONFIG_BLE_CONTROLLER_RF_TX_BUF_NUM                 16

// <o> BLE Controller Packet Encrypt Time (unit:us)
#define CONFIG_BLE_CONTROLLER_LL_ENC_TIME                   100

// <o> BLE Controller More Data Number
#define CONFIG_BLE_CONTROLLER_MORE_DATA_NUM                 6

// <o> BLE Controller WhiteList Number
#define CONFIG_BLE_CONTROLLER_WIHTELIST_NUM                 1

// <o> BLE Controller Resolving List Number
#define CONFIG_BLE_CONTROLLER_RESOLVELIST_NUM               0

// <o> BLE Controller Master Link Margin (unit:0.625ms)
#define CONFIG_BLE_CONTROLLER_MASTER_LINK_MARGIN            6

// <q> Use Chip unique Mac Address
#define CONFIG_USER_CHIP_MAC_ADDR                           1

// <o> TX power
// <0=> 0dBm
// <1=> 1dBm
// <2=> 2dBm
// <3=> 3dBm
// <4=> 4dBm
// <5=> 5dBm
// <6=> 6dBm
// <7=> 7dBm
// <8=> 8dBm
// <9=> 9dBm
#define CONFIG_BT_CTLR_TX_POWER_DFT                         0

// <q> BLE Timing Debug
#define CONFIG_BT_CTLR_LINK_LAYER_DEBUG                     0

// </h>
// ========================================================== BLE Resource


// ==========================================================
// <h> BLE Security Manager

// <o> Security level selection
#define MYNEWT_VAL_BLE_SM_SC_LVL                            0

// <q> Support SM legacy pair
#define MYNEWT_VAL_BLE_SM_LEGACY                            0

// <q> Support SM security pair
#define MYNEWT_VAL_BLE_SM_SC                                0

// <o> IO capability
// <0=> DisplayOnly
// <1=> DisplayYesN
// <2=> KeyboardOnly
// <3=> NoInputNoOutput
// <4=> KeyboardDisplay Only
#define CONFIG_HS_IO_CAPABILITY                             3

// <q> Support SM Bonding
#define MYNEWT_VAL_BLE_SM_BONDING                           0

// <q> Support SM MITM
#define MYNEWT_VAL_BLE_SM_MITM                              0

// <q> Support SM oob
#define MYNEWT_VAL_BLE_SM_OOB_DATA_FLAG                     0

// <q> Support persist store key
#define MYNEWT_VAL_BLE_STORE_CONFIG_PERSIST                 0

// <o> Support maximun store bonded devices
#define MYNEWT_VAL_BLE_STORE_MAX_BONDS                      1

// <o> Support maximun store bonded device's cccd
#define MYNEWT_VAL_BLE_STORE_MAX_CCCDS                      8

// <q> Support host software rpa feature
#define MYNEWT_VAL_HOST_SOFTWARE_RPA                        0

// </h>
// ========================================================== BLE Security Manager


// ==========================================================
// <h> BLE Services



// </h>
// ========================================================== BLE Services



// <h> Flash KVStore Area
// ==========================================================

// <i> Configure Key-Value-Store (kvstore) Area on Flash
// <o> Start Address
#define CONFIG_SETTINGS_START_ADDR                          0x78000

// <o> Number of Flash Sectors (>=2)
// <i> 1 flash sector means 4096 Bytes
#define CONFIG_SETTINGS_SECTOR_NUM                          4
// </h>
// ========================================================== Flash KVStore Area


// <h> Flash Map
// ==========================================================
// <q> Support BootLoader
#define CONFIG_BOOT_ENABLE                              	0

// <o> Flash Image Size
// <0xFF000=>  1M
// <0x7F000=>  512K
// <0x3F000=>  256K

#define CONFIG_FLASH_MAP_SIZE                              	520192

// <q> Support Back-up area in OTA model
#define CONFIG_OTA_IN_APP                                   0

// </h>
// ========================================================== Flash Map

//*** <<< end of configuration section >>>    ***

#endif /* SDK_CONFIG_H */
