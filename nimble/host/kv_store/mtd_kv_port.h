#ifndef _MTD_KV_PORT_
#define _MTD_KV_PORT_

#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#define COMPONENT_RTOS_AWARE

/** @ref cy_rslt_t return value indicating success */
#define CY_RSLT_SUCCESS                    ((cy_rslt_t)0x00000000U)

/** Used with RTOS calls that require a timeout.  This implies the call will never timeout. */
#define CY_RTOS_NEVER_TIMEOUT ( (uint32_t)0xffffffffUL )


/** Simple macro to suppress the unused parameter warning by casting to void. */
#define CY_UNUSED_PARAMETER(x) ( (void)(x) )

typedef uint32_t cy_rslt_t;

typedef struct
{
    SemaphoreHandle_t mutex_handle;
    bool              is_recursive;
} cy_mutex_t;

typedef QueueHandle_t      cy_queue_t;
typedef SemaphoreHandle_t  cy_semaphore_t;
typedef TaskHandle_t       cy_thread_t;
typedef EventGroupHandle_t cy_event_t;
typedef TimerHandle_t      cy_timer_t;
typedef uint32_t           cy_timer_callback_arg_t;
typedef void*              cy_thread_arg_t;
typedef uint32_t           cy_time_t;
typedef BaseType_t         cy_rtos_error_t;

/** Halt the processor in the debug state
 */
static inline void CY_HALT(void)
{
    __asm("    bkpt    1");
}

#ifdef CY_ASSERT
#undef CY_ASSERT
#endif // ifdef(CY_ASSERT)

/** Utility macro when neither NDEBUG or CY_NO_ASSERT is not declared to check a condition and, if
   false, trigger a breakpoint */
#if defined(NDEBUG) || defined(CY_NO_ASSERT)
    #define CY_ASSERT(x)    do {                \
                            } while(false)
#else
    #define CY_ASSERT(x)    do {                \
                                if(!(x))        \
                                {               \
                                    CY_HALT();  \
                                }               \
                            } while(false)
#endif // defined(NDEBUG)

                            
/**
 * Define codes to identify the module from which an error originated.
 * @note This is provided as a debugging convenience tool, not as a definitive
 * list of all module IDs. Due to the distributed nature of ModusToolbox?, each
 * library has its own release schedule. It is possible that some module IDs
 * may not appear in the enumeration yet. Missing items are expected to be
 * added over time.
 */
