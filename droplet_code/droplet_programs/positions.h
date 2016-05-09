#pragma once

#include "droplet_init.h"

/*
 * rnb takes 142 ms
 * messages take 2.5ms per byte. 
 * paddleMsg is 3 bytes. header is 8 bytes, so PaddleMsg should take 27.5
 * neighbMsg is 10 bytes. header is 8 bytes, so NeighbMsg should take 45ms
 * ballMsg is 7 bytes. header is 8 bytes, so ballMsg should take 37.5ms 
 */
#define RNB_DUR		145
#define PADDLE_MSG_DUR		30
#define NEIGHB_MSG_DUR		45
#define BALL_MSG_DUR		40

#define BALL_MSG_FLAG			'B'
#define NEIGHB_MSG_FLAG			'N'
#define N_PADDLE_MSG_FLAG		'P'
#define S_PADDLE_MSG_FLAG		'S'
#define SLOT_LENGTH_MS			271
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

#define GRID_DIMENSION	6.0

//For each of the positions in our immediate 6-neighborhood, this array stores
//the x,y coordinate of that position, in cm.
#define M_SQRT3		1.73205081
#define X_MAX_DIM				180 //540 //180
#define Y_MAX_DIM				164 //528 //268 //164

#define X2_OFFSET	GRID_DIMENSION
#define X_OFFSET	(X2_OFFSET/2.0)
#define Y_OFFSET	X_OFFSET*M_SQRT3
const float neighbPos[NEIGHBORHOOD_SIZE][2] =  {{ X_OFFSET,   Y_OFFSET}, { X2_OFFSET,  0.0},
												{ X_OFFSET,  -Y_OFFSET}, {-X_OFFSET,  -Y_OFFSET},
												{-X2_OFFSET,  0.0},		 {-X_OFFSET,   Y_OFFSET}};
#define FLOOR_WIDTH			(X_MAX_DIM-MIN_DIM)
#define PADDLE_WIDTH		(FLOOR_WIDTH/3)
#define PADDLE_VEL				0.5
#define NUM_SEEDS				4
#define MIN_DIM					0

#define HALF_BOT	((int16_t)(GRID_DIMENSION*5.0))
//The below array is only used for robots to determine if they are a seed.
//Other robots will accept any ID as a seed if they receive a hop message.
//That way, only seeds need reprogramming.
const uint16_t SEED_IDS[NUM_SEEDS] = {0x0029, 0x4ed3, 0xbd2d, 0xccd1};
const int16_t  SEED_X[NUM_SEEDS]   = {MIN_DIM+HALF_BOT, MIN_DIM+HALF_BOT, X_MAX_DIM-HALF_BOT, X_MAX_DIM-HALF_BOT};
const int16_t  SEED_Y[NUM_SEEDS]   = {MIN_DIM+HALF_BOT, Y_MAX_DIM-HALF_BOT, MIN_DIM+HALF_BOT, Y_MAX_DIM-HALF_BOT};


typedef struct ball_msg_struct{
	char flag;
	uint8_t xPos;
	uint8_t yPos;
	uint8_t extraBits; //bits 7-5 are three high bits for xPos; bits 4-2 are three high bits for yPos; bits 0-1 are two low bits for id.
	int8_t xVel;
	int8_t yVel;
	uint8_t radius; //bits 0-1 are two high bits for id. rest is radius (which must be divisible by 4)
}BallMsg;

typedef struct neighb_msg_struct{
	char flag;	
	uint8_t ords[NEIGHBORHOOD_SIZE];
	uint8_t x;
	uint8_t y;
	uint8_t xyHighBits;
}NeighbMsg;

typedef struct paddle_msg_struct{
	char flag;
	int16_t deltaPos;
}PaddleMsg;

typedef struct ball_dat_struct{
	uint32_t lastUpdate;	
	float xPos;
	float yPos;
	int8_t xVel;
	int8_t yVel;
	uint8_t id;
	uint8_t radius;
}BallDat;
BallDat theBall;

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
	PIXEL,		//0
	PIXEL_N,	//1
	PIXEL_S,	//2
	CTRL_NE,	//3
	CTRL_NW,	//4
	CTRL_SE,	//5
	CTRL_SW,	//6
	CTRL_UNKNWN //7
} State;
State myState;

