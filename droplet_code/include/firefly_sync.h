#pragma once

#include <avr/io.h>
#include "droplet_init.h"
#include "random.h"

#define FFSYNC_FULL_PERIOD_MS		2333
//#define FFSYNC_REFR_PERIOD_MS		503
//#define FFSYNC_TRANSMIT_DELAY_MS	16

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD		/*	58648	//*/(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
//#define FFSYNC_REFR_PERIOD		/*	5914	//*/(uint16_t)(FFSYNC_REFR_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
								
#define FFSYNC_MAX_DEVIATION	/*	42		//*/(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_EPSILON			25.0
#define FFSYNC_D				164
#define FFSYNC_W				200
//#define FFSYNC_B				2.75
//#define FFSYNC_EPS				0.25
//#define FFSYNC_ALPHA		/*	1.5		//*/exp(FFSYNC_B*FFSYNC_EPS)
//#define FFSYNC_BETA			/*	0.075	//*/((FFSYNC_ALPHA - 1) / (exp(FFSYNC_B) - 1))
//#define FFSYNC_BETA_PERIOD	/*	4399	//*/(FFSYNC_BETA*FFSYNC_FULL_PERIOD)

void firefly_sync_init();

uint8_t sync_blink_r, sync_blink_g, sync_blink_b;
uint8_t sync_def_r, sync_def_g, sync_def_b;
uint16_t sync_blink_duration;

void set_sync_blink_default(uint8_t r, uint8_t g, uint8_t b);
void set_sync_blink_color(uint8_t r, uint8_t g, uint8_t b);
void enable_sync_blink(uint16_t phase_offset_ms);
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
		//return;
		//printf("beep.\r\n");
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