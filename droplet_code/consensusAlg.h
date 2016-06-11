/*
 * consensusAlg.h
 *
 * Created: 5/25/2016 9:04:54 PM
 * Author : Yang Li and Joe Jackson
 */ 

#pragma once

#include "droplet_init.h"

#define HIST_MSG_FLAG 'H'
#define NUM_PATTERNS 3
#define NUM_DIRS 4
#define NUM_GRADIENT 1000
#define NUM_CONSENSUS 50
#define NUM_DROPLETS 5

#define SLOT_LENGTH_MS 300
#define SLOTS_PER_FRAME (NUM_DROPLETS+1)
#define FRAME_LENGTH_MS (((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS   17

#define PI (3.1415926f)

const uint16_t dropletsID_set[NUM_DROPLETS] =
//{0xA649, 0x4DB0, 0x43BA, 0xC24B, 0x8b46}; //Joe
{0xF60A, 0x2B4E, 0x0A0B, 0x4177, 0x1B4B}; //Yang

typedef struct RGB_struct{
	char flag;
	uint16_t droplet_ID;
	uint16_t RGB[3];
}rgbMsg;

typedef struct Pattern_struct{
	char flag;
	uint16_t droplet_ID;
	uint8_t degree;
	uint16_t patterns[NUM_PATTERNS];
}patternMsg;

typedef struct Drolet_postion_struct{
	uint16_t droplet_ID;
	float range;
	float bearing;
}dropletPosStruct;

void init();
void loop();
void handle_msg	(ir_msg* msg_struct);
void gradientPhase();
void consensusPhase();
void turingPhase();

uint32_t frameStart;
uint16_t frameCount;
uint16_t mySlot;
uint16_t loopID;

uint8_t phase;
// gradient phase
uint8_t countGradient;			// to exit gradient phase
rgbMsg myGRB;
rgbMsg fourDirRGB[NUM_DIRS];	// 0, 1, 2, 3 for left, right, top, bottom
dropletPosStruct fourDirDroplet[NUM_DIRS];	// record ids to store RGB rightly
uint8_t myPattern;				// 0, 1, 2 for h, v, dots

// consensus phase
uint8_t countConsensus;			// to exit consensus phase
float curPatternHist[NUM_PATTERNS];
float oriPatternHist[NUM_PATTERNS];
float prePatternHist[NUM_PATTERNS];
patternMsg neighborHist[NUM_DROPLETS];
uint8_t finalDegree;
uint8_t myDegree;				// update each Frame, the number includes itself
uint8_t countNeighbor;

//turing phase

uint8_t get_droplet_order_camouflage(uint16_t id)
{
	for(uint8_t i=0; i<NUM_DROPLETS; i++){
		if(dropletsID_set[i] == id) return i;
	}
	return 0xFF;
}