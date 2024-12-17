/*
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#ifndef IMAGE_MAP_CONFIG__H
#define IMAGE_MAP_CONFIG__H

#define CONFIG_BARE_IMAGE     			1
#define CONFIG_OTA_IN_BOOTLOADER 		0



#define SIZE_512K 						0x7F000
#define SIZE_1M							0xFF000
#define SIZE_BOOTLOADER            		0xA000
#define SIZE_FIXED_APP_IMAGE            0x37000
#define SIZE_FIXED_USER_FLASH			0x7000


/* bare program which means it can work without bootloader
 * note image start at 0 address,and user can modify CONFIG_USER_FLASH_EXTENDED_SIZE size
 */
#if (CONFIG_BARE_IMAGE)
#define SIZE_IMAGE_HEADER				 0
#define CONFIG_USER_FLASH_EXTENDED_SIZE (0) /* user can change the value */
#define APP_IMAGE_START 0x00000000
#define APP_IMAGE_SIZE	(SIZE_512K - SIZE_FIXED_USER_FLASH - CONFIG_USER_FLASH_EXTENDED_SIZE)

/* ota in bootloader and there are have no backup area. so it can expand image size user can use
 * note it does not support ble ota
 */
#elif (CONFIG_OTA_IN_BOOTLOADER)
#define SIZE_IMAGE_HEADER				 512
#define CONFIG_IMAGE_EXTENDED_SIZE		(120 * 1024)	/* user can change the value */	
#define CONFIG_USER_FLASH_EXTENDED_SIZE (100 * 1024)	/* user can change the value */
#define APP_IMAGE_START 0x0000A000
#define APP_IMAGE_SIZE	(SIZE_FIXED_APP_IMAGE + CONFIG_IMAGE_EXTENDED_SIZE)
#if (APP_IMAGE_SIZE + CONFIG_USER_FLASH_EXTENDED_SIZE + SIZE_BOOTLOADER > SIZE_512K) /* todo author:chao */
	#error config ota in bootloader flash map over 508K
#endif

/* ota in bootloader and there are have a backup area. 
 * note it is used to adapt to condition that use want to ota in ble ways
 */
#else
#define SIZE_IMAGE_HEADER				 512
#define CONFIG_USER_FLASH_EXTENDED_SIZE (0)		/* user can change the value */
#define APP_IMAGE_START 0x0000A000
#define APP_IMAGE_SIZE	(SIZE_FIXED_APP_IMAGE - CONFIG_USER_FLASH_EXTENDED_SIZE)
#endif

#endif
