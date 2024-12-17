/*
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "endpoint0.h"
#include "descript.h"
#include "PanSeries.h"
#include "endpoint.h"
#include "usb_panchip.h"

/*  #define SYS_USB_LOG printf */
/* Pointer to current configuration descriptor */
void *gpCurCfg;
/* Current device state */
uint32_t gnDevState = DEVSTATE_DEFAULT;

/* Pointer to current interface descriptors */
USB_InterfaceDscrDef *gpCurIf[M_MAXIFS];

/* Current alternate interface values */
uint8_t gbyCurIfVal[M_MAXIFS];

volatile uint8_t config_id;
volatile uint8_t interface_id;
volatile uint16_t status_state;
volatile uint8_t class_protocol;
volatile uint8_t class_idle;
/*  zf tuning */

/* Configuration structures as defined in config.h */
USB_ConfigTypeDef cfg1;
USB_ConfigTypeDef_2 cfg2;
USB_ConfigPointTypeDef cfg;
USB_HidDscrDef gHidDsc;

static USB_EndPointStatusDef ep0state;
static USB_CommandDef cmd;
static volatile uint16_t _bmRequestType, _wValue, _wLength, _wIndex;
static volatile uint8_t _bNumConfigurations;

uint8_t usb_driver_init_finish;

#define DEF_Intface0_ReportDes_SIZE 63
#define DEF_Intface1_ReportDes_SIZE 188
#define DEF_Intface2_ReportDes_SIZE 33

#define MOUSE_INTERFACE                         (0x0000)
#define KEYBOARD_INTERFACE                      (0x0001)
#define DFU_INTERFACE                           (0x0002)

#define INTERFACE_NUM                                   3
#define USB_WBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF)

