#ifndef power_h
#define power_h

#include <avr/io.h>
#include "RGB_LED.h"

void power_init(); //just calls cap_monitor and leg_monitor init
void cap_monitor_init();
void leg_monitor_init();

uint8_t cap_status();			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t leg_status(uint8_t leg);
int8_t leg1_status();			// Returns 0 if leg is floating,
int8_t leg2_status();			//         1 if leg is on power
int8_t leg3_status();			//         -1 if leg is on ground


int8_t legs_powered();

uint8_t light_if_unpowered(uint8_t r, uint8_t g, uint8_t b);

#endif
