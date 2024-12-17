/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "comm_prf.h"
#include "mouse_prf.h"
#include "ring_buffer.h"
#include "common.h"
#include "info.h"

bool test_mode_auto_circle = true;
struct ring_buf ringbuf_raw;
uint8_t ringbuf_data[RINGBUFFER_SIZE];
uint16_t report_rate = 1000;

void auto_circle(int16_t *x_value, int16_t *y_value)
{
	static volatile uint16_t usb_cycle_cnt;
	#define CIRCLE_SIZE 250

	if (usb_cycle_cnt < CIRCLE_SIZE) {
		*x_value = 0;
		*y_value = 2;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 2 * CIRCLE_SIZE) {
		*x_value = 2;
		*y_value = 0;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 3 * CIRCLE_SIZE) {
		*x_value = 0;
		*y_value = -2;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 4 * CIRCLE_SIZE) {
		*x_value = -2;
		*y_value = 0;
		usb_cycle_cnt++;
	} else {
		*x_value = 0;
		*y_value = 2;
		usb_cycle_cnt = 1;
	}
}

void Sys_Init(void)
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init System Clock                                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Unlock protected registers */
	SYS_UnlockReg();

	ANA->LP_FSYN_LDO &= ~0X1;
	CLK->XTH_CTRL &= ~CLK_XTHCTL_XTH_EN_Msk;

	ANA->LP_FSYN_LDO |= 0X1;
	// MCU
	CLK->XTH_CTRL = (CLK->XTH_CTRL & ~CLK_XTHCTL_XO_CAP_SEL_Msk) | (0 << CLK_XTHCTL_XO_CAP_SEL_Pos);
	CLK_XthStartupConfig();

	CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
	// MCU
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_48M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	// APB Enable
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All, ENABLE);

	CLK_APB1PeriphClockCmd(CLK_APB1Periph_All, ENABLE);
	CLK_APB2PeriphClockCmd(CLK_APB2Periph_All, ENABLE);
	SYS_LockReg();
}

void Uart_Init(void)
{
	SYS_SET_MFP(P1, 7, UART0_RX);
	SYS_SET_MFP(P1, 6, UART0_TX);
	GPIO_EnableDigitalPath(P1, BIT7);

	UART_InitTypeDef Init_Struct;

	Init_Struct.UART_BaudRate = 921600;
	Init_Struct.UART_LineCtrl = Uart_Line_8n1;

	/* Init UART1 for printf */
	UART_Init(UART0, &Init_Struct);
	UART_EnableFifo(UART0);
}

void data_printf(uint8_t const *data, uint32_t len)
{
	uint32_t i = 0;

	if (len == 0) {
		return;
	}
	for (; i < len; i++) {
		printf("0x%02X ", data[i]);
	}
	printf("\n");
}

void SysTick_Handler(void)
{
	prf_data_process();
}

int main(void)
{
	OTP_STRUCT_T otp;
	
	Sys_Init();

	Uart_Init();

	GPIO_EnableDigitalPath(P2, BIT0 | BIT1);
	SYS_SET_MFP(P2, 0, GPIO);
	SYS_SET_MFP(P2, 1, GPIO);
	GPIO_SetMode(P2, BIT0 | BIT1, GPIO_MODE_OUTPUT);

	SYS_TEST("Try to load HW calibration data..");
	if (!SystemHwParamLoader(&otp)) {
		SYS_TEST("\nWARNING: Cannot find valid calib data in current chip!\n");
	} else {
		SYS_TEST(" DONE.\n");
		SYS_TEST("- Chip Info         : 0x%x\n", otp.m.chip_info);
		SYS_TEST("- Chip CP Version   : %d\n", otp.m.cp_version);
		SYS_TEST("- Chip FT Version   : %d\n", otp.m.ft_version);
		SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
			 otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
		SYS_TEST("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
			 otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);
	}
	SYS_TEST("- Chip Flash UID    : ");
	for (uint32_t i = 0; i < 16; i++) {
		SYS_TEST("%02X", flash_ids.uid[i]);
	}
	SYS_TEST("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

	printf("check info\n");
    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}
	
	ring_buf_init(&ringbuf_raw, RINGBUFFER_SIZE, ringbuf_data);

	mouse_prf_init();

	printf("mouse sample init2\n");

	SysTick_Config(48000000 / report_rate);

	while (1) {

	}
}