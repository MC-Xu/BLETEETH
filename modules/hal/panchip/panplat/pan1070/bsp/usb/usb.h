/**************************************************************************//**
 * @file     usb.h
 * @version  V1.00
 * $Revision: 3$
 * $Date: 20/08/04 14:50 $ 
 * @brief    Panchip series usb driver header file
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/ 

#ifndef __USB_H__
#define __USB_H__

#ifdef __cplusplus
extern "C"
{
#endif

//#define DEVSTATE_DEFAULT        0
//#define DEVSTATE_ADDRESS        1
//#define DEVSTATE_CONFIG         2


void USB_Read(uint32_t u32EPn, uint32_t u32Size, void * pDst);
void USB_Write(uint32_t u32EPn, uint32_t u32Size, void * pSrc);



#ifdef __cplusplus
}
#endif

#endif //__USB_H__

