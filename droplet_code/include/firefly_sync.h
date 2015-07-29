#pragma once

#include <avr/io.h>
#include "droplet_init.h"



#define FFSYNC_FULL_PERIOD_MS		2000
#define FFSYNC_REFR_PERIOD_MS		150
#define FFSYNC_TRANSMIT_DELAY_MS	16

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD			(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
#define FFSYNC_REFR_PERIOD			(uint16_t)(FFSYNC_REFR_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
#define FFSYNC_TRANSMIT_DELAY		(uint16_t)(FFSYNC_TRANSMIT_DELAY_MS*FFSYNC_MS_CONVERSION_FACTOR)

#define FFSYNC_MAX_DEVIATION		(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_EPSILON 0.1

volatile uint16_t next_count_start;     

volatile uint32_t light_start;

void firefly_sync_init();


inline void update_firefly_counter()
{
	uint16_t the_count = (TCE0.CNT-FFSYNC_TRANSMIT_DELAY)%FFSYNC_FULL_PERIOD;
    if(the_count<FFSYNC_REFR_PERIOD)
        next_count_start += (uint16_t)(the_count*FFSYNC_EPSILON); 
}