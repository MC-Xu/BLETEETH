#include "signal.h"
#include "flash_manager.h"
#include "prf_ota.h"
#include "comm_prf.h"

bool sig_key1_push_down(void)
{
	GPIO_SetMode(P2, BIT0, GPIO_MODE_INPUT);
	GPIO_EnablePullupPath(P2, BIT0);
	
	for (uint16_t i = 0; i < 1000; i++) {
		if (P20 == 1) {
			return false;
		}
	}
	return true;
}

bool sig_key2_push_down(void)
{

	GPIO_SetMode(P2, BIT1, GPIO_MODE_INPUT);
	GPIO_EnablePullupPath(P2, BIT1);
	
	for (uint16_t i = 0; i < 1000; i++) {
		if (P21 == 1) {
			return false;
		}
	}
	return true;
}

bool sig_special_ram_value_detected(void)
{
	return false;
}

bool sig_ota_start_received(void)
{
	return panchip_prf_ota_start();
}

bool sig_back_up_is_completed_image(void)
{
	return fm_image_completed_check(FLASH_AREA_BACK_UP_START);
}

void sig_hardware_recovery(void)
{
	GPIO_DisablePullupPath(P2, BIT1);
	GPIO_SetMode(P2, BIT1, GPIO_MODE_INPUT);
	GPIO_DisableDigitalPath(P2, BIT1);
	GPIO_DisablePullupPath(P2, BIT0);
	GPIO_SetMode(P2, BIT0, GPIO_MODE_INPUT);
	GPIO_DisableDigitalPath(P2, BIT0);

	/* PHY Reset */
	CLK->IPRST0 |= (BIT3 | BIT7 | BIT8);
	CLK->IPRST0 &= ~(BIT3 | BIT7 | BIT8);
}
