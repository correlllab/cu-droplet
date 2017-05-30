#pragma once
#include "droplet_base.h"
#include "ir_comm.h"
#include "random.h"

#define FFSYNC_FULL_PERIOD_MS		5153

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD		/*	58648	//*/(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
								
#define FFSYNC_MAX_DEVIATION	/*	42		//*/(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_EPSILON			60.0

#define FFSYNC_D				160
#define FFSYNC_W				200

void firefly_sync_init(void);

void set_sync_blink_color(uint8_t r, uint8_t g, uint8_t b);
void set_sync_blink_duration(uint16_t dur);
void enable_sync_blink(uint16_t phase_offset_ms);
uint8_t sync_blink_enabled(void);
void disable_sync_blink(void);
void processObsQueue(void);
void sendPing(uint16_t val);

typedef struct obs_queue_struct{
	uint16_t obs;
	struct obs_queue_struct* next;
	struct obs_queue_struct* prev;
} ObsQueue;

ObsQueue* obsStart;

inline void update_firefly_counter(volatile uint16_t count, volatile uint8_t delay){
	uint16_t theDelay = (delay+2)*FFSYNC_MS_CONVERSION_FACTOR;
	uint16_t obs;
	ObsQueue* node;
	if(count<=theDelay){
		obs = count + (FFSYNC_FULL_PERIOD-theDelay);
	}else{
		obs = count - theDelay;
	}
	node = (ObsQueue*)myMalloc(sizeof(ObsQueue));
	if(node==NULL){
		return;
	}
	ObsQueue* curr = obsStart->prev;
	while(obs < curr->obs){
		curr = curr->prev;
	}
	(curr->next)->prev = node;
	node->next = curr->next;
	node->prev = curr;
	curr->next = node;
			
	node->obs = obs;
}