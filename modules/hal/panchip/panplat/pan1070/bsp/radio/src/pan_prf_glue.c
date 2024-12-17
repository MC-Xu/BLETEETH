/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "comm_prf.h"
#include "pan_phy.h"
#include "pan_sys.h"
#include "info.h"

#ifdef CONFIG_USE_PRF_ISR_CB
#if (CONFIG_PRF_TRNS_TIME_XN297_MODE == 1)
#define XN297_RX_TO_TX_TIME			(XN297_RX_TO_TX_TIME_VICE - PRTX_SPI_TRANS_TIME - PRTX_LEGENCY)
#define MEASURE_COMPENSATE_R_TO_T	(20)
#define XN297_RX_TO_TX_TIME_VICE	(150 + MEASURE_COMPENSATE_R_TO_T)
#define PRTX_SPI_TRANS_TIME			(22)
#define PRTX_LEGENCY				(12)
#define PRTX_TRANS_TIME				(XN297_RX_TO_TX_TIME - 96) // 150 - 22 - 12 - 96 - 32 (162) = 20

#define XN297_TX_TO_RX_TIME			(XN297_TX_TO_RX_TIME_VICE - PTRX_SPI_TRANS_TIME)
#define MEASURE_COMPENSATE_T_TO_R	(9)
#define XN297_TX_TO_RX_TIME_VICE	(120 + MEASURE_COMPENSATE_T_TO_R)
#define PTRX_SPI_TRANS_TIME			(29)
#define PTRX_TRANS_TIME				(XN297_TX_TO_RX_TIME - 59) // 130 - 88 - 32 (120)= 42

#elif (CONFIG_PRF_TRNS_TIME_XN297_MODE == 2)
#define XN297_RX_TO_TX_TIME			(XN297_RX_TO_TX_TIME_VICE - PRTX_SPI_TRANS_TIME - PRTX_LEGENCY)
#define XN297_RX_TO_TX_TIME_VICE	(390)
#define PRTX_SPI_TRANS_TIME			(22)
#define PRTX_LEGENCY				(12)
#define PRTX_TRANS_TIME				(XN297_RX_TO_TX_TIME - 96)

#define XN297_TX_TO_RX_TIME			(XN297_TX_TO_RX_TIME_VICE - PTRX_SPI_TRANS_TIME)
#define XN297_TX_TO_RX_TIME_VICE	(381)
#define PTRX_SPI_TRANS_TIME			(29)
#define PTRX_TRANS_TIME				(XN297_TX_TO_RX_TIME - 59)

#elif (CONFIG_PRF_TRNS_TIME_XN297_MODE == 3)

#endif

#else
#define XN297_RX_TO_TX_TIME			(XN297_RX_TO_TX_TIME_VICE - PRTX_SPI_TRANS_TIME - PRTX_LEGENCY)
#define XN297_RX_TO_TX_TIME_VICE	(390)
#define PRTX_SPI_TRANS_TIME			(22)
#define PRTX_LEGENCY				(12)
#define PRTX_TRANS_TIME				(XN297_RX_TO_TX_TIME - 96)

#define XN297_TX_TO_RX_TIME			(XN297_TX_TO_RX_TIME_VICE - PTRX_SPI_TRANS_TIME)
#define XN297_TX_TO_RX_TIME_VICE	(381)
#define PTRX_SPI_TRANS_TIME			(29)
#define PTRX_TRANS_TIME				(XN297_TX_TO_RX_TIME - 59)
#endif

#define NRF_RX_TO_TX_TIME		(110)
#define NRF_TX_TO_RX_TIME		(110)

#define REG_WRITE_BITS(reg, mask, value)    (reg) = (((reg) & ~(mask)) | (value))

#define __nop()     __asm("nop")
__ramfunc void prf_delay_nop(uint32_t times)
{
	while (times--) {
		__nop();
	}
}


uint32_t data_addr_tx = 0;
uint32_t data_addr_rx = 0x400 * 4;
uint8_t prf_init_flag = 0;
int8_t prf_rssi = 0;
static uint8_t buck_vout_trim = 0xff;
static uint32_t raw_phy_config_data[7] = {0xffffffff, };