uint8_t standard_keyboard_report[DEF_Intface0_ReportDes_SIZE] = {
	0x05, 0x01,             /* 0    GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x09, 0x06,             /* 2    LOCAL_USAGE(Keyboard)		*/
	0xA1, 0x01,             /* 4    MAIN_COLLECTION(Applicatior)		*/
	0x05, 0x07,             /* 6    GLOBAL_USAGE_PAGE(Keyboard/Keypad)		*/
	0x19, 0xE0,             /* 8    LOCAL_USAGE_MINIMUM(224)		*/
	0x29, 0xE7,             /* 10   LOCAL_USAGE_MAXIMUM(231)	*/
	0x15, 0x00,             /* 12   GLOBAL_LOGICAL_MINIMUM(0)		*/
	0x25, 0x01,             /* 14   GLOBAL_LOCAL_MAXIMUM(1)		*/
	0x75, 0x01,             /* 16   GLOBAL_REPORT_SIZE(1)		*/
	0x95, 0x08,             /* 18   GLOBAL_REPORT_COUNT(8)		*/
	0x81, 0x02,             /* 20   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )	Input 1.0	*/
	0x95, 0x01,             /* 22   GLOBAL_REPORT_COUNT(1)		*/
	0x75, 0x08,             /* 24   GLOBAL_REPORT_SIZE(8)		*/
	0x81, 0x03,             /* 26   MAIN_INPUT(const var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )	Input 2.0	*/
	0x95, 0x05,             /* 28   GLOBAL_REPORT_COUNT(5)		*/
	0x75, 0x01,             /* 30   GLOBAL_REPORT_SIZE(1)		*/
	0x05, 0x08,             /* 32   GLOBAL_USAGE_PAGE(LEDs)		*/
	0x19, 0x01,             /* 34   LOCAL_USAGE_MINIMUM(1)		*/
	0x29, 0x05,             /* 36   LOCAL_USAGE_MAXIMUM(5)		*/
	0x91, 0x02,             /* 38   MAIN_OUTPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )	Output 0.5	*/
	0x95, 0x01,             /* 40   GLOBAL_REPORT_COUNT(1)		*/
	0x75, 0x03,             /* 42   GLOBAL_REPORT_SIZE(3)		*/
	0x91, 0x03,             /* 44   MAIN_OUTPUT(const var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Output 1.0	*/
	0x95, 0x06,             /* 46   GLOBAL_REPORT_COUNT(6)		*/
	0x75, 0x08,             /* 48   GLOBAL_REPORT_SIZE(8)		*/
	0x15, 0x00,             /* 50   GLOBAL_LOGICAL_MINIMUM(0)		*/
	0x25, 0xFF,             /* 52   GLOBAL_LOCAL_MAXIMUM(255)		*/
	0x05, 0x07,             /* 54   GLOBAL_USAGE_PAGE(Keyboard/Keypad)		*/
	0x19, 0x00,             /* 56   LOCAL_USAGE_MINIMUM(0)		*/
	0x29, 0x65,             /* 58   LOCAL_USAGE_MAXIMUM(101)		*/
	0x81, 0x00,             /* 60   MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 8.0	*/
	0xC0,                   /* 62   MAIN_COLLECTION_END		*/
};
#if DEF_Intface1_ReportDes_SIZE == 188
uint8_t keyboard_report[DEF_Intface1_ReportDes_SIZE] = {
	/*   bit key   */
	0x05, 0x01,             /* 0    GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x09, 0x06,             /* 2    LOCAL_USAGE(Keyboard)	*/
	0xA1, 0x01,             /* 4    MAIN_COLLECTION(Applicatior)		*/
	0x85, 0x02,             /* 6    GLOBAL_REPORT_ID(2)		*/
	0x75, 0x01,             /* 8    GLOBAL_REPORT_SIZE(1)		*/
	0x95, 0x08,             /* 10   GLOBAL_REPORT_COUNT(8)		*/
	0x05, 0x07,             /* 12   GLOBAL_USAGE_PAGE(Keyboard/Keypad)		*/
	0x95, 0x78,             /* 14   GLOBAL_REPORT_COUNT(120)	*/
	0x75, 0x01,             /* 16   GLOBAL_REPORT_SIZE(1)		*/
	0x15, 0x00,             /* 18   GLOBAL_LOGICAL_MINIMUM(0)	*/
	0x25, 0x01,             /* 20   GLOBAL_LOCAL_MAXIMUM(1)		*/
	0x05, 0x07,             /* 22   GLOBAL_USAGE_PAGE(Keyboard/Keypad)		*/
	0x19, 0x00,             /* 24   LOCAL_USAGE_MINIMUM(0)		*/
	0x29, 0x78,             /* 26   LOCAL_USAGE_MAXIMUM(120)		*/
	0x81, 0x02,             /* 28   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )	Input 15.0	*/
	0xC0,                   /* 30   MAIN_COLLECTION_END		*/

	/*  media key */
	0x05, 0x0C,             /* 31   GLOBAL_USAGE_PAGE(Consumer)		*/
	0x09, 0x01,             /* 33   LOCAL_USAGE(	Consumer Control        )		*/
	0xA1, 0x01,             /* 35   MAIN_COLLECTION(Applicatior)		*/
	0x85, 0x03,             /* 37   GLOBAL_REPORT_ID(3)		*/
	0x19, 0x00,             /* 39   LOCAL_USAGE_MINIMUM(0)	*/
	0x2A, 0x00, 0x04,       /* 41   LOCAL_USAGE_MAXIMUM(1024)	*/
	0x15, 0x00,             /* 44   GLOBAL_LOGICAL_MINIMUM(0)	*/
	0x26, 0x00, 0x04,       /* 46   GLOBAL_LOCAL_MAXIMUM(1024)		*/
	0x95, 0x01,             /* 49   GLOBAL_REPORT_COUNT(1)		*/
	0x75, 0x10,             /* 51   GLOBAL_REPORT_SIZE(16)		*/
	0x81, 0x00,             /* 53   MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 17.0	*/
	0xC0,                   /* 55   MAIN_COLLECTION_END		*/

	/*   power key   */
	0x05, 0x01,             /* 56   GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x09, 0x80,             /* 58   LOCAL_USAGE()		*/
	0xA1, 0x01,             /* 60   MAIN_COLLECTION(Applicatior)	*/
	0x85, 0x04,             /* 62   GLOBAL_REPORT_ID(4)		*/
	0x05, 0x01,             /* 64   GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x19, 0x81,             /* 66   LOCAL_USAGE_MINIMUM(129)	*/
	0x29, 0x83,             /* 68   LOCAL_USAGE_MAXIMUM(131)	*/
	0x15, 0x00,             /* 70   GLOBAL_LOGICAL_MINIMUM(0)		*/
	0x25, 0x01,             /* 72   GLOBAL_LOCAL_MAXIMUM(1)	*/
	0x95, 0x03,             /* 74   GLOBAL_REPORT_COUNT(3)	*/
	0x75, 0x01,             /* 76   GLOBAL_REPORT_SIZE(1)		*/
	0x81, 0x02,             /* 78   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 17.3*/
	0x95, 0x01,             /* 80   GLOBAL_REPORT_COUNT(1)		*/
	0x75, 0x05,             /* 82   GLOBAL_REPORT_SIZE(5)		*/
	0x81, 0x01,             /* 84   MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 18.0*/
	0xC0,                   /* 86   MAIN_COLLECTION_END		*/

	/* mouse */
	0x05, 0x01,             /* 87   GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x09, 0x02,             /* 89   LOCAL_USAGE(Mouse)		*/
	0xA1, 0x01,             /* 91   MAIN_COLLECTION(Applicatior)	*/
	0x85, 0x05,             /* 93   GLOBAL_REPORT_ID(5)		*/
	0x09, 0x01,             /* 95   LOCAL_USAGE(Pointer)		*/
	0xA1, 0x00,             /* 97   MAIN_COLLECTION(Physical)	*/
	0x05, 0x09,             /* 99   GLOBAL_USAGE_PAGE(Button)		*/
	0x15, 0x00,             /* 101  GLOBAL_LOGICAL_MINIMUM(0)	*/
	0x25, 0x01,             /* 103  GLOBAL_LOCAL_MAXIMUM(1)		*/
	0x19, 0x01,             /* 105  LOCAL_USAGE_MINIMUM(1)		*/
	0x29, 0x05,             /* 107  LOCAL_USAGE_MAXIMUM(5)	*/
	0x75, 0x01,             /* 109  GLOBAL_REPORT_SIZE(1)		*/
	0x95, 0x05,             /* 111  GLOBAL_REPORT_COUNT(5)	*/
	0x81, 0x02,             /* 113  MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 18.5*/
	0x95, 0x03,             /* 115  GLOBAL_REPORT_COUNT(3)	*/
	0x81, 0x01,             /* 117  MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )Input 19.0*/
	0x05, 0x01,             /* 119  GLOBAL_USAGE_PAGE(Generic Desktop Controls)		*/
	0x16, 0x00, 0x80,       /* 121  GLOBAL_LOGICAL_MINIMUM(32768)		*/
	0x26, 0xFF, 0x7F,       /* 124  GLOBAL_LOCAL_MAXIMUM(32767/32767)		*/
	0x09, 0x30,             /* 127  LOCAL_USAGE(X)		*/
	0x09, 0x31,             /* 129  LOCAL_USAGE(Y)		*/
	0x75, 0x10,             /* 131  GLOBAL_REPORT_SIZE(16)		*/
	0x95, 0x02,             /* 133  GLOBAL_REPORT_COUNT(2)	*/
	0x81, 0x06,             /* 135  MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )Input 23.0*/
	0x15, 0x81,             /* 137  GLOBAL_LOGICAL_MINIMUM(-127)		*/
	0x25, 0x7F,             /* 139  GLOBAL_LOCAL_MAXIMUM(127)		*/
	0x09, 0x38,             /* 141  LOCAL_USAGE(Wheel)		*/
	0x75, 0x08,             /* 143  GLOBAL_REPORT_SIZE(8)	*/
	0x95, 0x01,             /* 145  GLOBAL_REPORT_COUNT(1)	*/
	0x81, 0x06,             /* 147  MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )Input 24.0*/
	0x05, 0x0C,             /* 149  GLOBAL_USAGE_PAGE(Consumer)		*/
	0x0A, 0x38, 0x02,       /* 151  LOCAL_USAGE(	AC Pan  )		*/
	0x95, 0x01,             /* 154  GLOBAL_REPORT_COUNT(1)		*/
	0x81, 0x06,             /* 156  MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )Input 25.0*/
	0xC0,                   /* 158  MAIN_COLLECTION_END		*/
	0xC0,                   /* 159  MAIN_COLLECTION_END		*/

	/* vendor  */
	0x06, 0x55, 0xFF,       /* 160  GLOBAL_USAGE_PAGE(Reserved or Other)		*/
	0x0A, 0x02, 0x02,       /* 163  LOCAL_USAGE()		*/
	0xA1, 0x01,             /* 166  MAIN_COLLECTION(Applicatior)	*/
	0x85, 0x06,             /* 168  GLOBAL_REPORT_ID(6)		*/
	0x75, 0x08,             /* 170  GLOBAL_REPORT_SIZE(8)		*/
	0x95, 0x3F,             /* 172  GLOBAL_REPORT_COUNT(63)		*/
	0x15, 0x00,             /* 174  GLOBAL_LOGICAL_MINIMUM(0)		*/
	0x26, 0xFF, 0x00,       /* 176  GLOBAL_LOCAL_MAXIMUM(255/255)		*/
	0x09, 0x02,             /* 179  LOCAL_USAGE()		*/
	0x81, 0x00,             /* 181  MAIN_INPUT(NoWrap linear PreferredState NoNullPosition NonVolatile )Input 88.0*/
	0x09, 0x02,             /* 183  LOCAL_USAGE()		*/
	0x91, 0x00,             /* 185  MAIN_OUTPUT(NoWrap linear PreferredState NoNullPosition NonVolatile )Output 63.0*/
	0xC0,                   /* 187  MAIN_COLLECTION_END		*/
};
#elif DEF_Intface1_ReportDes_SIZE == 138
uint8_t keyboard_report[DEF_Intface1_ReportDes_SIZE] = {
	/*  -------media key------------------------------- */
	0x05, 0x0C,                     /*  0    GLOBAL_USAGE_PAGE(Consumer)	 */
	0x09, 0x01,                     /*  2    LOCAL_USAGE(	Consumer Control        )	 */
	0xA1, 0x01,                     /*  4    MAIN_COLLECTION(Applicatior)	 */
	0x85, 0x01,                     /*  6    GLOBAL_REPORT_ID(1)	 */
	0x19, 0x00,                     /*  8    LOCAL_USAGE_MINIMUM(0)	 */
	0x2A, 0x00, 0x04,               /*  10   LOCAL_USAGE_MAXIMUM(1024)	 */
	0x15, 0x00,                     /*  13   GLOBAL_LOGICAL_MINIMUM(0)	 */
	0x26, 0x00, 0x04,               /*  15   GLOBAL_LOCAL_MAXIMUM(1024)	 */
	0x95, 0x01,                     /*  18   GLOBAL_REPORT_COUNT(1)	 */
	0x75, 0x10,                     /*  20   GLOBAL_REPORT_SIZE(16)	 */
	0x81, 0x00,                     /*  22   MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )*/
	0xC0,                           /*  24   MAIN_COLLECTION_END	 */

	/*  -------ACPI key------------------------------------- */
	0x05, 0x01,                     /*  25   GLOBAL_USAGE_PAGE(Generic Desktop Controls)	 */
	0x09, 0x80,                     /*  27   LOCAL_USAGE()	 */
	0xA1, 0x01,                     /*  29   MAIN_COLLECTION(Applicatior)	 */
	0x85, 0x02,                     /*  31   GLOBAL_REPORT_ID(2)	 */
	0x05, 0x01,                     /*  33   GLOBAL_USAGE_PAGE(Generic Desktop Controls)	 */
	0x19, 0x81,                     /*  35   LOCAL_USAGE_MINIMUM(129) */
	0x29, 0x83,                     /*  37   LOCAL_USAGE_MAXIMUM(131) */
	0x15, 0x00,                     /*  39   GLOBAL_LOGICAL_MINIMUM(0) */
	0x25, 0x01,                     /*  41   GLOBAL_LOCAL_MAXIMUM(1) */
	0x95, 0x03,                     /*  43   GLOBAL_REPORT_COUNT(3) */
	0x75, 0x01,                     /*  45   GLOBAL_REPORT_SIZE(1) */
	0x81, 0x02,                     /*  47   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile ) */
	0x95, 0x01,                     /*  49   GLOBAL_REPORT_COUNT(1) */
	0x75, 0x05,                     /*  51   GLOBAL_REPORT_SIZE(5) */
	0x81, 0x01,                     /*  53   MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile ) */
	0xC0,                           /*  55   MAIN_COLLECTION_END */

	/*  -------normal key------------------------------------- */
	0x05, 0x01,                     /*  56   GLOBAL_USAGE_PAGE(Generic Desktop Controls) */
	0x09, 0x06,                     /*  58   LOCAL_USAGE(Keyboard) */
	0xA1, 0x01,                     /*  60   MAIN_COLLECTION(Applicatior) */
	0x85, 0x03,                     /*  62   GLOBAL_REPORT_ID(3) */
	/*  bitmap of modifiers */
	0x75, 0x01,                     /*  64   GLOBAL_REPORT_SIZE(1) */
	0x95, 0x08,                     /*  66   GLOBAL_REPORT_COUNT(8) */
	0x05, 0x07,                     /*  68   GLOBAL_USAGE_PAGE(Keyboard/Keypad) */
	0x19, 0xE0,                     /*  70   LOCAL_USAGE_MINIMUM(224) */
	0x29, 0xE7,                     /*  72   LOCAL_USAGE_MAXIMUM(231) */
	0x15, 0x00,                     /*  74   GLOBAL_LOGICAL_MINIMUM(0) */
	0x25, 0x01,                     /*  76   GLOBAL_LOCAL_MAXIMUM(1) */
	0x81, 0x02,                     /*  78   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile ) */
	/*  bitmap of keys */
	0x95, 0x70,                     /*  80   GLOBAL_REPORT_COUNT(112) */
	0x75, 0x01,                     /*  82   GLOBAL_REPORT_SIZE(1) */
	0x15, 0x00,                     /*  84   GLOBAL_LOGICAL_MINIMUM(0) */
	0x25, 0x01,                     /*  86   GLOBAL_LOCAL_MAXIMUM(1) */
	0x05, 0x07,                     /*  88   GLOBAL_USAGE_PAGE(Keyboard/Keypad) */
	0x19, 0x00,                     /*  90   LOCAL_USAGE_MINIMUM(0) */
	0x29, 0x77,                     /*  92   LOCAL_USAGE_MAXIMUM(119) */
	0x81, 0x02,                     /*  94   MAIN_INPUT     Input 19.0 */
	/*  LED output report */
	0x95, 0x05,                     /*  96   GLOBAL_REPORT_COUNT(5) */
	0x75, 0x01,                     /*  98   GLOBAL_REPORT_SIZE(1) */
	0x05, 0x08,                     /*  100  GLOBAL_USAGE_PAGE(LEDs) */
	0x19, 0x01,                     /*  102  LOCAL_USAGE_MINIMUM(1) */
	0x29, 0x05,                     /*  104  LOCAL_USAGE_MAXIMUM(5) */
	0x91, 0x02,                     /*  106  MAIN_OUTPUT Output 0.5 */
	0x95, 0x01,                     /*  108  GLOBAL_REPORT_COUNT(1) */
	0x75, 0x03,                     /*  110  GLOBAL_REPORT_SIZE(3) */
	0x91, 0x03,                     /*  112  MAIN_OUTPUT Output 1.0 */
	0xC0,                           /*  114  MAIN_COLLECTION_END */

	/*  -------vendor key----------------------- */
	0x06, 0xFF, 0xFF,               /*  115  GLOBAL_USAGE_PAGE(Reserved or Other) */
	0x09, 0x01,                     /*  118  LOCAL_USAGE() */
	0xA1, 0x01,                     /*  120  MAIN_COLLECTION(Applicatior) */
	0x85, 0x04,                     /*  122  GLOBAL_REPORT_ID(4) */
	0x09, 0x01,                     /*  124  LOCAL_USAGE() */
	0x15, 0x00,                     /*  126  GLOBAL_LOGICAL_MINIMUM(0) */
	0x26, 0xFF, 0x00,               /*  128  GLOBAL_LOCAL_MAXIMUM(255/255) */
	0x75, 0x08,                     /*  131  GLOBAL_REPORT_SIZE(8) */
	0x95, 0x03,                     /*  133  GLOBAL_REPORT_COUNT(3) */
	0x81, 0x02,                     /*  135  MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )	Input 22.0 */
	0xC0,                           /*  137  MAIN_COLLECTION_END */
};
#endif

