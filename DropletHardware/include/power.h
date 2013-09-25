#include <avr/io.h>

#ifndef power_h
#define power_h


void cap_monitor_init();
void leg_monitor_init();

uint8_t cap_status();			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t leg1_status();			// Returns 0 if leg is floating,
int8_t leg2_status();			//         1 if leg is on power
int8_t leg3_status();			//         -1 if leg is on ground

int8_t leg_status(uint8_t leg);

int8_t legs_powered();

uint8_t light_if_unpowered(uint8_t r, uint8_t g, uint8_t b);


// Using these prevents RGB or IR measurements
// Use monitors instead if possible
void cap_measure_init();
void leg_measure_init();


//uint8_t findPower();

void droplet_reboot();


#endif
