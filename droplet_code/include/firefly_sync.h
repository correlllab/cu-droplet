#pragma once
#include "droplet_base.h"
#include "ir_comm.h"
#include "random.h"

#define FFSYNC_FULL_PERIOD_MS		8009

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD		/*	58648	//*/(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
								
#define FFSYNC_MAX_DEVIATION	/*	42		//*/(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_FFC				50
#define FFSYNC_EPSILON			(1.0/FFSYNC_FFC)


#define FFSYNC_D				403 //Requirement: FFSYNC_D << FFSYNC_FULL_PERIOD_MS
#define FFSYNC_W				503 //Requirement: FFSYNC_W > FFSYNC_D  && FFSYNC_W << FFSYNC_FULL_PERIOD_MS

void fireflySyncInit(void);

void setSyncBlinkColor(uint8_t r, uint8_t g, uint8_t b);
void setSyncBlinkDuration(uint16_t dur);
void enableSyncBlink(uint16_t phase_offset_ms);
uint8_t syncBlinkEnabled(void);
void disableSyncBlink(void);
void processObsQueue(void);
void sendPing(uint16_t val);

typedef struct obs_queue_struct{
	uint16_t obs;
	struct obs_queue_struct* next;
	struct obs_queue_struct* prev;
} ObsQueue;

ObsQueue* obsStart;

inline uint16_t calculatePhaseJump(uint16_t obs){
	return (uint16_t)(obs/FFSYNC_FFC); //As FFSYNC_FFC = 1/FFSYNC_EPSILON, this is equivalent to obs*FFSYNC_EPSILON.
}

inline void updateFireflyCounter(volatile uint16_t count, volatile uint16_t delay){
	//printf("%u\r\n", delay);
	uint16_t rescaledDelay = delay*FFSYNC_MS_CONVERSION_FACTOR;
	uint16_t obs;
	ObsQueue* node;
	if(count<=rescaledDelay){
		obs = count + (FFSYNC_FULL_PERIOD-rescaledDelay);
	}else{
		obs = count - rescaledDelay;
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
	printf("\r\nGot Sync Msg::%u::%u::",delay,count);//NEW_TESTs
}