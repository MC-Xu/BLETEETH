/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "signal_slot_manager.h"
#include "uart_dfu.h"
#include "usb_dfu.h"
#include "flash_manager.h"
#include "prf_ota.h"
#include "boot_config.c"

static void on_image_load_enter(void)
{
	fm_image_move(FLASH_AREA_BACK_UP_START, FLASH_AREA_IMAGE_START);
}

void sys_clock_init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
    
    ANA->LP_FSYN_LDO |= 0X1;

	CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);

	CLK_HCLKConfig(0);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M); 
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	
	SYS_LockReg();
}

static void jump_to_app(void)
{
    void (*app_init)();
    
    uint32_t msp = *(volatile uint32_t*)(FLASH_AREA_IMAGE_START + sizeof(image_header_t));
    uint32_t addr = *(uint32_t*)(FLASH_AREA_IMAGE_START + sizeof(image_header_t) + 4);
    app_init = (void(*)())(uint32_t*)addr;
	
	FLCTL->X_FL_REMAP_ADDR = FLASH_AREA_IMAGE_START + sizeof(image_header_t); /* fmc remap */
	
    __set_MSP(msp);
	
    app_init();
}

int main(void)
{	
	sys_clock_init();
	
	/* when checking backup image is valid, the on_image_load_enter function will be handled */
	ss_connect(0, sig_back_up_is_completed_image, on_image_load_enter);
	
	#if BOOT_FROM_UART
	/* when detecting key1 down, the on_uart_dfu_enter function will be handled */
	ss_connect(1, sig_key1_push_down, on_uart_dfu_enter);
	#endif
	
	#if BOOT_FROM_USB
	/* when dectecting key2 down, the on_usb_dfu_enter function will be handled */
	ss_connect(2, sig_key2_push_down, on_usb_dfu_enter);
	#endif
	
	#if BOOT_FROM_PRF_OTA
	/* when receive a ota start packet, the on_prf_ota_enter function will be handled */
	ss_connect(3, sig_ota_start_received, on_prf_ota_enter);
	#endif
	
	/* handle all of events related signal fuction*/
	ss_events_handle(); 
	
	/* recovery gpio status that you used to trigger signal */
	sig_hardware_recovery();
	
	jump_to_app();
	
	return 0;  
}