typedef enum
{
    /** Module identifier for the SAR driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SAR = 0x0001,
    /** Module identifier for the Device Firmware Update (DFU) driver. Asset(s): (dfu) */
    CY_RSLT_MODULE_DRIVER_DFU = 0x0006,
    /** Module identifier for the Capsense driver. Asset(s): (capsense) */
    CY_RSLT_MODULE_DRIVER_CAPSENSE = 0x0007,
    /** Module identifier for the USB Device driver. Asset(s): (usbdev) */
    CY_RSLT_MODULE_DRIVER_USB_DEV = 0x0008,
    /** Module identifier for the Continuous Time Block (CTB) driver. Asset(s): (mtb-pdl-cat1,
       mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_CTB = 0x000b,
    /** Module identifier for the Crypto driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_CRYPTO = 0x000c,
    /** Module identifier for the SysPm driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SYSPM = 0x0010,
    /** Module identifier for the SysLib driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SYSLIB = 0x0011,
    /** Module identifier for the SysClk driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SYSCLK = 0x0012,
    /** Module identifier for the DMA driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_DMA = 0x0013,
    /** Module identifier for the Flash driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_FLASH = 0x0014,
    /** Module identifier for the SysInt driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SYSINT = 0x0015,
    /** Module identifier for the GPIO driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_GPIO = 0x0016,
    /** Module identifier for the Programmable Analog Sub System (PASS) driver. Asset(s):
       (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_SYSANALOG = 0x0017,
    /** Module identifier for the Continuous Time DAC (CTDAC) driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_CTDAC = 0x0019,
    /** Module identifier for the eFuse driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_EFUSE = 0x001a,
    /** Module identifier for the Emulated EEPROM driver. Asset(s): (emeeprom) */
    CY_RSLT_MODULE_DRIVER_EM_EEPROM = 0x001b,
    /** Module identifier for the Profile driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_PROFILE = 0x001e,
    /** Module identifier for the I2S driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_I2S = 0x0020,
    /** Module identifier for the IPC driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_IPC = 0x0022,
    /** Module identifier for the Low Power Comparator (LPCOMP) driver. Asset(s): (mtb-pdl-cat1,
       mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_LPCOMP = 0x0023,
    /** Module identifier for the PDM-PCM driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_PDM_PCM = 0x0026,
    /** Module identifier for the RTC driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_RTC = 0x0028,
    /** Module identifier for the Serial Communications Block (SCB) driver.
       Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SCB = 0x002a,
    /** Module identifier for the Serial Memory Interface (SMIF) driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_SMIF = 0x002c,
    /** Module identifier for the Timer/Counter/PWM (TCPWM) driver.
       Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_TCPWM = 0x002d,
    /** Module identifier for the Protection driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_PROT = 0x0030,
    /** Module identifier for the TRIGMUX driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_TRIGMUX = 0x0033,
    /** Module identifier for the WDT driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_WDT = 0x0034,
    /** Module identifier for the (WDC / MCWDT) driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_MCWDT = 0x0035,
    /** Module identifier for the LIN driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_LIN = 0x0037,
    /** Module identifier for the LVD driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_LVD = 0x0039,
    /** Module identifier for the SD_HOST driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_SD_HOST = 0x003a,
    /** Module identifier for the USBFS driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_USBFS = 0x003b,
    /** Module identifier for the DMAC driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_DMAC = 0x003f,
    /** Module identifier for the SegLCD driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SEGLCD = 0x0040,
    /** Module identifier for the CSD driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_CSD = 0x0041,
    /** Module identifier for the SmartIO driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_SMARTIO = 0x0042,
    /** Module identifier for the CSDIDAC driver. Asset(s): (csdidac) */
    CY_RSLT_MODULE_DRIVER_CSDIDAC = 0x0044,
    /** Module identifier for the CAN FD driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_CANFD = 0x0045,
    /** Module identifier for the PRA driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_PRA = 0x0046,
    /** Module identifier for the MSC driver. Asset(s): (mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_MSC = 0x0047,
    /** Module identifier for the ADC Mic driver. Asset(s): (mtb-pdl-cat1)
     *  Module identifier for the USB PD driver. Asset(s): (mtb-pdl-cat2)
     */
    CY_RSLT_MODULE_DRIVER_ADCMIC = 0x0048,
    /** Module identifier for the MSC LP driver. Asset(s): (mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_MSCLP = 0x0049,
    /** Module identifier for the Event Gen driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_EVTGEN = 0x004a,
    /** Module identifier for the SAR v2 driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_SAR2 = 0x004b,
    /** Module identifier for the Key Scan driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_KEYSCAN = 0x0072,
    /** Module identifier for the PDM-PCM v2 driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_PDM_PCM2 = 0x0073,
    /** Module identifier for the Crypto Lite driver. Asset(s): (mtb-pdl-cat1, mtb-pdl-cat2) */
    CY_RSLT_MODULE_DRIVER_CRYPTOLITE = 0x0074,
    /** Module identifier for the SysFault driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_SYSFAULT = 0x0076,
    /** Module identifier for the LVD HT driver. Asset(s): (mtb-pdl-cat1) */
    CY_RSLT_MODULE_DRIVER_LVD_HT = 0x0078,
    /** Module identifier for the WiFi Host Driver. Asset(s): (wifi-host-driver) */
    CY_RSLT_MODULE_DRIVER_WHD = 0x0080,

    /** Module identifier for the Hardware Abstraction Layer.
       Asset(s): (mtb-hal-cat1, mtb-hal-cat2, ...) */
    CY_RSLT_MODULE_ABSTRACTION_HAL = 0x0100,
    /** Module identifier for board support package. Asset(s): (BSPs) */
    CY_RSLT_MODULE_ABSTRACTION_BSP = 0x0180,
    /** Module identifier for file system abstraction */
    CY_RSLT_MODULE_ABSTRACTION_FS = 0x0181,
    /** Module identifier for resource abstraction. Asset(s): (abstraction-resource) */
    CY_RSLT_MODULE_ABSTRACTION_RESOURCE = 0x0182,
    /** Module identifier for RTOS abstraction. Asset(s): (abstraction-rtos) */
    CY_RSLT_MODULE_ABSTRACTION_OS = 0x0183,

    /** Module identifier for the Retarget IO Board Library. Asset(s): (retarget-io) */
    CY_RSLT_MODULE_BOARD_LIB_RETARGET_IO = 0x1A0,
    /** Module identifier for the RGB LED Board Library. Asset(s): (rgb-led) */
    CY_RSLT_MODULE_BOARD_LIB_RGB_LED = 0x01A1,
    /** Module identifier for the Serial Flash Board Library. Asset(s): (serial-flash) */
    CY_RSLT_MODULE_BOARD_LIB_SERIAL_FLASH = 0x01A2,
    /** Module identifier for the WiFi Host Driver + Board Support Integration Library.
       Asset(s): (whd-bsp-integration) */
    CY_RSLT_MODULE_BOARD_LIB_WHD_INTEGRATION = 0x01A3,

    /** Module identifier for Shield Board CY8CKIT-028-EPD. Asset(s): (CY8CKIT-028-EPD) */
    CY_RSLT_MODULE_BOARD_SHIELD_028_EPD = 0x01B8,
    /** Module identifier for Shield Board CY8CKIT-028-TFT. Asset(s): (CY8CKIT-028-TFT) */
    CY_RSLT_MODULE_BOARD_SHIELD_028_TFT = 0x01B9,
    /** Module identifier for Shield Board CY8CKIT-032. Asset(s): (CY8CKIT-032) */
    CY_RSLT_MODULE_BOARD_SHIELD_032 = 0x01BA,
    /** Module identifier for Shield Board CY8CKIT-028-SENSE. Asset(s): (CY8CKIT-028-SENSE) */
    CY_RSLT_MODULE_BOARD_SHIELD_028_SENSE = 0x01BB,

    /** Module identifier for the BMI160 Motion Sensor Library. Asset(s): (sensor-motion-bmi160) */
    CY_RSLT_MODULE_BOARD_HARDWARE_BMI160 = 0x01C0,
    /** Module identifier for the E2271CS021 E-Ink Controller Library.
       Asset(s): (display-eink-e2271cs021) */
    CY_RSLT_MODULE_BOARD_HARDWARE_E2271CS021 = 0x01C1,
    /** Module identifier for the NTC GPIO Thermistor Library. Asset(s): (thermistor) */
    CY_RSLT_MODULE_BOARD_HARDWARE_THERMISTOR = 0x01C2,
    /** Module identifier for the SSD1306 OLED Controller Library.
       Asset(s): (display-oled-ssd1306) */
    CY_RSLT_MODULE_BOARD_HARDWARE_SSD1306 = 0x01C3,
    /** Module identifier for the ST7789V TFT Controller Library. Asset(s): (display-tft-st7789v) */
    CY_RSLT_MODULE_BOARD_HARDWARE_ST7789V = 0x01C4,
    /** Module identifier for the Light Sensor Library. Asset(s): (sensor-light) */
    CY_RSLT_MODULE_BOARD_HARDWARE_LIGHT_SENSOR = 0x01C5,
    /** Module identifier for the AK4954A Audio Codec Library. Asset(s): (audio-codec-ak4954a) */
    CY_RSLT_MODULE_BOARD_HARDWARE_AK4954A = 0x01C6,
    /** Module identifier for the BMI160 Motion Sensor Library.
       Asset(s): (sensor-orientation-bmx160) */
    CY_RSLT_MODULE_BOARD_HARDWARE_BMX160 = 0x01C7,
    /** Module identifier for the XENSIV? DPS3XX Pressure Sensor Library.
       Asset(s): (sensor-xensiv-dps3xx) */
    CY_RSLT_MODULE_BOARD_HARDWARE_DPS3XX = 0x01C8,
    /** Module identifier for the WM8960 Audio Codec Library. Asset(s): (audio-codec-wm8960) */
    CY_RSLT_MODULE_BOARD_HARDWARE_WM8960 = 0x01C9,
    /** Module identifier for the XENSIV? PAS CO2 Sensor Library.
       Asset(s): (sensor-xensiv-pasco2) */
    CY_RSLT_MODULE_BOARD_HARDWARE_XENSIV_PASCO2 = 0x01CA,
    /** Module identifier for the XENSIV? BGT60TRxx Sensor Library.
       Asset(s): (sensor-xensiv-bgt60trxx) */
    CY_RSLT_MODULE_BOARD_HARDWARE_XENSIV_BGT60TRXX = 0x01CC,
    /** Module identifier for the LM49450 Audio Codec Library. Asset(s): (audio-codec-lm49450) */
    CY_RSLT_MODULE_BOARD_HARDWARE_LM49450 = 0x01CE,

    /** Module identifier for the MDNS library. Asset(s): (mdns) */
    CY_RSLT_MODULE_MIDDLEWARE_MNDS = 0x200,
    /** Module identifier for the ASW IoT library. Asset(s): (aws-iot) */
    CY_RSLT_MODULE_MIDDLEWARE_AWS = 0x201,
    /** Module identifier for the JSON parser library. Asset(s): (connectivity-utilities) */
    CY_RSLT_MODULE_MIDDLEWARE_JSON = 0x202,
    /** Module identifier for the Linked List library. Asset(s): (connectivity-utilities) */
    CY_RSLT_MODULE_MIDDLEWARE_LINKED_LIST = 0x203,
    /** Module identifier for the Command Console library. Asset(s): (command-console) */
    CY_RSLT_MODULE_MIDDLEWARE_COMMAND_CONSOLE = 0x204,
    /** Module identifier for the HTTP Server library. Asset(s): (http-server) */
    CY_RSLT_MODULE_MIDDLEWARE_HTTP_SERVER = 0x205,
    /** Module identifier for the Enterprise Security library. Asset(s): (enterprise-security) */
    CY_RSLT_MODULE_MIDDLEWARE_ENTERPRISE_SECURITY = 0x206,
    /** Module identifier for the TCP/IP library. Asset(s): (connectivity-utilities) */
    CY_RSLT_MODULE_MIDDLEWARE_TCPIP = 0x207,
    /** Module identifier for the Generic middleware library.
       Asset(s): (connectivity-utilities, wifi-mw-core) */
    CY_RSLT_MODULE_MIDDLEWARE_MW = 0x208,
    /** Module identifier for the TLS library. Asset(s): (cy-mbedtls) */
    CY_RSLT_MODULE_MIDDLEWARE_TLS = 0x209,
    /** Module identifier for the Secure Sockets library. Asset(s): (secure-sockets) */
    CY_RSLT_MODULE_MIDDLEWARE_SECURE_SOCKETS = 0x20a,
    /** Module identifier for the WiFi Connection Manager library (WCM).
       Asset(s): (wifi-connection-manager) */
    CY_RSLT_MODULE_MIDDLEWARE_WCM = 0x20b,
    /** Module identifier for the lwIP WHD port library. Asset(s): (wifi-mw-core) */
    CY_RSLT_MODULE_MIDDLEWARE_LWIP_WHD_PORT = 0x20c,
    /** Module identifier for the OTA Update library. Asset(s): (anycloud-ota) */
    CY_RSLT_MODULE_MIDDLEWARE_OTA_UPDATE = 0x20d,
    /** Module identifier for the HTTP Clinet library. Asset(s): (http-client) */
    CY_RSLT_MODULE_MIDDLEWARE_HTTP_CLIENT = 0x20e,
    /** Module identifier for the Machine Learning(ML) library. Asset(s): (ml-middleware) */
    CY_RSLT_MODULE_MIDDLEWARE_ML = 0x20f,

    /** Module identifier for the KV Store Middleware Library. Asset(s): (kv-store) */
    CY_RSLT_MODULE_MIDDLEWARE_KVSTORE = 0x250,
    /** Module identifier for the LIN Middleware Library. Asset(s): (lin) */
    CY_RSLT_MODULE_MIDDLEWARE_LIN = 0x0251,
    /** Module identifier for the UBM Middleware Library. Asset(s): (ubm) */
    CY_RSLT_MODULE_MIDDLEWARE_UBM = 0x0252
} cy_en_rslt_module_t;

