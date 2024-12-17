#include "rf_test.h"

void rf_test_start_constant_tone(uint16_t ch)
{
	uint32_t constant_tone[] = {
		0x1A080002,
		0x24080a02,
		0x00480002,
		0x00947A02,  
		0x00017402,
		0x201c7012,
	};
	
    if (ch <= 39) {
        RF_SetChannel(ch);
    }
    
    RF_WriteReg(constant_tone, (sizeof(constant_tone) >> 2));
}
