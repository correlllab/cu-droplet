#pragma once

#include "droplet_init.h"
//
//#define OCC_DEBUG_MODE
//#define POS_CALC_DEBUG_MODE
//#define POS_MSG_DEBUG_MODE
#define GEN_DEBUG_MODE
//#define RNB_DEBUG_MODE
#define MY_POS_DEBUG_MODE

#ifdef OCC_DEBUG_MODE
#define OCC_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define OCC_DEBUG_PRINT(format, ...)
#endif

#ifdef POS_CALC_DEBUG_MODE
#define POS_CALC_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define POS_CALC_DEBUG_PRINT(format, ...)
#endif

#ifdef POS_MSG_DEBUG_MODE
#define POS_MSG_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define POS_MSG_DEBUG_PRINT(format, ...)
#endif

#ifdef GEN_DEBUG_MODE
#define GEN_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define GEN_DEBUG_PRINT(format, ...)
#endif

#ifdef RNB_DEBUG_MODE
#define RNB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define RNB_DEBUG_PRINT(format, ...)
#endif

#ifdef MY_POS_DEBUG_MODE
#define MY_POS_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define MY_POS_DEBUG_PRINT(format, ...)
#endif

/*
 * rnb takes 188 ms
 * messages take 2.5ms per byte. 
 * paddleMsg is 3 bytes. header is 8 bytes, so PaddleMsg should take 27.5
 * ballMsg is 7 bytes. header is 8 bytes, so ballMsg should take 37.5ms 
 * BotMeasMsg is 20 bytes. Header is 8 bytes, so msg should take 70ms (tripled)
 * BotPosMsg is 7 bytes. Header is 8 bytes, so msg should take 37.5ms
 */
#define RNB_DUR				220
#define PADDING_DUR			20 //padding.
#define POS_MSG_DUR			45
#define MEAS_MSG_DUR		72

#define SLOT_LENGTH_MS			499
#define SLOTS_PER_FRAME			28
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

///*
 //* See the top of page 16 in my notebook for basis for measCovar stuff below.
 //*/
Matrix measCovarClose  = {{94.3976, -0.688106, 0.1434}, {-0.688106, 0.0329521, 0.0233468}, {0.1434, 0.0233468, 0.0615333}};
Matrix measCovarMed = {{430.31, 0.132676, 0.639717}, {0.132676, 0.0539288, 0.0777032}, {0.639717, 0.0777032, 0.214657}};
Matrix measCovarFar = {{1888.16, -4.29818, -0.0605562}, {-4.29818, 0.593595, 0.443393}, {-0.0605562, 0.443393, 1.04668}};
//Matrix deltaPosCovarClose  = {{388.428, 56.7799, -1.42906}, {56.7799, 198.959, 0.28115}, {-1.42906,
//0.28115, 0.125886}};
//Matrix deltaPosCovarMed = {{4819.52, 173.129, -2.82006}, {173.129, 1767.47, 1.98938}, {-2.82006,
//1.98938, 0.575794}};
//Matrix deltaPosCovarFar = {{14864.9, -3475.5, -12.4073}, {-3475.5,
//17978.1, -27.1459}, {-12.4073, -27.1459, 1.65012}};

#define NUM_SEEDS 4
#define NUM_SHARED_BOTS 6
#define NUM_TRACKED_BOTS 12
const id_t	   SEED_IDS[NUM_SEEDS]	   = {0x6C66, 0x9669, 0x7EDF, 0x1361};
//const int16_t  SEED_X[NUM_SEEDS]   = {100, 600, 100, 600};
//const int16_t  SEED_Y[NUM_SEEDS]   = {600, 600, 100, 100};
const int16_t  SEED_X[NUM_SEEDS]   = {100, 600, 100, 600};
const int16_t  SEED_Y[NUM_SEEDS]   = {600, 600, 100, 100};
#define MIN_X 0
#define MIN_Y 0
#define MAX_X 700
#define MAX_Y 700

#define UNDF	((int16_t)0x8000)

#define DROPLET_DIAMETER_MM		44.4
#define BOT_MEAS_MSG_FLAG		'X'
#define BOT_POS_MSG_FLAG		'P'

#define POS_DEFINED(pos) ((((pos)->x)!=UNDF)&&(((pos)->y)!=UNDF)&&(((pos)->o)!=UNDF))

typedef struct ball_msg_struct{
	char flag;
	uint8_t xPos;
	uint8_t yPos;
	uint8_t extraBits; //bits 7-5 are three high bits for xPos; bits 4-2 are three high bits for yPos; bits 0-1 are two low bits for id.
	int8_t xVel;
	int8_t yVel;
	uint8_t radius; //bits 0-1 are two high bits for id. rest is radius (which must be divisible by 4)
}BallMsg;

typedef union flex_byte_union{
	uint16_t u;
	int16_t d;
}FlexByte;

typedef FlexByte DensePosCovar[6];

typedef struct bot_pos_struct{
	int16_t x;
	int16_t y;
	int16_t o;
} BotPos;

typedef struct bot_meas_msg_struct{
	BotPos pos; //6 bytes
	DensePosCovar covar; //12 bytes
	//uint8_t seedIdx;
	char flag;
}BotMeasMsg;

typedef struct bot_pos_msg_struct{
	BotPos pos; //6 bytes
	char flag;
}BotPosMsg;

typedef struct bot_meas_struct{
	id_t id;
	uint16_t r;
	int16_t b;
	int16_t h;
} BotMeas;

