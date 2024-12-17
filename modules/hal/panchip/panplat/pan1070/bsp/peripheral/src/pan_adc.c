/**************************************************************************//**
 * @file     adc.c
 * @version  V1.00
 * $Revision:  2$
 * $Date: 16/02/25 15:53 $
 * @brief    Panchip series ADC driver source file
 *
 * @note
 * Copyright (C) 2016 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "PanSeries.h"
#include "pan_adc.h"
#include "pan_sys.h"
#include "math.h"

/** @addtogroup Panchip_Device_Driver Panchip Device Driver
  @{
*/

/** @addtogroup Panchip_ADC_Driver ADC Driver
  @{
*/


/** @addtogroup Panchip_ADC_EXPORTED_FUNCTIONS ADC Exported Functions
  @{
*/

ADC_OPT_T m_adc_opt;
uint32_t global_calc_vbat_mv = 3300;

static void bubble_sort_uint16(uint16_t *arr, size_t len)
{
    uint16_t temp;

    for (size_t i = 0; i < len - 1; i++) {
        for (size_t j = 0; j < len - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

static void bubble_sort_float(float *arr, size_t len)
{
    float temp;

    for (size_t i = 0; i < len - 1; i++) {
        for (size_t j = 0; j < len - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/**
  * @brief Init ADC config parameters
  * @param[in] ADCx     Base address of ADC module
  * @param[in] buf_en   Enable ADC buffer or not
  * @return None
  */
bool ADC_Init(ADC_T *ADCx, bool buf_en)
{
    uint32_t tmpreg;

    ADCx->BV_CTL |= BIT0; // Enable Bias Voltage Ctrl
    ADCx->BV_CTL = (ADCx->BV_CTL & ~BIT1) | (buf_en ? BIT1 : 0u); // Enable or disable buffer

    if (!isFtDataValid)
    {
        return false;
    }

    if (m_adc_opt.ft_version >= 6)
    {
        ADCx->CTL2 = m_adc_opt.adc_ctrl2;
        ADCx->EXTSMPT = m_adc_opt.adc_extsmpt;
    }
    else
    {
        // bit13:11 0'b100, bit10:8 0'b101
        tmpreg = ADCx->CTL2;
        tmpreg &= ~(0x7ul << 8);
        tmpreg |= 5 << 8;
        tmpreg &= ~(0x7ul << 11);
        tmpreg |= 4 << 11;
        tmpreg &= ~(0x7ul << 20);
        tmpreg |= 7 << 20;
        tmpreg &= ~(0x7ul << 24);
        tmpreg |= 7 << 24;
        ADCx->CTL2 = tmpreg;
        ADCx->EXTSMPT = 0x00000008;
    }

    tmpreg = ADCx->BV_CTL;
    tmpreg &= ~(0x3Fu << 3);
    tmpreg |= m_adc_opt.adc_vbg_1v20_trim << 3;
    ADCx->BV_CTL = tmpreg;

    return true;
}

/**
  * @brief Prepare ADC VBG fitting curve paramters due to SoC VBAT level
  * @param[in] ADCx Base address of ADC module
  * @return None
  */
void ADC_PrepareVbgCalibData(ADC_T *ADCx)
{
    // NOTE1: We assume ADC is already initialized!
    // NOTE2: We assume ADC input range is already specified!

    uint32_t chen_bkp = 0;
    uint32_t sample_code = 0;

    if (m_adc_opt.ft_version < 3) {
        // Only use in the SoC which ft_verison >= 3
        return;
    }

    if (ADCx->CTL2 & ADC_SEL_VREF_Msk) {
        // Only use when the ADC reference is 1.2v VBG
        return;
    }

    if (ADCx->CHEN) {
        chen_bkp = ADCx->CHEN;
    }
    // Enable channel 10 (internal channel connected to vbat/4)
    ADC_Open(ADCx, ADC_CHEN_CH10_VDD_4_Msk);

    // Power on ADC
    ADC_PowerOn(ADCx);

    // Delay a while to wait adc stable
    if (m_adc_opt.chip_info >= 0x20) {
        SYS_delay_10nop(500); // Delay 100us+
    } else {
        SYS_delay_10nop(10000); // Delay 3ms+
    }

    for (size_t i = 0; i < 8; i++) {
        /* Start sampling */
        ADC_StartConvert(ADC);
        /* Wait for sampling done */
        while (!ADC_IsDataValid(ADC)) {}
        /* Process sampling result data */
        sample_code += ADC_GetConversionData(ADC);
        // Delay 200us+ to wait adc stable
        if (m_adc_opt.chip_info < 0x20) {
            SYS_delay_10nop(1000);
        }
    }
    sample_code /= 8;

    // Power down ADC
    ADC_PowerDown(ADC);

    if (chen_bkp) {
        ADCx->CHEN = chen_bkp;
    }

    global_calc_vbat_mv = ((uint32_t)(m_adc_opt.adc_vbat_k / 10000.0 * sample_code + m_adc_opt.adc_vbat_b / 100.0));;
}


/**
  * @brief Convert adc code to voltage in uV
  * @param[in] ADCx Base address of ADC module
  * @param[in] adc_code Code sampled by ADC module
  * @return ADC sample voltage output in mV
  */
float ADC_OutputVoltage(ADC_T *ADCx, uint32_t adc_code)
{
    float voltage_mv = 0;

    if (ADCx->CTL2 & ADC_SEL_VREF_Msk)  // Ref src is Vbat
    {
        voltage_mv = m_adc_opt.adc_vdd_k / 10000.0 * adc_code + m_adc_opt.adc_vdd_b; // code to code
        voltage_mv = voltage_mv * global_calc_vbat_mv / 4096;
    }
    else    // Ref src is ADC VBG (1.2v)
    {
        if (m_adc_opt.ft_version >= 3)
        {
            uint32_t calc_vbat_mv = global_calc_vbat_mv;
            uint32_t vol_list[] = {1800, 2100, 2400, 2700, 3000, 3300, 3600};
            float V1, V2;
            float A;
            uint8_t vol_index = 0;
            for (uint8_t i = 0; i < 7; i++)
            {
                if (calc_vbat_mv <= vol_list[0])
                {
                    voltage_mv = (float)m_adc_opt.adc_vbg_kb[0].adc_vbg_k / 100000.0 * adc_code + m_adc_opt.adc_vbg_kb[0].adc_vbg_b / 100.0;
                    break;
                }
                else if (calc_vbat_mv >= vol_list[6])
                {
                    voltage_mv = (float)m_adc_opt.adc_vbg_kb[6].adc_vbg_k / 100000.0 * adc_code + m_adc_opt.adc_vbg_kb[6].adc_vbg_b / 100.0;
                    break;
                }

                if (calc_vbat_mv < vol_list[i])
                {
                    vol_index = i;
                    V1 = (float)m_adc_opt.adc_vbg_kb[vol_index - 1].adc_vbg_k / 100000.0 * adc_code + m_adc_opt.adc_vbg_kb[vol_index - 1].adc_vbg_b / 100.0;
                    V2 = (float)m_adc_opt.adc_vbg_kb[vol_index].adc_vbg_k / 100000.0 * adc_code + m_adc_opt.adc_vbg_kb[vol_index].adc_vbg_b / 100.0;
                    A = (float)(calc_vbat_mv - vol_list[vol_index - 1]) / 300;
                    voltage_mv = A * V2 + (1 - A) * V1;
                    break;
                }
            }
        }
        else
        {
            voltage_mv = m_adc_opt.adc_vbg_k / 100000.0 * adc_code + m_adc_opt.adc_vbg_b / 100.0;
        }
    }

    return voltage_mv;
}

/**
  * @brief get caliration params relatd adc ft process
  * @param[in] ADC_OPT_T    hold adc ft parameters
  * @return NULL
  */
void ADC_SetCalirationParams(OTP_STRUCT_T *opt)
{
    m_adc_opt.chip_info = opt->m.chip_info;
    m_adc_opt.ft_version = opt->m.ft_version;
    m_adc_opt.adc_temp_volt = opt->m.adc_temp_volt;
    m_adc_opt.adc_vbg_1v20_trim = opt->m.adc_vbg_1v20_trim;
    m_adc_opt.adc_vbg_b = opt->m.adc_vbg_b;
    m_adc_opt.adc_vbg_k = opt->m.adc_vbg_k;
    m_adc_opt.adc_vdd_b = opt->m.adc_vdd_b;
    m_adc_opt.adc_vdd_k = opt->m.adc_vdd_k;
    m_adc_opt.current_temp_value = opt->m.current_temp_value;
    m_adc_opt.adc_vbat_k = opt->m_v2.adc_vbat_k;
    m_adc_opt.adc_vbat_b = opt->m_v2.adc_vbat_b;
    m_adc_opt.adc_vbat_dtemp_k = opt->m_v2.adc_vbat_dtemp_k;
    m_adc_opt.adc_vbat_dtemp_b = opt->m_v2.adc_vbat_dtemp_b;
    memcpy(m_adc_opt.adc_vbg_kb, opt->m_v2.adc_vbg_kb, sizeof(ADC_VBG_KB_T) * 7);
    m_adc_opt.adc_temp_k = opt->m_v2.adc_temp_k;
    m_adc_opt.adc_ctrl2 = opt->m_v2.adc_ctrl2;
    m_adc_opt.adc_extsmpt = opt->m_v2.adc_extsmpt;
}

/**
  * @brief Start sampling many data codes, trim several largest and smallest codes and calculate average valu
  * @param[in] ADCx         Base address of ADC module
  * @param[in] sample_buf   Buffer to temporarily store ADC sample code
  * @param[in] sample_cnt   Total sampling count
  * @param[in] trim_cnt     Total trim count, that means throw away the smallest trim_cnt/2 and largest trim_cnt/2 sample codes
  * @return ADC calculated sample code
  */
uint32_t ADC_SamplingCodeTrimMean(ADC_T *ADCx, uint16_t *sample_buf, uint32_t sample_cnt, uint32_t trim_cnt)
{
    uint32_t sample_code = 0;

    // Handle exception
    if (sample_cnt <= trim_cnt) {
        return UINT32_MAX;  // Error occurred
    }

    // Start sampling
    for (int i = 0; i < sample_cnt; i++) {
        ADC_StartConvert(ADCx);
        // Wait for sampling done
        while (!ADC_IsDataValid(ADCx)) {
        }
        // Process sampling result data
        sample_buf[i] = ADC_GetConversionData(ADCx);
        // Delay 200us+ to wait adc stable
        if (m_adc_opt.chip_info < 0x20) {
            SYS_delay_10nop(1000);
        }
    }

    // Filter out several largest and smallest data and calculate average value
    bubble_sort_uint16(sample_buf, sample_cnt);
    for (int i = trim_cnt / 2; i < (sample_cnt - trim_cnt / 2); i++) {
        sample_code += sample_buf[i];
    }
    sample_code /= sample_cnt - trim_cnt;

    return sample_code;
}

/**
  * @brief Measure SoC Temperature using the ADC internal Channel 9
  * @param[in] ADCx Base address of ADC module
  * @return ADC measured temperature in Celsius
  */
float ADC_MeasureSocTemperature(ADC_T *ADCx)
{
    uint32_t u32TempReg = 0;
    float temp_param_k = (m_adc_opt.chip_info >= 0x20) ? (m_adc_opt.ft_version >= 6 ? m_adc_opt.adc_temp_k : 0.35) : -0.69; // t = k * (v - vFT) + tFT
    float voltage = 0;
    float temp_voltage_arr[32];

    // Enable ADC channel 9
    ADC_Open(ADCx, ADC_CHEN_CH9_TMP_Msk);

    // Change ADC Ref to ADC-VBG (1.2v) if is not
    if (ADCx->CTL2 & ADC_SEL_VREF_Msk) {
        ADC_SelInputRange(ADCx, ADC_INPUTRANGE_LOW);
    }

    // Prepare ADC vbg calib paramter due to currnet soc vbat after setting input range to the 0~1.2v ref
    ADC_PrepareVbgCalibData(ADCx);

    // Power on ADC
    ADC_PowerOn(ADCx);

    // Delay a while to wait adc stable
    if (m_adc_opt.chip_info >= 0x20) {
        SYS_delay_10nop(500); // Delay 100us+
    } else {
        SYS_delay_10nop(50000); // Delay 15ms+
    }

    // Start sampling
    for (int i = 0; i < 32; i++) {
        ADC_StartConvert(ADCx);
        // Wait for sampling done
        while (!ADC_IsDataValid(ADCx)) {
        }
        // Process sampling result data
        u32TempReg = ADC_GetConversionData(ADCx);
        temp_voltage_arr[i] = ADC_OutputVoltage(ADCx, u32TempReg);
    }

    // Power down ADC
    ADC_PowerDown(ADCx);

    // Filter out largest 8 and smallest 8 data and calculate average value
    bubble_sort_float(temp_voltage_arr, 32);
    for (int i = 4; i < 28; i++) {
        voltage += temp_voltage_arr[i];
    }
    voltage /= 24;

    // Calculate and return current temp by formula: t = k * (v - vFT) + tFT - tDelta
    return temp_param_k * (voltage - (double)m_adc_opt.adc_temp_volt / 10.0) + (double)m_adc_opt.current_temp_value / 100.0
        - (m_adc_opt.ft_version >= 5 ? ((double)m_adc_opt.adc_vbat_dtemp_k / 10000000.0 * global_calc_vbat_mv + (double)m_adc_opt.adc_vbat_dtemp_b / 1000.0) : 0);
}

/**
  * @brief Measure SoC Temperature More Fast using the ADC internal Channel 9
  * @param[in] ADCx Base address of ADC module
  * @return ADC measured temperature in Celsius
  */
float ADC_MeasureSocTemperatureFast(ADC_T *ADCx)
{
    uint32_t temp_code = 0;
    float temp_param_k = (m_adc_opt.chip_info >= 0x20) ? (m_adc_opt.ft_version >= 6 ? m_adc_opt.adc_temp_k : 0.35) : -0.69; // t = k * (v - vFT) + tFT
    float voltage = 0;
    uint16_t temp_code_arr[128];

    // Check if current chip support this API
    if (m_adc_opt.chip_info < 0x20) {
        return -1.0f;
    }

    // Enable ADC channel 9
    ADC_Open(ADCx, ADC_CHEN_CH9_TMP_Msk);

    // Change ADC Ref to ADC-VBG (1.2v) if is not
    if (ADCx->CTL2 & ADC_SEL_VREF_Msk) {
        ADC_SelInputRange(ADCx, ADC_INPUTRANGE_LOW);
    }

    // Prepare ADC vbg calib paramter due to currnet soc vbat after setting input range to the 0~1.2v ref
    ADC_PrepareVbgCalibData(ADCx);

    // Power on ADC
    ADC_PowerOn(ADCx);

    // Delay a while to wait adc stable
    SYS_delay_10nop(500); // Delay 100us+

    // Start sampling
    for (int i = 0; i < 128; i++) {
        ADC_StartConvert(ADCx);
        // Wait for sampling done
        while (!ADC_IsDataValid(ADCx)) {
        }
        // Process sampling result data
        temp_code_arr[i] = ADC_GetConversionData(ADCx);
    }

    // Power down ADC
    ADC_PowerDown(ADCx);

    // Filter out largest 8 and smallest 8 data and calculate average value
    bubble_sort_uint16(temp_code_arr, 128);
    for (int i = 8; i < 120; i++) {
        temp_code += temp_code_arr[i];
    }
    temp_code /= 112;

    // Calculate voltage of temp sensor
    voltage = ADC_OutputVoltage(ADCx, temp_code);

    // Calculate and return current temp by formula: t = k * (v - vFT) + tFT - tDelta
    return temp_param_k * (voltage - (double)m_adc_opt.adc_temp_volt / 10.0) + (double)m_adc_opt.current_temp_value / 100.0
        - (m_adc_opt.ft_version >= 5 ? ((double)m_adc_opt.adc_vbat_dtemp_k / 10000000.0 * global_calc_vbat_mv + (double)m_adc_opt.adc_vbat_dtemp_b / 1000.0) : 0);
}

/**
  * @brief Measure SoC VBAT using the ADC internal Channel 10
  * @param[in] ADCx Base address of ADC module
  * @return ADC measured VBAT voltage in mV
  */
uint16_t ADC_MeasureSocVbat(ADC_T *ADCx)
{
    uint32_t u32TempReg = 0;
    float voltage = 0;
    float temp_voltage_arr[32];

    // Enable channel 10 (internal channel connected to vbat/4)
    ADC_Open(ADCx, ADC_CHEN_CH10_VDD_4_Msk);

    // Change ADC Ref to ADC-VBG (1.2v) if is not
    if (ADCx->CTL2 & ADC_SEL_VREF_Msk) {
        ADC_SelInputRange(ADCx, ADC_INPUTRANGE_LOW);
    }

//    if (m_adc_opt.chip_info < 0x20) {
//        // Prepare ADC vbg calib paramter due to currnet soc vbat after setting input range to the 0~1.2v ref
//        ADC_PrepareVbgCalibData(ADCx);
//    }

    // Power on ADC
    ADC_PowerOn(ADCx);

    // Delay a while to wait adc stable
    if (m_adc_opt.chip_info >= 0x20) {
        SYS_delay_10nop(500); // Delay 100us+
    } else {
        SYS_delay_10nop(50000); // Delay 15ms+
    }

    // Start sampling
    for (int i = 0; i < 32; i++) {
        ADC_StartConvert(ADCx);
        // Wait for sampling done
        while (!ADC_IsDataValid(ADCx)) {
        }
        // Process sampling result data
        u32TempReg = ADC_GetConversionData(ADCx);
        if (m_adc_opt.ft_version >= 3) {
            temp_voltage_arr[i] = ((float)(m_adc_opt.adc_vbat_k / 10000.0 * u32TempReg + m_adc_opt.adc_vbat_b / 100.0));
        } else {
            temp_voltage_arr[i] = ADC_OutputVoltage(ADCx, u32TempReg);
        }
    }

    // Power down ADC
    ADC_PowerDown(ADCx);

    // Filter out largest 8 and smallest 8 data and calculate average value
    bubble_sort_float(temp_voltage_arr, 32);
    for (int i = 8; i < 24; i++) {
        voltage += temp_voltage_arr[i];
    }
    voltage /= 16;

    if (m_adc_opt.ft_version < 3) {
        voltage = (uint16_t)(4 * voltage + 0.5); // Rounding off
    }

    return voltage;
}

/**
  * @brief Disable ADC Power
  * @param[in] adc Base address of ADC module
  * @return None
  */
void ADC_Disable(ADC_T *ADCx)
{
//    SYS->IPRST1 |= SYS_IPRST1_ADCRST_Msk;
//    SYS->IPRST1 &= ~SYS_IPRST1_ADCRST_Msk;
	
	ADCx->CTL &= ~ADC_CTL_ADCEN_Msk;
    return;

}

/**
  * @brief Disable ADC module
  * @param[in] adc Base address of ADC module
  * @return None
  */
void ADC_Close(void)
{
//    SYS->IPRST1 |= SYS_IPRST1_ADCRST_Msk;
//    SYS->IPRST1 &= ~SYS_IPRST1_ADCRST_Msk;
//    return;

}

/**
  * @brief Configure the hardware trigger condition and enable hardware trigger
  * @param[in] adc Base address of ADC module
  * @param[in] u32Source Decides the hardware trigger source. Valid values are:
  *                 - \ref ADC_TRIGGER_BY_EXT_PIN
  *                 - \ref ADC_TRIGGER_BY_PWM
  * @param[in] u32Param While ADC trigger by PWM, this parameter is used to set the delay between PWM
  *                     trigger and ADC conversion. Valid values are from 0 ~ 0xFF, and actual delay
  *                     time is (4 * u32Param * HCLK). While ADC trigger by external pin, this parameter
  *                     is used to set trigger condition. Valid values are:
  *                 - \ref ADC_FALLING_EDGE_TRIGGER
  *                 - \ref ADC_RISING_EDGE_TRIGGER
  * @return None
  */
void ADC_EnableHWTrigger(ADC_T *ADCx,uint32_t Source,uint32_t TrgCondition)
{
    ADCx->CTL &= ~(ADC_TRIGGER_BY_PWM | ADC_RISING_EDGE_TRIGGER | ADC_CTL_HWTRGEN_Msk);
    if(Source == ADC_TRIGGER_BY_EXT_PIN) {
        ADCx->CTL &= ~(ADC_CTL_HWTRGSEL_Msk | ADC_CTL_HWTRGCOND_Msk);
        ADCx->CTL |= (Source | TrgCondition);
    } else {
        ADCx->CTL |=  Source;
    }
    ADCx->CTL |=  ADC_CTL_HWTRGEN_Msk;
    return;
}

/**
  * @brief Disable hardware trigger ADC function.
  * @param[in] adc Base address of ADC module
  * @return None
  */
void ADC_DisableHWTrigger(ADC_T *ADCx)
{
    ADCx->CTL &= ~(ADC_TRIGGER_BY_PWM | ADC_RISING_EDGE_TRIGGER | ADC_CTL_HWTRGEN_Msk);
}

/**
  * @brief Enable the interrupt(s) selected by u32Mask parameter.
  * @param[in] adc Base address of ADC module
  * @param[in] u32Mask  The combination of interrupt status bits listed below. Each bit
  *                     corresponds to a interrupt status. This parameter decides which
  *                     interrupts will be enabled.
  *                     - \ref ADC_ADIF_INT
  *                     - \ref ADC_CMP0_INT
  *                     - \ref ADC_CMP1_INT
  * @return None
  */
void ADC_EnableInt(ADC_T *ADCx, uint32_t Mask)
{
    if(Mask & ADC_ADIF_INT)
        ADCx->CTL |= ADC_CTL_ADCIEN_Msk;
    if(Mask & ADC_CMP0_INT)
        ADCx->CMP0 |= ADC_CMP0_ADCMPIE_Msk;
    if(Mask & ADC_CMP1_INT)
        ADCx->CMP1 |= ADC_CMP1_ADCMPIE_Msk;

    return;
}

/**
  * @brief Disable the interrupt(s) selected by u32Mask parameter.
  * @param[in] adc Base address of ADC module
  * @param[in] u32Mask  The combination of interrupt status bits listed below. Each bit
  *                     corresponds to a interrupt status. This parameter decides which
  *                     interrupts will be disabled.
  *                     - \ref ADC_ADIF_INT
  *                     - \ref ADC_CMP0_INT
  *                     - \ref ADC_CMP1_INT
  * @return None
  */
void ADC_DisableInt(ADC_T *ADCx, uint32_t Mask)
{
    if(Mask & ADC_ADIF_INT)
        ADCx->CTL &= ~ADC_CTL_ADCIEN_Msk;
    if(Mask & ADC_CMP0_INT)
        ADCx->CMP0 &= ~ADC_CMP0_ADCMPIE_Msk;
    if(Mask & ADC_CMP1_INT)
        ADCx->CMP1 &= ~ADC_CMP1_ADCMPIE_Msk;

    return;
}

/**
  * @brief ADC PWM Sequential Mode Control.
  * @param[in] adc Base address of ADC module
  * @param[in] u32SeqTYPE   This parameter decides which type will be selected.
  *                     - \ref ADC_SEQMODE_TYPE_23SHUNT
  *                     - \ref ADC_SEQMODE_TYPE_1SHUNT
  * @param[in] u32ModeSel  This parameter decides which mode will be selected.
  *                     - \ref ADC_SEQMODE_MODESELECT_CH01
  *                     - \ref ADC_SEQMODE_MODESELECT_CH12
  *                     - \ref ADC_SEQMODE_MODESELECT_CH02
  *                     - \ref ADC_SEQMODE_MODESELECT_ONE
  * @return None
  */
void ADC_SeqModeEnable(ADC_T *ADCx, uint32_t SeqType, uint32_t ModeSel)
{
    // Enable ADC Sequential Mode
    ADCx->SEQCTL = ADCx->SEQCTL  | ADC_SEQCTL_SEQEN_Msk;
    // Select ADC Sequential Mode Type
    ADCx->SEQCTL = (ADCx->SEQCTL & ~(ADC_SEQCTL_SEQTYPE_Msk)) | (SeqType << ADC_SEQCTL_SEQTYPE_Pos);
    // Select ADC Sequential Mode Type
    ADCx->SEQCTL = (ADCx->SEQCTL & ~(ADC_SEQCTL_MODESEL_Msk)) | (ModeSel << ADC_SEQCTL_MODESEL_Pos);

    return;
}

/**
  * @brief ADC PWM Sequential Mode PWM Trigger Source and type.
  * @param[in] adc Base address of ADC module
  * @param[in] u32SeqModeTriSrc1  This parameter decides first PWM trigger source and type.
  *
  *
  * @return None
  */
void ADC_SeqModeTriggerSrc(ADC_T *ADCx, uint32_t SeqModeTriSrc)
{
    // Select PWM Trigger Source Selection for TRG1CTL or TRG2CTL
    if(ADCx->SEQCTL & ADC_SEQCTL_TRG_SEL_Msk)
        ADCx->SEQCTL = (ADCx->SEQCTL & ~(ADC_SEQCTL_TRG2CTL_Msk)) | (SeqModeTriSrc << ADC_SEQCTL_TRG2CTL_Pos);
    else
        ADCx->SEQCTL = (ADCx->SEQCTL & ~(ADC_SEQCTL_TRG1CTL_Msk)) | (SeqModeTriSrc << ADC_SEQCTL_TRG1CTL_Pos);
    return;
}
/**
  * @brief Configure the comparator 0 and enable it
  * @param[in] adc Base address of ADC module
  * @param[in] ChNum  Specifies the source channel, valid value are from 0 to 7
  * @param[in] CmpCondition Specifies the compare condition
  *                     - \ref ADC_CMP0_LESS_THAN
  *                     - \ref ADC_CMP0_GREATER_OR_EQUAL_TO
  * @param[in] CmpData Specifies the compare value. Valid value are between 0 ~ 0x3FF
  * @param[in] MatchCnt Specifies the match count setting, valid values are between 1~16
  * @param[in] CmpSelect comparator select,0 or 1
  * @return None
  * @details For example, ADC_CompareEnable(ADC, 5, ADC_CMP_GREATER_OR_EQUAL_TO, 0x800, 10,ADC_COMPARATOR_0);
  *          Means ADC will assert comparator 0 flag if channel 5 conversion result is 
  *          greater or equal to 0x800 for 10 times continuously.
  * \hideinitializer
  */ 
void ADC_CompareEnable(ADC_T *ADCx,
                       uint32_t ChNum,
                       uint32_t CmpCondition,
                       uint32_t CmpData,
                       uint32_t MatchCnt,
                       uint32_t CmpSelect)
{
    uint32_t TmpRegister;
    
    if(ADC_COMPARATOR_1 == CmpSelect)
        TmpRegister = ADC->CMP1;
    else
        TmpRegister = ADC->CMP0;
    
    TmpRegister |= (ChNum << ADC_CMP0_CMPCH_Pos); //select compare channel
    TmpRegister |= CmpCondition;    //select compare condition(less than or greater than )
    TmpRegister |= (CmpData << ADC_CMP0_CMPDAT_Pos);    //set expect compare data
    TmpRegister |= ((MatchCnt - 1) << ADC_CMP0_CMPMCNT_Pos); //set compare match count
    if(ADC_COMPARATOR_1 == CmpSelect){
        TmpRegister |= ADC_CMP1_ADCMPEN_Msk;    //comparator enable
        ADC->CMP1 = TmpRegister;
    }
    else{
        TmpRegister |= ADC_CMP0_ADCMPEN_Msk;    //comparator enable  
        ADC->CMP0 = TmpRegister;
    }        
}

/**
  * @brief set ADC PWM one channel Sequential Mode configuration.
  * @param[in] adc Base address of ADC module
  * @param[in] Trig  This parameter decides first PWM trigger source and type.
  * @param[in] Level  This parameter decides fifo threshold value.
  * @param[in] DmaEn  This parameter decides dma is used or not.
  * @param[in] HwClrEN  This parameter decides PWM trigger flag cleared by hardware or software.
  * @param[in] AdcCh  This parameter decides which adc channel selected.
  * @return None
  */
void ADC_SeqOneChModeConfig(ADC_T *ADCx, 
							uint32_t Trig,
							uint8_t Level,
							uint8_t DmaEn,
							uint8_t HwClrEN)
{
    // ADC Hardware External trigger Enabled by PMW
    ADC_EnableHWTrigger(ADCx, ADC_TRIGGER_BY_PWM, Trig);
	ADC_SetFifoTrigLevel(ADCx,Level);
	if(DmaEn)
		ADC_DmaModeEnable(ADCx,ENABLE);
	else
		ADC_DmaModeEnable(ADCx,DISABLE);

	// ADC Sequential mode & type Enable
    ADC_SeqModeEnable(ADCx,ADC_SEQMODE_TYPE_23SHUNT,ADC_SEQMODE_MODESELECT_ONE);
	ADC_SeqModeOneChEn(ADCx,ENABLE);
	if(HwClrEN)
			ADC_ClearByHw(ADCx,ENABLE);
	else
			ADC_ClearByHw(ADCx,DISABLE);		
}

/*@}*/ /* end of group Panchip_ADC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group Panchip_ADC_Driver */

/*@}*/ /* end of group Panchip_Device_Driver */

/*** (C) COPYRIGHT 2016 Panchip Technology Corp. ***/
