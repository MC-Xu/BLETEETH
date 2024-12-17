/**************************************************************************//**
 * @file     main.c
 * @version  V1.0.0
 *
 * @brief    main.c
 *
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 ******************************************************************************/
#include "FreeRTOS.h"

#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"
	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif


ENCRYPT_SECTION void encrypted_test_function(uint32_t *calculation)
{
    /* Do some secret operations/calculations here */
    *calculation = *(uint32_t *)((uint32_t)(&encrypted_test_function) & 0xFFFFFFFE);
    printf("Hello from %s!\n\n", __func__);
}

void app_init(void)
{
#if CONFIG_BOOT_ENABLE
    print_version_info();
#endif

    uint32_t secret_calc;

    printf("\nHello World!\n\n");
    encrypted_test_function(&secret_calc);
    printf("Secret calcucation result: 0x%x\n", secret_calc);
}
