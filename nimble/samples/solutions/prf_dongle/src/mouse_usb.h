/**************************************************************************
 * @file     mouse_usb.h
 * @version  V1.00
 * $Revision: 3$
 * $Date: 2023/11/21 $
 * @brief    Panchip series CLK driver header file
 *
 * @note
 * Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __MOUSE_USB_H__
#define __MOUSE_USB_H__

#include "PANSeries.h"
#include "common.h"

#define STAND_KB_ENDPOINT               1
#define COMPOSITE_ENDPOINT              2       /* contain mouse report id 5*/
#define VENDOR_ENDPOINT                 3       /* dfu & emi */

/**
 * @brief mouse usb Interface
 * @defgroup mouse usb Interface
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

enum usb_stat_t {
	usb_active,
	usb_suspending,
	usb_suspended,
	usb_resuming,
	usb_plug_out
};

void usb_init(void);
__ramfunc void usb_report(void);

/**@} */

#ifdef __cplusplus
}
#endif

#endif // __MOUSE_USB_H__

/*** (C) COPYRIGHT 2023 Panchip Technology Corp. ***/
