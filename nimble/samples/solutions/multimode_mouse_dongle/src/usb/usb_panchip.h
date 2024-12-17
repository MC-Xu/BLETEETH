/*
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __USB_H__
#define __USB_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SYS_TEST_USB           0
#if SYS_TEST_USB
	#define SYS_USB_LOG        printf
#else
	#define SYS_USB_LOG(...)
#endif

// #define __ramfunc __attribute__((section(".ram.text")))

void USB_Read(uint32_t u32EPn, uint32_t u32Size, void *pDst);
void USB_Write(uint32_t u32EPn, uint32_t u32Size, void *pSrc);

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */
