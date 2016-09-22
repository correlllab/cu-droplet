#pragma once

#include "droplet_init.h"


#define POS_DEBUG_MODE
#define GEN_DEBUG_MODE
#define RNB_DEBUG_MODE
//#define NB_DEBUG_MODE
//#define BALL_DEBUG_MODE

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

#ifdef BALL_DEBUG_MODE
#define BALL_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define BALL_DEBUG_PRINT(format, ...)
#endif

#ifdef GEN_DEBUG_MODE
#define GEN_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define GEN_DEBUG_PRINT(format, ...)
#endif

#ifdef NB_DEBUG_MODE
#define NB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define NB_DEBUG_PRINT(format, ...)
#endif

#ifdef RNB_DEBUG_MODE
#define RNB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define RNB_DEBUG_PRINT(format, ...)
#endif

/*
 * rnb takes 142 ms
 * messages take 2.5ms per byte. 
 * paddleMsg is 3 bytes. header is 8 bytes, so PaddleMsg should take 27.5
 * neighbMsg is 38 bytes. header is 8 bytes, so NeighbMsg should take 115ms
 * ballMsg is 7 bytes. header is 8 bytes, so ballMsg should take 37.5ms 
 */
#define RNB_DUR		145
#define PADDLE_MSG_DUR		20
#define NEIGHB_MSG_DUR		120
#define BALL_MSG_DUR		20

#define UNDF	0x8000

#define SLOT_LENGTH_MS			307
#define SLOTS_PER_FRAME			37
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

const id_t SEED_IDS[NUM_SEEDS] = {0x12AD, 0x1562, 0xCD6B, 0x5F2D};
const int16_t  SEED_X[NUM_SEEDS]   = {0, 28, 932, 929};
const int16_t  SEED_Y[NUM_SEEDS]   = {0, 948, 948, 30};

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 1000
#define MAX_Y 1000

#define NUM_PARTICLES 100
#define PROB_ONE 50000
#define LIKELIHOOD_THRESH (PROB_ONE/NUM_PARTICLES)/10;


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
	PackedBotMeas shared[NUM_SHARED_BOTS];
	int16_t x;
	int16_t y;
	int8_t posConf;
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

typedef struct particle_struct
{
	int16_t x;  //xPos
	int16_t y;  //yPos
	int16_t o;  //orientation
	uint16_t l; //likelihood
} Particle;
Particle particles[NUM_PARTICLES];

typedef struct other_bot_rnb_struct{
	BotMeas meas;
	BotPos pos;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

typedef struct hard_bot_struct{
	id_t id;
	struct hard_bot_struct* next;
} HardBot;
HardBot* hardBotsList;

uint32_t time_before;

uint8_t		lastBallID;
uint8_t		seedFlag;
uint8_t		numNearBots;
uint8_t		myState;
uint32_t	frameCount;
uint32_t	frameStart;
uint32_t	lastBallMsg;
//uint32_t	lastPaddleMsg;
uint32_t	lastLightCheck;
uint16_t	mySlot;
uint16_t	loopID;

//float		paddleChange;
//int16_t		paddleStart;
//int16_t		paddleEnd;
uint8_t		isCovered;

BotPos myPos;
int16_t myO;
uint16_t myDist;
uint16_t otherDist;

void		init();
void		loop();
void		handleMySlot();
void		initParticles();
void		updateParticles(OtherBot* bot);
void		handleFrameEnd();
void		cullParticles();
void		updateHardBots();
void		degradeConfidence();
void		updatePos();
void		useNewRnbMeas();
void		updateBall();
//check_bounce is a helper function for updateBall.
void		check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel);
void		updateColor();
//Coverage functions below are helper functions for 'updateColor'.
float		getBallCoverage();
//float getPaddleCoverage();

void		checkLightLevel();

