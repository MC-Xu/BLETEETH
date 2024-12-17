
/**************************************************************************//**
* @file     pan_hal_gpio.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series GPIO (General Purpose Input/Output) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"

HAL_GPIO_CallbackFunc HAL_GPIO_CallbackArray[GPIO_P0_PIN_NUM + GPIO_P1_PIN_NUM + GPIO_P2_PIN_NUM + GPIO_P3_PIN_NUM];

void HAL_GPIO_Init(HAL_GPIO_PinId pinID, HAL_GPIO_InitTypeDef *initStruct)
{
    uint32_t position;
    uint8_t data;

    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;

    GPIO_T *GPIOx = (GPIO_T *)(P0_BASE + 0x40 * port);
    position = 1UL << bit;

    /*--------------------- GPIO Pull Configuration ------------------------*/
    if ((initStruct->mode == HAL_GPIO_MODE_INPUT_DIGITAL) || (initStruct->mode == HAL_GPIO_MODE_QUASI_BIDIRECTIONAL))
    {
        GPIOx->DINOFF &= ~(position << 16);
    }
    else if (initStruct->mode == HAL_GPIO_MODE_INPUT_ANALOG)
    {
        GPIOx->DINOFF |= (position << 16);
    }

    switch (initStruct->pull)
    {
    case HAL_GPIO_PULL_UP:
        GPIOx->DINOFF &= ~(position << 8);
        GPIOx->DINOFF |= position;
        break;
    case HAL_GPIO_PULL_DOWN:
        GPIOx->DINOFF &= ~position;
        GPIOx->DINOFF |= (position << 8);
        break;
    case HAL_GPIO_PULL_DISABLE:
        GPIOx->DINOFF &= ~position;
        GPIOx->DINOFF &= ~(position << 8);
        break;
    }

    /*--------------------- GPIO Init Level Configuration ------------------------*/
    data = GPIO_GetOutData(GPIOx);
    data = (data & ~(BIT0 << bit)) | initStruct->level << bit;
    GPIO_SetOutData(GPIOx, data);

    /*--------------------- GPIO Mode Configuration ------------------------*/
    GPIOx->MODE = (GPIOx->MODE & ~(0x3 << (bit << 1))) | (initStruct->mode << (bit << 1));

    /*--------------------- Do manual 3v aon sync if need ------------------------*/
    if (((GPIOx == P0) && (position & BIT0)) || ((GPIOx == P0) && (position & BIT1)) || ((GPIOx == P0) && (position & BIT2)))
    {
        // Several bits of these Pins' should be synced to 3v aon area
        CLK_Wait3vSyncReady();
    }
}

void HAL_GPIO_WritePin(HAL_GPIO_PinId pinID, HAL_GPIO_Level pinLevel)
{
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;
    uint32_t data;

    data = GPIO_GetOutData((GPIO_T *)(P0_BASE + 0x40 * port));
    data = (data & ~(BIT0 << bit)) | pinLevel << bit;

    GPIO_SetOutData((GPIO_T *)(P0_BASE + 0x40 * port), data);
}

HAL_GPIO_Level HAL_GPIO_ReadPin(HAL_GPIO_PinId pinID)
{
    HAL_GPIO_Level bitstatus;
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;

    if ((GPIO_GetInData((GPIO_T *)(P0_BASE + 0x40 * port)) & (BIT0 << bit)))
    {
        bitstatus = HAL_GPIO_LEVEL_HIGH;
    }
    else
    {
        bitstatus = HAL_GPIO_LEVEL_LOW;
    }
    return bitstatus;
}

void HAL_GPIO_TogglePin(HAL_GPIO_PinId pinID)
{
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;

    GPIO_Toggle((GPIO_T *)(P0_BASE + 0x40 * port), BIT(bit));
}

void HAL_GPIO_SetPull(HAL_GPIO_PinId pinID, HAL_GPIO_Pull pull)
{
    uint8_t port = pinID / 0x10;
    uint8_t bit = pinID % 0x10;
    GPIO_T *GPIOx = (GPIO_T *)(P0_BASE + 0x40 * port);

    switch (pull)
    {
    case HAL_GPIO_PULL_UP:
        GPIO_EnablePullupPath(GPIOx, BIT(bit));
        GPIO_DisablePulldownPath(GPIOx, BIT(bit));
        break;
    case HAL_GPIO_PULL_DOWN:
        GPIO_EnablePulldownPath(GPIOx, BIT(bit));
        GPIO_DisablePullupPath(GPIOx, BIT(bit));
        break;
    case HAL_GPIO_PULL_DISABLE:
        GPIO_DisablePulldownPath(GPIOx, BIT(bit));
        GPIO_DisablePullupPath(GPIOx, BIT(bit));
        break;
    }
}

