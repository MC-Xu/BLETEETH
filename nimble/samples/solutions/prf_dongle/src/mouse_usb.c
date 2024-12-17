/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.h"
#include "mouse_usb.h"
#include "musbfsfc.h"
#include "descript.h"
#include "endpoint.h"
#include "endpoint0.h"
#include "PanSeries.h"
#include "usb_panchip.h"

enum usb_stat_t usb_stat;
#define TEST_USB 0
#if !TEST_USB
__ramfunc void usb_report(void)
{
	if (usb_stat == usb_suspending) {
		printf("usb_suspending\n");
		SYS_delay_10nop(5000000);// 1.5s
		printf("usb_suspended\n");
		usb_stat = usb_suspended;
		ring_buf_reset(&ringbuf_raw);
		return;
	} else if (usb_stat == usb_suspended) {
		if (!ring_buf_is_empty(&ringbuf_raw)) {
			usb_stat = usb_resuming;
			printf("resuming\n");
			USB->POWER |= 0x4;
			SYS_delay_10nop(100000); // 30ms;
			USB->POWER &= ~0x4;
			printf("resuming done\n");
		}
		return;
	}
	uint8_t index_before;

	index_before = USB->INDEX;

	WRITE_REG(USB->INDEX, 1);

	if (((USB->CSR0_INCSR1 & 0X2) == 0) && (!(ring_buf_is_empty(&ringbuf_raw)))) {
		uint8_t mouse_data[MOUSE_REPORT_SIZE] = { 0 };
		uint8_t mouse_report_data[USB_MOUSE_REPORT_SIZE] = { 0 };
		ring_buf_get(&ringbuf_raw, (uint8_t *)&mouse_data, MOUSE_REPORT_SIZE);

		/* report id */
		mouse_report_data[0] = MOUSE_REPORT_ID;
		/* key data */
		mouse_report_data[0] = mouse_data[3];
		/* x data */
		mouse_report_data[1] = mouse_data[4];
		mouse_report_data[2] = mouse_data[5];
		/* y data */
		mouse_report_data[3] = mouse_data[6];
		mouse_report_data[4] = mouse_data[7];
		/* roll data */
		mouse_report_data[5] = mouse_data[8];
		mouse_report_data[6] = 0;

		USB_Write((uint32_t)1, (uint32_t)7, (void *)mouse_report_data);
		WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);
	}

	WRITE_REG(USB->INDEX, COMPOSITE_ENDPOINT);

	if (((USB->CSR0_INCSR1 & 0X2) == 0) && (!(ring_buf_is_empty(&ringbuf_raw2)))) {
		uint8_t mouse_data[MOUSE_REPORT_SIZE] = { 0 };
		uint8_t mouse_report_data[USB_MOUSE_REPORT_SIZE] = { 0 };
		ring_buf_get(&ringbuf_raw2, (uint8_t *)&mouse_data, MOUSE_REPORT_SIZE);

		/* report id */
		mouse_report_data[0] = MOUSE_REPORT_ID;
		/* key data */
		mouse_report_data[1] = mouse_data[3];
		/* x data */
		mouse_report_data[2] = mouse_data[4];
		mouse_report_data[3] = mouse_data[5];
		/* y data */
		mouse_report_data[4] = mouse_data[6];
		mouse_report_data[5] = mouse_data[7];
		/* roll data */
		mouse_report_data[6] = mouse_data[8];
		mouse_report_data[7] = 0;

		USB_Write((uint32_t)COMPOSITE_ENDPOINT, (uint32_t)USB_MOUSE_REPORT_SIZE, (void *)mouse_report_data);
		WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);
	}

	WRITE_REG(USB->INDEX, index_before);
}
#else
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

__ramfunc void usb_report(void)
{
	uint8_t index_before;

	index_before = USB->INDEX;

	WRITE_REG(USB->INDEX, 1);

	if (((USB->CSR0_INCSR1 & 0X2) == 0)) {
		uint8_t mouse_report_data[USB_MOUSE_REPORT_SIZE] = { 0 };
		int16_t x_value, y_value;
		auto_circle(&x_value, &y_value);
		/* key data */
		mouse_report_data[0] = 0;
		/* x data */
		mouse_report_data[1] = x_value & 0xff;
		mouse_report_data[2] = x_value >> 8;
		/* y data */
		mouse_report_data[3] = y_value & 0xff;
		mouse_report_data[4] = y_value >> 8;
		/* roll data */
		mouse_report_data[5] = 0;
		mouse_report_data[6] = 0;

		USB_Write((uint32_t)1, (uint32_t)7, (void *)mouse_report_data);
		WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);
	}

	WRITE_REG(USB->INDEX, index_before);
}
#endif