uint8_t dfu_report[DEF_Intface2_ReportDes_SIZE] = {
	0x05, 0x8C,                     /*  0x01 */ /* 0x8C, */
	0x09, 0x06,
	0xa1, 0x01,                     /*  app clooection c0 end */
	0x09, 0x06,
	0x15, 0x00,
	0x26, 0xff,
	0x00, 0x75,
	0x08, 0x95,
	0x40, 0x91,
	0x02, 0x09,
	0x06, 0x15,
	0x00, 0x26,
	0xff, 0x00,
	0x75, 0x08,
	0x95, 0X40,
	0x81, 0x02,
	0xc0,
};

uint8_t DeviceDscDat[] = {
	0x12,           /*  bLength; */
	0x01,           /*  bDescriptorType; */
	0x00, 0x02,     /*  bcdUSB; */
	0x00,           /*  bDeviceClass; */
	0x00,           /*  bDeviceSubClass; */
	0x00,           /*  bDeviceProtocol; */
	0x40,           /*  bMaxPacketSize0; */
	0x6d, 0x05,     /*  idVendor; */
	0x77, 0xc0,     /*  idProduct; */
	0x00, 0x01,     /*  bcdDevice; */
	0x01,           /*  iManufacturer; */
	0x02,           /*  iProduct; */
	0x03,           /*  iSerialNumber; */
	0x01            /*  bNumConfigurations; */
};