/**
 * Defines codes to identify the type of result
 */
typedef enum
{
    /** The result code is informational-only */
    CY_RSLT_TYPE_INFO = 0U,
    /** The result code is warning of a problem but will proceed */
    CY_RSLT_TYPE_WARNING = 1U,
    /** The result code is an error */
    CY_RSLT_TYPE_ERROR = 2U,
    /** The result code is a fatal error */
    CY_RSLT_TYPE_FATAL = 3U
} cy_en_rslt_type_t;

/** Mask for the bit at position "x" */
#define CY_BIT_MASK(x)                     ((1UL << (x)) - 1U)

/** Bit position of the result type */
#define CY_RSLT_TYPE_POSITION              (16U)
/** Bit width of the result type */
#define CY_RSLT_TYPE_WIDTH                 (2U)
/** Bit position of the module identifier */
#define CY_RSLT_MODULE_POSITION            (18U)
/** Bit width of the module identifier */
#define CY_RSLT_MODULE_WIDTH               (14U)
/** Bit position of the submodule identifier */
#define CY_RSLT_SUBMODULE_POSITION         (8U)
/** Bit width of the submodule identifier */
#define CY_RSLT_SUBMODULE_WIDTH            (8U)
/** Bit position of the result code */
#define CY_RSLT_CODE_POSITION              (0U)
/** Bit width of the result code */
#define CY_RSLT_CODE_WIDTH                 (16U)

