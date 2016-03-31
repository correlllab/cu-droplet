#pragma once

#include "droplet_init.h"

#define BALL_MSG_FLAG			'B'
#define NEIGHB_MSG_FLAG			'N'
#define SLOT_LENGTH_MS			631
#define SLOTS_PER_FRAME			116 //69
#define FRAME_LENGTH_MS		(SLOT_LENGTH_MS*SLOTS_PER_FRAME)
#define LOOP_DELAY_MS			17
#define NUM_TRACKED_BOTS		12
#define NUM_POSSIBLE_BOTS		12
#define NUM_TRACKED_BAYESIAN	4
#define NEW_PROBS_SIZE			NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN

//#define BAYES_CLOSE_THRESH		2.5
//#define BAYES_FAR_THRESH		6
//#define UNKNOWN_DIST			10
#define NEIGHBORHOOD_SIZE		6
#define INIT_LIKELIHOOD_EMPTY	0.2
#define UNMEASURED_NEIGHBOR_LIKELIHOOD	0.95

typedef enum{
	NOT_BALL,
	NOT_BALL_ALERT,
	BALL
} State;

#define NUM_SEEDS				4	
#define MIN_DIM					0
#define X_MAX_DIM				480
#define Y_MAX_DIM				520
//The below array is only used for robots to determine if they are a seed.
//Other robots will accept any ID as a seed if they receive a hop message.
//That way, only seeds need reprogramming.
const uint16_t SEED_IDS[NUM_SEEDS] = {0x086b, 0x1562, 0x7066, 0x8521};
const int16_t  SEED_X[NUM_SEEDS]   = {MIN_DIM, MIN_DIM, X_MAX_DIM, X_MAX_DIM};
const int16_t  SEED_Y[NUM_SEEDS]   = {MIN_DIM, Y_MAX_DIM, MIN_DIM, Y_MAX_DIM};

#define GRID_DIMENSION	6.0

//For each of the positions in our immediate 6-neighborhood, this array stores
//the x,y coordinate of that position, in cm.
#define M_SQRT3		1.73205081

#define X2_OFFSET	GRID_DIMENSION
#define X_OFFSET	(X2_OFFSET/2.0)
#define Y_OFFSET	X_OFFSET*M_SQRT3
const float neighbPos[NEIGHBORHOOD_SIZE][2] =  {{ X_OFFSET,   Y_OFFSET}, { X2_OFFSET,  0.0},
												{ X_OFFSET,  -Y_OFFSET}, {-X_OFFSET,  -Y_OFFSET},
												{-X2_OFFSET,  0.0},		 {-X_OFFSET,   Y_OFFSET}};
	
typedef struct ball_msg_struct{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallMsg;

typedef struct neighb_msg_struct{
	uint16_t ids[NEIGHBORHOOD_SIZE];
	int16_t x;
	int16_t y;
	char flag;
}NeighbMsg;

typedef struct bot_pos_struct
{
	uint16_t id;
	float r;
	float b;
	float h;
	float rV;
	float bV;
	float hV;
} BotPos;

typedef struct other_bot_rnb_struct{
	uint16_t id;
	BotPos pos;
	float conf;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

typedef struct bayes_bot_struct{
	float P;
	uint16_t id;
} BayesBot;

typedef struct new_bayes_slot_struct{
	BayesBot candidates[NEW_PROBS_SIZE];
	float emptyProb;
	float untrackedProb;
} NewBayesSlot;

typedef struct bayes_slot_struct{
	BayesBot candidates[NUM_TRACKED_BAYESIAN];
	float emptyProb;
	float untrackedProb;
} BayesSlot;
BayesSlot bayesSlots[NEIGHBORHOOD_SIZE];

typedef struct neighbs_list_struct{
	uint16_t id;
	int16_t x;
	int16_t y;
	uint16_t neighbs[NEIGHBORHOOD_SIZE];
} NeighbsList;
NeighbsList neighbsList[NEIGHBORHOOD_SIZE];

uint32_t time_before;

State myState;
uint8_t ballSeqPos;

uint8_t		seedFlag;
uint8_t		numNearBots;
uint8_t		posColorMode;
uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	lastLoop;

uint8_t outwardDir;
uint16_t outwardDirID;
float myX, myY;

uint8_t failureCounts[NEIGHBORHOOD_SIZE];
uint8_t successCounts[NEIGHBORHOOD_SIZE];

void		init();
void		loop();
void		updatePos();
void		setColor();
void		sendNeighbMsg();
void		sendBallMsg();
void		handle_msg			(ir_msg* msg_struct);

void printNeighbsList();
void processNeighborData();
void printNeighborCountResults();
void cleanupNeighbsList();

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]);
float calculateFactor(float dist, float conf, uint8_t dir, uint16_t id, float* dfP, float* nfP);
void addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* neighbor, uint8_t dir);
void processOtherBotData();
void bayesDebugPrintout();

void useNewRnbMeas();

//These three functions are for the figuring out where the ball bounces to, 
//and communicating this information.
uint8_t check_bounce(uint8_t in_dir);
void calculateOutboundDir(uint16_t inID);

//These four functions are for interacting with the OtherBot data structure.
OtherBot* getOtherBot(uint16_t id);
void removeOtherBot(uint16_t id);
OtherBot* addOtherBot(uint16_t id, float conf);
void cleanOtherBot(OtherBot* other);

//int nearBotsCmpFunc(const void* a, const void* b);
//int bayesCmpFunc(const void* a, const void* b);

//Helper functions:
static uint8_t inline getOppDir(uint8_t dir){
	return (dir+(NEIGHBORHOOD_SIZE/2))%NEIGHBORHOOD_SIZE;	
}

uint16_t reciprocationTracker[NEIGHBORHOOD_SIZE];

static void inline getVarsFromConf(float conf, float* rVar, float* bVar, float* hVar){
	*rVar = 43.41/conf;
	*bVar = 12.17/conf;
	*hVar = 12.17/conf;
}

static float getConfFromVars(float rVar, float bVar, float hVar){
	float rConf, bConf, hConf;
	rConf =  43.41/rVar;
	bConf =  12.17/bVar;
	hConf =  12.17/hVar;
	float newConf = (rConf + bConf + hConf)/3.0;
	if(newConf>600) newConf=600.; //189
	return newConf;
}

static int nearBotsCmpFunc(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	float aC = aN->conf;
	float bC = bN->conf;
	if(aN->id==0){
		return 1;
	}
	if(bN->id==0){
		return -1;
	}
	if(aC < bC){
		return 1;
	}else if(bC < aC){
		return -1;
	}else{
		return 0;
	}
}

static int bayesCmpFunc(const void* a, const void* b){
	BayesBot* aN = (BayesBot*)a;
	BayesBot* bN = (BayesBot*)b;
	float aP = aN->P;
	float bP = bN->P;
	//printf("\t%04X @(%p) : %10f || %04X @(%p) : %10f\r\n", aN->id, aN, aP, bN->id, bN, bP);
	if(bP<aP){
		return -1;
	}else if(bP>aP){
		return 1;
	}else{
		return 0;
	}
}