#pragma once

#include "droplet_init.h"

/*
 * rnb takes 142 ms
 * messages take 2.5ms per byte. 
 * paddleMsg is 3 bytes. header is 8 bytes, so PaddleMsg should take 27.5
 * neighbMsg is 37 bytes. header is 8 bytes, so NeighbMsg should take 112.5ms
 * ballMsg is 7 bytes. header is 8 bytes, so ballMsg should take 37.5ms 
 */
#define RNB_DUR		145
#define PADDLE_MSG_DUR		30
#define NEIGHB_MSG_DUR		115
#define BALL_MSG_DUR		40

#define UNDF	0x8000

#define SLOT_LENGTH_MS			331
#define SLOTS_PER_FRAME			39
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

#define BALL_MSG_FLAG			'B'
#define NEAR_BOTS_MSG_FLAG		'N'
//#define N_PADDLE_MSG_FLAG		'P'
//#define S_PADDLE_MSG_FLAG		'S'

//TODO: Make paddle_width dynamically calculated!
//#define PADDLE_WIDTH		(FLOOR_WIDTH/3)
#define PADDLE_VEL				0.1
#define NUM_SEEDS 4
#define NUM_SHARED_BOTS 4
#define NUM_TRACKED_BOTS 12

const id_t SEED_IDS[NUM_SEEDS] = {0x12AD, 0xCD6B, 0x32A7, 0x5264};
const int16_t  SEED_X[NUM_SEEDS]   = {0, 0, 300, 300};
const int16_t  SEED_Y[NUM_SEEDS]   = {0, 300, 0, 300};

#define STATE_PIXEL		0x1
#define STATE_NORTH		0x2
#define STATE_SOUTH		0x4

#define NORTH_PIXEL(state)		((state&STATE_PIXEL)&&(state&STATE_NORTH))
#define SOUTH_PIXEL(state)		((state&STATE_PIXEL)&&(state&STATE_SOUTH))
#define NS_PIXEL(state)			((state&STATE_PIXEL)&&((state&STATE_NORTH) || (state&STATE_SOUTH)))

typedef enum {
	POS,
	SYNC_TEST,
	OFF
} ColorMode;
ColorMode	colorMode;

typedef enum {
	PONG,
	BOUNCE
} GameMode;
GameMode gameMode;

typedef struct ball_msg_struct{
	char flag;
	uint8_t xPos;
	uint8_t yPos;
	uint8_t extraBits; //bits 7-5 are three high bits for xPos; bits 4-2 are three high bits for yPos; bits 0-1 are two low bits for id.
	int8_t xVel;
	int8_t yVel;
	uint8_t radius; //bits 0-1 are two high bits for id. rest is radius (which must be divisible by 4)
}BallMsg;

typedef struct packed_bot_meas_struct{
	id_t id;
	uint8_t range;
	int8_t conf;
	int8_t b;
	int8_t h;
}PackedBotMeas;

typedef struct near_bots_msg_struct{
	int16_t minX;
	int16_t minY;
	int16_t maxX;
	int16_t maxY;
	int16_t x;
	int16_t y;
	int8_t posConf;
	PackedBotMeas shared[NUM_SHARED_BOTS];
	char flag;	
}NearBotsMsg;

typedef struct ball_dat_struct{
	uint32_t lastUpdate;	
	int16_t xPos;
	int16_t yPos;
	int8_t xVel;
	int8_t yVel;
	uint8_t id;
	uint8_t radius;
}BallDat;
BallDat theBall;

typedef struct bot_meas_struct
{
	id_t id;
	uint16_t r;
	int16_t b;
	int16_t h;
	int8_t conf;	
} BotMeas;

typedef struct bot_pos_struct
{
	int16_t x;
	int16_t y;
	int8_t conf;
} BotPos;

typedef struct other_bot_rnb_struct{
	BotMeas meas;
	BotPos pos;
	BotMeas shared[NUM_SHARED_BOTS];
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

uint32_t time_before;

uint8_t		lastBallID;
uint8_t		seedFlag;
uint8_t		numNearBots;
uint8_t		myState;
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

BotPos myPos;
int16_t minX, minY;
int16_t maxX, maxY;
uint16_t myDist;

void		init();
void		loop();
void		handleMySlot();
void		handleFrameEnd();

void		updatePos();
void		useNewRnbMeas();
void		updateBall();
//check_bounce is a helper function for updateBall.
void		check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel);
void		updateColor();
//Coverage functions below are helper functions for 'updateColor'.
float		getBallCoverage();
float getPaddleCoverage();

