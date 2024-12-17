/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#include "rgb_light.h"
#include <string.h>
#include <stdio.h>
#include "PanSeries.h"
#include "pan_pwm.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "ble_rgb_sens.h"


#define RGB_LIGHT_FREQ                          200                   /* 5khz */

static uint8_t onoff_flag = 1;

void show_reg(uint8_t const *data, uint32_t len)
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


uint32_t write_rgb(uint8_t *buf, uint16_t len)
{
	uint8_t *value = (uint8_t *)buf;
	uint8_t data[2] = { 0x16, 0xbb };
	const uint8_t arr1[10] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
	const uint8_t arr2[10] = { 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (memcmp(value, arr1, 10) == 0 || memcmp(value, arr2, 10) == 0) {

	} else {
		show_reg(value, len);
	}
	/* Set RGB */
	if ((len == 5) && (value[0] == 0xAA) &&
	 (value[1] == 0x16) && (onoff_flag == 1)) {
		light_set_rgb(value[2] * 10, value[3] * 10, value[4] * 10);
	} else if ((len == 2) && (value[0] == 0xAA) &&
	 (value[1] == 0x03)) {
		printf("close light\r\n");
		onoff_flag = 0;
		light_set_rgb(0, 0, 0);
	} else if ((len == 2) && (value[0] == 0xAA) &&
	 ((value[1] == 0x02) || (value[1] == 0x06))) {
		printf("open light\r\n");
		light_set_rgb(value[2] * 10, value[3] * 10, value[4] * 10);
		onoff_flag = 1;
	}
	rgb_notify(data, sizeof(data));

	return len;
}

void rgb_notify(uint8_t *data, uint8_t len)
{
	extern void notify_data(uint8_t *data, uint16_t len);
	notify_data(data, len);
}

static void pwn_start(uint8_t ch, uint32_t duty_cycle)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(PWM, ch);
    // Config Target Output Channel
    PWM_ConfigOutputChannel(PWM, ch, RGB_LIGHT_FREQ, duty_cycle, OPERATION_EDGE_ALIGNED);
	
	PWM_EnableIndependentMode(PWM);
	
    // Enable output of TGT_PWM channel
    PWM_EnableOutput(PWM, BIT(ch));
    // Start
    PWM_Start(PWM, BIT(ch));
}

void light_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	pwn_start(PWM_CH7, r);
	pwn_start(PWM_CH4, g);
	pwn_start(PWM_CH2, b);
}

void light_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_All, ENABLE);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH23, ENABLE);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH45, ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH67, ENABLE);
    
	SYS_SET_MFP(P1, 1, PWM_CH7);
    SYS_SET_MFP(P1, 2, PWM_CH4);
    SYS_SET_MFP(P1, 4, PWM_CH2);
	
	light_set_rgb(0, 100, 0);
}
