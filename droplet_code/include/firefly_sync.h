#pragma once

#include <avr/io.h>
#include "droplet_init.h"



#define REFRACTORY_PERIOD			0x1FFF	//~(1/32)ms
#define FIREFLY_SYNC_FULL_PERIOD	0xFFFF	//~(1/32)ms
#define FIREFLY_SYNC_MS_PERIOD		(FIREFLY_SYNC_FULL_PERIOD/32)
#define TRANSMIT_AND_DECODE_DELAY	560 //~(1/32)ms
#define FIREFLY_SYNC_WAIT_TIME		(FIREFLY_SYNC_FULL_PERIOD-TRANSMIT_AND_DECODE_DELAY)

#define FIREFLY_SYNC_B 2.75
#define FIREFLY_SYNC_EPS 0.1
#define FIREFLY_SYNC_ALPHA exp(FIREFLY_SYNC_B*FIREFLY_SYNC_EPS)
#define FIREFLY_SYNC_BETA ((FIREFLY_SYNC_ALPHA - 1) / (exp(FIREFLY_SYNC_B) - 1))

void firefly_sync_init();

inline void update_firefly_counter()
{
	printf("Updating firefly counter.\r\n");
	uint16_t the_count = TCE0.CNT;
	if(the_count>REFRACTORY_PERIOD)
		TCE0.CNT =  (uint16_t)(fmin(FIREFLY_SYNC_ALPHA * the_count/FIREFLY_SYNC_FULL_PERIOD + FIREFLY_SYNC_BETA, 1.) * ((float)FIREFLY_SYNC_FULL_PERIOD));
}