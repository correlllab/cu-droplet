#pragma once

#include "droplet_init.h"

#define HOP_MSG_FLAG			'H'
#define SLOT_LENGTH_MS			200 
#define SLOTS_PER_FRAME			38
#define FRAME_LENGTH_MS			(SLOT_LENGTH_MS*SLOTS_PER_FRAME)
#define NUM_SEEDS				4
#define MAX_HOP_COUNT			12
#define EST_BOT_COUNT			12

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

uint32_t	frameCount;
uint32_t	frameStart;
uint8_t		myMsgLoop;
uint16_t	lastLoop;

void		init();
void		loop();
void setColor(uint8_t loopID);
uint8_t		propagateAsNecessary();
void		sendHopMsg(uint16_t id, uint8_t hC);

uint8_t		addHop(uint16_t id, uint8_t hopCount);

void		handle_msg			(ir_msg* msg_struct);



static void inline propagateHop(uint8_t idx){
	sendHopMsg(trackedHops[idx].id, trackedHops[idx].hopCount+1);
	trackedHops[idx].flag = 0;
}