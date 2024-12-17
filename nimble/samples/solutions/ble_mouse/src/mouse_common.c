/*
 * Copyright (c) 2020 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "mouse_common.h"

bool test_mode_auto_circle = false;
uint8_t ringbuf_data[RINGBUFFER_SIZE];
uint16_t report_rate = 1000;

SemaphoreHandle_t sem_timer = NULL;

void GPIO0_IRQHandler(void)
{
	printf("GPIO ISR in..\n");

	if (GPIO_GetIntFlag(P0, BIT4)) {
		GPIO_ClrIntFlag(P0, BIT4);
		printf("P04 occurred\n");
		if (test_mode_auto_circle) {
			printf("test_mode_auto_circle off\n");
			test_mode_auto_circle = false;
		} else {
			printf("test_mode_auto_circle on\n");
			test_mode_auto_circle = true;
		}
	}
	if (GPIO_GetIntFlag(P0, BIT5)) {
		GPIO_ClrIntFlag(P0, BIT5);
		printf("P05 occurred\n");
	}

	if (GPIO_GetIntFlag(P0, BIT6)) {
		GPIO_ClrIntFlag(P0, BIT6);
		if (test_mode_auto_circle) {
			printf("test_mode_auto_circle off\n");
			test_mode_auto_circle = false;
		} else {
			printf("test_mode_auto_circle on\n");
			test_mode_auto_circle = true;
		}
		printf("P06 occurred\n");
	}
}

void auto_circle(int16_t *x_value, int16_t *y_value)
{
	static volatile uint16_t usb_cycle_cnt;

	if (usb_cycle_cnt < 250) {
		*x_value = 0;
		*y_value = 2;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 500) {
		*x_value = 2;
		*y_value = 0;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 750) {
		*x_value = 0;
		*y_value = -2;
		usb_cycle_cnt++;
	} else if (usb_cycle_cnt < 1000) {
		*x_value = -2;
		*y_value = 0;
		usb_cycle_cnt++;
	} else {
		*x_value = 0;
		*y_value = 2;
		usb_cycle_cnt = 1;
	}
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

void key_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
	SYS_SET_MFP(P0, 4, GPIO);
	SYS_SET_MFP(P0, 5, GPIO);
	GPIO_EnableDigitalPath(P0, BIT4 | BIT5);
	GPIO_SetMode(P0, BIT4 | BIT5, GPIO_MODE_INPUT);
	GPIO_EnablePullupPath(P0, BIT4 | BIT5);
	GPIO_EnableInt(P0, 4, GPIO_INT_FALLING);
	GPIO_EnableInt(P0, 5, GPIO_INT_FALLING);
	NVIC_EnableIRQ(GPIO0_IRQn);

	SYS_SET_MFP(P0, 6, GPIO);
	GPIO_EnableDigitalPath(P0, BIT6);
	GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
	GPIO_EnablePullupPath(P0, BIT6);
	GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
	NVIC_EnableIRQ(GPIO0_IRQn);
}

uint32_t tmrRealCntFreq = 0;    // Timer Real Count Frequency
void TMR0_IRQHandler(void)
{
	if (TIMER_GetIntFlag(TIMER0)) {
		/* Clear Timer interrupt flag */
		TIMER_ClearIntFlag(TIMER0);
		xSemaphoreGiveFromISR(sem_timer, NULL);
	}
}

void timer_start(void)
{
	TIMER_Start(TIMER0);
}

void timer_freq_set(uint32_t tmrCmpValue)
{
	TIMER_SetCmpValue(TIMER0, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);
}

static void TIMER_PeriodicModeSet(void)
{
	uint32_t tmrExpCntFreq = 0;     // Timer Expected Count Frequency
	uint32_t tmrCmpValue = 0;       // Comparison Value

	// Select Timer clock source
	CLK_SetTmrClkSrc(TIMER0, CLK_APB1_TMR0SEL_APB1CLK);

	// Set Timer work mode and expected clock frequency
	tmrExpCntFreq = FREQ_16MHZ;
	tmrRealCntFreq = TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, tmrExpCntFreq);

	timer_freq_set(tmrRealCntFreq / 133);

	// Enable interrupt
	TIMER_EnableInt(TIMER0);
	NVIC_EnableIRQ(TMR0_IRQn);

	// Start Timer counting
	printf("\nTimer0, Compare Value:%d\n", tmrCmpValue);
	printf("Expected Cnt Freq:%d, Real Cnt Freq:%d\n", tmrExpCntFreq, tmrRealCntFreq);
	printf("Expected Timeout:%llums, Real Timeout:%llums\n",
	       1000ull * tmrCmpValue / tmrExpCntFreq, 1000ull * tmrCmpValue / tmrRealCntFreq);
	printf("Start Timer...\n");
	timer_start();
}

void timer_init(void)
{
	printf("Init: TIMER @ mode \n\n");

	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1, ENABLE);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);

	/* open timer isr */
	TIMER0->CTL |= (0X7 << 8);
	TIMER_PeriodicModeSet();
}

void mouse_common_init(void)
{
	sem_timer = xSemaphoreCreateBinary();

	key_init();

	timer_init();
}