__ramfunc void panchip_prf_isr_proc(void)
{
	uint8_t ptxprx = 0;     /* 0:ptx; 1:prx */
	uint8_t enhc = 0;       /* 0:norm; 1:enhc */
	uint8_t normal_m1 = 0;
	uint8_t noack = 0;

	FLCTL->X_FL_IRQ_CTL &= ~(uint32_t)(1UL << ((uint32_t)LL_IRQn)); //disable irq

	ptxprx = PRI_RF_IsRxSel(PRI_RF);
	enhc = PRI_RF_IsEnhance(PRI_RF);
	normal_m1 = (uint8_t)((PRI_RF->R04_RX_CTL & R04_RX_CTL_NORMAL_M1_Msk) >> R04_RX_CTL_NORMAL_M1_Pos);
	noack = PRI_RF_IsTxNoAckEn(PRI_RF);

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_TX_TIMEOUT_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_TX_TIMEOUT_IRQ_MASK_Msk)?(false):(true))) {
		while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
		}			/* wait rx done */
		PHY_strt_stp(LLHWC_PHY_STOP);
		PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);

		if (isr_cb.rx_timeout_cb != NULL) {
			(*isr_cb.rx_timeout_cb)();
		}
	}

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_IRQ_MASK_Msk)?(false):(true))) {
		if (((enhc == 0) && (normal_m1 == 0) && ptxprx) || !ptxprx) {
			while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
			}			/* wait rx done */
			PHY_strt_stp(LLHWC_PHY_STOP);
			PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		}
			
		prf_rssi = ((int16_t)(LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) & 0xffff) - 16384) / 4;
		if (isr_cb.rx_cb != NULL) {
			(*isr_cb.rx_cb)();
		}
	}

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_CRC_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_CRC_ERR_MASK_Msk)?(false):(true))) {
		while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
		}			/* wait rx done */
		PHY_strt_stp(LLHWC_PHY_STOP);
		PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);

		prf_rssi = ((int16_t)(LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) & 0xffff) - 16384) / 4;
		if (isr_cb.rx_crc_err_cb != NULL) {
			(*isr_cb.rx_crc_err_cb)();
		}
	}

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_LENGTH_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_LENGTH_ERR_MASK_Msk)?(false):(true))) {
        while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
		}			/* wait rx done */
		PHY_strt_stp(LLHWC_PHY_STOP);
        PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);

		if (isr_cb.rx_len_err_cb != NULL) {
			(*isr_cb.rx_len_err_cb)();
		}
    }
	
	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_ACCADDR_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_ACCADDR_ERR_MASK_Msk)?(false):(true))) {
        while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
		}			/* wait rx done */
		PHY_strt_stp(LLHWC_PHY_STOP);
        PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);

		if (isr_cb.rx_acc_adr_err_cb != NULL) {
			(*isr_cb.rx_acc_adr_err_cb)();
		}
    }

	if ((PRI_RF->R01_INT & R01_INT_RX_PID_ERR_IRQ_FLAG_Msk) &&
		((PRI_RF->R01_INT & R01_INT_PID_ERR_MASK_Msk)?(false):(true))) {
		while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)) {
		}			/* wait rx done */
		if (ptxprx == 0) {
			PHY_strt_stp(LLHWC_PHY_STOP);
		}

		if (isr_cb.rx_pid_err_cb != NULL) {
			(*isr_cb.rx_pid_err_cb)();
		}
	}
	
	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_TX_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_TX_IRQ_MASK_Msk)?(false):(true))) {
		if (((((enhc == 0) && (normal_m1 == 0)) || ((enhc == 1) && noack)) && !ptxprx) || ptxprx) {
			while (!(PRI_RF->R01_INT & R01_INT_TX_DONE_IRQ_FLAG_Msk)) {
			}			/* wait tx done */
			PHY_strt_stp(LLHWC_PHY_STOP);
			PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		}

		if (isr_cb.tx_cb != NULL) {
			(*isr_cb.tx_cb)();
		}
	}

	PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
}

#ifdef PRF_BLE_DUAL_MODE
__ramfunc uint8_t panchip_prf_isr_resume_ble_cb(void)
{
	uint8_t ptxprx = 0;     /* 0:ptx; 1:prx */
	uint8_t enhc = 0;       /* 0:norm; 1:enhc */
	uint8_t normal_m1 = 0;
	uint8_t noack = 0;

	ptxprx = PRI_RF_IsRxSel(PRI_RF);
	enhc = PRI_RF_IsEnhance(PRI_RF);
	normal_m1 = (uint8_t)((PRI_RF->R04_RX_CTL & R04_RX_CTL_NORMAL_M1_Msk) >> R04_RX_CTL_NORMAL_M1_Pos);
	noack = PRI_RF_IsTxNoAckEn(PRI_RF);

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_TX_TIMEOUT_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_TX_TIMEOUT_IRQ_MASK_Msk)?(false):(true))) {
		while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk)); 		/* wait rx timeout */
		PHY_strt_stp(LLHWC_PHY_STOP);
		PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		panchip_prf_ble_resume();

		if (isr_cb.rx_timeout_cb != NULL) {
			(*isr_cb.rx_timeout_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
	}

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_IRQ_MASK_Msk)?(false):(true))) {
		if (((enhc == 0) && (normal_m1 == 0) && ptxprx) || !ptxprx) {
			while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk));	/* wait rx done */
			PHY_strt_stp(LLHWC_PHY_STOP);
			PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
			panchip_prf_ble_resume();
		}

		prf_rssi = ((int16_t)(LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) & 0xffff) - 16384) / 4;
		if (isr_cb.rx_cb != NULL) {
			(*isr_cb.rx_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
	}

	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_CRC_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_CRC_ERR_MASK_Msk)?(false):(true))) {
		while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk));		/* wait rx crc err */
		PHY_strt_stp(LLHWC_PHY_STOP);
		PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		panchip_prf_ble_resume();

		prf_rssi = ((int16_t)(LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) & 0xffff) - 16384) / 4;
		if (isr_cb.rx_crc_err_cb != NULL) {
			(*isr_cb.rx_crc_err_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
	}
	
	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_LENGTH_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_LENGTH_ERR_MASK_Msk)?(false):(true))) {
        while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk));  //wait rx done
		PHY_strt_stp(LLHWC_PHY_STOP);
        PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		panchip_prf_ble_resume();

		if (isr_cb.rx_len_err_cb != NULL) {
			(*isr_cb.rx_len_err_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
    }
	
	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_RX_ACCADDR_ERR_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_RX_ACCADDR_ERR_MASK_Msk)?(false):(true))) {
        while (!(PRI_RF->R01_INT & R01_INT_RX_DONE_IRQ_FLAG_Msk));  //wait rx done
		PHY_strt_stp(LLHWC_PHY_STOP);
        PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
		panchip_prf_ble_resume();

		if (isr_cb.rx_acc_adr_err_cb != NULL) {
			(*isr_cb.rx_acc_adr_err_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
    }
	
	if ((PRI_RF_IntFlag(PRI_RF, R01_INT_TX_IRQ_Msk)) &&
		((PRI_RF->R01_INT & R01_INT_TX_IRQ_MASK_Msk)?(false):(true))) {
		if (((((enhc == 0) && (normal_m1 == 0)) || ((enhc == 1) && noack)) && !ptxprx) || ptxprx) {
			while (!(PRI_RF->R01_INT & R01_INT_TX_DONE_IRQ_FLAG_Msk));	/* wait tx done */
			PHY_strt_stp(LLHWC_PHY_STOP);
			PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, 0);
			panchip_prf_ble_resume();
		}

		if (isr_cb.tx_cb != NULL) {
			(*isr_cb.tx_cb)();
		}
		PRI_RF_ClearAllIrqEn(PRI_RF, ENABLE);
		return 1;
	}

	return 0;
}

__ramfunc uint8_t panchip_prf_ble_handler(void)
{
	if (panchip_prf_isr_resume_ble_cb() == 1) {
		return 1;
	}

	if (PRI_RF_GetChipMode(PRI_RF) != PRF_CHIP_MODE_SEL_BLE) {
		return 1;
	}
	
	return 0;
}
#endif

#ifndef PRF_BLE_DUAL_MODE
__ramfunc void LL_IRQHandler(void)
{
	/*prf isr handler*/
	panchip_prf_isr_proc();
}
#endif

__weak void panchip_prf_irq_enable(void)
{

}

__weak void panchip_prf_isr_init(void)
{

}


void panchip_prf_module_enable(pan_prf_config_t *p_config)
{	
	memcpy(&rf_config, p_config, sizeof(pan_prf_config_t));
	if(prf_init_flag == 0 || wake_from_lp_mode)
	{
		prf_init_flag = 1;
		#ifndef PRF_BLE_DUAL_MODE
		panchip_prf_module_init();
		panchip_prf_set_tx_pwr(p_config->tx_power);
		#else
		panchip_prf_isr_init();
		#endif
	}

	PRI_RF_ChipModeSel(PRI_RF, p_config->chip_mode);
}

__ramfunc void panchip_prf_set_chn(uint16_t rf_channel)
{
	rf_config.rf_channel = rf_channel;
	uint32_t channel_map = 0;
	uint32_t cmd_channel[] = {
        0x00805602,
        0x000CB302,
        0x0000B402,
        0x0000B502,
        0x0000BE02,
        0x0000B602,
		0x0000B702,
        0x0000B812,
    };
	
	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		//	panchip_prf_reset();
		__disable_irq();
	}
	
	channel_map = rf_config.rf_channel;
	uint8_t  tmp_int_part ;
	uint8_t  divider_ratio;//freq_int_part
    int32_t  tmp_frac_part; 
    uint32_t offset       ;//freq_frac_part
 
    tmp_int_part  = channel_map - (channel_map/32)*32;
	divider_ratio = (tmp_int_part>=16) ? channel_map/32 -32  + 1  :  channel_map/32 -32 ;
	tmp_frac_part = (tmp_int_part>=16) ? tmp_int_part - 32 : tmp_int_part ;
	offset        = (tmp_frac_part>0) ? tmp_frac_part*4096 : 262144 + tmp_frac_part*4096 ; 
 
    cmd_channel[0] |= (divider_ratio << 16);
    cmd_channel[1] |= (((offset >> 16) & 0x03) << 16);
    cmd_channel[2] |= (((offset >>  8) & 0xFF) << 16);
    cmd_channel[3] |= (((offset >>  0) & 0xFF) << 16);
	
	if((rf_config.phy == PRF_PHY_CODED_S8) || (rf_config.phy == PRF_PHY_CODED) || (rf_config.phy == PRF_PHY_250K))
	{
		channel_map    = channel_map - PRF_PHY_1M;
	}
	else
	{
		channel_map    = channel_map - rf_config.phy;
	}

	tmp_int_part  = channel_map - (channel_map/32)*32;
	divider_ratio = (tmp_int_part>=16) ? channel_map/32 -32  + 1  :  channel_map/32 -32 ;
	tmp_frac_part = (tmp_int_part>=16) ? tmp_int_part - 32 : tmp_int_part ;
	offset        = (tmp_frac_part>0) ? tmp_frac_part*4096 : 262144 + tmp_frac_part*4096 ;

	cmd_channel[4] |= (divider_ratio << 16);
    cmd_channel[5] |= (((offset >> 16) & 0x03) << 16);
    cmd_channel[6] |= (((offset >>  8) & 0xFF) << 16);
    cmd_channel[7] |= (((offset >>  0) & 0xFF) << 16);
	
    PHY_WriteCfgFifoBatch(cmd_channel, (sizeof cmd_channel >> 2), NULL);
	
	if((rf_config.phy == PRF_PHY_CODED_S8) || (rf_config.phy == PRF_PHY_CODED) || (rf_config.phy == PRF_PHY_250K))
	{
		channel_map    = channel_map + PRF_PHY_1M;
	}
	else
	{
		channel_map    = channel_map + rf_config.phy;
	}
	

	if((channel_map > 2480) || (channel_map < 2402) || (channel_map == 2470))
	{
		ana_prf_ldo_en();
		
		uint32_t init_channel[] =
		{
			0x00007D12,
		};
		
		init_channel[0] |= (34 << 16);
		PHY_WriteCfgFifoBatch(init_channel, (sizeof init_channel >> 2), NULL);
		
		const uint32_t init_vco_cal[] =
        {
            0x0000ff02,
            
            //vco-calib
			0x00000002,
			0x02080202,
			0x64080A02,
			0xff980302,
			0x00900302,
			0x00000A02,
			0x00000212,		
        };
        // vco Calibration
        PHY_WriteCfgFifoBatch(init_vco_cal, (sizeof init_vco_cal >> 2), NULL);
		/* 30us delay */
		prf_delay_nop(200);
		
		const uint32_t init_pll_bw_cal[] = 
        {
            0x0000FF02,
            
            0x02040202,
            0x14080A02,
            
            0xFF242D02,
            0x00042D02,
            
            0x00000A02,
            0x00000212,
        };
        // PLL-BW Calibration
        PHY_WriteCfgFifoBatch(init_pll_bw_cal, (sizeof init_pll_bw_cal >> 2), NULL);
		
		ana_prf_ldo_dis();
	}
	else
	{
        uint8_t idx = rf_config.rf_channel - 2400;
        
		uint32_t init_channel[] =
		{
			0x00000302,
			0x00201202,
			0x00202702,
			0x00007D12,
		};
		if(idx % 2)
		{
			idx = idx / 2 ;
		}
		else
		{
			idx = idx / 2 - 1;
		}

		init_channel[3] |= (idx << 16);

		PHY_WriteCfgFifoBatch(init_channel, (sizeof init_channel >> 2), NULL);
	}
	
	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		__enable_irq();
	}
}

void panchip_prf_b250k_set_channel(uint8_t idx)
{
	uint32_t channel_map[11] = {2404055, 2410003, 2421899, 2424699, 2441843, 2449191, 2461787, 2469485, 2474383, 2476833, 2479282};
	
	uint32_t cmd_channel[] =
	{
        0x00805602,
        0x000CB302,
        0x0000B402,
        0x0000B502,
        0x0000BE02,
        0x0000B602,
		0x0000B702,
        0x0000B812,
    };
	
	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		//	panchip_prf_reset();
		__disable_irq();
	}
    
	uint16_t  tmp_int_part ;
	uint16_t  divider_ratio;//freq_int_part
    int32_t   tmp_frac_part; 
    uint32_t  offset       ;//freq_frac_part
		
    tmp_int_part  = channel_map[idx] - (channel_map[idx]/32000)*32000;
	divider_ratio = (tmp_int_part>=16000) ? channel_map[idx]/32000 -32  + 1  :  channel_map[idx]/32000 -32 ;
	tmp_frac_part = (tmp_int_part>=16000) ? tmp_int_part - 32000 : tmp_int_part ;
	offset        = (tmp_frac_part>0) ? tmp_frac_part*4096/1000 : 262144 + tmp_frac_part*4096/1000 ; 
		
    cmd_channel[0] |= (divider_ratio << 16);
    cmd_channel[1] |= (((offset >> 16) & 0x03) << 16);
    cmd_channel[2] |= (((offset >>  8) & 0xFF) << 16);
    cmd_channel[3] |= (((offset >>  0) & 0xFF) << 16);
	
	channel_map[idx]    = channel_map[idx] - 1000;
	tmp_int_part  = channel_map[idx] - (channel_map[idx]/32000)*32000;
	divider_ratio = (tmp_int_part>=16000) ? channel_map[idx]/32000 -32  + 1  :  channel_map[idx]/32000 -32 ;
	tmp_frac_part = (tmp_int_part>=16000) ? tmp_int_part - 32000 : tmp_int_part ;
	offset        = (tmp_frac_part>0) ? tmp_frac_part*4096/1000 : 262144 + tmp_frac_part*4096/1000 ; 

	cmd_channel[4] |= (divider_ratio << 16);
    cmd_channel[5] |= (((offset >> 16) & 0x03) << 16);
    cmd_channel[6] |= (((offset >>  8) & 0xFF) << 16);
    cmd_channel[7] |= (((offset >>  0) & 0xFF) << 16);
	
    PHY_WriteCfgFifoBatch(cmd_channel, (sizeof cmd_channel >> 2), NULL);
	
	uint32_t init_channel[] =
	{
		0x00007D02,
		0x00000302,
		0x00201202,
		0x00202712,
	};
	
	channel_map[idx]    = channel_map[idx] + 1000;
	if((channel_map[idx] % 1000) > 500)
	{
		idx = ((channel_map[idx] / 1000 - 2402) / 2) + 1;
	}
	else
	{
		idx = ((channel_map[idx] / 1000 - 2402) / 2);
	}
	init_channel[0] |= (idx << 16);
	PHY_WriteCfgFifoBatch(init_channel, (sizeof init_channel >> 2), NULL);
	
	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		__enable_irq();
	}
}

__ramfunc void data_convert(uint8_t *src, uint8_t *dst, uint8_t *iv)
{
	uint8_t i = 0;

	for(i = 0; i < 16; i++) {
		*dst = (*src ^ *iv);
		src++;
		iv++;
		dst++;
	}
}

void panchip_prf_refresh_phy_page(uint8_t idx)
{
	uint32_t cmd[] = {
		0x0001ff02,
		0x00322002,
		0x00882202,
		0x001a9f02,
		0x00025602,
		0x003e1e02,
		0x00201f02,

		0x01e20f02,
		0x01400102,
		0x0000ff12
	};
	PHY_WriteCfgFifoBatch(cmd, (sizeof(cmd) >> 2), NULL);
}

uint8_t phy_page1_reg62_val = 0;

