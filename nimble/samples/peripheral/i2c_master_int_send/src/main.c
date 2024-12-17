/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    main.c
 *
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "PANSeries.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pan_hal.h"

#pragma diag_suppress 188  /*remove compile warning 188*/

#define I2C_SLAVE_ADDRESS7           (0x50)
#define I2C_MASTER_ADDRESS7          (0x51)

#define TEST_BYTE_LEN                       (32)

#if (CONFIG_BOOT_ENABLE)
static void print_version_info(void)
{
    #include "app_version.h"

    struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

    printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

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

void TestFillBuffer(uint8_t *pBuffer, uint32_t size)
{
    uint32_t idx = 0;
    
    for(idx = 0; idx < size; idx++)
    {
        pBuffer[idx] = (uint8_t)(idx % 0x100);
    }
}

volatile bool test_send_over = false;
volatile bool test_recv_over = false;

static void  Test_Send_Callback(I2C_Cb_Flag_Opt Flag,uint8_t *Buf,uint16_t Size){
   SYS_TEST("tx\r\n");
   //this log as delay for wait HAL_I2C_Slave_SendData_INT
   SYS_TEST("Send:\r\n");
   for(uint16_t i=0;i<Size;i++){SYS_TEST("%02X ",Buf[i]);}SYS_TEST("\r\n");

   test_send_over = true;
}

static void Test_Receive_Callback(I2C_Cb_Flag_Opt Flag,uint8_t *Buf,uint16_t Size){
	test_recv_over = true;
    SYS_TEST("rx\r\n");
    SYS_TEST("Receive\r\n");    
    for(uint16_t i=0;i<Size;i++){SYS_TEST("%02X ",Buf[i]);}SYS_TEST("\r\n");

}

void hal_bsp_init(void)
{    
    uint8_t TestBuf[TEST_BYTE_LEN] = {0};
    TestFillBuffer(TestBuf, TEST_BYTE_LEN);

    /* Configure Pinmux for I2C0 SCL and SDA */
    SYS_SET_MFP(P1, 4, I2C0_SCL);
    GPIO_EnableDigitalPath(P1, BIT4);
    GPIO_EnablePullupPath(P1, BIT4);
    
    SYS_SET_MFP(P1, 3, I2C0_SDA);
    GPIO_EnableDigitalPath(P1, BIT3);
    GPIO_EnablePullupPath(P1, BIT3);
    
    I2C_OBJ.I2C_InitObj.Role = I2C_ROLE_MASTER;
    I2C_OBJ.I2C_InitObj.DutyCycle = I2C_DUTYCYCLE_2;
    I2C_OBJ.I2C_InitObj.AddressMode = I2C_ADDR_7BIT;
    I2C_OBJ.I2C_InitObj.ClockSpeed = I2C_SPEED_100K;
    I2C_OBJ.I2C_InitObj.OwnAddress = I2C_MASTER_ADDRESS7;
    
    HAL_I2C_Init(&I2C_OBJ);    
    
    printf("master send start\n");
    HAL_I2C_Master_SendData_INT(&I2C_OBJ, I2C_SLAVE_ADDRESS7, TestBuf, TEST_BYTE_LEN, Test_Send_Callback);
    printf("master send stop\n");

    while(!test_send_over)
    {
    }
    SYS_delay_10nop(200000);
    
    printf("master receive start\n");
    HAL_I2C_Master_ReceiveData_INT(&I2C_OBJ, I2C_SLAVE_ADDRESS7, TestBuf, TEST_BYTE_LEN, Test_Receive_Callback);
    printf("master receive stop\n");
    
    while(!test_recv_over){}
	HAL_I2C_DeInit(&I2C_OBJ);
}

void app_init(void)
{
	#if (CONFIG_BOOT_ENABLE)
    print_version_info();
	#endif
	
    hal_bsp_init();
}