void		sendBallMsg();
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime);
void		sendNearBotsMsg();
void		handleNearBotsMsg(NearBotsMsg* msg, id_t senderID);
//void		sendPaddleMsg();
//void		handlePaddleMsg(char flag, int16_t delta);
void		handle_msg			(ir_msg* msg_struct);

void		frameEndPrintout();

//These four functions are for interacting with the OtherBot data structure.
OtherBot*	getOtherBot(id_t id);
void		findAndRemoveOtherBot(id_t id);
void		removeOtherBot(uint8_t idx);
OtherBot*	addOtherBot(id_t id, int8_t conf);
void		cleanOtherBot(OtherBot* other);

void		addHardBot(id_t id);
void		cleanHardBots();

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
	range = (range>510) ? 510 : range;
	return ((uint8_t)((range+1)>>1)); //dividing by two, rounding towards closest.
}

static uint16_t inline unpackRange(uint8_t packedRange){
	return (((uint16_t)packedRange)<<1); 
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

static int8_t inline checkBallCrossedMe(){
	return sgn(((theBall.yVel*(theBall.yPos-myPos.y-theBall.xVel) + theBall.xVel*(theBall.xPos-myPos.x+theBall.yVel))));
}

static int8_t inline checkBounceHard(int16_t Bx, int16_t By, int32_t timePassed){
	int16_t Ax = myPos.x;
	int16_t Ay = myPos.y;
	int16_t x = theBall.xPos;
	int16_t y = theBall.yPos;
	int8_t signBefore = sgn((Bx-Ax)*(y-Ay) - (By-Ay)*(x-Ax));
	int16_t xAfter = x + (int16_t)((((int32_t)(theBall.xVel))*timePassed)/1000.0);
	int16_t yAfter = y + (int16_t)((((int32_t)(theBall.yVel))*timePassed)/1000.0);
	int8_t signAfter = sgn((Bx-Ax)*(yAfter-Ay) - (By-Ay)*(xAfter-Ax));
	BALL_DEBUG_PRINT("(%4d, %4d) [%hd] -> (%4d, %4d) [%hd]\r\n", x, y, signBefore, xAfter, yAfter, signAfter);
	if(signBefore!=signAfter){
		return 1;
	}else{
		return 0;
	}
}

/*Code below from http://stackoverflow.com/questions/573084/how-to-calculate-bounce-angle */
static void inline calculateBounce(int16_t Bx, int16_t By){
	int16_t vX = theBall.xVel;
	int16_t vY = theBall.yVel;
	int16_t normX = -(By-myPos.y);
	int16_t normY = (Bx-myPos.x);
	int16_t nDotN = normX*normX + normY*normY;
	int16_t vDotN = vX*normX + vY*normY;
	int16_t uX = normX*vDotN/nDotN;
	int16_t uY = normY*vDotN/nDotN;
	theBall.xVel = vX - 2*uX;
	theBall.yVel = vY - 2*uY;
}

////This function assumes that bP->x and bP->y are not UNDF.
//static void inline getOtherXY(BotMeas* bM, BotPos* bP, int16_t* oX, int16_t* oY){
	//int16_t xOffset = 
	//
//}

static int nearBotsConfCmpFunc(const void* a, const void* b){
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

static int nearBotMeasBearingCmpFunc(const void* a, const void* b){
	BotMeas* aN = (BotMeas*)a;
	BotMeas* bN = (BotMeas*)b;
	int16_t aC = aN->b;
	int16_t bC = bN->b;
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

static void inline killBall(){
	set_rgb(255,0,0);
	theBall.id = 0x0F;
}

static void inline initPositions(){
	myPos.x = UNDF;
	myPos.y = UNDF;
	myPos.conf = 0;	
	myDist = UNDF;

	seedFlag = 0;	
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(get_droplet_id()==SEED_IDS[i]){
			seedFlag = 1;
			myPos.x = SEED_X[i];
			myPos.y = SEED_Y[i];
			myPos.conf = 63;
			break;
		}
	}
	//paddleChange = 0.0;
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