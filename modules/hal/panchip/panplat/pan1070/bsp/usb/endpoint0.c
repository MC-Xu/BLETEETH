/**************************************************************************//**
* @file     endpoint0.c
* @version  V1.00
* $Revision: 2 $
* $Date: 20/08/04 14:25 $
* @brief    Endpoint 0 interrupt routine for MUSBFSFC firmware
*
* @note
* Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "endpoint0.h"
#include "descript.h"
#include "PanSeries.h"
#include "endpoint.h"
#include "usb.h"
//#define SYS_TEST /*.....*/

/* Pointer to current configuration descriptor */
void *          gpCurCfg = NULL;
/* Current device state */
uint32_t	gnDevState = DEVSTATE_DEFAULT;

/* Pointer to current interface descriptors */
USB_InterfaceDscrDef *gpCurIf[M_MAXIFS];

/* Current alternate interface values */
uint8_t            gbyCurIfVal[M_MAXIFS];

/****************************************
  Standard Device Request
  This routine is called when a standard device request has been received.
  The bRequest field of the received command is decoded.
****************************************/
void StdDevReq (USB_EndPointStatusDef *pep0state, USB_CommandDef *pcmd)
{
	uint32_t	bError = FALSE, bNoData = FALSE, n;
	uint8_t		by;
	uint8_t		byConfig;
	USB_CfgDscrDef *	pcfg;
	uint8_t *		*ppby;

//	SYS_TEST("pcmd->bRequest:%d,pcmd->wValue:%x\r\n",pcmd->bRequest,pcmd->wValue);
	switch (pcmd->bRequest) {
		case SET_ADDRESS:
			SYS_TEST("S_ADDR\r\n");
			/* Store device function address until status stage of request */
			if (pcmd->bmRequestType != M_CMD_STDDEVOUT)
				bError = TRUE;
			else if (gnDevState <= DEVSTATE_ADDRESS) {
				pep0state->byFAddr = (uint8_t)pcmd->wValue;
				bNoData = TRUE;
			}
			else
				bError = TRUE;
		break;
		case GET_DESCRIPTOR:
		/* Decode the required descriptor from the command */
			if (pcmd->bmRequestType == M_CMD_STDIFIN)
			{ 
				pep0state->nBytesLeft = sizeof(report);
				pep0state->pData = (void*)report;
				pep0state->nState = M_EP0_TX;	
			}
			else if(pcmd->bmRequestType == M_CMD_STDDEVIN){
				switch (pcmd->wValue & M_CMD_DESCMASK) {
					case M_CMD_DEVICE:
						SYS_TEST("DEVICE\r\n");
						/* Prepare to return Standard Device Descriptor */
						pep0state->nBytesLeft = sizeof(USB_DeviceDscrDef);
						/* Check host is allowing a descriptor this long */
						if (pcmd->wLength < pep0state->nBytesLeft)
							pep0state->nBytesLeft = pcmd->wLength;
						pep0state->pData = (void*)&stddevdsc;
						pep0state->nState = M_EP0_TX;
					break;
					case M_CMD_DEVQUAL:
						SYS_TEST("DEVQUAL\r\n");
						/* No alternate speed supported */
						bError = TRUE;
					break;
					case M_CMD_CONFIG:
						SYS_TEST("CFG\r\n");
						byConfig = (uint8_t)(pcmd->wValue & 0x00FF);
//						SYS_TEST("byConfig:%d\r\n",byConfig);
						if (byConfig >= stddevdsc.bNumConfigurations)
							bError = TRUE;
						else {
							/* Get pointer to requested configuration descriptor */
							ppby = (uint8_t**)&cfg;
							ppby += byConfig;
							pcfg = (USB_CfgDscrDef *)*ppby;
							/* Prepare to return Configuration Descriptors */
							pep0state->nBytesLeft = (int)pcfg->wTotalLength;
							pep0state->pData = (void*)pcfg;
							pep0state->nState = M_EP0_TX;
							/* Check host is allowing a descriptor this long */
							if (pcmd->wLength < pep0state->nBytesLeft)
								pep0state->nBytesLeft = pcmd->wLength;
						}
					break;
					case M_CMD_OTHERSPEED:
						/* No alternate speed supported */
						bError = TRUE;
					break;	
					case M_CMD_REPORT:
						SYS_TEST("report\r\n");
						break;	
					default: bError = TRUE;
				}
			}
			else{
				bError = TRUE;
			}
		break;
		case SET_CONFIGURATION:
			SYS_TEST("S_CFG\r\n");
			byConfig = (uint8_t)(pcmd->wValue & 0x00FF);
			//	SYS_TEST("set_config : %x %x %x %x %x \r\n",pcmd->bmRequestType,pcmd->bRequest,pcmd->wValue,pcmd->wIndex,pcmd->wLength);
			if (gnDevState == DEVSTATE_DEFAULT)
				bError = TRUE;
			/* Assumes configurations are numbered 1 to NumConfigurations */
			else if (byConfig > stddevdsc.bNumConfigurations)
				bError = TRUE;
			else if (!byConfig)
				gnDevState = DEVSTATE_ADDRESS;
			else {
				/* Get pointer to requested configuration descriptor */
				ppby = (uint8_t**)&cfg;
				ppby += byConfig-1;
				gpCurCfg = (void *)*ppby;
				/* Set all alternate settings to zero */
				for (n=0; n<M_MAXIFS; n++)
					gbyCurIfVal[n] = 0;
				/* Configure endpoints */
				ConfigureIfs();
				gnDevState = DEVSTATE_CONFIG;
				WRITE_REG(USB->INDEX, 0);
				bNoData = TRUE;
			}
		break;
		case SET_INTERFACE:
			SYS_TEST("S_INF\r\n");
			if (gnDevState > DEVSTATE_ADDRESS) {
				n = (int)pcmd->wIndex;
				pcfg = (USB_CfgDscrDef *)gpCurCfg;
				if ((uint8_t)n >= pcfg->bNumInterfaces)
					bError = TRUE;
				else {
					gbyCurIfVal[n] = (uint8_t)pcmd->wValue;
					if (ConfigureIfs() == FALSE)
						bError = TRUE;
					WRITE_REG(USB->INDEX, 0);
					bNoData = TRUE;
				}
			}
			else
				bError = TRUE;
		break;

		case GET_CONFIGURATION:
			SYS_TEST("G_CFG\r\n");
			if ((gnDevState == DEVSTATE_ADDRESS) && (!pcmd->wValue)) {
				/* Prepare to return zero */
				pep0state->nBytesLeft = 1;
				pep0state->pData = (void*)&pcmd->wValue;
				pep0state->nState = M_EP0_TX;
			}
			else if (gnDevState > DEVSTATE_ADDRESS) {
				/* Prepare to return configuration value */
				pcfg = (USB_CfgDscrDef *)gpCurCfg;
				pep0state->nBytesLeft = 1;
				pep0state->pData = (void*)&pcfg->bConfigurationValue;
				pep0state->nState = M_EP0_TX;
			}
			else
			bError = TRUE;
		break;

		case GET_INTERFACE:
			SYS_TEST("G_INTF\r\n");
			if (gnDevState > DEVSTATE_ADDRESS) {
				n = (int)pcmd->wIndex;
				pcfg = (USB_CfgDscrDef *)gpCurCfg;
				if ((uint8_t)n >= pcfg->bNumInterfaces)
					bError = TRUE;
				else {
					/* Prepare to return interface value */
					pep0state->nBytesLeft = 1;
					pep0state->pData = (void*)&gpCurIf[n]->bInterfaceNumber;
					pep0state->nState = M_EP0_TX;
				}
			}
			else
				bError = TRUE;
		break;

		case SET_FEATURE:
			SYS_TEST("S_FT\r\n");
			switch (pcmd->bmRequestType) {
				case M_CMD_STDDEVOUT:
					switch (pcmd->wValue) {
						case M_FTR_DEVREMWAKE:
						default:
						bError = TRUE;
					break;
				}
				break;
				case M_CMD_STDIFIN:
				/* Add interface features here */
				case M_CMD_STDEPIN:
				/* Add endpoint features here */
				default:bError = TRUE;break;
			}
		break;
			
		case CLEAR_FEATURE:
			SYS_TEST("C_FT\r\n");
			break;	
		default:
			SYS_TEST("bError\r\n");
			/* Stall the command if an unrecognized request is received */
			bError = TRUE;
		break;
	}

	/* Stall command if an error occurred */
	if (bError)
		by = M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL;
	else if (bNoData)
		by = M_CSR0_SVDOUTPKTRDY | M_CSR0_DATAEND;
	else
		by = M_CSR0_SVDOUTPKTRDY;
	WRITE_REG(USB->CSR0_INCSR1, by);
}