void update_usb_vid(uint8_t *data)
{
	DeviceDscDat[8] = data[0];
	DeviceDscDat[9] = data[1];
}

void update_usb_pid(uint8_t *data)
{
	DeviceDscDat[10] = data[0];
	DeviceDscDat[11] = data[1];
}

const uint8_t hid1_desc[] = {
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHIDH */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface0_ReportDes_SIZE), /*  wItemLengthH */
};

const uint8_t hid2_desc[] = {
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHID */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface1_ReportDes_SIZE), /*  wItemLengthH */
};

const uint8_t hid3_desc[] = {
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHID */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface2_ReportDes_SIZE), /*  wItemLengthH */
};

const uint8_t ConfigDscDat[] = {
/* ============== CONFIGURATION 1 =========== */
	/* Configuration 1 descriptor */
	0x09,           /*  CbLength */
	0x02,           /*  CbDescriptorType */
	0x5b,           /*  91 */
	0x00,
	0x03,           /*  CbNumInterfaces */
	0x01,           /*  CbConfigurationValue */
	0x00,           /*  CiConfiguration */
	0xA0,           /*  CbmAttributes Bus powered */
	0xF8,           /*  CMaxPower: 100mA */
/* ============== interface 0 =============== */
/* Interfac Descriptor */
	0x09,                                   /*  bLength */
	0x04,                                   /*  bDescriptorType */
	0x00,                                   /*  bInterfaceNumber */
	0x00,                                   /*  bAlternateSetting */
	0x01,                                   /*  bNumEndpoints */
	0x03,                                   /*  bInterfaceClass: HID code */
	0x01,                                   /*  bInterfaceSubclass */
	0x02,                                   /*  0: None, 1: keyboad, 2: mouse */
	0x00,                                   /*  iInterface */
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHIDH */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface0_ReportDes_SIZE), /*  wItemLengthH */
	/* Endpoint 1 descriptor */
	0x07,                                   /*  bLength */
	0x05,                                   /*  bDescriptorType		Endpoint */
	0x81,                                   /*  bEndpointAddress		EP 02 - IN */
	0x03,                                   /*  bmAttributes			INT */
	USB_WBVAL(0x40),                        /*  wMaxPacketSize */
	0x01,                                   /*  bInterval			1 ms */
/* ============== interface 1 =============== */
	/* Interface Descriptor */
	0x09,                                   /*  bLength */
	0x04,                                   /*  bDescriptorType */
	0x01,                                   /*  bInterfaceNumber */
	0x00,                                   /*  bAlternateSetting */
	0x01,                                   /*  bNumEndpoints */
	0x03,                                   /*  bInterfaceClass: HID code */
	0x01,                                   /*  bInterfaceSubclass */
	0x01,                                   /*  0: None, 1: keyboad, 2: mouse */
	0x00,                                   /*  iInterface */
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHID */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface1_ReportDes_SIZE), /*  wItemLengthH */
	/* Endpoint descriptor */
	0x07,                                   /*  bLength */
	0x05,                                   /*  bDescriptorType		Endpoint */
	0x82,                                   /*  bEndpointAddress		EP 02 - IN */
	0x03,                                   /*  bmAttributes			INT */
	USB_WBVAL(0x40),                        /*  wMaxPacketSize */
	0x01,                                   /*  bInterval			1 ms */
/* ============== interface 2 =============== */
	/* Interface Descriptor  */
	0x09,                                   /*  bLength */
	0x04,                                   /*  bDescriptorType */
	0x02,                                   /*  bInterfaceNumber */
	0x00,                                   /*  bAlternateSetting */
	0x02,                                   /*  bNumEndpoints */
	0x03,                                   /*  bInterfaceClass: HID code */
	0x00,                                   /*  bInterfaceSubclass */
	0x00,                                   /*  0: None, 1: keyboad, 2: mouse */
	0x00,                                   /*  iInterface */
	/* HID Descriptor */
	0x09,                                   /*  bLength */
	0x21,                                   /*  bDescriptor type - HID Descriptor */
	USB_WBVAL(0x111),                       /*  bcdHID */
	0x00,                                   /*  bCountryCode */
	0x01,                                   /*  bNumDescriptors */
	0x22,                                   /*  bDescriptorType */
	USB_WBVAL(DEF_Intface2_ReportDes_SIZE), /*  wItemLengthH */
	/* Endpoint descriptor */
	0x07,                                   /*  bLength */
	0x05,                                   /*  bDescriptorType		Endpoint */
	0x83,                                   /*  bEndpointAddress		EP 03 - IN */
	0x03,                                   /*  bmAttributes			INT */
	USB_WBVAL(0x40),                        /* wMaxPacketSize */
	0x01,                                   /*  bInterval			1 ms */

	/* Endpoint descriptor */
	0x07,                                   /*  bLength */
	0x05,                                   /*  bDescriptorType		Endpoint */
	0x03,                                   /*  bEndpointAddress		EP 03 - OUT */
	0x03,                                   /*  bmAttributes			INT */
	USB_WBVAL(0x40),                        /* wMaxPacketSize */
	0x01,                                   /*  bInterval			1 ms */
};
#pragma pack(1)