/** Mask for the result type */
#define CY_RSLT_TYPE_MASK                  CY_BIT_MASK(CY_RSLT_TYPE_WIDTH)
/** Mask for the module identifier */
#define CY_RSLT_MODULE_MASK                CY_BIT_MASK(CY_RSLT_MODULE_WIDTH)
/** Mask for the result code */
#define CY_RSLT_CODE_MASK                  CY_BIT_MASK(CY_RSLT_CODE_WIDTH)
/** Mask for submodule identifier */
#define CY_RSLT_SUBMODULE_MASK             CY_BIT_MASK(CY_RSLT_SUBMODULE_WIDTH)

/**
 * @brief Get the value of the result type field
 * @param x the @ref cy_rslt_t value from which to extract the result type
 */
#define CY_RSLT_GET_TYPE(x)                (((x) >> CY_RSLT_TYPE_POSITION) & CY_RSLT_TYPE_MASK)
/**
 * @brief Get the value of the module identifier field
 * @param x the @ref cy_rslt_t value from which to extract the module id
 */
#define CY_RSLT_GET_MODULE(x)              (((x) >> CY_RSLT_MODULE_POSITION) & CY_RSLT_MODULE_MASK)
/**
 * @brief Get the value of the result code field
 * @param x the @ref cy_rslt_t value from which to extract the result code
 */