typedef struct other_bot_rnb_struct{
	BotMeas myMeas;
	BotPos posFromMe;
	DensePosCovar posCovar;
	uint8_t occluded;
	//uint8_t seedIdx;
	//uint8_t hasNewInfo;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS+1];

BotPos myPos;
DensePosCovar myPosCovar;
Vector eigValMax;
//BotPos perSeedPos[NUM_SEEDS];
//DensePosCovar perSeedCovars[NUM_SEEDS];
//BotMeasMsg preppedMsg;
//id_t preppedMsgTarget;
//uint8_t preppedMsgDirMask;
//uint16_t msgExtraDelay;

uint8_t		seedFlag;
uint8_t		myState;
uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	loopID;
int16_t		xRange;
int16_t		yRange;
int16_t		maxRange;

uint8_t targetBotIdxs[2] = {0xFF, 0xFF};
uint8_t slotTasksSwitch;
#define SLOT_TASK_RNB_bm		0x01
#define SLOT_TASK_POS_MSG_bm	0x02
#define SLOT_TASK_MEAS_MSG0_bm  0x04
#define SLOT_TASK_MEAS_MSG1_bm	0x08

void		init();
void		loop();
void		handleMySlot(uint32_t frameTime);
void		handleFrameEnd(); 


void		getWeightedRandOtherBots(uint8_t* botIdxs, uint8_t n);

uint8_t     nearBotUseabilityCheck(uint8_t i);
float		chooseOmega(Matrix* myPinv, Matrix* yourPinv);
void		covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
void		covarUnion(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
void		updatePos(BotPos* pos, Matrix* yourP);
void		fusePerSeedMeas();
void		updatePositions();
void		processMeasurement(uint8_t botIdx, Matrix* myP);
void		getMeasCovar(Matrix* R, Vector* meas);
void		calcRelativePose(Vector* pose, Vector* meas);
void		populateGammaMatrix(Matrix* G, Vector* pos);
void		populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you);
void		compressP(Matrix* P, DensePosCovar* covar);
void		decompressP(Matrix* P, DensePosCovar* covar);

void		useNewRnbMeas(uint16_t id, uint16_t range, int16_t bearing, int16_t heading);

void		sendBotPosMsg();
void		sendBotMeasMsg(uint8_t i);
void		handleBotMeasMsg(BotMeasMsg* msg, id_t senderID);
void		handleBotPosMsg(BotPosMsg* msg, id_t senderID);

void		updateColor();
//Coverage functions below are helper functions for 'updateColor'.
void		updateNearBotOcclusions();
void		handle_msg			(ir_msg* msg_struct);

//void		fusePerSeedMeas();

void		printPosCovar(DensePosCovar* P);
void		printTwoPosCovar(DensePosCovar* P1, DensePosCovar* P2);
void		frameEndPrintout();

//These four functions are for interacting with the OtherBot data structure.
OtherBot*	getOtherBot(id_t id);
void		findAndRemoveOtherBot(id_t id);
void		removeOtherBot(uint8_t idx);
OtherBot*	addOtherBot(id_t id);
void		cleanOtherBot(OtherBot* other);

void printNearBots();
void printOtherBot(OtherBot* bot);

inline static void copyBotPos(BotPos* src, BotPos* dest){
	dest->x = src->x;
	dest->y = src->y;
	dest->o = src->o;
}

inline static uint8_t dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

static int nearBotsRangeCmp(uint16_t aRange, uint16_t bRange){
	if(aRange < bRange){
		return -1;
		}else if(bRange < aRange){
		return 1;
		}else{
		return 0;
	}
}

static int smallRangeSorter(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	return nearBotsRangeCmp((aN->myMeas).r, (bN->myMeas).r);
}

//uint8_t eigValToleranceQ(Vector* eigVals){
	//uint8_t retVal = 0;
	//uint8_t xO =  (*eigVals)[0] > eigValMax[0];
	//uint8_t yO = (*eigVals)[1] > eigValMax[1];
	//uint8_t oO = (*eigVals)[2] > eigValMax[2];
	//retVal = (xO+yO+oO)>=2; //in other words, tolerances are exceeded if at least two of the thresholds are esxceeded.
	//if(retVal){
		//for(uint8_t i=0;i<3;i++){
			//eigValMax[i] *= 1.5;
		//}
	//}
	//return retVal;
//}
//
//#define EIG_VAL_BASE_MAX_XY 10000
//#define EIG_VAL_BASE_MAX_O  2.4674
//
//inline void resetEigValMax(Matrix* myP){
	//Vector eigVals;
	//eigenvalues(&eigVals, myP);
	//eigValMax[0] = EIG_VAL_BASE_MAX_XY > eigVals[0] ? EIG_VAL_BASE_MAX_XY : eigVals[0];
	//eigValMax[1] = EIG_VAL_BASE_MAX_XY > eigVals[1] ? EIG_VAL_BASE_MAX_XY : eigVals[1];
	//eigValMax[2] = EIG_VAL_BASE_MAX_O  > eigVals[2] ? EIG_VAL_BASE_MAX_O  : eigVals[2];
//}

inline static void initPositions(){
	myPos.x = UNDF;
	myPos.y = UNDF;
	myPos.o = UNDF;
	for(uint8_t i=0;i<6;i++){
		myPosCovar[i].u = 0;
	}
	seedFlag = 0;	
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(get_droplet_id()==SEED_IDS[i]){
			seedFlag = 1;
			myPos.x = SEED_X[i];
			myPos.y = SEED_Y[i];
			myPos.o = 0;
			myPosCovar[0].u = 1; //the actual value used will be this*8
			myPosCovar[3].u = 1; //the actual value used will be this*8
			myPosCovar[5].u = 16; //the actual value used will be this/256
			//eigValMax[0] = 64;
			//eigValMax[1] = 64;
			//eigValMax[2] = 0.00390625;
			break;
		}
	}
}

inline static uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}