#define GET_REPORT		(0X1)
#define GET_IDLE		(0X2)
#define GET_PROTOCOL	(0X3)
#define SET_REPORT		(0X9)
#define SET_IDLE		(0XA)
#define SET_PROTOCOL	(0XB)

void StdClassReq(USB_CommandDef *pcmd)
{
//	uint8_t	reqType;
	
	switch(pcmd->bRequest){
		case GET_REPORT:
			break;
		case SET_IDLE:
			break;		
		default:break;
	}
}

/****************************************
  Endpoint 0 command
  This routine is called when a command has been received in the SETUP phase.
****************************************/
void Endpoint0_Command (USB_EndPointStatusDef *pep0state, USB_CommandDef *pcmd)
{
//	uint8_t	byType;

	/* Check request type */
	switch (pcmd->bmRequestType & M_CMD_TYPEMASK) {
		case M_CMD_STDREQ:
			StdDevReq (pep0state, pcmd);break;
		case M_CMD_CLASSREQ:
//			SYS_TEST("class req \r\n");
//			break;
		/* Add call to external routine for handling class requests */
		case M_CMD_VENDREQ:
		/* Add call to external routine for handling vendor requests */
		default:
			/* Stall the command if a reserved request is received */
			WRITE_REG(USB->CSR0_INCSR1, M_CSR0_SVDOUTPKTRDY | M_CSR0_SENDSTALL);
			break;
	}
}