#define CY_RSLT_GET_CODE(x)                (((x) >> CY_RSLT_CODE_POSITION) & CY_RSLT_CODE_MASK)

#define CY_RSLT_CREATE(type, module, code) \
    ((((uint16_t) (module) & CY_RSLT_MODULE_MASK) << CY_RSLT_MODULE_POSITION) | \
    ((((uint16_t) code) & CY_RSLT_CODE_MASK) << CY_RSLT_CODE_POSITION) | \
    ((((uint16_t) type) & CY_RSLT_TYPE_MASK) << CY_RSLT_TYPE_POSITION))


/** Requested operation did not complete in the specified time */
#define CY_RTOS_TIMEOUT                     \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 0)
/** The RTOS could not allocate memory for the specified operation */
#define CY_RTOS_NO_MEMORY                   \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 1)
/** An error occured in the RTOS */
#define CY_RTOS_GENERAL_ERROR               \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 2)
/** A bad argument was passed into the APIs */
#define CY_RTOS_BAD_PARAM                   \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 5)
/** A memory alignment issue was detected. Ensure memory provided is aligned per \ref
   CY_RTOS_ALIGNMENT_MASK */
#define CY_RTOS_ALIGNMENT_ERROR             \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 6)
/** Requested operation not supported with this RTOS */
#define CY_RTOS_UNSUPPORTED                  \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_OS, 7)