__ramfunc void usb_intr(void)
{
	volatile uint8_t IntrUSB;
	volatile int16_t IntrIn;
	volatile int16_t IntrOut;

	/* Read interrupt registers */
	/* Mote if less than 8 IN endpoints are configured then */
	/* only M_REG_INTRIN1 need be read. */
	/* Similarly if less than 8 OUT endpoints are configured then */
	/* only M_REG_INTROUT1 need be read. */
	IntrUSB = READ_REG(USB->INT_USB);
	IntrIn = (uint16_t)READ_REG(USB->INT_IN2);
	IntrIn <<= 8;
	IntrIn |= (uint16_t)READ_REG(USB->INT_IN1);
	IntrOut = (uint16_t)READ_REG(USB->INT_OUT2);
	IntrOut <<= 8;
	IntrOut |= (uint16_t)READ_REG(USB->INT_OUT1);

	/* ep1 finish in trans isr */
	if (IntrIn & (1 << STAND_KB_ENDPOINT)) {
		if (usb_stat != usb_active) {
			printf("usb_stat %d\n", usb_stat);
			usb_stat = usb_active;
			USB->POWER |= 0x1;
		}
	}

	/* ep2 finish in trans isr */
	if (IntrIn & (1 << COMPOSITE_ENDPOINT)) {
		if (usb_stat != usb_active) {
			printf("usb_stat %d\n", usb_stat);
			usb_stat = usb_active;
			USB->POWER |= 0x1;
		}
	}

	if (IntrOut & (1 << VENDOR_ENDPOINT)) {

	}

	/* Check for system interrupts */
	if (IntrUSB & M_INTR_PLUG) {
		if ((IntrUSB & M_INTR_PLUG_OUT) == M_INTR_PLUG_OUT) {
			printf("plug out\n");
			WRITE_REG(USB->INT_USBE, READ_REG(USB->INT_USBE) & (~0x8));
		} else if (IntrUSB & M_INTR_PLUG_OUT) {
			printf("plug in\n");
//			WRITE_REG(USB->INT_USBE, READ_REG(USB->INT_USBE) | 0x8);
		}
	}

	if (IntrUSB & M_INTR_SUSPEND) {
		printf("USB isr in: Suspend evt\n");
		usb_stat = usb_suspending;
	}

	if (IntrUSB & M_INTR_RESUME) {
		printf("USB isr in: Resume evt\n");
		usb_stat = usb_active;
	}

	if (IntrUSB & M_INTR_RESET) {
		printf("USB isr in: Reset evt\n");
		usb_stat = usb_active;
//		WRITE_REG(USB->INT_USBE, READ_REG(USB->INT_USBE) | 0x8);

		/* Panchip USB Reset Proc */
		USB_Reset();
	}

	/* Check for endpoint 0 interrupt */
	if (IntrIn & M_INTR_EP0) {
		uint8_t index_before;

		index_before = USB->INDEX;
		WRITE_REG(USB->INDEX, 0);
		/* Panchip USB EP0 Proc */
		Endpoint0(M_EP_NORMAL);
		WRITE_REG(USB->INDEX, index_before);
	}

	return;
}

void USB_IRQHandler(void)
{
	usb_intr();
}

void usb_init(void)
{
	uint32_t reg_tmp;

	ANA->ANA_MISC_3V |= ((0x1 << 27) | (0x1 << 28));
	ANA->ANA_MISC_3V |= ((0X1 << 26));
	/*usb debounce time*/
	SYS->CTRL1 = ((SYS->CTRL1 & ~0xffff) | 100);

	reg_tmp = READ_REG(USB->INT_USBE);
	reg_tmp |= 0x10;
	WRITE_REG(USB->INT_USBE, reg_tmp);

	printf("usb_init\n");
	NVIC_SetPriority(USB_IRQn, 1);
	NVIC_EnableIRQ(USB_IRQn);
}
