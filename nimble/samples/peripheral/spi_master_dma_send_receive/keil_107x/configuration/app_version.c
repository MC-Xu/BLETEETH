#include "app_version.h"

#if (CONFIG_BOOT_ENABLE)

__attribute__((section(".ARM.__at_0xA000"))) const struct smp_image_header z_image_header = {
	.ih_magic = IMAGE_MAGIC,
	.ih_hdr_size = IMAGE_HEADER_SIZE,
	.ih_img_size = 0,
};

#endif

