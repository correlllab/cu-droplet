/*
* camouflage.h
* For Camouflage Project
* Created: 5/25/2016, re-factored on 6/30/2016
* Author : Yang Li
 */ 

#pragma once

#include "droplet_init.h"

#define NUM_NEIGHBOR_12		12
#define NUM_NEIGHBOR_8		8
#define NUM_NEIGHBOR_4		4

#define NEIGHBOR_MSG_FLAG	'N'
#define RGB_MSG_FLAG		'R'
#define PATTERN_MSG_FLAG	'P'
#define TURING_MSG_FLAG		'T'
#define NUM_PATTERNS		2
#define NUM_PREPARE			20 //20
#define NUM_GRADIENT		10
#define NUM_CONSENSUS		10 //30
#define NUM_TURING			10 //20
#define NUM_DROPLETS		9

#define SLOT_LENGTH_MS		300
#define SLOTS_PER_FRAME		11
#define FRAME_LENGTH_MS		(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS		17

//Turing Pattern related
#define TURING_F			(0.55f)
#define TURING_RANDOM		(0.06f)						// A threshold for random pattern
#define PI_6				0.523598775598298873077  // pi/6
#define PI_12				0.261799387799149436538  // pi/12
#define TEST_PREPARE		1
#define TEST_GRADIENT		1
#define TEST_CONSENSUS		1
#define TEST_TURING			1

const uint16_t dropletsID_set[NUM_DROPLETS] =
//{0xA649, 0x4DB0, 0x43BA, 0xC24B, 0x8b46}; //Joe
{0xF60A, 0x2B4E, 0x0A0B, 0x4177, 0x1B4B, 0xA649, 0x4DB0, 0x43BA, 0x8b46}; //Yang
//0xC24B,

const uint8_t vIndex[NUM_NEIGHBOR_4] = {0, 2, 8, 10};
const uint8_t hIndex[NUM_NEIGHBOR_4] = {1, 3, 9, 11};

/*  */
typedef struct Droplet_struct{
	uint16_t dropletId;
	uint16_t neighborIds[NUM_NEIGHBOR_12];
	
	int16_t rgb[3];
	
	uint8_t mySlot;
	uint8_t myDegree;
	uint8_t turing_color;
	
	float myPattern_f;
} Droplet;

typedef struct NeighborId_struct{
	char flag;
	uint16_t dropletId;
	uint16_t Ids[NUM_NEIGHBOR_4];
	uint8_t  gotMsg_flags[NUM_NEIGHBOR_4];
} neighborMsg;

typedef struct RGB_struct{
	char flag;
	uint16_t dropletId;
	int16_t rgb[3];
} rgbMsg;

typedef struct Pattern_struct{
	char flag;
	uint16_t dropletId;
	uint16_t pattern;
	uint8_t degree;
} patternMsg;

typedef struct Turing_struct{
	char flag;
	uint16_t dropletId;
	uint8_t color;
} turingMsg;


Droplet me;

neighborMsg myFourDr;
// store 4 neighbor's neighbor information
neighborMsg fourNeiInfo[NUM_NEIGHBOR_4];
// store 4 neighbor's RGB information
rgbMsg fourNeiRGB[NUM_NEIGHBOR_4];
// store 8 neighbor's Pattern information
patternMsg eightNeiPattern[NUM_NEIGHBOR_8];
// store 12 neighbor's Turing Color information
turingMsg twelveNeiTuring[NUM_NEIGHBOR_12];

/*       Print data        */ 
// RGB reading
rgbMsg allRGB[NUM_PREPARE];
float  allPattern[NUM_CONSENSUS];

// Store information from neighboring Droplets
/*
Neighbor Index document:
//////////////////////////////
        8
    7	0	4
11	3	X	1	9
    6	2	5
       10
//////////////////////////////
*/
Droplet neighborDroplets[NUM_NEIGHBOR_12];

uint32_t frameStart;
uint32_t frameCount;
uint16_t loopID;
uint8_t phase;
uint8_t counter;			// to exit phases

void init();
void loop();
void handle_msg	(ir_msg* msg_struct);
uint8_t user_handle_command(char* command_word, char* command_args);

void preparePhase();
void gradientPhase();
void consensusPhase();
void turingPhase();

uint8_t get_droplet_order_camouflage(uint16_t id)
{
	for(uint8_t i=0; i<NUM_DROPLETS; i++){
		if(dropletsID_set[i] == id) return i;
	}
	return 0xFF;
}


/*
in math.h
#define M_PI		3.14159265358979323846	pi 
#define M_PI_2		1.57079632679489661923	pi/2
#define M_PI_4		0.78539816339744830962	pi/4
#define M_1_PI		0.31830988618379067154	1/pi

*/