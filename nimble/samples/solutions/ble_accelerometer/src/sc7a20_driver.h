/*
 * Copyright (c) 2021-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "PanSeries.h"

#define I2C_MASTER_ADDRESS7 (0x18)

typedef struct sc7a20_handle {
	bool (*IIC_ReadReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum);		//IIC read
	bool (*IIC_WriteReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum);	//IIC write
	uint8_t slaveAddr;
	uint8_t modeConfigData;
}sc7a20_handle_t;



//SC7A20
bool sc7a20_init(sc7a20_handle_t *pHandle, uint8_t SlaveAddr, 
	bool (*IIC_ReadReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum), 
	bool (*IIC_WriteReg)(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pDataBuff, uint16_t byteNum));
 
bool sc7a20_read_acceleration(sc7a20_handle_t* pHandle, int16_t* pXa, int16_t* pYa, int16_t* pZa);//SC7A20??????
bool sc7a20_get_z_axis_angle(sc7a20_handle_t* pHandle, int16_t acceBuff[3], float* pAngleZ);//SC7A20 ??Z???

void period_notify_change(uint8_t period_time);
void coordinate_notify(void);
void sc7a20_sensor_init(void);

extern uint16_t current_period_time;

#ifdef __cplusplus
}
#endif
