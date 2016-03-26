#pragma once

#include "droplet_init.h"

#define HOP_MSG_FLAG			'H'
#define SLOT_LENGTH_MS			100 
#define SLOTS_PER_FRAME			69 //116
#define FRAME_LENGTH_MS			(SLOT_LENGTH_MS*SLOTS_PER_FRAME)
#define NUM_SEEDS				4
#define COLOR_SLOTS				(NUM_SEEDS+1)
#define MM_PER_HOP				75
#define NUM_HUES				7
#define EST_BOT_COUNT			12

#define MIN_DIM					0
#define MAX_DIM					500

//The below array is only used for robots to determine if they are a seed. 
//Other robots will accept any ID as a seed if they receive a hop message.
//That way, only seeds need reprogramming.
const uint16_t SEED_IDS[NUM_SEEDS] = {0x0120, 0x5264, 0x5f2d, 0xd0ae};
const int16_t  SEED_X[NUM_SEEDS]   = {MIN_DIM, MIN_DIM, MAX_DIM, MAX_DIM};
const int16_t  SEED_Y[NUM_SEEDS]   = {MIN_DIM, MAX_DIM, MIN_DIM, MAX_DIM};

typedef struct hop_msg_struct{
	uint16_t id;
	int16_t x;
	int16_t y;
	char flag;
	uint8_t hopCount;
}HopMsg;

typedef struct hop_struct{
	uint32_t time;
	uint16_t id;
	int16_t x;
	int16_t y;
	uint8_t flag;
	uint8_t hopCount;
}Hop;
Hop trackedHops[NUM_SEEDS];

float myX, myY;
uint32_t	frameCount;
uint32_t	frameStart;
uint8_t		myMsgLoop;
uint16_t	msgPower;
uint8_t		seed;
uint8_t		posColorMode;
uint8_t		prevColorSlot;
uint8_t		randomThresh;
uint16_t	lastLoop;

uint32_t	get_time_wrapper();

void		init();
void		clearTrackedHops();
void		initializeSeeds();
void		loop();
void		setColor(uint8_t loopID);
void		setPosColor();
void		setSeedDataColor(uint8_t loopID);
uint8_t		propagateAsNecessary();
void		sendHopMsg(uint16_t id, int16_t x, int16_t y, uint8_t hC);


uint8_t		addHop(uint16_t id, int16_t x, int16_t y, uint8_t hopCount);
void		sortTrackedHops();
void		handle_msg			(ir_msg* msg_struct);
void		updatePos();

static uint8_t inline countSeeds(){
	uint8_t count=0;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(trackedHops[i].id && trackedHops[i].hopCount!=255){
			count++;
		}
	}
	return count;
}

static void inline propagateHop(uint8_t idx){
	sendHopMsg(trackedHops[idx].id, trackedHops[idx].x, trackedHops[idx].y, trackedHops[idx].hopCount+1);
	trackedHops[idx].flag = 0;
}

static void inline swapTwoTrackedHops(int8_t a, int8_t b){
	printf("Swapping %hd and %hd.\r\n", a, b);
	Hop tmp;
	tmp.time				= trackedHops[a].time;
	tmp.id					= trackedHops[a].id;
	tmp.x					= trackedHops[a].x;
	tmp.y					= trackedHops[a].y;
	tmp.flag				= trackedHops[a].flag;
	tmp.hopCount			= trackedHops[a].hopCount;
	
	trackedHops[a].time		= trackedHops[b].time;
	trackedHops[a].id		= trackedHops[b].id;
	trackedHops[a].x		= trackedHops[b].x;
	trackedHops[a].y		= trackedHops[b].y;
	trackedHops[a].flag		= trackedHops[b].flag;
	trackedHops[a].hopCount = trackedHops[b].hopCount;
	
	trackedHops[b].time		= tmp.time;
	trackedHops[b].id		= tmp.id;
	trackedHops[b].x		= tmp.x;
	trackedHops[b].y		= tmp.y;
	trackedHops[b].flag		= tmp.flag;
	trackedHops[b].hopCount	= tmp.hopCount;
}