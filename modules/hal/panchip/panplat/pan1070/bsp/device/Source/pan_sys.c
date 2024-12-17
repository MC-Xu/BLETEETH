/**************************************************************************//**
 * @file     sys.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 16/03/24 19:02 $
 * @brief    Panchip series SYS driver source file
 *
 * @note
 * Copyright (C) 2016 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "PanSeries.h"
#include "pan_sys.h"

/** @addtogroup Panchip_Device_Driver Panchip Device Driver
  @{
*/

/** @addtogroup Panchip_SYS_Driver SYS Driver
  @{
*/


/** @addtogroup Panchip_SYS_EXPORTED_FUNCTIONS SYS Exported Functions
  @{
*/

#define __nop()     __asm("nop")

void SYS_delay_10nop(uint32_t u32NopCnt)
{
	while(u32NopCnt--)
	{
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
	}
}

/**
  * @brief  This function check register write-protection bit setting
  * @return 0: Write-protection function is disabled.
  *         1: Write-protection function is enabled.
  */
uint32_t SYS_IsRegLocked(void)
{
    return !(SYS->REGLCTL & SYS_REGLCTL_REGLCTL_Msk);
}


/*@}*/ /* end of group Panchip_SYS_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group Panchip_SYS_Driver */

/*@}*/ /* end of group Panchip_Device_Driver */

/*** (C) COPYRIGHT 2016 Panchip Technology Corp. ***/
