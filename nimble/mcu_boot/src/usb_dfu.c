#include "usb_dfu.h"


void on_usb_dfu_enter(void)
{
	SYS_UnlockReg();
	CLK_ResetSystemToRomMode();
	SYS_LockReg();
}
