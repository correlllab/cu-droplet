#pragma once

#include "droplet_init.h"

//#define RNB_DEBUG_MODE
//#define BALL_DEBUG_MODE
//#define PADDLE_DEBUG_MODE

#ifdef RNB_DEBUG_MODE
#define RNB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define RNB_DEBUG_PRINT(format, ...)
#endif

#ifdef BALL_DEBUG_MODE
#define BALL_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define BALL_DEBUG_PRINT(format, ...)
#endif

#ifdef PADDLE_DEBUG_MODE
#define PADDLE_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define PADDLE_DEBUG_PRINT(format, ...)
#endif

#define DROPLET_DIAMETER_MM		44.4
#define BALL_MSG_FLAG			'B'
#define BOT_POS_MSG_FLAG		'P'
#define N_PADDLE_MSG_FLAG		'N'
#define S_PADDLE_MSG_FLAG		'S'

#define SLOT_LENGTH_MS			397
#define SLOTS_PER_FRAME			38
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

#define NUM_TRACKED_BOTS 12

#define PADDLE_VEL				0.1
#define PADDLE_WIDTH			10

#define STATE_PIXEL		0x01
#define STATE_CTRL		0x02
#define STATE_NORTH		0x11
#define STATE_SOUTH		0x12
#define STATE_EAST		0x14
#define STATE_WEST		0x18
#define NORTH_PIXEL_Q(state)		((state&STATE_PIXEL)&&(state&STATE_NORTH))
#define SOUTH_PIXEL_Q(state)		((state&STATE_PIXEL)&&(state&STATE_SOUTH))
#define CTRL_Q(state)				(state&STATE_CTRL)
#define NE_Q(state)					((state&STATE_NORTH)&&(state&STATE_EAST))
#define NW_Q(state)					((state&STATE_NORTH)&&(state&STATE_WEST))
#define SE_Q(state)					((state&STATE_SOUTH)&&(state&STATE_EAST))
#define SW_Q(state)					((state&STATE_SOUTH)&&(state&STATE_WEST))
#define NS_PIXEL_Q(state)			((state&STATE_PIXEL)&&((state&STATE_NORTH) || (state&STATE_SOUTH)))
#define EW_PIXEL_Q(state)			((state&STATE_PIXEL)&&((state&STATE_EAST) || (state&STATE_WEST)))

typedef enum {
	POS,
	GAME
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

typedef struct bot_pos_msg_struct{
	BotPos pos; //6 bytes
	char flag;
}BotPosMsg;

typedef struct paddle_msg_struct{
	char flag;
	int16_t deltaPos;
}PaddleMsg;

typedef struct other_bot_rnb_struct{
	BotPos pos;
	id_t id;
	//uint8_t hasNewInfo;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS+1];

typedef struct hard_bot_struct{
	id_t id;
	struct hard_bot_struct* next;
} HardBot;
HardBot* hardBotsList;

uint8_t		lastBallID;
uint8_t		myState;
uint32_t	lastBallMsg;
uint32_t	lastPaddleMsg;
uint32_t	lastLightCheck;
float		paddleChange;
int16_t		paddleStart;
int16_t		paddleEnd;
uint8_t		isCovered;
uint16_t    myDist;
uint16_t    otherDist;

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	loopID;

uint8_t		numNearBots;
void		init();
void		loop();
void		handle_msg(ir_msg* msg_struct);
float		getPaddleCoverage();
void		checkLightLevel();
void		sendBotPosMsg();
void		handleBotPosMsg(BotPosMsg* msg, id_t senderID);

void		updateBall();
void		updateColor();
float		getBallCoverage();

void		updateHardBots();
void		sendBallMsg();
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime);
void		sendPaddleMsg();
void		handlePaddleMsg(char flag, int16_t delta);

OtherBot*	getOtherBot(id_t id);
void		findAndRemoveOtherBot(id_t id);
void		removeOtherBot(uint8_t idx);
OtherBot*	addOtherBot(id_t id);
void		cleanOtherBot(OtherBot* other);
void		printNearBots();

void		addHardBot(id_t id);
void		cleanHardBots();

inline static void copyBotPos(BotPos* src, BotPos* dest){
	dest->x = src->x;
	dest->y = src->y;
	dest->o = src->o;
}

inline static float getCoverageRatioA(uint8_t rad, uint16_t dist){ //when ball radius less than bot radius.
	const float intermediate = (((float)rad)/(2*DROPLET_RADIUS*DROPLET_RADIUS));
	return intermediate*(rad+DROPLET_RADIUS-dist);
}

inline static float getCoverageRatioB(uint8_t rad, uint16_t dist){ //when bot radius less than ball radius.
	const float intermediate = 1.0/(2.0*DROPLET_RADIUS);
	return intermediate*(rad+DROPLET_RADIUS-dist);
}

inline static int8_t checkBallCrossedMe(){
	return sgn(((theBall.yVel*(theBall.yPos-myPos.y-theBall.xVel) + theBall.xVel*(theBall.xPos-myPos.x+theBall.yVel))));
}

inline static int8_t checkBounceHard(int16_t Bx, int16_t By, int32_t timePassed){
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
inline static void calculateBounce(int16_t Bx, int16_t By){
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

static int nearBotsDistCmp(const void* a, const void* b){
	BotPos* aPos = &(((OtherBot*)a)->pos);
	BotPos* bPos = &(((OtherBot*)b)->pos);
	float aDist = hypot(aPos->y - myPos.y, aPos->x - myPos.x);
	float bDist = hypot(bPos->y - myPos.y, bPos->x - myPos.x);
	if(aDist < bDist){
		return -1;
	}else if(bDist < aDist){
		return 1;
	}else{
		return 0;
	}
}

static int nearBotsBearingCmp(const void* a, const void* b){
	BotPos* aPos = &(((OtherBot*)a)->pos);
	BotPos* bPos = &(((OtherBot*)b)->pos);
	float aBearing = atan2(aPos->y - myPos.y, aPos->x - myPos.x);
	float bBearing = atan2(bPos->y - myPos.y, bPos->x - myPos.x);
	if(aBearing < bBearing){
		return -1;
	}else if(bBearing < aBearing){
		return 1;
	}else{
		return 0;
	}
}

inline static void killBall(){
	set_rgb(255,0,0);
	theBall.id = 0x0F;
}

static inline uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}

