#pragma once

#include "droplet_init.h"

//#define RNB_DEBUG_MODE
#define BALL_DEBUG_MODE
//#define CHECK_BOUNCE_DEBUG_MODE
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

#ifdef CHECK_BOUNCE_DEBUG_MODE
#define CHECK_BOUNCE_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define CHECK_BOUNCE_DEBUG_PRINT(format, ...)
#endif

#ifdef PADDLE_DEBUG_MODE
#define PADDLE_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define PADDLE_DEBUG_PRINT(format, ...)
#endif

#define BALL_VALID() (!isnan(theBall.xPos) && (!isnan(theBall.yPos)) && (theBall.id!=0x0F))

#define DROPLET_DIAMETER_MM		44.4
#define BALL_MSG_FLAG			'B'
#define BOT_POS_MSG_FLAG		'P'
#define N_PADDLE_MSG_FLAG		'N'
#define S_PADDLE_MSG_FLAG		'S'

#define SLOT_LENGTH_MS			509
#define SLOTS_PER_FRAME			17
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
	int16_t xPos;
	int16_t yPos;
	uint8_t id;
	int8_t xVel;
	int8_t yVel;
	uint8_t radius; //bits 0-1 are two high bits for id. rest is radius (which must be divisible by 4)
}BallMsg;

typedef struct ball_dat_struct{
	uint32_t lastUpdate;
	float xPos;
	float yPos;
	float xVel;
	float yVel;
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
	int16_t xStart;
	int16_t yStart;
	int16_t xEnd;
	int16_t yEnd;
	struct hard_bot_struct* next;
} HardEdge;
HardEdge* hardEdges;

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

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msg_struct);
float		getPaddleCoverage(void);
void		checkLightLevel(void);
void		sendBotPosMsg(void);
void		reducedBroadcastRnbData(void);
void		handleBotPosMsg(BotPosMsg* msg, id_t senderID);

void		updateBall(void);
void		updateColor(void);
float		getBallCoverage(void);

void		updateHardEdges(void);
void		sendBallMsg(void);
void		handleBallMsg(BallMsg* msg, uint32_t arrivalTime, id_t senderID);
void		sendPaddleMsg(void);
void		handlePaddleMsg(char flag, int16_t delta);

OtherBot*	getOtherBot(id_t id);
void		findAndRemoveOtherBot(id_t id);
void		removeOtherBot(uint8_t idx);
OtherBot*	addOtherBot(id_t id);
void		cleanOtherBot(OtherBot* other);
void		printNearBots(void);

void		addHardEdge(int16_t fromX, int16_t fromY, int16_t toX, int16_t toY);
void		cleanHardEdges(void);

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

inline static int8_t checkBallCrossedMe(void){
	return sgn(((theBall.yVel*(theBall.yPos-myPos.y-theBall.xVel) + theBall.xVel*(theBall.xPos-myPos.x+theBall.yVel))));
}