void panchip_set_phy_reg_before_wfi(void)
{
	uint32_t response;
    uint32_t value;

    uint32_t ed_cmd[] = {
		0x00006213,
    }; 

	CLK->AHB_CLK_CTRL |= ((1<<6) | (1<<7));
	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_MEM_SOFT_RST_N, CTRL_MEM_SOFT_RST_N_MSK, CTRL_MEM_SOFT_RST_N_SHFT, 0);
	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_MEM_SOFT_RST_N, CTRL_MEM_SOFT_RST_N_MSK, CTRL_MEM_SOFT_RST_N_SHFT, 1);

    LLHWC_WRITE_MASK32_REG(0x0000, SLPTMR1_SLPTMR_REST_N, SLPTMR1_SLPTMR_REST_N_MSK, SLPTMR1_SLPTMR_REST_N_SHFT, 0);
    LLHWC_WRITE_MASK32_REG(0x0000, SLPTMR1_SLPTMR_REST_N, SLPTMR1_SLPTMR_REST_N_MSK, SLPTMR1_SLPTMR_REST_N_SHFT, 1);

	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_PWR_MOD, CTRL_PWR_MOD_MSK, CTRL_PWR_MOD_SHFT, 1);
	
    PHY_WriteCfgFifoBatch(&ed_cmd[0], 1, &response);
    value = response & 0xFF;
	phy_page1_reg62_val = (uint8_t)value;

	uint32_t cmd[] = {
		0x0000FF02,
		0x00006212
	};
	PHY_WriteCfgFifoBatch(cmd, (sizeof(cmd) >> 2), NULL);
}

void panchip_reload_phy_reg(void)
{
	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_MEM_SOFT_RST_N, CTRL_MEM_SOFT_RST_N_MSK, CTRL_MEM_SOFT_RST_N_SHFT, 0);
	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_MEM_SOFT_RST_N, CTRL_MEM_SOFT_RST_N_MSK, CTRL_MEM_SOFT_RST_N_SHFT, 1);

    LLHWC_WRITE_MASK32_REG(0x0000, SLPTMR1_SLPTMR_REST_N, SLPTMR1_SLPTMR_REST_N_MSK, SLPTMR1_SLPTMR_REST_N_SHFT, 0);
    LLHWC_WRITE_MASK32_REG(0x0000, SLPTMR1_SLPTMR_REST_N, SLPTMR1_SLPTMR_REST_N_MSK, SLPTMR1_SLPTMR_REST_N_SHFT, 1);

	LLHWC_WRITE_MASK32_REG(0x0000, CTRL_PWR_MOD, CTRL_PWR_MOD_MSK, CTRL_PWR_MOD_SHFT, 1);

	uint32_t cmd[] = {
		0x0000FF02,
		0x00036212
	};
	cmd[1] = (cmd[1] & 0xff00ffff) | (phy_page1_reg62_val << 16);
	PHY_WriteCfgFifoBatch(cmd, (sizeof(cmd) >> 2), NULL);
}
__ramfunc void data_swap(uint8_t *src)
{
	for (uint8_t i = 0; i < 16 / 2; i++)
	{
		uint8_t swap_var = src[i];
		src[i] = src[15 - i];
		src[15 - i] = swap_var;
	}
}

