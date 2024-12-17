#ifndef SIGNAL__H_
#define SIGNAL__H_

#include <stdbool.h>
#include "PanSeries.h"

bool sig_key1_push_down(void);
bool sig_key2_push_down(void);

bool sig_special_ram_value_detected(void);
bool sig_ota_start_received(void);
bool sig_back_up_is_completed_image(void);
void sig_hardware_recovery(void);
#endif
