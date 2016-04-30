#pragma once

#include "droplet_init.h"

#define BALL_MSG_FLAG			'B'
#define NEIGHB_MSG_FLAG			'N'
#define SLOT_LENGTH_MS			313
#define SLOTS_PER_FRAME			121
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17
#define NUM_TRACKED_BOTS		12
#define NUM_POSSIBLE_BOTS		12
#define NUM_TRACKED_BAYESIAN	4
#define NEW_PROBS_SIZE			NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN

//#define BAYES_CLOSE_THRESH	2.5
//#define BAYES_FAR_THRESH		6
//#define UNKNOWN_DIST			10
#define NEIGHBORHOOD_SIZE		6
#define INIT_LIKELIHOOD_EMPTY	0.2
#define UNMEASURED_NEIGHBOR_LIKELIHOOD	0.95

#define NUM_SEEDS				4	
#define MIN_DIM					0
#define X_MAX_DIM				180 //540 //180
#define Y_MAX_DIM				268 //528 //268
//The below array is only used for robots to determine if they are a seed.
//Other robots will accept any ID as a seed if they receive a hop message.
//That way, only seeds need reprogramming.
const uint16_t SEED_IDS[NUM_SEEDS] = {0x0029, 0x4ed3, 0xbd2d, 0xccd1};
const int16_t  SEED_X[NUM_SEEDS]   = {MIN_DIM+30, MIN_DIM+30, X_MAX_DIM-30, X_MAX_DIM-30};
const int16_t  SEED_Y[NUM_SEEDS]   = {MIN_DIM+30, Y_MAX_DIM-30, MIN_DIM+30, Y_MAX_DIM-30};

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
													//
//typedef struct ball_msg_struct{
	//char flag;
	//uint16_t id;
	//uint8_t seqPos;
//}BallMsg;
typedef struct ball_msg_struct{
	char flag;
	int8_t xPos;
	int8_t yPos;
	uint8_t xyPosLow;
	int8_t xVel;
	int8_t yVel;
}BallMsg;

typedef struct ball_dat_struct{
	uint32_t lastUpdate;	
	float xPos;
	float yPos;
	int8_t xVel;
	int8_t yVel;
	uint8_t id;
}BallDat;
BallDat theBall;

int16_t sPaddleStart;
int16_t sPaddleEnd;
int16_t nPaddleStart;
int16_t nPaddleEnd;

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
NeighbsList potNeighbsList[NEIGHBORHOOD_SIZE];

uint32_t time_before;

typedef enum{
	PIXEL,
	PIXEL_N,
	PIXEL_S,
	CTRL_N,
	CTRL_S
} State;
State myState;

typedef enum {
	POS,
	NEIGHB,
	OFF
} ColorMode;
ColorMode	colorMode;

uint8_t lastBallID;
uint8_t		seedFlag;
uint8_t		numNearBots;
uint32_t	frameCount;
uint32_t	frameStart;
uint32_t lastBallMsg;
uint16_t	mySlot;
uint16_t	lastLoop;

//uint8_t outwardDir;
//uint16_t outwardDirID;
float myX, myY, myDist;

int16_t consistency[NEIGHBORHOOD_SIZE];

void		init();
void		loop();
void		updateBall();
void		updatePos();
void		setPixelColor();
void		addBallAndPaddleColor();
void		sendNeighbMsg();
void		sendBallMsg();
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime);
void		handle_msg			(ir_msg* msg_struct);

void printNeighbsList();
int16_t checkNeighborChange(uint8_t dir, NeighbsList* neighb);
int16_t processNeighborData(uint8_t update);
void updateState();
void updateNeighbsList();

static uint8_t inline dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

static int8_t inline checkBallCrossed(){
	return sgn(((theBall.yVel*(theBall.yPos-myY-theBall.xVel) + theBall.xVel*(theBall.xPos-myX+theBall.yVel))));
}

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]);
float calculateFactor(float dist, float conf, uint8_t dir, uint16_t id, float* dfP, float* nfP);
void addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* neighbor, uint8_t dir);
void processOtherBotData();
void bayesDebugPrintout();

void useNewRnbMeas();

////These three functions are for the figuring out where the ball bounces to, 
////and communicating this information.
void check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel);
//void calculateOutboundDir(uint16_t inID);

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

static void inline initPositions(){
	myX = NAN;
	myY = NAN;
	myDist = NAN;
	seedFlag = 0;	
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(get_droplet_id()==SEED_IDS[i]){
			seedFlag = 1;
			myX = SEED_X[i];
			myY = SEED_Y[i];
			break;
		}
	}
	theBall.lastUpdate = 0;
	theBall.xPos = NAN;
	theBall.yPos = NAN;
	theBall.xVel = 0;
	theBall.yVel = 0;
	theBall.id = 0;
	int16_t sPaddleHalfWidth = (sPaddleEnd-sPaddleStart)>>1;
	int16_t nPaddleHalfWidth = (nPaddleEnd-nPaddleStart)>>1;
	int16_t halfFloorWidth = (X_MAX_DIM-MIN_DIM)>>1;
	sPaddleStart	= MIN_DIM + halfFloorWidth - sPaddleHalfWidth;
	sPaddleEnd		= MIN_DIM + halfFloorWidth + sPaddleHalfWidth;
	nPaddleStart	= MIN_DIM + halfFloorWidth - nPaddleHalfWidth;
	nPaddleEnd		= MIN_DIM + halfFloorWidth + nPaddleHalfWidth;	
}

static void inline initBayesDataStructs(){
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		bayesSlots[i].emptyProb=INIT_LIKELIHOOD_EMPTY;
		bayesSlots[i].untrackedProb=1.0-INIT_LIKELIHOOD_EMPTY;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN-1;j++){
			bayesSlots[i].candidates[j].P=0.0;
			bayesSlots[i].candidates[j].id=0;
		}
		neighbsList[i].id = 0;
		neighbsList[i].x = 0x8000;
		neighbsList[i].y = 0x8000;
		for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
			neighbsList[i].neighbs[j] = 0;
		}
	}
}