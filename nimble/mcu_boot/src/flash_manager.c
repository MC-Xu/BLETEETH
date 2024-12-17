/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "flash_manager.h"

static uint32_t m_erased_area;

void fm_status_refresh(void)
{
	m_erased_area = FLASH_AREA_IMAGE_START;
}

void fm_write_flash(unsigned int addr, unsigned char *buf, unsigned int len)
{	
	if ((addr + len) <= m_erased_area) { /* means the specified flash has be erased before */
		FMC_WriteStream(FLCTL, addr, buf, len); /* todo chao*/
	} else {
		if (FMC_EraseSector(FLCTL, m_erased_area) == 0) {
			m_erased_area += SECTOR_SIZE;
			FMC_WriteStream(FLCTL, addr, buf, len);
		} else {
			ASSERT;
		}
	}	
}

int fm_read_flash(unsigned int addr, unsigned char *buf, unsigned int len)
{
	return FMC_ReadStream(FLCTL, addr, CMD_FAST_READ, buf, len);
}

static void check_sum_add(uint32_t *init_val, uint8_t *val, uint16_t len)
{
	for (uint16_t i =0 ; i < len; i++) {
		*init_val += *(val + i);
	}
}

int fm_image_make_invalid(uint32_t im_addr)
{
	return FMC_EraseSector(FLCTL, im_addr);
}

static uint8_t read_value[256];

void fm_image_move(uint32_t src_addr, uint32_t dst_addr)
{
	uint32_t current_index = 0;
	image_header_t header;
	
	/* fm write to image area, should handle one time fm_status_refresh */
	fm_status_refresh();
	
	/* move header */
	fm_read_flash(src_addr, (unsigned char *)&header, sizeof(image_header_t)); 
	fm_write_flash(dst_addr, (unsigned char *)&header, sizeof(image_header_t));
	src_addr += sizeof(image_header_t);
	dst_addr += sizeof(image_header_t);
	
	header.ih_img_size += 40; /*smp signed related*/
	/* move completed image*/
	while(current_index < header.ih_img_size) {
		if (current_index + 256 <= header.ih_img_size){ /* remaining packets over than 256 bytes */
			fm_read_flash(src_addr + current_index, read_value, 256);
			fm_write_flash(dst_addr+ current_index, read_value, 256);
			current_index += 256;
		} 
		else {
			fm_read_flash(src_addr + current_index, read_value, header.ih_img_size - current_index); /* remaining packets less than 256 bytes */
			fm_write_flash(dst_addr+ current_index, read_value, header.ih_img_size - current_index);
			current_index = header.ih_img_size;
		}
	}
	/* escape move at next time, should be make it invalid */
	fm_image_make_invalid(src_addr);
}

bool fm_image_completed_check(uint32_t im_addr)
{
	
	uint32_t check_sum = 0;
	uint32_t current_index = 0;
	image_header_t header;
	
	fm_read_flash(im_addr, (unsigned char *)&header, sizeof(image_header_t));
	
	if (header.ih_img_size > FLASH_IMAGE_MAX_SIZE  || header.ih_img_size <= 256) /* image size should be obtained a basic rules */
		return false;
	
	im_addr += sizeof(image_header_t);
	
	while(current_index < header.ih_img_size) {
		if (current_index + 256 <= header.ih_img_size){ /* remaining packets over than 256 bytes */
			fm_read_flash(im_addr + current_index, read_value, 256);
			current_index += 256;
			check_sum_add(&check_sum, read_value, 256);
		} 
		else {
			fm_read_flash(im_addr + current_index, read_value, header.ih_img_size - current_index); /* remaining packets less than 256 bytes */
			check_sum_add(&check_sum, read_value, header.ih_img_size - current_index);
			current_index = header.ih_img_size;
		}
	}
	
	if (header.checksum == check_sum) {
		return true;
	} else 
		return false;
}

#if false
const uint8_t flash_image[] = {
0x1, 0x2, 0x3, 0x4,
0x1, 0x1, 0x0, 0x0,
0x7B, 0x5, 0x0, 0x0,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7	
};

void t_fm_image_completed_check_test(void)
{
	bool ret;
		
	ret = fm_image_completed_check((uint32_t)flash_image);
	if (ret) {
		while(1);
	}
}
#endif

