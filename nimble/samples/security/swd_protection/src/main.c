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
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "pan_hal.h"
#include "os_lp.h"

#if CONFIG_BOOT_ENABLE
static void print_version_info(void)
{
    #include "app_version.h"
	struct smp_image_header *image_header_p = (void *)SIZE_BOOTLOADER;

	printf("APP version: %d.%d.%d\n", image_header_p->ih_ver.iv_major, image_header_p->ih_ver.iv_minor, image_header_p->ih_ver.iv_revision);
}
#endif

static void DumpEfuseKeysAndSecureCtrlFlag(void)
{
    // Unlock Protected Registers
    SYS_UnlockReg();
    // Init eFuse module
    EFUSE_Init(EFUSE);
    // Elevate privilege
    EFUSE->EF_FLASH_PERMISSION |= EFUSE_FLASH_PERMISSION_CTRL_Msk;

    SYS_TEST("Debug Protect Key in eFuse:\n");
    SYS_TEST("\tAddr:Data\tAddr:Data\tAddr:Data\tAddr:Data\n\t");
    for (size_t address = 0x10; address < 0x18; address++)
    {
        // Read eFuse
        uint8_t data = EFUSE_ReadByte(EFUSE, address);

        SYS_TEST("0x%02x:0x%02x", address, data);

        // Check if previous Read Operation succeeded or not
        if (EFUSE_GetErrorStatus(EFUSE) == EFUSE_STATUS_FAIL)
        {
            EFUSE_ClrErrorStatus(EFUSE);
            SYS_TEST("(N/A)");
        }
        else
        {
            SYS_TEST("(OK)");
        }
        if ((address + 1) % 4 == 0)
            SYS_TEST("\n\t");
        else
            SYS_TEST("\t");
    }
    SYS_TEST("\nSecure Enable Control Flag in eFuse (Address 0x1B):\n\t0x%02x\n", EFUSE_ReadByte(EFUSE, 0x1B));
    SYS_TEST("Note:\n");
    SYS_TEST("\t- BIT0: FW Encryption Enable Ctrl\n");
    SYS_TEST("\t- BIT1: Anti-injection Enable Ctrl\n");
    SYS_TEST("\t- BIT2: Debug Protection Enable Ctrl\n");

    // Reduce privilege
    EFUSE->EF_FLASH_PERMISSION &= ~EFUSE_FLASH_PERMISSION_CTRL_Msk;
    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);
    // Relock Protected Registers
    SYS_UnlockReg();
}

void app_init(void)
{
#if CONFIG_BOOT_ENABLE
    print_version_info();
#endif

    SYS_TEST("\nDump eFuse content after reset:\n");
    DumpEfuseKeysAndSecureCtrlFlag();
    SYS_TEST("Done.\n");
}