/** Create a mutex which can support recursion or not.
 *
 * Creates a binary mutex which can be used for mutual exclusion to prevent simulatenous
 * access of shared resources. Created mutexes can support priority inheritance if recursive.
 *
 * \note Not all RTOS implementations support non-recursive mutexes. In this case a recursive
 * mutex will be created.
 *
 * @param[out] mutex     Pointer to the mutex handle to be initialized
 * @param[in]  recursive Should the created mutex support recursion or not
 *
 * @return The status of mutex creation request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_init_mutex2(mutex, recursive) \
    cy_rtos_mutex_init(mutex, recursive)

/** Create a mutex which can support recursion.
 *
 * Creates a binary mutex which can be used for mutual exclusion to prevent simulatenous
 * access of shared resources. Created mutexes can support priority inheritance if recursive.
 *
 * \note Not all RTOS implementations support non-recursive mutexes. In this case a recursive
 * mutex will be created.
 *
 * @param[out] mutex     Pointer to the mutex handle to be initialized
 *
 * @return The status of mutex creation request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_NO_MEMORY, \ref
 *         CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_init_mutex(mutex) \
    cy_rtos_mutex_init(mutex, true)

/** Get a mutex.
 *
 * If the mutex is available, it is acquired and this function returned.
 * If the mutex is not available, the thread waits until the mutex is available
 * or until the timeout occurs.
 *
 * @note This function must not be called from an interrupt context as it may block.
 *
 * @param[in] mutex       Pointer to the mutex handle
 * @param[in] timeout_ms  Maximum number of milliseconds to wait while attempting to get
 *                        the mutex. Use the \ref CY_RTOS_NEVER_TIMEOUT constant to wait forever.
 *
 * @return The status of the get mutex. Returns timeout if mutex was not acquired
 *                    before timeout_ms period. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_TIMEOUT, \ref
 *                    CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_get_mutex(mutex, timeout_ms) \
    cy_rtos_mutex_get(mutex, timeout_ms)

/** Set a mutex.
 *
 * The mutex is released allowing any other threads waiting on the mutex to
 * obtain the semaphore.
 *
 * @param[in] mutex   Pointer to the mutex handle
 *
 * @return The status of the set mutex request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 *
 */
#define cy_rtos_set_mutex(mutex)       cy_rtos_mutex_set(mutex)

/** Deletes a mutex.
 *
 * This function frees the resources associated with a sempahore.
 *
 * @param[in] mutex Pointer to the mutex handle
 *
 * @return The status to the delete request. [\ref CY_RSLT_SUCCESS, \ref CY_RTOS_GENERAL_ERROR]
 */
#define cy_rtos_deinit_mutex(mutex)    cy_rtos_mutex_deinit(mutex)

    
#endif