void USB_SendStall(void)
{
	SYS_USB_LOG("stall \r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
}

void USB_SendACK(void)
{
	SYS_USB_LOG("ack \r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND);
}

void USB_GetStatus(void)
{
	ep0state.nBytesLeft = 2;
	ep0state.pData = (void *)&status_state;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	SYS_USB_LOG("get status \r\n");
}

void USB_SetAddr(void)
{
	SYS_USB_LOG("set addr\r\n");
	/* Store device function address until status stage of request */
	if (_bmRequestType != M_CMD_STDDEVOUT) {
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
	} else if (gnDevState <= DEVSTATE_ADDRESS) {
		ep0state.byFAddr = (uint8_t)cmd.wValue;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND);
	} else {
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
	}
}

void USB_GetMouseReportDesc(void)
{
	ep0state.nBytesLeft = sizeof(standard_keyboard_report);
	ep0state.pData = (void *)standard_keyboard_report;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_GetKeyReportDesc(void)
{
	ep0state.nBytesLeft = sizeof(keyboard_report);
	ep0state.pData = (void *)keyboard_report;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_GetDfuReportDesc(void)
{

	SYS_USB_LOG("USB_GetDfuReportDesc\n");
	ep0state.nBytesLeft = sizeof(dfu_report);
	ep0state.pData = (void *)dfu_report;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_GetReportDesc(void)
{
	SYS_USB_LOG("get report %d \r\n", _wIndex);

	switch (_wIndex) {
	case MOUSE_INTERFACE:
		USB_GetMouseReportDesc();
		break;
	case DFU_INTERFACE:
		USB_GetDfuReportDesc();
		break;
	case KEYBOARD_INTERFACE:
		USB_GetKeyReportDesc();
		break;
	default:
		break;
	}

	usb_driver_init_finish = 1;
}

void USB_GetDeviceDesc(void)
{
	SYS_USB_LOG("get device dsc\r\n");

	/* Prepare to return Standard Device Descriptor */
	ep0state.nBytesLeft = sizeof(DeviceDscDat);
	/* Check host is allowing a descriptor this long */
	if (_wLength < ep0state.nBytesLeft) {
		ep0state.nBytesLeft = _wLength;
	}
	ep0state.pData = (void *)DeviceDscDat;
	_bNumConfigurations = DeviceDscDat[sizeof(DeviceDscDat) - 1];

	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_GetDeviceQualDesc(void)
{
	SYS_USB_LOG("get device qual dsc\r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
}

void USB_GetConfigDesc(void)
{
	uint8_t byConfig;

	SYS_USB_LOG("get config dsc\r\n");
	byConfig = (uint8_t)(_wValue & 0x00FF);
	if (byConfig >= _bNumConfigurations) {
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
	} else {
		/* Get pointer to requested configuration descriptor */
		if (_wLength == 0xff) {
			ep0state.nBytesLeft = sizeof(ConfigDscDat);
		} else {
			ep0state.nBytesLeft = (int)_wLength;
		}
		ep0state.pData = (void *)ConfigDscDat;
		ep0state.nState = M_EP0_TX;
		/* Check host is allowing a descriptor this long */
		if (_wLength < ep0state.nBytesLeft) {
			ep0state.nBytesLeft = _wLength;
		}

		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	}
}
const char ManufacturerDescriptor1[] = {
	0x10,   /*  length */
	0x03,   /*  str mark */
	'p', 0x00,
	'a', 0x00,
	'n', 0x00,
	'c', 0x00,
	'h', 0x00,
	'i', 0x00,
	'p', 0x00
};

const char ManufacturerDescriptor2[] = {
	0x16,   /*  length */
	0x03,   /*  str mark */
	'm', 0x00,
	'u', 0x00,
	'l', 0x00,
	't', 0x00,
	'i', 0x00,
	'm', 0x00,
	'o', 0x00,
	'u', 0x00,
	's', 0x00,
	'e', 0x00,
};

const char ManufacturerDescriptor3[] = {
	0x0a,   /*  length */
	0x03,   /*  str mark */
	'1', 0x00,
	'2', 0x00,
	'3', 0x00,
	'4', 0x00,
};

const char StringDescriptor[] = {
	0x04,
	0x03,
	0x09,
	0x04,
};

void USB_GetDevStrDesc1(void)
{
	SYS_USB_LOG("get device string dsc1\r\n");

	if (_wLength == 4) {
		ep0state.nBytesLeft = 4;
		ep0state.pData = (void *)ManufacturerDescriptor1;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	} else {
		ep0state.nBytesLeft = sizeof(ManufacturerDescriptor1);
		ep0state.pData = (void *)ManufacturerDescriptor1;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	}

}

void USB_GetDevStrDesc2(void)
{
	SYS_USB_LOG("get device string dsc 2\r\n");

	if (_wLength == 4) {
		ep0state.nBytesLeft = 4;
		ep0state.pData = (void *)ManufacturerDescriptor2;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	} else {
		ep0state.nBytesLeft = sizeof(ManufacturerDescriptor2);
		ep0state.pData = (void *)ManufacturerDescriptor2;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	}
}

void USB_GetDevStrDesc3(void)
{
	SYS_USB_LOG("get device string dsc 3\r\n");

	if (_wLength == 4) {
		ep0state.nBytesLeft = 4;
		ep0state.pData = (void *)ManufacturerDescriptor3;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	} else {
		ep0state.nBytesLeft = sizeof(ManufacturerDescriptor3);
		ep0state.pData = (void *)ManufacturerDescriptor3;
		ep0state.nState = M_EP0_TX;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	}
}

void USB_GetDevStrDesc(void)
{
	SYS_USB_LOG("get device string dsc\r\n");

	ep0state.nBytesLeft = sizeof(StringDescriptor);
	ep0state.pData = (void *)StringDescriptor;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_GetDevOtherSpeedDesc(void)
{
	SYS_USB_LOG("get device other speed dsc\r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
}

void USB_GetHidDesc(void)
{
	SYS_USB_LOG("get hid desc %d \r\n", _wIndex);

	switch (_wIndex) {
	case MOUSE_INTERFACE:
		ep0state.nBytesLeft = sizeof(hid1_desc);
		ep0state.pData = (void *)hid1_desc;
		break;
	case KEYBOARD_INTERFACE:
		ep0state.nBytesLeft = sizeof(hid2_desc);
		ep0state.pData = (void *)hid2_desc;
		break;
	case DFU_INTERFACE:        /*  interface 1 report */
		ep0state.nBytesLeft = sizeof(hid3_desc);
		ep0state.pData = (void *)hid3_desc;
		break;
	default:
		break;
	}
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}


void USB_SetConfig(void)
{
	uint8_t byConfig;
	uint8_t **ppby;

	byConfig = (uint8_t)(_wValue & 0x00FF);
	SYS_USB_LOG("set config %d!!!,%d\r\n", byConfig, gnDevState);
	if (gnDevState == DEVSTATE_DEFAULT) {
		SYS_USB_LOG("a\n");
		USB_SendStall();
		/* Assumes configurations are numbered 1 to NumConfigurations */
	} else if (byConfig > _bNumConfigurations) {
		SYS_USB_LOG("b\n");
		USB_SendStall();
	} else if (!byConfig) {
		SYS_USB_LOG("c\n");
		gnDevState = DEVSTATE_ADDRESS;
		WRITE_REG(USB->INDEX, 0);
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND);
	} else {
		SYS_USB_LOG("d\n");
		/* Get pointer to requested configuration descriptor */
		ppby = (uint8_t **)&cfg;
		ppby += byConfig - 1;
		gpCurCfg = (void *)*ppby;
		/* Set all alternate settings to zero */
		for (uint32_t n = 0; n < M_MAXIFS; n++) {
			gbyCurIfVal[n] = 0;
		}
		/* Configure endpoints */
		ConfigureIfs();
		config_id = byConfig;
		gnDevState = DEVSTATE_CONFIG;
		WRITE_REG(USB->INDEX, 0);
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND);
	}
}

void USB_SetInterface(void)
{
	SYS_USB_LOG("set interface \r\n");
	/*  WRITE_REG(USB->CSR0_INCSR1, M_CSR0_INPKTRDY | M_CSR0_SVDOUTPKTRDY); */
	/*  WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT | M_OUTCSR_FF); */
	USB_CfgDscrDef *pcfg;

	if (gnDevState > DEVSTATE_ADDRESS) {
		pcfg = (USB_CfgDscrDef *)gpCurCfg;
		if ((uint8_t)_wIndex >= pcfg->bNumInterfaces) {
		} else {
			gbyCurIfVal[_wIndex] = _wValue;
			if (ConfigureIfs() == false) {
			}
		}
	} else {
		/*  USB_SendACK(); */
	}
	/*  interface_id = _wIndex; */
	/*  USB_SendACK(); */
	USB_SendStall();
}


void USB_GetConfig(void)
{
//	USB_CfgDscrDef *pcfg;
	SYS_USB_LOG("get config \r\n");
	if ((gnDevState == DEVSTATE_ADDRESS) && (!_wValue)) {
		SYS_USB_LOG("1\n");
		/* Prepare to return zero */
		ep0state.nBytesLeft = 1;
		ep0state.pData = (void *)&_wValue;
		ep0state.nState = M_EP0_TX;
	} else if (gnDevState > DEVSTATE_ADDRESS) {
		SYS_USB_LOG("2\n");
		/* Prepare to return configuration value */
//		pcfg = (USB_CfgDscrDef *)gpCurCfg;
		ep0state.nBytesLeft = 1;
		ep0state.pData = (void *)&config_id;
		/*  ep0state.pData = (void*)&pcfg->bConfigurationValue; */
		ep0state.nState = M_EP0_TX;
	} else {
		SYS_USB_LOG("3");
		USB_SendStall();
	}
}

void USB_GetInterface(void)
{
	USB_CfgDscrDef *pcfg;

	SYS_USB_LOG("get interface\r\n");
	if (gnDevState > DEVSTATE_ADDRESS) {
		pcfg = (USB_CfgDscrDef *)gpCurCfg;
		if ((uint8_t)_wIndex >= pcfg->bNumInterfaces) {
			USB_SendStall();
		} else {
			/* Prepare to return interface value */
			ep0state.nBytesLeft = 1;
			ep0state.pData = (void *)&gpCurIf[_wIndex]->bInterfaceNumber;
			ep0state.nState = M_EP0_TX;
		}
	} else {
		USB_SendStall();
	}
}

void USB_SetFeature(void)
{
	SYS_USB_LOG("set feature\r\n");
	status_state = 1;
	switch (_bmRequestType) {
	case M_CMD_STDDEVOUT:
		switch (_wValue) {
		case M_FTR_DEVREMWAKE:
			status_state = 3;
			/* Enable USB Suspend Mode */
			USB->POWER |= 0x1;
			USB->INT_USBE |= 0x03;
			break;
		default:
			break;
		}
		break;
	case M_CMD_STDIFIN:
	/* Add interface features here */
	case M_CMD_STDEPIN:
	/* Add endpoint features here */
	default:
		break;
	}
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND);
	/*  WRITE_REG(USB->CSR0_INCSR1, M_CSR0_INPKTRDY | M_CSR0_SVDOUTPKTRDY); */
	/*  WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT | M_OUTCSR_FF); */
}

void USB_StandardInterfaceInProcess(void)
{
	switch (_wValue & M_CMD_DESCMASK) {
	case M_CMD_HID_DSC:
		USB_GetHidDesc();
		break;
	case M_CMD_REPORT:
		USB_GetReportDesc();
		break;
	default:
		USB_SendStall();
		break;
	}
}

void USB_StandardDevInProcess(void)
{
	switch (_wValue & M_CMD_DESCMASK) {
	case M_CMD_DEVICE:
/*                      _wLength = pcmd->wLength; */
		USB_GetDeviceDesc();
		break;
	case M_CMD_DEVQUAL:
		USB_GetDeviceQualDesc();
		break;
	case M_CMD_CONFIG:
/*                      _wLength = pcmd->wLength; */
		USB_GetConfigDesc();
		break;
	case M_CMD_STRING:
		if ((_wValue & 0xff) == 0x01) {
			USB_GetDevStrDesc1();
		} else if ((_wValue & 0xff) == 0x02) {
			USB_GetDevStrDesc2();
		} else if ((_wValue & 0xff) == 0x03) {
			USB_GetDevStrDesc3();
		}  else if ((_wValue & 0xff) == 0x00) {
			USB_GetDevStrDesc();
		} else {
			SYS_USB_LOG("_wValue %d\n", _wValue & 0xff);
			USB_SendStall();
		}
		break;
	case M_CMD_OTHERSPEED:
		/* No alternate speed supported */
		USB_GetDevOtherSpeedDesc();
		break;
	default:
		USB_SendStall();
		break;
	}
}
void USB_GetDevDscProcess(void)
{
	/* Decode the required descriptor from the command */
	switch (_bmRequestType) {
	case M_CMD_STDIFIN:
		USB_StandardInterfaceInProcess();
		break;
	case M_CMD_STDDEVIN:
		USB_StandardDevInProcess();
		break;
	default:
		USB_SendStall();
		break;
	}
}

void StdDevReq(USB_EndPointStatusDef *pep0state, USB_CommandDef *pcmd)
{
	switch (pcmd->bRequest) {
	case GET_STATUS:
		USB_GetStatus();
		break;
	case SET_ADDRESS:
		_bmRequestType = pcmd->bmRequestType;
		USB_SetAddr();
		break;
	case GET_DESCRIPTOR:
		_bmRequestType = pcmd->bmRequestType;
		_wValue = pcmd->wValue;
		_wLength = pcmd->wLength;
		_wIndex = pcmd->wIndex;
		USB_GetDevDscProcess();
		break;
	case SET_CONFIGURATION:
		_wValue = pcmd->wValue;
		USB_SetConfig();
		break;
	case SET_INTERFACE:
		_wIndex = (uint16_t)pcmd->wIndex;
		_wValue = pcmd->wValue;
		USB_SetInterface();
		break;

	case GET_CONFIGURATION:
		_wValue = pcmd->wValue;
		USB_GetConfig();
		break;

	case GET_INTERFACE:
		_wValue = pcmd->wValue;
		USB_GetInterface();
		break;
	case SET_FEATURE:
		_bmRequestType = pcmd->bmRequestType;
		_wValue = pcmd->wValue;
		USB_SetFeature();
		break;
	case CLEAR_FEATURE:
		SYS_USB_LOG("clear feature \r\n");
		if (USB->POWER & 0x1) {
			/* Enable USB Suspend Mode */
			USB->POWER &= ~0x1;
			USB->INT_USBE &= ~0x03;
		}
		status_state = 0;
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);

		/*  WRITE_REG(USB->CSR0_INCSR1, M_CSR0_INPKTRDY | M_CSR0_SVDOUTPKTRDY); */
		/*  WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT | M_OUTCSR_FF); */
		break;
	default:
		USB_SendStall();
		break;
	}
}


#define GET_REPORT              (0X1)
#define GET_IDLE                (0X2)
#define GET_PROTOCOL            (0X3)
#define SET_REPORT              (0X9)
#define SET_IDLE                (0XA)
#define SET_PROTOCOL            (0XB)

void USB_ClassGetReport(void)
{
	SYS_USB_LOG("class get report \r\n");
	WRITE_REG(USB->INDEX, 0);
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
}

void USB_ClassGetIdle(void)
{
	SYS_USB_LOG("class get idle \r\n");
	ep0state.nBytesLeft = 1;
	ep0state.pData = (void *)&class_idle;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

void USB_ClassGetProtocol(void)
{
	SYS_USB_LOG("class get protocol \r\n");
	ep0state.nBytesLeft = 1;
	ep0state.pData = (void *)&class_protocol;
	ep0state.nState = M_EP0_TX;
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}
uint16_t ep0_rx_len;
void USB_ClassSetReport(USB_CommandDef *pcmd)
{
	SYS_USB_LOG("class set report len %d\r\n", pcmd->wLength);
	SYS_USB_LOG("class set report \r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
	ep0state.nState = M_EP0_RX;
	ep0_rx_len = pcmd->wLength;
}

void USB_ClassSetIdle(USB_CommandDef *pcmd)
{
	SYS_USB_LOG("class set idle \r\n");
	/*  WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY); */
	/*  ep0state.nState = M_EP0_RX; */

	class_idle = (uint8_t)(pcmd->wValue >> 8);
	USB_SendACK();
}

void USB_ClassSetProtocol(USB_CommandDef *pcmd)
{
	SYS_USB_LOG("class set protocol \r\n");
	/*  WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY); */
	/*  ep0state.nState = M_EP0_RX; */

	class_protocol = pcmd->wValue;
	USB_SendACK();
}

void StdClassReq(USB_CommandDef *pcmd)
{
	SYS_USB_LOG("class req \r\n");
	switch (pcmd->bRequest) {
	case GET_REPORT:
		USB_ClassGetReport();
		break;
	case GET_IDLE:
		USB_ClassGetIdle();
		break;
	case GET_PROTOCOL:
		USB_ClassGetProtocol();
		break;
	case SET_REPORT:
		USB_ClassSetReport(pcmd);
		break;
	case SET_IDLE:
		USB_ClassSetIdle(pcmd);
		break;
	case SET_PROTOCOL:
		USB_ClassSetProtocol(pcmd);
		break;
	default:
		break;
	}
}

void StdVendorReq(USB_CommandDef *pcmd)
{
	SYS_USB_LOG("vendor req \r\n");
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
}

void Endpoint0_Command(USB_EndPointStatusDef *pep0state, USB_CommandDef *pcmd)
{
	/* Check request type */
	switch (pcmd->bmRequestType & M_CMD_TYPEMASK) {
	case M_CMD_STDREQ:
		StdDevReq(pep0state, pcmd);
		break;
	case M_CMD_CLASSREQ:
		StdClassReq(pcmd);
		break;
	/* Add call to external routine for handling class requests */
	case M_CMD_VENDREQ:
		StdVendorReq(pcmd);
		break;
	/* Add call to external routine for handling vendor requests */
	default:
		/* Stall the command if a reserved request is received */
		WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
		break;
	}
}

bool USB_EndPont0Reset(int32_t nCallState)
{
	bool ret =  false;

	/* Check for USB reset of endpoint 0 */
	if (nCallState == M_EP_RESET) {
		ep0state.nState = M_EP0_IDLE;
		ep0state.byFAddr = 0xFF;
		/* Clear current configuration pointer */
		gpCurCfg = NULL;
		ret = true;
	}
	return ret;
}

__ramfunc void Endpoint0(int32_t nCallState)
{
	uint8_t byCSR0;

	if (USB_EndPont0Reset(nCallState)) {
		return;
	}

	/* Read CSR0 */
	WRITE_REG(USB->INDEX, 0);
	byCSR0 = READ_REG(USB->CSR0_INCSR1);

	/* Check for status stage of a request */
	if (!(byCSR0 & M_CSR0_OUTPKTRDY)) {
		/* Complete SET_ADDRESS command */
		if (ep0state.byFAddr != 0xFF) {
			WRITE_REG(USB->FADDR, ep0state.byFAddr);
			SYS_USB_LOG("addr : %x\r\n", ep0state.byFAddr);
			if ((gnDevState == DEVSTATE_DEFAULT) && ep0state.byFAddr) {
				gnDevState = DEVSTATE_ADDRESS;
			} else if ((gnDevState == DEVSTATE_ADDRESS) && !ep0state.byFAddr) { /*  out of range ,add by zf */
				gnDevState = DEVSTATE_DEFAULT;
			}
		}
	}
	/* Clear pending commands */
	ep0state.byFAddr = 0xFF;

	/* Check for SentStall */
	if (byCSR0 & M_CSR0_SENTSTALL) {
		WRITE_REG(USB->CSR0_INCSR1, (byCSR0 & ~M_CSR0_SENDSTALL));
		ep0state.nState = M_EP0_IDLE;
	}

	/* Check for SetupEnd */
	if (byCSR0 & M_CSR0_SETUPEND) {
		WRITE_REG(USB->CSR0_INCSR1, (byCSR0 | M_CSR0_SVDSETUPEND));
		ep0state.nState = M_EP0_IDLE;
	}

	/* Call relevant routines for endpoint 0 state */
	if (ep0state.nState == M_EP0_RX) {
		if (ep0_rx_len <= 64) {
			Endpoint0_Rx();
			ep0state.nState = M_EP0_IDLE;
			USB_SendACK();
			return;
		}
	}
	if (ep0state.nState == M_EP0_IDLE) {
		/* If no packet has been received, */
		/* assume that this was a STATUS phase complete. */
		/* Otherwise load new command */
		if (byCSR0 & M_CSR0_INPKTRDY) {
			Endpoint0_Rx();
		} else if (byCSR0 & M_CSR0_OUTPKTRDY) {
			/* Read the 8-uint8_t command from the FIFO */
			/* There is no need to check that OutCount is set to 8 */
			/* as the MUSBFSFC will reject SETUP packets that are not 8 bytes long. */
			USB_Read(0, 8, &cmd);/*  add zf */
			SYS_USB_LOG("\n%02x %02x %04x %04x %04x\n", cmd.bmRequestType, cmd.bRequest, cmd.wValue, cmd.wIndex, cmd.wLength);
			Endpoint0_Command(&ep0state, &cmd);
		}
	}
	if (ep0state.nState == M_EP0_TX) {
		Endpoint0_Tx(&ep0state);
	}
}

void Endpoint0_Tx(USB_EndPointStatusDef *pep0state)
{
	uint32_t nBytes;
	uint8_t by;

	/* Determine number of bytes to send */
	if (pep0state->nBytesLeft <= M_EP0_MAXP) {
		nBytes = pep0state->nBytesLeft;
		pep0state->nBytesLeft = 0;
	} else {
		nBytes = M_EP0_MAXP;
		pep0state->nBytesLeft -= M_EP0_MAXP;
	}

	USB_Write(0, nBytes, pep0state->pData);/*  add zf */
	pep0state->pData = (uint8_t *)pep0state->pData + nBytes;
	if (nBytes < M_EP0_MAXP) {
		by = M_CSR0_INPKTRDY | M_CSR0_DATAEND;
		pep0state->nState = M_EP0_IDLE;
		WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT | M_OUTCSR_FF);
	} else {
		by = M_CSR0_INPKTRDY;
	}
	WRITE_REG(USB->CSR0_INCSR1, by);
}

__weak void Endpoint0_Rx(void)
{
	uint8_t rec_data[8] = { 0 };
	uint8_t recbytes;

	WRITE_REG(USB->INDEX, 0);

	recbytes = READ_REG(USB->OUT_COUNT1);
	USB_Read(0, recbytes, rec_data);

	WRITE_REG(USB->INDEX, 0);
	WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY);
}

uint32_t ConfigureIfs(void)
{
	USB_CfgDscrDef *pcfg;
	uint8_t byIf, byAltIf, byEP, byNumEPs, by;
	uint8_t         *pby;
	uint8_t         *pbyIfVal;
	USB_InterfaceDscrDef *pif;
	USB_EndPointDscrDef *pep;

	/* Set pointer to first interface descriptor in current configuration */
	pby = (uint8_t *)gpCurCfg;
	pby += sizeof(USB_CfgDscrDef);
	pif = (USB_InterfaceDscrDef *)pby;

	/* Loop through all interfaces in the current configuration */
	pcfg = (USB_CfgDscrDef *)gpCurCfg;
/*      SYS_USB_LOG("pcfg->bNumInterfaces:%d\n",pcfg->bNumInterfaces); */
	pbyIfVal = (uint8_t *)&gbyCurIfVal;
	for (byIf = 0; byIf < pcfg->bNumInterfaces; byIf++, pbyIfVal++) {
		/* Advance pointer to selected alternate interface descriptor */
		if (*pbyIfVal) {
			for (byAltIf = 0; byAltIf < *pbyIfVal; byAltIf++) {
				byNumEPs = pif->bNumEndpoints;
				pby += sizeof(USB_InterfaceDscrDef) + byNumEPs * sizeof(USB_EndPointDscrDef);
				pif = (USB_InterfaceDscrDef *)pby;
				/* Check an alternate setting > number of alternates not specified */
				if (!pif->bAlternateSetting) {
					return false;
				}
			}
		}

		/* Store pointer to interface in global array */
		gpCurIf[byIf] = pif;
		/* Loop through all endpoints in interface */
		byNumEPs = pif->bNumEndpoints;
		pby += sizeof(USB_InterfaceDscrDef);
		/*  #ifdef HAS_REPORT_DESC */
		pby += sizeof(USB_HidDscrDef);
		/*  #endif */
		for (byEP = 0; byEP < byNumEPs; byEP++) {
			pep = (USB_EndPointDscrDef *)pby;
			/* Configure the endpoint */
			WRITE_REG(USB->INDEX, (pep->bEndpointAddress & 0x0F));
			/* Round up max packet size to a multiple of 8 for writing to MaxP registers */
			by = (uint8_t)((pep->wMaxPacketSize + 7) >> 3);
			if (pep->bEndpointAddress & 0x80) {
				SYS_USB_LOG("ConfigureIfs USB->MAX_PKT_IN %d!!!\n", by);
				WRITE_REG(USB->MAX_PKT_IN, by);
				by = READ_REG(USB->IN_CSR2);
				switch (pep->bmAttributes & M_EP_TFMASK) {
				case M_EP_ISO:
					by |= M_INCSR2_ISO;
					break;
				case M_EP_BULK:
				case M_EP_INTR:
				default:
					by &= ~M_INCSR2_ISO;
					break;
				}
				/* Set mode bit high (only strictly necessary if sharing a FIFO) */
				by |= M_INCSR2_MODE;
				WRITE_REG(USB->IN_CSR2, by);
				/* Other configuration for an IN endpoint */
				/* e.g. AutoClr, DMA */
				/* should be added here */
			} else {
				SYS_USB_LOG("ConfigureIfs USB->MAX_PKT_OUT %d!!!\n", by);
				WRITE_REG(USB->MAX_PKT_OUT, by);
				by = READ_REG(USB->OUT_CSR2);
				switch (pep->bmAttributes & M_EP_TFMASK) {
				case M_EP_ISO:
					by |= M_OUTCSR2_ISO;
					break;
				case M_EP_BULK:
				case M_EP_INTR:
				default:
					by &= ~M_OUTCSR2_ISO;
					break;
				}
				WRITE_REG(USB->OUT_CSR2, by);
				/* Other configuration for an OUT endpoint */
				/* e.g. AutoSet, DMA */
				/* should be added here */
			}
			pby += sizeof(USB_EndPointDscrDef);
		}

		/* Skip any alternate setting interfaces */
		pif = (USB_InterfaceDscrDef *)pby;
		while (pif->bAlternateSetting) {
			byNumEPs = pif->bNumEndpoints;
			pby += sizeof(USB_InterfaceDscrDef) + byNumEPs * sizeof(USB_EndPointDscrDef);
			pif = (USB_InterfaceDscrDef *)pby;
		}
	}
	return true;
}
#pragma pack()