/**************************************************************************//**
* @file     endpoint.c
* @version  V1.00
* $Revision: 2 $
* $Date: 20/08/04 14:25 $
* @brief    Endpoint interrupt routine for MUSBFSFC firmware
*
* @note
* Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "endpoint.h"
#include "PanSeries.h"
#include "usb.h"

USB_BulkInStatus  gepbin1 = {0,}, gepbin2 = {0,}, gepbin3 = {0,};
USB_BulkOutStatus gepbout1 = {0,}, gepbout2 = {0,}, gepbout3 = {0,};
/****************************************
  EndpointBulkIn service routine
  This routine provides simple handling for bulk in endpoints.
  Endpoint MaxP is defined in endpoint.h.
  While the nBytesLeft field of the pbostate structure is >= 0
  the endpoint will send packets of MaxP, with the residue in the last packet.
  If the data to send is an exact multiple of MaxP then a null data packet
  will be sent.
****************************************/
void EndpointBulkIn(USB_BulkInStatus *pbistate, uint32_t nCallState)
{
	uint32_t     nBytes;
	uint8_t    byInCSR;

	/* Reset endpoint */
	if (nCallState == M_EP_RESET) {
		pbistate->nBytesLeft = M_EP_NODATA;
		WRITE_REG(USB->INDEX, pbistate->byEP);
		
		byInCSR = M_INCSR_CDT | M_INCSR_FF;
		WRITE_REG(USB->CSR0_INCSR1, byInCSR);
		return;
	}
	
	/* Check whether there is any data to send */ 
	if (pbistate->nBytesLeft == M_EP_NODATA)
		return;

	/* Determine number of bytes to send */
	if (pbistate->nBytesLeft < M_EP_MAXP) {
		nBytes = pbistate->nBytesLeft;
		pbistate->nBytesLeft = M_EP_NODATA;
	}
	else {
		nBytes = M_EP_MAXP;
		pbistate->nBytesLeft -= M_EP_MAXP;
	}

	///* Load FIFO */
	WRITE_REG(USB->INDEX, pbistate->byEP);
	USB_Write((uint32_t)pbistate->byEP, nBytes, pbistate->pData);
	pbistate->pData = (uint8_t *)pbistate->pData + nBytes;

	byInCSR = M_INCSR_IPR;
	WRITE_REG(USB->CSR0_INCSR1, byInCSR);
}


/****************************************
  EndpointBulkOut service routine
  This routine provides simple handling for bulk out endpoints.
  Endpoint MaxP is defined in endpoint.h.
  It assumes a transfer is complete when a packet of less than MaxP is received.
****************************************/
void EndpointBulkOut (USB_BulkOutStatus *pbostate, uint32_t nCallState)
{
	uint32_t     nBytes;
	uint8_t    byOutCSR;	

	/* Reset endpoint */
	if (nCallState == M_EP_RESET) {
		pbostate->nBytesRecv = 0;
		pbostate->nBuffLen = 0;
		WRITE_REG(USB->INDEX, pbostate->byEP);
		byOutCSR = M_OUTCSR_CDT | M_OUTCSR_FF;
		WRITE_REG(USB->OUT_CSR1, byOutCSR);
		return;
	}

	/* Set index register */
	WRITE_REG(USB->INDEX, pbostate->byEP);
	/* Read OUT CSR register */
	byOutCSR = READ_REG(USB->OUT_CSR1);

	/* Get packet, */
	/* may need to unload more than one packet if double buffering enabled */
	while (byOutCSR & M_OUTCSR_OPR) {
		/* Get packet size */
		nBytes = (int)READ_REG(USB->OUT_COUNT2);
		nBytes <<= 8;
		nBytes |= (int)READ_REG(USB->OUT_COUNT1);
		pbostate->nBuffLen = READ_REG(USB->MAX_PKT_OUT)<<3;
		
		/* Check there is room in the buffer */
//		SYS_TEST("AA :%x,%x,%x \r\n",pbostate->nBytesRecv,nBytes,pbostate->nBuffLen);
		if (pbostate->nBytesRecv + nBytes > pbostate->nBuffLen) {
			/* Call to function to handle buffer over run */
			WRITE_REG(USB->OUT_CSR1, 0);
			return;
		}

	  /* Unload FIFO */
		USB_Read ((int)pbostate->byEP, nBytes, pbostate->pData);
		/* Update status */
		pbostate->nBytesRecv += nBytes;
		pbostate->pData = (uint8_t *)pbostate->pData + nBytes;

		/* Check for end of transfer */
		if (nBytes < M_EP_MAXP) {
		/* Call function to handle received data */
		}

		/* Clear OutPktRdy */
		WRITE_REG(USB->OUT_CSR1, 0);
		/* Check for another packet */
		byOutCSR = READ_REG(USB->OUT_CSR1);
//		WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT);
	}
}

