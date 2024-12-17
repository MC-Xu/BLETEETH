/*
 * Copyright (c) 2020-2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef FLASH_MANAGER__H_
#define FLASH_MANAGER__H_

#include "PanSeries.h"
#include "utilities.h"

#define FLASH_AREA_BACK_UP_START 	0x41000
#define FLASH_AREA_IMAGE_START		0xA000
#define FLASH_IMAGE_MAX_SIZE        0x73000

struct image_version {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};

typedef struct {
	uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size; /* Size of image header (bytes). */
    uint16_t _pad2;
    uint32_t ih_img_size; /* Does not include header. */
    uint32_t ih_flags;    /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad3;
	uint32_t checksum;
	uint8_t padding[476];
} __attribute__((packed)) image_header_t;

void fm_status_refresh(void);
void fm_write_flash(unsigned int addr, unsigned char *buf, unsigned int len);
int fm_read_flash(unsigned int addr, unsigned char *buf, unsigned int len);

bool fm_image_completed_check(uint32_t im_addr);
int fm_image_make_invalid(uint32_t im_addr);
void fm_image_move(uint32_t src_addr, uint32_t dst_addr);
#endif

