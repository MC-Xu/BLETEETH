#ifndef PAN_POWER__H_
#define PAN_POWER__H_
#include "PanSeries.h"


typedef struct {
	uint8_t buck_out_trim;                  // offset: 0x56 (bit 0 ~ 4)                        // offset: 0x56 (bit 5 ~ 7)
	uint8_t lph_ldo_vref_trim;
	uint8_t lpl_ldo_trim;
	uint8_t hp_ldo_trim;
} power_param_t;

bool PW_ParamIsHas(void);
void PW_ParamsSet(OTP_STRUCT_T *p_otp);
void PW_AutoOptimizeParams(int16_t temp);
#endif