uint32_t panchip_prf_encrypt_decrypt(uint8_t *plain_text,
							uint16_t plain_text_len,
							uint8_t *key,
							uint8_t *iv,
							uint8_t *encry_data,
							prf_enc_dec_mode_t enc_mode)
{
    uint32_t plain_txt_ofst, key_ofst;
    uint32_t status = 0;

    uint8_t i = 0;
	uint8_t j = 0;
    uint8_t *ptr_plain_text_ofst;
	uint8_t *ptr_decypt_text_ofst;

	uint32_t timeout_cnt = 200;
	uint32_t done_flag;
	
	uint32_t pkt_cnt = 0;
	uint8_t tmp_key[16], tpm_iv[16];

	if ((plain_text_len > 0x800) || (0 == plain_text_len)) {
		status = (uint32_t)1;
		goto func_exit;
	}

	memcpy(tmp_key, key, 16);
	memcpy(tpm_iv, iv, 16);
	
	/* Swap to Big-endian */
	data_swap(tmp_key);

	/* Switch to active secure mode */
	LLHWC_WRITE_MASK32_REG(REG_FILE_OFST, CTRL_PWR_MOD, CTRL_PWR_MOD_MSK, CTRL_PWR_MOD_SHFT, 3);

    key_ofst = TX_RX_RAM_OFST + 20;
    plain_txt_ofst = key_ofst + 16;

    memcpy((uint8_t *)(key_ofst + BLE_PERIPH_BASE), tmp_key, 16);

    ptr_plain_text_ofst = (uint8_t *)(plain_txt_ofst + BLE_PERIPH_BASE);
	pkt_cnt = (plain_text_len % 16) ? (plain_text_len / 16 + 1) : (plain_text_len / 16);
	ptr_decypt_text_ofst = (uint8_t *)(pkt_cnt * 16 + plain_txt_ofst + BLE_PERIPH_BASE);

	if (plain_text_len % 16) {
		for (j = 0; j < 16 - plain_text_len % 16; j++)
		{
			ptr_plain_text_ofst[i+j] = 0;
		}
	}

	PRI_RF->SECURE1 &= ~SECURE1_SEC_PKT_ENABLES_Msk;
    /* Write the offset of the encryption key */
	PRI_RF->SECURE4 = (PRI_RF->SECURE4 & ~SECURE4_SEC_KEY1_RX_ADDR_Msk) | (key_ofst << SECURE4_SEC_KEY1_RX_ADDR_Pos);
    /* Write the offset of the encryption key */
	PRI_RF->FSM13 = (PRI_RF->FSM13 & ~FSM13_SM_RX_PACKET_ADDR_1_Msk) | (plain_txt_ofst << FSM13_SM_RX_PACKET_ADDR_1_Pos);
    /* Set encryption mode to ECB */
	if (enc_mode == PRF_ENCRYPT_MODE) {
		PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_MODE_Msk) | (2 << SECURE1_SEC_MODE_Pos);
	} else {
		PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_MODE_Msk) | (0 << SECURE1_SEC_MODE_Pos);
		PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_DECRYPT_EN_Msk) | (1 << SECURE1_SEC_DECRYPT_EN_Pos);
	}

    /* Encrypt only one packet */
	PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_PKT_ENABLES_Msk) | (1 << SECURE1_SEC_PKT_ENABLES_Pos);

    /* Set MIC, AD and MD Length */
	PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_MIC_LEN_Msk) | (0 << SECURE1_SEC_MIC_LEN_Pos);
	PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_AD_LEN_Msk) | (0 << SECURE1_SEC_AD_LEN_Pos);
	PRI_RF->SECURE7 = (PRI_RF->SECURE7 & ~SECURE7_SEC_MD_LEN_Msk) | (16 << SECURE7_SEC_MD_LEN_Pos); //len
	PRI_RF->LL_MAC_CTRL = (PRI_RF->LL_MAC_CTRL & ~LL_MAC_CTRL_CTRL_LL_EN_Msk) | (1 << LL_MAC_CTRL_CTRL_LL_EN_Pos);


	for (uint16_t i = 0; i < pkt_cnt; i++) {
		if (enc_mode == PRF_ENCRYPT_MODE) {
			data_convert(plain_text + i * 16, ptr_plain_text_ofst + i * 16, tpm_iv);
			PRI_RF->FSM13 = (PRI_RF->FSM13 & ~FSM13_SM_RX_PACKET_ADDR_1_Msk) | ((plain_txt_ofst + i * 16) << FSM13_SM_RX_PACKET_ADDR_1_Pos);
		} else {
			PRI_RF->FSM13 = (PRI_RF->FSM13 & ~FSM13_SM_RX_PACKET_ADDR_1_Msk) | ((plain_txt_ofst + (pkt_cnt - 1 - i) * 16) << FSM13_SM_RX_PACKET_ADDR_1_Pos);
		}

		/* Enable encryption */
		PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_ENABLE_Msk) | (1 << SECURE1_SEC_ENABLE_Pos); //len
		do {
			if (timeout_cnt == 0){
				status = (uint32_t)1;
				goto func_exit;
			}
			timeout_cnt--;
			done_flag = ((PRI_RF->INTR1 & INTR1_IC_SEC_DONE_Msk) >> INTR1_IC_SEC_DONE_Pos);
		} while (done_flag == 0);

		/* Clear SEC done signal */
		PRI_RF->INTCLR = (PRI_RF->INTCLR & ~INTCLR_IC_SEC_DONE_CLR_Msk) | (1 << INTCLR_IC_SEC_DONE_CLR_Pos);
		PRI_RF->INTCLR = (PRI_RF->INTCLR & ~INTCLR_IC_SEC_DONE_CLR_Msk) | (0 << INTCLR_IC_SEC_DONE_CLR_Pos);

		/* Disable encryption */
		PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_ENABLE_Msk) | (0 << SECURE1_SEC_ENABLE_Pos);
		if (enc_mode == PRF_ENCRYPT_MODE) {
			memcpy(tpm_iv, ptr_plain_text_ofst + i * 16, 16);
		} else {
			ptr_decypt_text_ofst = ptr_plain_text_ofst + (pkt_cnt - 1 - i) * 16;
			if (i >= pkt_cnt - 1) {
				data_convert(ptr_decypt_text_ofst, ptr_decypt_text_ofst, tpm_iv);
				PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_DECRYPT_EN_Msk) | (0 << SECURE1_SEC_DECRYPT_EN_Pos);
			} else {
				data_convert(ptr_decypt_text_ofst, ptr_decypt_text_ofst, ptr_decypt_text_ofst - 16);
			}
		}
	}
	PRI_RF->SECURE1 = (PRI_RF->SECURE1 & ~SECURE1_SEC_PKT_ENABLES_Msk) | (0 << SECURE1_SEC_PKT_ENABLES_Pos);

	memcpy(encry_data, (uint8_t *)(plain_txt_ofst + BLE_PERIPH_BASE), plain_text_len);
func_exit:
    LLHWC_WRITE_MASK32_REG(REG_FILE_OFST, CTRL_PWR_MOD, CTRL_PWR_MOD_MSK, CTRL_PWR_MOD_SHFT, 1);
    return status;
}

void panchip_prf_set_phy(prf_phy_t phy)
{
	rf_config.phy = phy;

	panchip_prf_mode_conf_set(rf_config.mode_conf);
	
	if(((enum tx_rx_phy)rf_config.phy == LE_CODED) || ((enum tx_rx_phy)rf_config.phy) == LE_CODED_S8)
	{
		PHY_SetTrxSeq_prf_LR((enum tx_rx_phy)rf_config.phy);
	}
	else
	{
		PHY_SetTrxSeq_prf_250K((enum tx_rx_phy)rf_config.phy);
	}
}


void panchip_prf_set_tx_pwr(int8_t tx_pwr)
{
	uint8_t buck_vout_trim_val = (uint8_t)((ANA->LP_BUCK_3V >> 2) & 0x0f);

	if (0xff == buck_vout_trim) {
		buck_vout_trim = buck_vout_trim_val;
	}

	if (tx_pwr >= 9) {
		buck_vout_trim_val = buck_vout_trim + 3;
	} else {
		buck_vout_trim_val = buck_vout_trim;
	}

	if (buck_vout_trim_val > 0x0f) {
		buck_vout_trim_val = 0x0f;
	}

	ANA->LP_BUCK_3V = (ANA->LP_BUCK_3V & ~(0xFu << 2)) | ((buck_vout_trim_val & 0x0f) << 2);

	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		//	panchip_prf_reset();
		__disable_irq();
	}

	if(tx_pwr <= 0)
	{
		tx_pwr = tx_pwr - 1;
	}
	
	rf_config.tx_power = tx_pwr;
	PHY_set_tx_pwr(tx_pwr);

	if(rf_config.chip_mode != PRF_CHIP_MODE_SEL_BLE)
	{
		__enable_irq();
	}
}

