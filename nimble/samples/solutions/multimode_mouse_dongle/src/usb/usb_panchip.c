/*
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "usb_panchip.h"

void usb_self_plug_out(void)
{
//	ANA->ANA_MISC2 = 1;
	ANA->ANA_MISC_3V &= ~((0X1 << 26) | (0x1 << 27) | (0x1 << 28));
	/* need delay 200ms */
}

void usb_self_plug_in(void)
{
//	ANA->ANA_MISC2 = 500;
	ANA->ANA_MISC_3V |= ((0X1 << 26) | (0x1 << 27) | (0x1 << 28));
}

/**
 * @brief  This function read usb fifo
 * @param[in]  u32EPn:	endpoint number
 * @param[in]  u32Size:	data size
 * @param[in]  pDst:		data destination
 * @return None
 */
void USB_Read(uint32_t u32EPn, uint32_t u32Size, void *pDst)
{
	uint8_t *tmp = (uint8_t *)pDst;
	uint8_t *nAddr;

	nAddr = &((uint8_t *)&USB->FIFO_EP0)[u32EPn << 2];

	if (u32Size) {
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

static void data_printf(uint8_t const *data, uint32_t len)
{
	uint32_t i = 0;

	if (len == 0) {
		return;
	}

	for (; i < len; i++) {
		SYS_USB_LOG("%02X ", data[i]);
	}
	SYS_USB_LOG("\n");
}

void USB_Write(uint32_t u32EPn, uint32_t u32Size, void *pSrc)
{
	uint8_t *nAddr;
	uint8_t *tmp = (uint8_t *)pSrc;

	SYS_USB_LOG("len %d ep %d :", u32Size, u32EPn);

	data_printf(tmp, u32Size);

	nAddr = &((uint8_t *)&USB->FIFO_EP0)[u32EPn << 2];

	if (u32Size) {
		while (u32Size) {
			*nAddr = *tmp++;
			u32Size--;
		}
	}
}