//Returns 1 if  the line segment of the edge and the line segment between the ball's current and future position intersect.
//Additionally, uses pass-by-reference to return the calculated intersect point.
inline static int8_t checkBounce(HardEdge* edge, float* xIntersect, float* yIntersect, int32_t timePassed){
	CHECK_BOUNCE_DEBUG_PRINT("Checking Bounce!\r\n");
	float x1 = theBall.xPos;
	float y1 = theBall.yPos;
	float x2 = x1 + ((((int32_t)(theBall.xVel))*timePassed)/1000.0);
	float y2 = y1 + ((((int32_t)(theBall.yVel))*timePassed)/1000.0);
	float x3 = edge->xStart;
	float y3 = edge->yStart;
	float x4 = edge->xEnd;
	float y4 = edge->yEnd;
	CHECK_BOUNCE_DEBUG_PRINT("\tBall Line Seg: (%8.2f, %8.2f)<->(%8.2f, %8.2f)\r\n", x1, y1, x2, y2);
	CHECK_BOUNCE_DEBUG_PRINT("\tEdge Line Seg: (%8.2f, %8.2f)<->(%8.2f, %8.2f)\r\n", x3, y3, x4, y4);
	float denom = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
	if(denom==0){
		*xIntersect = UNDF;
		*yIntersect = UNDF;
		return 0; //Lines are parallel.
	}
	float xIntrs = (x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4);
	xIntrs /= denom;
	float yIntrs = (x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4);
	yIntrs /= denom;
	CHECK_BOUNCE_DEBUG_PRINT("\tIntersect: (%8.2f, %8.2f)\r\n", xIntrs, yIntrs);
	
	uint8_t ballXboundsCheck = ((x1 < xIntrs) && (xIntrs <= x2)) || ((xIntrs < x1) && (x2 <= xIntrs));
	uint8_t ballYboundsCheck = ((y1 < yIntrs) && (yIntrs <= y2)) || ((yIntrs < y1) && (y2 <= yIntrs));
	uint8_t edgeXboundsCheck = ((x3 < xIntrs) && (xIntrs <= x4)) || ((xIntrs < x3) && (x4 <= xIntrs));
	uint8_t edgeYboundsCheck = ((y3 < yIntrs) && (yIntrs <= y4)) || ((yIntrs < y3) && (y4 <= yIntrs));

	ballXboundsCheck = ballXboundsCheck || (x1==x2);
	ballYboundsCheck = ballYboundsCheck || (y1==y2);
	edgeXboundsCheck = edgeXboundsCheck || (x3==x4);
	edgeYboundsCheck = edgeYboundsCheck || (y3==y4);

	CHECK_BOUNCE_DEBUG_PRINT("\tBounds Checks: %hu %hu %hu %hu\r\n", ballXboundsCheck, ballYboundsCheck, edgeXboundsCheck, edgeYboundsCheck);

	if(ballXboundsCheck && ballYboundsCheck && edgeXboundsCheck && edgeYboundsCheck){
		BALL_DEBUG_PRINT("\tBoing!\r\n");
		*xIntersect = xIntrs;
		*yIntersect = yIntrs;
		return 1;
	}else{
		*xIntersect = UNDF;
		*yIntersect = UNDF;
		return 0; //Lines intersect outside of bounds.
	}
}

/*Code below from http://stackoverflow.com/questions/573084/how-to-calculate-bounce-angle */
inline static void calculateBounce(int16_t normX, int16_t normY){
	float vX = theBall.xVel;
	float vY = theBall.yVel;
	float nDotN = normX*normX + normY*normY;
	float vDotN = vX*normX + vY*normY;
	float uX = normX*vDotN/nDotN;
	float uY = normY*vDotN/nDotN;
	theBall.xVel = (vX - 2*uX);
	theBall.yVel =(vY - 2*uY);
}

static int nearBotsDistCmp(const void* a, const void* b){
	BotPos* aPos = &(((OtherBot*)a)->pos);
	BotPos* bPos = &(((OtherBot*)b)->pos);
	float aDist = hypot(aPos->y - myPos.y, aPos->x - myPos.x);
	float bDist = hypot(bPos->y - myPos.y, bPos->x - myPos.x);
	if(POS_DEFINED(aPos) && POS_DEFINED(bPos)){
		if(aDist < bDist){
			return -1;
			}else if(bDist < aDist){
			return 1;
			}else{
			return 0;
		}
		}else if(POS_DEFINED(aPos)){
		return -1;
		}else if(POS_DEFINED(bPos)){
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
	if(POS_DEFINED(aPos) && POS_DEFINED(bPos)){
		if(aBearing < bBearing){
			return -1;
			}else if(bBearing < aBearing){
			return 1;
			}else{
			return 0;
		}
		}else if(POS_DEFINED(aPos)){
		return -1;
		}else if(POS_DEFINED(bPos)){
		return 1;
		}else{
		return 0;
	}
}

inline static void killBall(void){
	setRGB(255,0,0);
	theBall.id = 0x0F;
}

static inline uint16_t getSlot(id_t id){
	switch(id){
		case 0x6C66: return 0;
		case 0x3D6C: return 1;
		case 0x9669: return 2;
		case 0xAF6A: return 3;
		case 0x1361: return 4;
		case 0x7066: return 5;
		case 0x7EDF: return 6;
		case 0x5D61: return 7;
		case 0xD2D7: return 8;
		case 0xDC62: return 9;
		case 0x9261: return 10;
		case 0xD76C: return 11;
		case 0x4E2E: return 12;
		case 0x2826: return 13;
		case 0xA250: return 14;
		case 0xD913: return 15;
		default: printf("ID Fetch Error\r\n");
	}
	return (id%(SLOTS_PER_FRAME-1));
}

void printNearBots(void);

void printOtherBot(OtherBot* bot);