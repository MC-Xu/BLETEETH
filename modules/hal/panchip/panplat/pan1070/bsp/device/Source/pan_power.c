#include "PanSeries.h"
#include "pan_power.h"

power_param_t m_power_param;

static uint8_t config_flag = 0xFF;

bool PW_ParamIsHas(void)
{
	return m_power_param.buck_out_trim;
}
void PW_ParamsSet(OTP_STRUCT_T *p_otp)
{
	m_power_param.buck_out_trim = p_otp->m.buck_out_trim;
	m_power_param.hp_ldo_trim = p_otp->m.hp_ldo_trim;
	m_power_param.lph_ldo_vref_trim = p_otp->m_v2.lph_ldo_vref_trim;
	m_power_param.lpl_ldo_trim = p_otp->m.lpl_ldo_trim;
}

void PW_AutoOptimizeParams(int16_t temp)
{
	uint32_t val = 0;
	
	if(temp <= 0) {
		if(config_flag == 1)
			return;
		config_flag = 1;
		
		//buck out(DCDC):default:8;  FT+2
		uint32_t tmp = ANA->LP_BUCK_3V; 
		tmp &= ~(0xFul<<2);
		tmp |= (((m_power_param.buck_out_trim >> 1) + 2) << 2);
		ANA->LP_BUCK_3V = tmp;
		
		//LPLDOH vref;  FT+2
		val = (m_power_param.lph_ldo_vref_trim + 2);
		if(val > 7) val = 7;
	
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0x7u << 21);
		tmp |= (val << 21);
		ANA->LP_LP_LDO_3V = tmp;
		
		//LPLDOL trim;   FT+1
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0xFu << 1);
		// tmp |= ((m_power_param.lpl_ldo_trim + 1) << 1);
		tmp |= (0xf << 1);
		ANA->LP_LP_LDO_3V = tmp;
		
		//HPLDO(DVDD) default:8; FT 
	    tmp = ANA->LP_HP_LDO;
		tmp &= ~(0xFul <<3);
		tmp |= ((m_power_param.hp_ldo_trim)<<3);
		ANA->LP_HP_LDO = tmp;
	}
	else if(temp >= 40) {
		if(config_flag == 2)
			return;
		config_flag = 2;
			
		//buck out(DCDC):default:8;   FT+2
		uint32_t tmp = ANA->LP_BUCK_3V; 
		tmp &= ~(0xFul<<2);
		tmp |= (((m_power_param.buck_out_trim >> 1) + 2) << 2);
		ANA->LP_BUCK_3V = tmp;
		
		//LPLDOH vref;  FT
		val = (m_power_param.lph_ldo_vref_trim);
		if(val > 7) val = 7;
		
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0x7u << 21);
		tmp |= (val << 21);
		ANA->LP_LP_LDO_3V = tmp;
		
		//LPLDOL trim;   FT
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0xFu << 1);
		// tmp |= ((m_power_param.lpl_ldo_trim) << 1);
		tmp |= (0xf << 1);
		ANA->LP_LP_LDO_3V = tmp;
		
		//HPLDO(DVDD) default:8;  FT 
	    tmp = ANA->LP_HP_LDO;
		tmp &= ~(0xFul <<3);
		tmp |= ((m_power_param.hp_ldo_trim)<<3);
		ANA->LP_HP_LDO = tmp;
	}
	else{
		if(config_flag == 0)
			return;
		config_flag = 0;
	
		//buck out(DCDC):default:8;   FT-2
		uint32_t tmp = ANA->LP_BUCK_3V; 
		tmp &= ~(0xFul<<2);
		tmp |= (((m_power_param.buck_out_trim >> 1) - 2) << 2);
		ANA->LP_BUCK_3V = tmp;

		//LPLDOH vref;  FT
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0x7u << 21);
		tmp |= ((m_power_param.lph_ldo_vref_trim) << 21);
		ANA->LP_LP_LDO_3V = tmp;
		
		//LPLDOL trim;   FT
		tmp = ANA->LP_LP_LDO_3V;
		tmp &= ~(0xFu << 1);
		tmp |= ((m_power_param.lpl_ldo_trim) << 1);
		ANA->LP_LP_LDO_3V = tmp;

	#if CONFIG_DVDD_VOL_OPTIMIZE_EN
		//HPLDO(DVDD) default - 1/2; default:8
		tmp = ANA->LP_HP_LDO;
		tmp &= ~(0xFul <<3);
		tmp |= ((m_power_param.hp_ldo_trim - 2)<<3); //~1.12V
		ANA->LP_HP_LDO = tmp;
	#else
		//HPLDO(DVDD) default:8;  FT-1
		tmp = ANA->LP_HP_LDO;
		tmp &= ~(0xFul <<3);
		tmp |= ((m_power_param.hp_ldo_trim - 1)<<3); //~1.16V
		ANA->LP_HP_LDO = tmp;
	#endif
	}
}