#if 1
void USB_DmaTransfer(uint8_t *pBuffer,uint8_t u8Ep,uint32_t data_len)														
{
//	uint32_t	ret= 0,i=0,k=0;
	uint32_t  usb_pkt_size,usbdma_pkt_size;

	WRITE_REG (USB->INDEX,u8Ep);
	if(data_len > 32){
		usbdma_pkt_size = DmaMode1Max4;
		WRITE_REG(USB->MAX_PKT_IN, 4);
	}
	else{
		usb_pkt_size = (data_len/8);
		usbdma_pkt_size = (data_len/8)<<8;
		WRITE_REG(USB->MAX_PKT_IN, usb_pkt_size);
	}

//	WRITE_REG (USB->IN_CSR2,M_INCSR2_MODE | M_INCSR2_DMAEnab | M_INCSR2_AUTOSET ); //set as in mode,AUTO SET INPKTRDY

	//set DMA CONTROL REGISTER
	WRITE_REG (USBDMA->ADDR1,(uint32_t)pBuffer); //ADDR1 32
	WRITE_REG (USBDMA->COUNT1, data_len);
//	WRITE_REG (USBDMA->CNTL1,(EnableDMA | DIR_IN |            (u8Ep<<4) | IntEnab| usbdma_pkt_size)); //CNTRL 
	WRITE_REG (USBDMA->CNTL1,(EnableDMA | DIR_IN | DMAMode1 | (u8Ep<<4) | IntEnab| usbdma_pkt_size)); //CNTRL 

//	while(!(READ_REG(USB->CSR0_INCSR1) & 0x01));//wait inpktrdy set
//	while((READ_REG(USB->CSR0_INCSR1) & 0x01));	//wait data sent
//																																		 
//	//SECOND PACKET	
//	WRITE_REG(USBDMA->CNTL1,(EnableDMA | DIR_IN | DMAMode1 | (u8Ep<<4) | IntEnab | DmaMode1Max4)); //CNTRL 
//	//	WRITE_REG (COUNT1, 64);

//	//	while(!(READ_REG(USB->CSR0_INCSR1) & 0x01));//wait inpktrdy set
//	while((READ_REG(USB->CSR0_INCSR1) & 0x02));	//wait data sent

//	//THIRD PACKET	
//	WRITE_REG (USB->MAX_PKT_IN, 1);
//	WRITE_REG (USBDMA->CNTL1,(EnableDMA | DIR_IN | DMAMode1 | (u8Ep<<4) | IntEnab | DmaMode1Max1)); //CNTRL 
//	WRITE_REG (USB->CSR0_INCSR1, 1);

//	//	while(!(READ_REG(USB->CSR0_INCSR1) & 0x01));//wait inpktrdy set
//	while((READ_REG(USB->CSR0_INCSR1) & 0x02));	//wait data sent
}

void USB_DmaReceivePre(uint8_t *pBuffer,uint8_t u8Ep,uint32_t data_len)														
{
//	uint32_t  ret= 0,i=0,k=0;
	uint32_t  /*reg_outcnt,*/usb_pkt_size,usbdma_pkt_size;

	//PID_OUT TEST
	WRITE_REG(USBDMA->CNTL1,DIR_OUT); //CNTRL    
	WRITE_REG(USB->INDEX,u8Ep);
	if(data_len > 32){
		usbdma_pkt_size = DmaMode1Max4;
		WRITE_REG(USB->MAX_PKT_OUT, 4);
	}
	else{
		usb_pkt_size = (data_len/8);
		usbdma_pkt_size = (data_len/8)<<8;
		WRITE_REG(USB->MAX_PKT_OUT, usb_pkt_size);
	}

//	WRITE_REG(USB->OUT_CSR2, M_OUTCSR2_DMAEnab | M_OUTCSR2_DMAMode | M_OUTCSR2_AutoClear);

	WRITE_REG(USBDMA->ADDR1,(uint32_t)pBuffer); //ADDR1 32
	WRITE_REG(USBDMA->COUNT1, data_len);
	WRITE_REG(USBDMA->CNTL1,(EnableDMA | DIR_OUT | DMAMode1 | EndPoint2 | IntEnab | usbdma_pkt_size)); //CNTRL 
//	reg_outcnt = (READ_REG(USB->OUT_COUNT1) | (READ_REG(USB->OUT_COUNT1)<<8));
//	SYS_TEST("USBDMA->CNTL1:%x \r\n",USBDMA->CNTL1);
}

#endif

