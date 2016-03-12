#pragma once

#include "droplet_init.h"

#define HOP_MSG_FLAG			'H'
#define SLOT_LENGTH_MS			150 
#define SLOTS_PER_FRAME			40
#define FRAME_LENGTH_MS			(LOOP_PERIOD_MS*LOOPS_PER_RNB)
#define NUM_SEEDS				4
#define MAX_HOP_COUNT			12

typedef struct hop_msg_struct{
	char flag;
	uint16_t id;
	uint8_t hopCount;
}HopMsg;

typedef struct hop_struct{
	uint32_t time;
	uint16_t id;
	uint8_t flag;
	uint8_t hopCount;
}Hop;
Hop trackedHops[NUM_SEEDS];

uint16_t	loopCount;
uint16_t	myMsgLoop;

void		init();
void		loop();
void		propagateAsNecessary();
void		sendHopMsg();

void		handle_msg			(ir_msg* msg_struct);



static void inline propagateHop(uint8_t idx){
	sendHopMsg(trackedHops[idx].id, trackedHops[idx].hopCount+1);
	trackedHops[idx].flag = 0;
}