void		checkLightLevel();

void		sendBallMsg();
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime);
void		sendNearBotsMsg();
void		handleNearBotsMsg(NearBotsMsg* msg, id_t senderID);
//void		sendPaddleMsg();
//void		handlePaddleMsg(char flag, int16_t delta);
void		handle_msg			(ir_msg* msg_struct);

void		frameEndPrintout();
void		updateMinMax(int16_t sX, int16_t sY, int16_t bX, int16_t bY);

//These four functions are for interacting with the OtherBot data structure.
OtherBot*	getOtherBot(id_t id);
void		findAndRemoveOtherBot(id_t id);
void		removeOtherBot(uint8_t idx);
OtherBot*	addOtherBot(id_t id, int8_t conf);
void		cleanOtherBot(OtherBot* other);

/*
 * BEGIN INLINE HELPER FUNCTIONS
 */

static int8_t inline packAngleMeas(int16_t angle){
	return (int8_t)(angle>>1);
}

static int16_t inline unpackAngleMeas(int8_t angle){
	return (((int16_t)angle)<<1);
}

static uint8_t inline packRange(uint16_t range){
	return ((uint8_t)(range>>2));
}

static uint16_t inline unpackRange(uint8_t packedRange){
	return (((uint16_t)packedRange)+2)>>2; //dividing by four, rounding towards closest.
}

static float inline getCoverageRatioA(uint8_t rad, uint16_t dist){ //when ball radius less than bot radius.
	const float intermediate = (((float)rad)/(2*DROPLET_RADIUS*DROPLET_RADIUS));
	return intermediate*(rad+DROPLET_RADIUS-dist);
}

static float inline getCoverageRatioB(uint8_t rad, uint16_t dist){ //when bot radius less than ball radius.
	const float intermediate = 1.0/(2.0*DROPLET_RADIUS);
	return intermediate*(rad+DROPLET_RADIUS-dist);
}

static uint8_t inline dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

static int8_t inline checkBallCrossed(){
	return sgn(((theBall.yVel*(theBall.yPos-myPos.y-theBall.xVel) + theBall.xVel*(theBall.xPos-myPos.x+theBall.yVel))));
}

////This function assumes that bP->x and bP->y are not UNDF.
//static void inline getOtherXY(BotMeas* bM, BotPos* bP, int16_t* oX, int16_t* oY){
	//int16_t xOffset = 
	//
//}

static int nearBotsCmpFunc(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	int8_t aC = (aN->meas).conf;
	int8_t bC = (bN->meas).conf;
	if((aN->meas).id==0){
		return 1;
	}
	if((bN->meas).id==0){
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

static void inline killBall(){
	set_rgb(255,0,0);
	theBall.id = 0x0F;
}

static void inline initPositions(){
	myPos.x = UNDF;
	myPos.y = UNDF;
	myPos.conf = 0;	
	myDist = UNDF;
	minX = UNDF;
	minY = UNDF;
	maxX = UNDF;
	maxY = UNDF;
	seedFlag = 0;	
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(get_droplet_id()==SEED_IDS[i]){
			seedFlag = 1;
			myPos.x = SEED_X[i];
			myPos.y = SEED_Y[i];
			myPos.conf = 63;
			minX = myPos.x;
			minY = myPos.y;
			maxX = myPos.x;	
			maxY = myPos.y;
			break;
		}
	}
	paddleChange = 0.0;
	theBall.lastUpdate = 0;
	theBall.xPos = UNDF;
	theBall.yPos = UNDF;
	theBall.xVel = 0;
	theBall.yVel = 0;
	theBall.id = 0;
	theBall.radius = 0;
	//paddleStart	    = MIN_DIM + (FLOOR_WIDTH>>1) - (PADDLE_WIDTH>>1);
	//paddleEnd		= MIN_DIM + (FLOOR_WIDTH>>1) + (PADDLE_WIDTH>>1);
}