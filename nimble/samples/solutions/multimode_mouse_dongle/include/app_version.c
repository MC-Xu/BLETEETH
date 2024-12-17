#include "image_map_config.h"
#include "app_version.h"

#if (!CONFIG_BARE_IMAGE)

__attribute__((section(".ARM.__at_0xA000"))) const struct smp_image_header z_image_header = {
	.ih_magic = IMAGE_MAGIC,
	.ih_hdr_size = IMAGE_HEADER_SIZE,
	.ih_img_size = 0,
};

#endif