__weak void panchip_prf_trx_trans_time(pan_prf_config_t *p_config)
{
	uint16_t trans_wait_time = 0;

	if ((p_config->work_mode == PRF_MODE_ENHANCE) ||
	(p_config->work_mode == PRF_MODE_NORMAL_M1)) {
		if((p_config->chip_mode == PRF_CHIP_MODE_SEL_XN297) && p_config->trx_mode == PRF_RX_MODE) {
			#ifdef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
			trans_wait_time = 0;
			#else
			trans_wait_time = XN297_RX_TO_TX_TIME - phy_time_cfg(p_config->phy - 1, 0);
			#endif
		} else if((p_config->chip_mode == PRF_CHIP_MODE_SEL_XN297) && p_config->trx_mode == PRF_TX_MODE) {
			#ifdef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
			trans_wait_time = 0;
			#else
			trans_wait_time = XN297_TX_TO_RX_TIME - phy_time_cfg(p_config->phy - 1, 1);
			#endif
		} else if((p_config->chip_mode == PRF_CHIP_MODE_SEL_NRF) && p_config->trx_mode == PRF_RX_MODE) {
			#ifdef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
			trans_wait_time = 0;
			#else
			trans_wait_time = NRF_RX_TO_TX_TIME - phy_time_cfg(p_config->phy - 1, 0);
			#endif
		} else if((p_config->chip_mode == PRF_CHIP_MODE_SEL_NRF) && p_config->trx_mode == PRF_TX_MODE) {
			#ifdef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
			trans_wait_time = 0;
			#else
			trans_wait_time = NRF_TX_TO_RX_TIME - phy_time_cfg(p_config->phy - 1, 1);
			#endif
		}

		PRI_RF_SetTrxTransWaitTime(PRI_RF, trans_wait_time);
		
		if(trans_wait_time == 0)
		{
			panchip_prf_reduce_trx_pre_post_delay_time(p_config, false);			//trx 25us,rtx 26us
		}
	}
}

__ramfunc void panchip_prf_trx_start(void)
{
	#ifdef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
	PRI_RF_SET_FUNC_ENABLE(PRI_RF,R07_SRAM_CTL,TX_READY,DISABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF,R07_SRAM_CTL,RX_READY,DISABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF,R07_SRAM_CTL,TX_READY,ENABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF,R07_SRAM_CTL,RX_READY,ENABLE);
	#else

	PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, DISABLE);
	PHY_strt_stp(LLHWC_PHY_START);
	PRI_RF_SetTrxRamReady(PRI_RF, PRI_RF_MODE_SEL_TRX, ENABLE);
	#endif
}

__ramfunc void ana_prf_ldo_en(void)
{
    REG_WRITE_BITS(ANA->ANA_ADC_LDO, 0x01, 0x01);
    REG_WRITE_BITS(ANA->ANA_RFFE_LDO, 0x01, 0x01);
    REG_WRITE_BITS(ANA->ANA_VCO_LDO, 0x01, 0x01);
    /* 3us delay */
	prf_delay_nop(30);
}

__ramfunc __weak void ana_prf_ldo_dis(void)
{
	#ifndef CONFIG_PRF_REDUCE_TRX_TRANS_TIME
    REG_WRITE_BITS(ANA->ANA_ADC_LDO, 0x01, 0x00);
    REG_WRITE_BITS(ANA->ANA_RFFE_LDO, 0x01, 0x00);
    REG_WRITE_BITS(ANA->ANA_VCO_LDO, 0x01, 0x00);
	/* 30us delay */
	prf_delay_nop(200);
	#endif
}

