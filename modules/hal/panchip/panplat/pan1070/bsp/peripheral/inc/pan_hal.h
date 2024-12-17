#ifndef __PAN_HAL_H__
#define __PAN_HAL_H__

#include "pan_hal_gpio.h"
#include "pan_hal_uart.h"
#include "pan_hal_dmac.h"
#include "pan_hal_spi.h"
#include "pan_hal_i2c.h"

#include "pan_hal_wdt.h"
#include "pan_hal_wwdt.h"
#include "pan_hal_pwm.h"
#include "pan_hal_timer.h"
#include "pan_hal_adc.h"

typedef enum {
	PAN_HAL_SUCCESS,
	PAN_HAL_INVALID_PARAMS,
	PAN_HAL_IO_ERROR,
	PAN_HAL_DEVICE_BUSY,
	PAN_HAL_NO_MEM,
	PAN_HAL_NO_HARDWARE_SOURCE,
	PAN_HAL_UNKNOWN,
} PanHalError;

#endif
