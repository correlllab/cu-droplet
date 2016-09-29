#pragma once

#include <avr/io.h>
#include "droplet_init.h"
#include "random.h"

#define FFSYNC_FULL_PERIOD_MS		5153
//#define FFSYNC_REFR_PERIOD_MS		503
//#define FFSYNC_TRANSMIT_DELAY_MS	16

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD		/*	58648	//*/(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
//#define FFSYNC_REFR_PERIOD		/*	5914	//*/(uint16_t)(FFSYNC_REFR_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
								
#define FFSYNC_MAX_DEVIATION	/*	42		//*/(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_EPSILON			60.0
//60.0 with no return is new best. I've seen it recover from being scattered to a very tight sync. 
//Still some drift, but it recovers.
//25.0 without return inferior to 50.0.
//50.0 without return is probably best so far.
//50.0 with return seems a little worse than 100.0?
//100.0 works acceptably well. return state has no effect.

#define FFSYNC_D				160
#define FFSYNC_W				200
//#define FFSYNC_B				2.75
//#define FFSYNC_EPS				0.25
//#define FFSYNC_ALPHA		/*	1.5		//*/exp(FFSYNC_B*FFSYNC_EPS)
//#define FFSYNC_BETA			/*	0.075	//*/((FFSYNC_ALPHA - 1) / (exp(FFSYNC_B) - 1))
//#define FFSYNC_BETA_PERIOD	/*	4399	//*/(FFSYNC_BETA*FFSYNC_FULL_PERIOD)

void firefly_sync_init();

uint8_t ffsync_blink_r, ffsync_blink_g, ffsync_blink_b;
uint8_t ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b;
uint16_t ffsync_blink_dur;
uint16_t ffsync_blink_phase_offset_ms;

void set_sync_blink_color(uint8_t r, uint8_t g, uint8_t b);
void set_sync_blink_duration(uint16_t dur);
void enable_sync_blink(uint16_t phase_offset_ms);
uint8_t sync_blink_enabled();
void disable_sync_blink();
void processObsQueue();
void updateRTC();
void sendPing(void* val);

typedef struct obs_queue_struct{
	uint16_t obs;
	struct obs_queue_struct* next;
	struct obs_queue_struct* prev;
} obsQueue;

obsQueue* obsStart;

inline void update_firefly_counter(volatile uint16_t count, volatile uint8_t delay){
	uint16_t theDelay = (delay+2)*FFSYNC_MS_CONVERSION_FACTOR;
	int16_t obs;
	obsQueue* node;
	if(count<=theDelay){
		obs = count + (FFSYNC_FULL_PERIOD-theDelay);
	}else{
		obs = count - theDelay;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		node = malloc(sizeof(obsQueue));
	}
	obsQueue* curr = obsStart->prev;
	while(obs < curr->obs){
		curr = curr->prev;
	}
	(curr->next)->prev = node;
	node->next = curr->next;
	node->prev = curr;
	curr->next = node;
			
	node->obs = obs;
}
	
	////printf("%u\r\n",theDelay);
	//if(TCE0.CNT>theDelay){
		//uint16_t theCount = TCE0.CNT - theDelay;
		//
		//if(theCount>FFSYNC_REFR_PERIOD){
			//uint16_t newCount = (uint16_t)fmin(FFSYNC_ALPHA*theCount + FFSYNC_BETA_PERIOD,FFSYNC_FULL_PERIOD-1);
			//if(newCount+theDelay
		//}
	//}
//}