typedef enum {
	POS,
	NEIGHB,
	SYNC_TEST,
	OFF
} ColorMode;
ColorMode	colorMode;

typedef enum {
	PONG,
	BOUNCE
} GameMode;
GameMode gameMode;

uint8_t		lastBallID;
uint8_t		seedFlag;
uint8_t		numNearBots;
uint32_t	frameCount;
uint32_t	frameStart;
uint32_t	lastBallMsg;
uint32_t	lastPaddleMsg;
uint32_t	lastLightCheck;
uint16_t	mySlot;
uint16_t	loopID;

float		paddleChange;
int16_t		paddleStart;
int16_t		paddleEnd;

int8_t		northSouthCount;

//uint8_t outwardDir;
//uint16_t outwardDirID;
float myX, myY, myDist;

int16_t consistency[NEIGHBORHOOD_SIZE];

void		init();
void		handleMySlot();
void		handleFrameEnd();
void		loop();

//This group of functions (and the functions they call) do most of the work!
void		processOtherBotData();
void		updateNeighbsList();
void		updateState();
int16_t		processNeighborData(uint8_t update);
void		updateBall();
void		updatePos();
void		updateColor();

//The five functions below are helper functions for the main functions above.
float		getBallCoverage();
float		getPaddleCoverage();
void		calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]);
float		calculateFactor(float dist, float conf, uint8_t dir, uint16_t id, float* dfP, float* nfP);
void		addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* neighbor, uint8_t dir);
int16_t		checkNeighborChange(uint8_t dir, NeighbsList* neighb);

void		check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel);

void		useNewRnbMeas();	//check for new rnb data and handle it appropriately.
void		checkLightLevel();	//check light sensors.

void		sendBallMsg();
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime);
void		sendNeighbMsg();
void		handleNeighbMsg(NeighbMsg* msg, uint16_t sender);
void		sendPaddleMsg();
void		handlePaddleMsg(char flag, int16_t delta);
void		handle_msg			(ir_msg* msg_struct);

void		bayesDebugPrintout();
void		printNeighbsList();
void		frameEndPrintout();

//These four functions are for interacting with the OtherBot data structure.
OtherBot*	getOtherBot(uint16_t id);
void		removeOtherBot(uint16_t id);
OtherBot*	addOtherBot(uint16_t id, float conf);
void		cleanOtherBot(OtherBot* other);

/*
 * BEGIN INLINE HELPER FUNCTIONS
 */

static float inline getCoverageRatioA(uint8_t rad, float dist){ //when ball radius less than bot radius.
	const float intermediate = (rad/(2*HALF_BOT*HALF_BOT));
	return intermediate*(rad+HALF_BOT-dist);
}

static float inline getCoverageRatioB(uint8_t rad, float dist){ //when bot radius less than ball radius.
	const float intermediate = 1.0/(2.0*HALF_BOT);
	return intermediate*(rad+HALF_BOT-dist);
}

static uint8_t inline getOppDir(uint8_t dir){
	return (dir+(NEIGHBORHOOD_SIZE/2))%NEIGHBORHOOD_SIZE;	
}

static uint8_t inline dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

static int8_t inline checkBallCrossed(){
	return sgn(((theBall.yVel*(theBall.yPos-myY-theBall.xVel) + theBall.xVel*(theBall.xPos-myX+theBall.yVel))));
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

static void inline killBall(){
	theBall.id = 0x0F;
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
	paddleChange = 0.0;
	theBall.lastUpdate = 0;
	theBall.xPos = NAN;
	theBall.yPos = NAN;
	theBall.xVel = 0;
	theBall.yVel = 0;
	theBall.id = 0;
	theBall.radius = 0;
	paddleStart	    = MIN_DIM + (FLOOR_WIDTH>>1) - (PADDLE_WIDTH>>1);
	paddleEnd		= MIN_DIM + (FLOOR_WIDTH>>1) + (PADDLE_WIDTH>>1);
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