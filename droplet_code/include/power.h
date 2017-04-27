#pragma once
#include "droplet_base.h"

void power_init(); //just calls cap_monitor and leg_monitor init
void cap_monitor_init();
void leg_monitor_init();

uint8_t cap_status();			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t leg_status(uint8_t leg);

uint8_t legs_powered();