__ramfunc void panchip_prf_reduce_trx_pre_post_delay_time(pan_prf_config_t *p_config, bool restore)
{
	extern uint32_t* g_phy_post_tx_ptr_prf;
	extern uint32_t* g_phy_pre_tx_1M_ptr_prf;
	extern uint32_t* g_phy_pre_rx_1M_ptr_prf;
	extern uint32_t* g_phy_pre_tx_2M_ptr_prf;
	extern uint32_t* g_phy_pre_rx_2M_ptr_prf;

	if (0xffffffff == raw_phy_config_data[0]) {
		raw_phy_config_data[0] = g_phy_post_tx_ptr_prf[2];
		raw_phy_config_data[1] = g_phy_pre_tx_1M_ptr_prf[3];
		raw_phy_config_data[2] = g_phy_pre_tx_1M_ptr_prf[6];
		raw_phy_config_data[3] = g_phy_pre_rx_1M_ptr_prf[6];
		raw_phy_config_data[4] = g_phy_pre_tx_2M_ptr_prf[3];
		raw_phy_config_data[5] = g_phy_pre_tx_2M_ptr_prf[6];
		raw_phy_config_data[6] = g_phy_pre_rx_2M_ptr_prf[6];
	}

	if (p_config->phy == PRF_PHY_1M) {
		if (restore) {
			g_phy_pre_tx_1M_ptr_prf[3] = raw_phy_config_data[1];
			g_phy_pre_tx_1M_ptr_prf[6] = raw_phy_config_data[2];
			g_phy_post_tx_ptr_prf[2] = raw_phy_config_data[0];
			/* 21us */
			g_phy_pre_rx_1M_ptr_prf[6] = raw_phy_config_data[3];
		} else {
			/* 24us */
			g_phy_pre_tx_1M_ptr_prf[3] = (g_phy_pre_tx_1M_ptr_prf[3] & 0x00ffffff);
			g_phy_pre_tx_1M_ptr_prf[6] = (g_phy_pre_tx_1M_ptr_prf[6] & 0x00ffffff) | (0x8 << 24);
			g_phy_post_tx_ptr_prf[2] = g_phy_post_tx_ptr_prf[2] & 0x00ffffff;
			/* 21us */
			g_phy_pre_rx_1M_ptr_prf[6] = (g_phy_pre_rx_1M_ptr_prf[6] & 0x00ffffff) | (0x0 << 24);
		}
	} else if (p_config->phy == PRF_PHY_2M) {

		if (restore) {
			g_phy_pre_tx_2M_ptr_prf[3] = raw_phy_config_data[4];
			g_phy_pre_tx_2M_ptr_prf[6] = raw_phy_config_data[5];
			g_phy_post_tx_ptr_prf[2] = raw_phy_config_data[0];
			/* 21us */
			g_phy_pre_rx_2M_ptr_prf[6] = raw_phy_config_data[6];
		} else {
			/* 24us */
			g_phy_pre_tx_2M_ptr_prf[3] = (g_phy_pre_tx_2M_ptr_prf[3] & 0x00ffffff);
			g_phy_pre_tx_2M_ptr_prf[6] = (g_phy_pre_tx_2M_ptr_prf[6] & 0x00ffffff) | (0x8 << 24);
			g_phy_post_tx_ptr_prf[2] = g_phy_post_tx_ptr_prf[2] & 0x00ffffff;
			/* 21us */
			g_phy_pre_rx_2M_ptr_prf[6] = (g_phy_pre_rx_2M_ptr_prf[6] & 0x00ffffff) | (0x0 << 24);
		}
	}
	
	extern uint32_t PHY_SeqRamInit(void);
	PHY_SeqRamInit();
}

__ramfunc int16_t panchip_prf_set_trx_trans_time(pan_prf_config_t *p_config,
					      prf_speed_sel_t speed,
					      uint16_t transfer_time)
{
	int16_t trans_wait_time = 0;
	uint32_t prescale_clk;
	uint32_t write_phy_time = 0, write_phy_delay_time = 0;
	uint32_t legency_time = 0;

	if ((speed == PRF_TRANSFER_SPEED_4K) || (speed == PRF_TRANSFER_SPEED_8K)) {
		panchip_prf_reduce_trx_pre_post_delay_time(p_config, false);
	}

	prescale_clk = PRI_RF_READ_REG_VALUE(PRI_RF, PHY1, IF_CLK_PRESCALE);
	/* calculate phy write time */
	if (p_config->trx_mode == PRF_TX_MODE) {
		if (p_config->phy == PRF_PHY_1M) {
			write_phy_time = (24 * prescale_clk + 8) * ((post_tx_interpacket_size >> 2));
			if (p_config->work_mode == PRF_MODE_ENHANCE) {
				write_phy_time += (24 * prescale_clk + 8) * ((pre_rx_interpacket_size >> 2) + 1) + 10;
				/* calculate phy write delay time */
				write_phy_delay_time = phy_time_cfg(p_config->phy - 1, 1);
			}
		} else if (p_config->phy == PRF_PHY_2M) {
			write_phy_time = (24 * prescale_clk + 8) * ((post_tx_interpacket_size >> 2));
			if (p_config->work_mode == PRF_MODE_ENHANCE) {
				write_phy_time += (24 * prescale_clk + 8) * ((pre_rx_interpacket_2M_size >> 2) + 1) + 10;
				/* calculate phy write delay time */
				write_phy_delay_time = phy_time_cfg(p_config->phy - 1, 1);
			}
		}
	}
	if (p_config->trx_mode == PRF_RX_MODE) {
		if (p_config->phy == PRF_PHY_1M) {
			legency_time = RX_LEGENCY_1M;
			write_phy_time = (24 * prescale_clk + 8) * ((post_rx_interpacket_size >> 2));
			if (p_config->work_mode == PRF_MODE_ENHANCE) {
				write_phy_time += (24 * prescale_clk + 8) * ((pre_tx_interpacket_size >> 2) + 1) + 10;
				/* calculate phy write delay time */
				write_phy_delay_time = phy_time_cfg(p_config->phy - 1, 0);
			}
		} else if (p_config->phy == PRF_PHY_2M) {
			legency_time = RX_LEGENCY_2M;
			write_phy_time = (24 * prescale_clk + 8) * ((post_rx_interpacket_size >> 2));
			if (p_config->work_mode == PRF_MODE_ENHANCE) {
				write_phy_time += (24 * prescale_clk + 8) * ((pre_tx_interpacket_2M_size >> 2) + 1) + 10;
				/* calculate phy write delay time */
				write_phy_delay_time = phy_time_cfg(p_config->phy - 1, 0);
			}
		}
	}
	/* phy clk is 32M, here right shift 5 bit means the time is microsecond */
	write_phy_time >>= 5;
	if (p_config->trx_mode == PRF_RX_MODE) {
		trans_wait_time = transfer_time - write_phy_time - write_phy_delay_time
				  - legency_time - RX_WINDOW;
	} else {
		trans_wait_time = transfer_time - write_phy_time - write_phy_delay_time - TX_WINDOW;
	}

	if (trans_wait_time < 0) {
		return trans_wait_time;
	}

	if (p_config->work_mode == PRF_MODE_ENHANCE) {
		PRI_RF_SetTrxTransWaitTime(PRI_RF, trans_wait_time);
	}

	return trans_wait_time;
}
