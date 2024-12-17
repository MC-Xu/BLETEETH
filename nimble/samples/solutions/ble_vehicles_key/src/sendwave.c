/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PANSeries.h"
#include "sendwave.h"

static int uart_fifo_fill(UART_T *uart, const uint8_t *tx_data,
				  int size)
{
	uint8_t num_tx = 0U;

	while ((size - num_tx > 0) &&
	       !UART_IsTxFifoFull(uart)) {
		/* Fill data in Tx FIFO if not full */
		UART_SendData(uart, tx_data[num_tx++]);
	}

	return num_tx;
}

#define BUFFER_SIZE			64
#define BAUDRATE			921600

enum {
	Ch_Num          = 16,
	Frame_MaxBytes  = 80,
	Frame_Head      = 0xA3,
	Frame_PointMode = 0xA8,
	Frame_SyncMode  = 0xA9,
	Frame_InfoMode  = 0xAA,
	Format_Int8     = 0x10,
	Format_Int16    = 0x20,
	Format_Int32    = 0x30,
	Format_Float    = 0x00
};

char ws_point_int8(char *buffer, char channel, int8_t value)
{
	if ((uint8_t)channel < Ch_Num) {
		*buffer++ = Frame_Head;
		*buffer++ = Frame_PointMode;
		*buffer++ = channel | Format_Int8;
		*buffer = value;
		return 4;
	}
	return 0;
}

char ws_point_int16(char *buffer, char channel, int16_t value)
{
	if ((uint8_t)channel < Ch_Num) {
		*buffer++ = Frame_Head;
		*buffer++ = Frame_PointMode;
		*buffer++ = channel | Format_Int16;
		*buffer++ = (value >> 8) & 0xFF;
		*buffer = value & 0xFF;
		return 5;
	}
	return 0;
}

char ws_point_int32(char *buffer, char channel, int32_t value)
{
	if ((uint8_t)channel < Ch_Num) {
		*buffer++ = Frame_Head;
		*buffer++ = Frame_PointMode;
		*buffer++ = channel | Format_Int32;
		*buffer++ = (value >> 24) & 0xFF;
		*buffer++ = (value >> 16) & 0xFF;
		*buffer++ = (value >> 8) & 0xFF;
		*buffer = value & 0xFF;
		return 7;
	}
	return 0;
}

char ws_point_float(char *buffer, char channel, float value)
{
	union {
		float f;
		uint32_t i;
	} temp;

	if ((uint8_t)channel < Ch_Num) {
		temp.f = value;
		*buffer++ = Frame_Head;
		*buffer++ = Frame_PointMode;
		*buffer++ = channel | Format_Float;
		*buffer++ = (temp.i >> 24) & 0xFF;
		*buffer++ = (temp.i >> 16) & 0xFF;
		*buffer++ = (temp.i >>  8) & 0xFF;
		*buffer = temp.i & 0xFF;
		return 7;
	}
	return 0;
}

void ws_frame_init(char *buffer)
{
	*buffer++ = Frame_Head;
	*buffer++ = Frame_SyncMode;
	*buffer = 0;
}

char ws_frame_length(const char *buffer)
{
	return buffer[2] + 3;
}

char ws_add_int8(char *buffer, char channel, int8_t value)
{
	char count = buffer[2];
	char *p = buffer + count + 3;

	count += 2;
	if (count <= Frame_MaxBytes && (uint8_t)channel < Ch_Num) {
		buffer[2] = count;
		*p++ = channel | Format_Int8;
		*p = value;
		return 1;
	}
	return 0;
}

char ws_add_int16(char *buffer, char channel, int16_t value)
{
	char count = buffer[2];
	char *p = buffer + count + 3;

	count += 3;
	if (count <= Frame_MaxBytes && (uint8_t)channel < Ch_Num) {
		buffer[2] = count;
		*p++ = channel | Format_Int16;
		*p++ = (value >> 8) & 0xFF;
		*p = value & 0xFF;
		return 1;
	}
	return 0;
}

char ws_add_int32(char *buffer, char channel, int32_t value)
{
	char count = buffer[2];
	char *p = buffer + count + 3;

	count += 5;
	if (count <= Frame_MaxBytes && (uint8_t)channel < Ch_Num) {
		buffer[2] = count;
		*p++ = channel | Format_Int32;
		*p++ = (value >> 24) & 0xFF;
		*p++ = (value >> 16) & 0xFF;
		*p++ = (value >> 8) & 0xFF;
		*p = value & 0xFF;
		return 1;
	}
	return 0;
}

char ws_add_float(char *buffer, char channel, float value)
{
	char count = buffer[2];
	char *p = buffer + count + 3;

	count += 5;
	if (count <= Frame_MaxBytes && (uint8_t)channel < Ch_Num) {
		union {
			float f;
			uint32_t i;
		} temp;
		buffer[2] = count;
		temp.f = value;
		*p++ = channel | Format_Float;
		*p++ = (temp.i >> 24) & 0xFF;
		*p++ = (temp.i >> 16) & 0xFF;
		*p++ = (temp.i >>  8) & 0xFF;
		*p = temp.i & 0xFF;
		return 1;
	}
	return 0;
}

char ws_send_timestamp(char *buffer, ws_timestamp_t *ts)
{
	uint8_t temp;

	*buffer++ = Frame_Head;
	*buffer++ = Frame_InfoMode;
	temp = (ts->year << 1) | ((ts->month >> 3) & 0x01);
	*buffer++ = (char)temp;
	temp = (ts->month << 5) | (ts->day & 0x1F);
	*buffer++ = (char)temp;
	temp = (ts->hour << 3) | ((ts->min >> 3) & 0x07);
	*buffer++ = (char)temp;
	temp = (ts->min << 5) | ((ts->sec >> 1) & 0x1F);
	*buffer++ = (char)temp;
	temp = (ts->sec << 7) | ((ts->msec >> 3) & 0x7F);
	*buffer++ = (char)temp;
	temp = (ts->msec << 5) | ((ts->sampleRate >> 16) & 0x1F);
	*buffer++ = (char)temp;
	*buffer++ = (char)((ts->sampleRate >> 8) & 0xFF);
	*buffer = (char)(ts->sampleRate & 0xFF);
	return 10;
}

static uint8_t fifo_data[BUFFER_SIZE];
static uint8_t data_len;

void sendBuffer(char *buffer, uint8_t len)
{
	data_len = len;
	memcpy(fifo_data, buffer, len);
    uart_fifo_fill(UART1, fifo_data, data_len);
}

void wave_uart_init(void)
{
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

    GPIO_EnableDigitalPath(P2, BIT4);
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    
    UART_InitTypeDef Init_Struct;

    Init_Struct.UART_BaudRate = BAUDRATE;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;

    /* Init UART1 for printf */
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);	

//    UART_EnablePtime(UART1);
//    UART_EnableIrq(UART1, UART_IRQ_THR_EMPTY);
    
    /** disable tx irq*/
   	UART_DisablePtime(UART1);
    UART_DisableIrq(UART1, UART_IRQ_THR_EMPTY);

    /** disable rx irq*/
    UART_DisableIrq(UART1, UART_IRQ_RECV_DATA_AVL);

}

void ws_init(void)
{
	printf("Start uart fifo demo..\n");
    
    wave_uart_init();

}
/* end of file sendwave.c */
