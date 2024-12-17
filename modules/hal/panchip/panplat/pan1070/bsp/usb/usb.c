/**************************************************************************//**
* @file     usb.c
* @version  V1.00
* $Revision: 2 $
* $Date: 20/08/04 14:25 $
* @brief    Panchip series usb driver source file
*
* @note
* Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
*****************************************************************************/

#include "PanSeries.h"
#include "usb.h"

/**
* @brief  This function read usb fifo
* @param[in]  u32EPn:	endpoint number
* @param[in]  u32Size:	data size
* @param[in]  pDst:		data destination
* @return None
*/
void USB_Read(uint32_t u32EPn, uint32_t u32Size, void * pDst)
{
	uint8_t *tmp = (uint8_t *)pDst;
	uint8_t *nAddr;
	
	nAddr = &((uint8_t *)&USB->FIFO_EP0)[u32EPn<<2];

	if(u32Size){
		while (u32Size) {
			*tmp++ = *nAddr;
			u32Size--;
		}
	}
}


/**
* @brief  This function write usb fifo
* @param[in]  nEP:		endpoint number
* @param[in]  nBytes:	data size
* @param[in]  pSrc:		data source
* @return None
*/
void USB_Write(uint32_t u32EPn, uint32_t u32Size, void * pSrc)
{
	uint8_t *nAddr;
	uint8_t *tmp = (uint8_t *)pSrc;
	
	nAddr = &((uint8_t *)&USB->FIFO_EP0)[u32EPn<<2];

	if(u32Size){
		while (u32Size) {
			*nAddr = *tmp++;
			u32Size--;
		}
	}
}