/****************************************
  Endpoint 0 interrupt service routine
****************************************/
void Endpoint0(int32_t nCallState)
{
	static USB_EndPointStatusDef		ep0state;
	static USB_CommandDef		cmd;
	uint8_t	byCSR0;

	/* Check for USB reset of endpoint 0 */
	if (nCallState == M_EP_RESET) {
		ep0state.nState = M_EP0_IDLE;
		ep0state.byFAddr = 0xFF;
		/* Clear current configuration pointer */
		gpCurCfg = NULL;
		return;
	}
	
 	 /* Read CSR0 */
	WRITE_REG(USB->INDEX, 0);
	byCSR0 = READ_REG(USB->CSR0_INCSR1);

	/* Check for status stage of a request */
	if (!(byCSR0 & M_CSR0_OUTPKTRDY)) {
		/* Complete SET_ADDRESS command */
		if (ep0state.byFAddr != 0xFF) {
			WRITE_REG(USB->FADDR,ep0state.byFAddr); 
			SYS_TEST("addr : %x\r\n",ep0state.byFAddr);
			if ((gnDevState == DEVSTATE_DEFAULT) && ep0state.byFAddr)
				gnDevState = DEVSTATE_ADDRESS;
			else if ((gnDevState == DEVSTATE_ADDRESS) && !ep0state.byFAddr)	//out of range ,add by zf
				gnDevState = DEVSTATE_DEFAULT;
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

	if (ep0state.nState == M_EP0_IDLE) {
	/* If no packet has been received, */
	/* assume that this was a STATUS phase complete. */
	/* Otherwise load new command */
		if (byCSR0 & M_CSR0_OUTPKTRDY) {
			/* Read the 8-uint8_t command from the FIFO */
			/* There is no need to check that OutCount is set to 8 */
			/* as the MUSBFSFC will reject SETUP packets that are not 8 bytes long. */
			USB_Read (0, 8, &cmd); 					//add zf
			Endpoint0_Command (&ep0state, &cmd);
		}
	}
	if (ep0state.nState == M_EP0_TX){
		Endpoint0_Tx(&ep0state);
	}
}


/****************************************
  Endpoint 0 Tx
  This routine is called when data is to be sent to the host.
****************************************/
void Endpoint0_Tx (USB_EndPointStatusDef *pep0state)
{
	uint32_t nBytes;
	uint8_t	by;

	/* Determine number of bytes to send */
	if (pep0state->nBytesLeft <= M_EP0_MAXP) {
		nBytes = pep0state->nBytesLeft;
		pep0state->nBytesLeft = 0;
	}
	else {
		nBytes = M_EP0_MAXP;
		pep0state->nBytesLeft -= M_EP0_MAXP;
	}

	USB_Write (0, nBytes, pep0state->pData);				//add zf
	pep0state->pData = (uint8_t *)pep0state->pData + nBytes;
	if (nBytes < M_EP0_MAXP) {
		by = M_CSR0_INPKTRDY | M_CSR0_DATAEND;
		pep0state->nState = M_EP0_IDLE;
	}
	else
		by = M_CSR0_INPKTRDY;
		WRITE_REG(USB->CSR0_INCSR1, by);
}

/****************************************
  ConfigureIfs
  This routine is called when a SET_CONFIGURATION or SET_INTERFACE
  command is received and will set the gpCurIF pointers
  to the selected interface descriptors and will set the
  maximum packet size and operating mode for the endpoints in
  the selected interfaces
****************************************/
uint32_t ConfigureIfs(void)
{
	USB_CfgDscrDef *	pcfg;
	uint8_t		byIf, byAltIf, byEP, byNumEPs, by;
	uint8_t		*pby;
	uint8_t		*pbyIfVal;
	USB_InterfaceDscrDef *	pif;
	USB_EndPointDscrDef *	pep;

	/* Set pointer to first interface descriptor in current configuration */
	pby = (uint8_t *)gpCurCfg;
	pby += sizeof(USB_CfgDscrDef);
	pif = (USB_InterfaceDscrDef *)pby;

	/* Loop through all interfaces in the current configuration */
	pcfg = (USB_CfgDscrDef *)gpCurCfg;
	pbyIfVal = (uint8_t*)&gbyCurIfVal;
	for (byIf=0; byIf<pcfg->bNumInterfaces; byIf++, pbyIfVal++) {
		/* Advance pointer to selected alternate interface descriptor */
		if (*pbyIfVal) {
			for (byAltIf=0; byAltIf<*pbyIfVal; byAltIf++) {
				byNumEPs = pif->bNumEndpoints;
				pby += sizeof(USB_InterfaceDscrDef) + byNumEPs * sizeof(USB_EndPointDscrDef);
				pif = (USB_InterfaceDscrDef *)pby;
				/* Check an alternate setting > number of alternates not specified */
				if (!pif->bAlternateSetting)
					return FALSE; 
			} 
		}

		/* Store pointer to interface in global array */
		gpCurIf[byIf] = pif;
		/* Loop through all endpoints in interface */
		byNumEPs = pif->bNumEndpoints;
		pby += sizeof(USB_InterfaceDscrDef);
		#ifdef HAS_REPORT_DESC
		pby += sizeof(USB_HidDscrDef);			//zf add
		#endif
		for (byEP=0; byEP<byNumEPs; byEP++) {
			pep = (USB_EndPointDscrDef *)pby;
			/* Configure the endpoint */
			WRITE_REG(USB->INDEX, (pep->bEndpointAddress & 0x0F));
			/* Round up max packet size to a multiple of 8 for writing to MaxP registers */
			by = (uint8_t)((pep->wMaxPacketSize+7)>>3);
			if (pep->bEndpointAddress & 0x80) {
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
			}
			else {
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

	return TRUE;
}
