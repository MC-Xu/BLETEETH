#ifndef APP_VERSION_H
#define APP_VERSION_H

#include <stdint.h>

#define IMAGE_MAGIC                 0x96f3b83d
#define IMAGE_TLV_INFO_MAGIC        0x6907
#define IMAGE_TLV_PROT_INFO_MAGIC   0x6908

#define IMAGE_HEADER_SIZE           512

/** Image trailer TLV types. */
#define IMAGE_TLV_SHA256            0x10   /* SHA256 of image hdr and body */

/** Image TLV-specific definitions. */
#define IMAGE_HASH_LEN              32

struct image_version {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};

typedef struct {
	uint32_t checksum;
	uint8_t padding[508-IMAGE_HASH_LEN];
} image_header_t;

/** Image header.  All fields are in little endian byte order. */
struct smp_image_header {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size; /* Size of image header (bytes). */
    uint16_t _pad2;
    uint32_t ih_img_size; /* Does not include header. */
    uint32_t ih_flags;    /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad3;
    image_header_t m_image_header;
};

#endif
