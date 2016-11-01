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

#define NEIGHBOR_ANGLE_THRESH 20

#define NEIGHBOR_MSG_FLAG	'N'
#define RGB_MSG_FLAG		'R'
#define PATTERN_MSG_FLAG	'P'
#define TURING_MSG_FLAG		'T'
#define NUM_PATTERNS		3
#define NUM_PREPARE			20 //20
#define NUM_GRADIENT		10
#define NUM_CONSENSUS		35 //30
#define NUM_TURING			20 //20
#define NUM_DROPLETS		9

#define SLOT_LENGTH_MS		263
#define SLOTS_PER_FRAME		37
#define FRAME_LENGTH_MS		(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS		17

//Turing Pattern related
#define TURING_F			(0.75f)
#define TURING_RANDOM		(0.02f)		// A threshold for random pattern
#define PI_6				0.523598775598298873077  // pi/6
#define PI_12				0.261799387799149436538  // pi/12
#define TEST_PREPARE		1
#define TEST_GRADIENT		1
#define TEST_CONSENSUS		1
#define TEST_TURING			1

const float rgb_weights[3] = {0.2989, 0.5870, 0.1140};  // RGB to Gray

const uint8_t vIndex[NUM_NEIGHBOR_4] = {0, 2, 8, 10};
const uint8_t hIndex[NUM_NEIGHBOR_4] = {1, 3, 9, 11};

/*  */
typedef struct Droplet_struct{
	float myPattern_f[NUM_PATTERNS];
	uint16_t neighborIds[NUM_NEIGHBOR_12];
	int16_t rgb[3];
	uint16_t dropletId;
	uint8_t mySlot;
	uint8_t myDegree;
	uint8_t turing_color;
} Droplet;

typedef struct NeighborId_struct{
	uint16_t Ids[NUM_NEIGHBOR_4];
	uint16_t dropletId;
	uint8_t  gotMsg_flags[NUM_NEIGHBOR_4];
	char flag;
} neighborMsg;

typedef struct RGB_struct{
	int16_t rgb[3];
	uint16_t dropletId;
	char flag;
} rgbMsg;

typedef struct Pattern_struct{
	float pattern_f[NUM_PATTERNS];
	uint16_t dropletId;
	uint8_t degree;
	char flag;
} patternMsg;

typedef struct Turing_struct{
	uint16_t dropletId;
	uint8_t color;
	char flag;
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
patternMsg  allPattern[NUM_CONSENSUS];

int16_t red_array[NUM_PREPARE];
int16_t green_array[NUM_PREPARE];
int16_t blue_array[NUM_PREPARE];



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

typedef enum{
	Prepare=0,
	Gradient=1,
	Consensus=2,
	Turing=3
} Phase;

typedef struct rnb_node_struct{
	int16_t range;
	int16_t bearing;
	id_t id;
	struct rnb_node_struct* next;
	uint8_t conf;
} RnbNode;
RnbNode* measRoot;
RnbNode* lastMeasAdded;

uint32_t frameStart;
uint32_t frameCount;
uint16_t loopID;
Phase phase;
uint8_t counter;			// to exit phases

int threshold_mottled = 40;

void init();
void loop();
void handle_msg	(ir_msg* msg_struct);
void handle_neighbor_msg(neighborMsg* msg);
void handle_rgb_msg(rgbMsg* msg);
void handle_pattern_msg(patternMsg* msg);
void handle_turing_msg(turingMsg* msg);

void handleRNB();

void extendNeighbors();
uint8_t user_handle_command(char* command_word, char* command_args);

void prepareSlot();
void prepareEOP();
void gradientEOP();
void consensusEOP();
void turingEOP();
void setColor();

void sendRGBMsg();
void sendPatternMsg();
void sendTuringMsg();
void sendNeiMsg();
void decidePattern();
void weightedAverage();
void changeColor();

void displayMenu();

/*
in math.h
#define M_PI		3.14159265358979323846	pi 
#define M_PI_2		1.57079632679489661923	pi/2
#define M_PI_4		0.78539816339744830962	pi/4
#define M_1_PI		0.31830988618379067154	1/pi

*/