/*
* Copyright (c) 2021-2024 Shanghai Panchip Microelectronics Co.,Ltd.
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <string.h>
#include <stdio.h>
#include "PanSeries.h"
#include "pan_pwm.h"
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#include "math.h"
#include "sc7a20_driver.h"
#include "ble_acc_sens.h"
#include "pan_hal.h"


#define SC7A20_REG_WHO_AM_I     0x0F
#define SC7A20_REG_CTRL_1		0x20
#define SC7A20_REG_CTRL_2		0x21
#define SC7A20_REG_CTRL_3		0x22
#define SC7A20_REG_CTRL_4		0x23
#define SC7A20_REG_X_L          0x28
#define SC7A20_REG_X_H          0x29
#define SC7A20_REG_Y_L          0x2A
#define SC7A20_REG_Y_H          0x2B
#define SC7A20_REG_Z_L          0x2C
#define SC7A20_REG_Z_H          0x2D
#define SC7A20_REG_STATUS		0x27

sc7a20_handle_t sensor_handle;
static uint8_t sensor_coordinate[16] = {0x12, };
uint16_t current_period_time = 1000;

I2C_HandleTypeDef I2C_OBJ = {
	.I2Cx = I2C0,
	.I2C_InitObj = {0},  // Init Object.
	.InterruptObj = {0}, // Interrupt Object.
	 
	.pTxBuffPtr = NULL, /*Used for TX*/
	.TxXferSize = 0,
	.TxXferCount = 0,
	 
	.pRxBuffPtr = NULL, /*Used for RX*/
	.RxXferSize = 0,
	.RxXferCount = 0,
	 
	.IRQn = I2C0_IRQn,
};

void delay_ms(uint16_t time_ms)
{
	while (time_ms--) {
		TIMER_Delay(TIMER0, 1000);
	}
}


void float_to_uint8(float dat, uint8_t *pBuf)
{
	pBuf[0] = ((uint8_t*)&dat)[0];
	pBuf[1] = ((uint8_t*)&dat)[1];
	pBuf[2] = ((uint8_t*)&dat)[2];
	pBuf[3] = ((uint8_t*)&dat)[3];
}

float uint8_to_float(uint8_t *pBuf)
{
	return *((float*)pBuf);
}


void hal_bsp_init(void)
{
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_I2C0, ENABLE);
	/* Configure Pinmux for I2C0 SCL and SDA */
	GPIO_EnableDigitalPath(P0, BIT7);
	GPIO_EnablePullupPath(P0, BIT7);
	GPIO_EnableDigitalPath(P1, BIT0);
	GPIO_EnablePullupPath(P1, BIT0);
	SYS_SET_MFP(P0, 7, I2C0_SCL);
	SYS_SET_MFP(P1, 0, I2C0_SDA);

	I2C_OBJ.I2C_InitObj.Role = I2C_ROLE_MASTER;
	I2C_OBJ.I2C_InitObj.DutyCycle = I2C_DUTYCYCLE_2;
	I2C_OBJ.I2C_InitObj.AddressMode = I2C_ADDR_7BIT;
	I2C_OBJ.I2C_InitObj.ClockSpeed = I2C_SPEED_100K;
	I2C_OBJ.I2C_InitObj.OwnAddress = I2C_MASTER_ADDRESS7;
	HAL_I2C_Init(&I2C_OBJ);
}

bool sensor_write_data(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum)
{
	memcpy(pDataBuff+1, pDataBuff, byteNum);
	pDataBuff[0] = regAddr;
	HAL_I2C_Master_SendData(&I2C_OBJ, slaveAddr, pDataBuff, byteNum+1, 0);
	return true;
}

bool sensor_read_data(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum)
{
	HAL_I2C_Master_SendData(&I2C_OBJ, slaveAddr, &regAddr, 1, 0);
	HAL_I2C_Master_ReceiveData(&I2C_OBJ, slaveAddr, pDataBuff, byteNum, 0);
	return true;
}

bool sc7a20_init(sc7a20_handle_t *pHandle, uint8_t slaveAddr, 
	bool (*IIC_ReadReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum), 
	bool (*IIC_WriteReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum))
{
	uint16_t i,j;
	uint8_t tempreg;
	uint8_t buff[16];

	if(pHandle == NULL) 
	{
		delay_ms(5000);
		return FALSE;
	}
	pHandle->slaveAddr = slaveAddr;
	pHandle->IIC_ReadReg = IIC_ReadReg;
	pHandle->IIC_WriteReg = IIC_WriteReg;
	
	tempreg = 0;
 
	delay_ms(50);
 
	for (i = 0; i < 10; i++)
	{
		buff[0] = buff[1] = 0;
		if (pHandle->IIC_ReadReg(pHandle->slaveAddr, SC7A20_REG_WHO_AM_I, buff, 1) == FALSE)
		{
			printf("Sensor connect fail\r\n");
		}
		else if(buff[0] == 0x11)
		{
			break;
		}
		else
		{
			printf("invalid id 0x%02X\r\n", buff[0]);
		}
 
		delay_ms(100);
	}
 
	tempreg = 0x47;
	pHandle->IIC_WriteReg(pHandle->slaveAddr, SC7A20_REG_CTRL_1, &tempreg, 1);
	tempreg = 0x00;
	pHandle->IIC_WriteReg(pHandle->slaveAddr, SC7A20_REG_CTRL_2, &tempreg, 1);
	tempreg = 0x00;
	pHandle->IIC_WriteReg(pHandle->slaveAddr, SC7A20_REG_CTRL_3, &tempreg, 1); 
	tempreg = 0x88;
	pHandle->IIC_WriteReg(pHandle->slaveAddr, SC7A20_REG_CTRL_4, &tempreg, 1);
 
	if (i < 10) return TRUE;
	return FALSE;
}
 
