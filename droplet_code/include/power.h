#pragma once
#include "droplet_base.h"

void powerInit(void); //just calls cap_monitor and leg_monitor init
void capMonitorInit(void);
void legMonitorInit(void);

uint8_t capStatus(void);			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t legStatus(uint8_t leg);

uint8_t legsPowered(void);
