#pragma once
#include "droplet_base.h"
#include "ir_comm.h"
#include "random.h"

#define FFSYNC_FULL_PERIOD_MS		5153

#define FFSYNC_MS_CONVERSION_FACTOR 7.8125

#define FFSYNC_FULL_PERIOD		/*	58648	//*/(uint16_t)(FFSYNC_FULL_PERIOD_MS*FFSYNC_MS_CONVERSION_FACTOR)
								
#define FFSYNC_MAX_DEVIATION	/*	42		//*/(((uint16_t)(FFSYNC_FULL_PERIOD_MS/182))+1)

#define FFSYNC_FFC				50
#define FFSYNC_EPSILON			(1.0/FFSYNC_FFC)


#define FFSYNC_D				403 //Requirement: FFSYNC_D << FFSYNC_FULL_PERIOD_MS
#define FFSYNC_W				503 //Requirement: FFSYNC_W > FFSYNC_D  && FFSYNC_W << FFSYNC_FULL_PERIOD_MS

void fireflySyncInit(void);
uint8_t sendPingPending;
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


/*
 * Firefly synchronization synchronizes the TCE0 period, and the RTC is 'slaved'(?) to
 * the TCE0 period. To avoid confusing extra effects, we would ideally ensure that that
 * none of the firefly synchronization timing is affected by the changes made in updateRTC()
 * in firefly_sync.c. I'm not sure it's possible to do so? But here's a catalogue of them:
 * On the receiving side: When the receiving Droplet gets the last byte of the sync
 *     message, it stores the time ('lastByte') which is passed to receivedIrSyncCmd
 *     in ir_comm.c. This is used to measure how much time passes between when the sync
 *     last byte of the sync message arrives and when 'count' is recorded. I suspect this
 *     gap is nigh-negligible, except in rare instances when an interrupt occurs between
 *     the two. It seems pretty unlikely that an updateRTC interrupt is being triggered
 *     during this tiny gap 5% of the time. But we can check?
 *
 * On the sending side:
 *     Greater cause for concern here. The getTime() result which is used to calculate the 
 *     delay is initially stored in firefly_sync.c:85. This happens immediately after the 
 *     call to updateRTC(). Just before the second-to-last byte of the sync message goes out,
 *     getTime() is called again, and the previously-referenced getTime() result is 
 *     subtracted from this result to calculate the delay. This theoretically presents a 
 *     long window in which the RTC might be changed in a way that messes with the calculated 
 *     delay. However, the start of that window is always right after the updateRTC() call. 
 *     In a vacuum, updateRTC() is called every FFSYNC_FULL_PERIOD_MS (5153) milliseconds.
 *     Other sync messages will reduce this, but could it be getting reduced enough to cause 
 *     an issue? 5% of the time? Maybe. But.. that would also result in two sendPing()'s 
 *     scheduled on top of one another. Which is possible. Would it produce consistent errors 
 *     like we see? Maybe.
 *
 */
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
}