int16_t sc7a20_12bit_complement(uint8_t msb, uint8_t lsb)
{
	int16_t temp;
 
	temp = msb << 8 | lsb;
	temp = temp >> 4;   //???12???
	temp = temp & 0x0fff;
	if (temp & 0x0800) //?? ??==>??
	{
		temp = temp & 0x07ff; //?????      
		temp = ~temp;
		temp = temp + 1;
		temp = temp & 0x07ff;
		temp = -temp;       //?????
	}
	return temp;
}
 
bool sc7a20_read_acceleration(sc7a20_handle_t* pHandle, int16_t* pXa, int16_t* pYa, int16_t* pZa)
{
	uint8_t buff[6];
	uint8_t i;
	int16_t temp;
 
	memset(buff, 0, 6);
	if (pHandle->IIC_ReadReg(pHandle->slaveAddr, SC7A20_REG_X_L|BIT7, buff, 6) == FALSE)
	{
		return FALSE;
	}
	else
	{
		
		pHandle->IIC_ReadReg(pHandle->slaveAddr, 0x27, &i, 1);
		
		/*for (i = 0; i < 6; i++)
		{
			uart_printf("%02X ",buff[i]);
			
		}
		uart_printf("r\n");*/
 
		//X?
		*pXa = buff[1];
		*pXa <<= 8;
		*pXa |= buff[0];
		*pXa >>= 4;	//?12bit??
 
		//Y?
		*pYa = buff[3];
		*pYa <<= 8;
		*pYa |= buff[2];
		*pYa >>= 4;	//?12bit??
		
		//Z?
		*pZa = buff[5];
		*pZa <<= 8;
		*pZa |= buff[4];
		*pZa >>= 4;	//?12bit??
 
		return TRUE;
	}
 
}
 
#define PI 3.1415926535898
bool sc7a20_get_z_axis_angle(sc7a20_handle_t* pHandle, int16_t acceBuff[3], float* pAngleZ)
{
	double fx, fy, fz;
	double A;
	int16_t Xa, Ya, Za;
 
	if (sc7a20_read_acceleration(pHandle, &Xa, &Ya, &Za) == FALSE) return FALSE;
	acceBuff[0] = Xa;	//x????
	acceBuff[1] = Ya;	//y????
	acceBuff[2] = Za;	//z????
 
	fx = Xa;
	fx *= 2.0 / 4096;
	fy = Ya;
	fy *= 2.0 / 4096;
	fz = Za;
	fz *= 2.0 / 4096;
 
	printf("fx:%.04f\tfy:%.04f\tfz:%.04f\t\r\n",fx,fy,fz);

	//Z??
	A = fx * fx + fy * fy;
	A = sqrt(A);
	A = (double)A / fz;
	A = atan(A);
	A = A * 180 / PI;
 
	*pAngleZ = A;

 	float_to_uint8(fx, &sensor_coordinate[0]);
	float_to_uint8(fy, &sensor_coordinate[4]);
	float_to_uint8(fz, &sensor_coordinate[8]);
	float_to_uint8(A, &sensor_coordinate[12]);

	return TRUE;
}

void period_notify_change(uint8_t period_time)
{
	switch (period_time) {
		case 1: current_period_time = 500;break;
		case 2: current_period_time = 1000;break;
		case 3: current_period_time = 1500;break;
		case 4: current_period_time = 2000;break;
		default: current_period_time = 1000;break;
	}
	printf("period = %d\n",current_period_time);
}

void coordinate_notify(void)
{
	int16_t acc[3];
	float zAngle;

	if (FALSE == sc7a20_get_z_axis_angle(&sensor_handle, acc, &zAngle)) {
		printf("read fail\r\n");
	} else {
		printf("Z:%f\r\n", zAngle);
	}
	
	extern void notify_data(uint8_t *data, uint16_t len);
	notify_data(sensor_coordinate, 16);
}

void sc7a20_sensor_init(void)
{
	hal_bsp_init();
	sc7a20_init(&sensor_handle, I2C_MASTER_ADDRESS7, &sensor_read_data, &sensor_write_data);
}