void HAL_GPIO_DeInit(HAL_GPIO_PinId pinID)
{
    HAL_GPIO_InitTypeDef initStruct;
    initStruct.mode = HAL_GPIO_MODE_INPUT_ANALOG;
    initStruct.pull = HAL_GPIO_PULL_DISABLE;
    initStruct.level = HAL_GPIO_LEVEL_HIGH,
    HAL_GPIO_Init(pinID, &initStruct);
}

void HAL_GPIO_InterruptInit(HAL_GPIO_PinId pinID, HAL_GPIO_IntInitTypeDef *intInitStruct)
{
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;
    GPIO_T *GPIOx = (GPIO_T *)(P0_BASE + 0x40 * port);

    HAL_GPIO_CallbackArray[port * GPIO_PIN_MAX + bit] = intInitStruct->callbackFunc;

    /*--------------------- GPIO Interrupt Configuration ------------------------*/
    if (intInitStruct->intMode == HAL_GPIO_INT_DISABLE)
    {
        GPIOx->INTTYPE &= ~(1UL << bit);
        GPIOx->INTEN &= ~((0x00010001UL) << bit);
    }
    else
    {
        GPIOx->INTTYPE |= (((intInitStruct->intMode >> 24) & 0xFFUL) << bit);
        GPIOx->INTEN |= ((intInitStruct->intMode & 0xFFFFFFUL) << bit);
    }

    /*--------------------- GPIO Interrupt Debounce Configuration ------------------------*/
    if (intInitStruct->debounce == ENABLE)
    {
        GPIOx->DBEN |= BIT(bit);
    }
    else if (intInitStruct->debounce == DISABLE)
    {
        GPIOx->DBEN &= ~BIT(bit);
    }
}

void HAL_GPIO_SetIntMode(uint32_t pinID, HAL_GPIO_IntMode intMode)
{
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;
    GPIO_T *GPIOx = (GPIO_T *)(P0_BASE + 0x40 * port);

    if (intMode == HAL_GPIO_INT_DISABLE)
    {
        GPIOx->INTTYPE &= ~(1UL << bit);
        GPIOx->INTEN &= ~((0x00010001UL) << bit);
    }
    else
    {
        GPIOx->INTTYPE |= (((intMode >> 24) & 0xFFUL) << bit);
        GPIOx->INTEN |= ((intMode & 0xFFFFFFUL) << bit);
    }
}

void HAL_GPIO_SetIntDebounce(uint32_t pinID, FunctionalState debounce)
{
    uint8_t port = pinID >> 4;
    uint8_t bit = pinID % 0x10;
    GPIO_T *GPIOx = (GPIO_T *)(P0_BASE + 0x40 * port);

    if (debounce == DISABLE)
    {
        GPIOx->DBEN &= ~BIT(bit);
    }
    else
    {
        GPIOx->DBEN |= BIT(bit);
    }
}

static void GPIO_IRQHandler(GPIO_T *GPIOx, uint32_t PortIdx, uint32_t pinNum)
{
    for (size_t i = 0; i < pinNum; i++)
    {
        if (GPIO_GetIntFlag(GPIOx, BIT(i)))
        {
            GPIO_ClrIntFlag(GPIOx, BIT(i));
            HAL_GPIO_IntMode attr = (HAL_GPIO_IntMode)(((GPIOx->INTEN & GPIO_INTEN_MASK_BASE << i) >> i) | ((GPIOx->INTTYPE & GPIO_INTTYPE_MASK_BASE << i) >> i << 24));
            if (HAL_GPIO_CallbackArray[PortIdx * GPIO_PIN_MAX + i] != NULL)
            {
                HAL_GPIO_CallbackArray[PortIdx * GPIO_PIN_MAX + i](attr);
            }
        }
    }
}

void GPIO0_IRQHandler(void)
{
    GPIO_IRQHandler(P0, 0, GPIO_P0_PIN_NUM);
}

void GPIO1_IRQHandler(void)
{
    GPIO_IRQHandler(P1, 1, GPIO_P1_PIN_NUM);
}

void GPIO2_IRQHandler(void)
{
    GPIO_IRQHandler(P2, 2, GPIO_P2_PIN_NUM);
}

void GPIO3_IRQHandler(void)
{
    GPIO_IRQHandler(P3, 3, GPIO_P3_PIN_NUM);
}
