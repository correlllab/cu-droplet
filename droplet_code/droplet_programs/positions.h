#pragma once

#include "droplet_init.h"

#define POS_CALC_DEBUG_MODE
#define POS_MSG_DEBUG_MODE
//#define RNB_DEBUG_MODE
#define MY_POS_DEBUG_MODE
//#define COVAR_DEBUG_MODE

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

#define SLOT_LENGTH_MS			397
#define SLOTS_PER_FRAME			36
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

///*
 //* See the top of page 16 in my notebook for basis for measCovar stuff below.
 //*/
Matrix measCovarClose  = {{100, -0.75, 0.1}, {-0.75, 0.03, 0.025}, {0.1, 0.025, 0.1}};
Matrix measCovarMed = {{450, 0.15, 0.75}, {0.15, 0.05, 0.1}, {0.75, 0.1, 0.25}};
Matrix measCovarFar = {{2000, -5, -0.1}, {-5, 0.6, 0.5}, {-0.1, 0.5, 1.0}};

//Matrix xyMeasCovarClose  = {{100, 2, 0.5}, {2, 100, 0.75}, {0.5, 0.75, 0.05}};
//Matrix xyMeasCovarMed = {{500, 100, -4}, {100, 500, -0.5}, {-4, -0.5, 0.2}};
//Matrix xyMeasCovarFar = {{8000, -1000, -0.25}, {-1000, 8000, -10}, {-0.25, -10, 1}};

#define NUM_SEEDS 4
const id_t	   SEED_IDS[NUM_SEEDS]	   = {0x6C66, 0x9669, 0x7EDF, 0x1361};
//const int16_t  SEED_X[NUM_SEEDS]   = {100, 600, 100, 600};
//const int16_t  SEED_Y[NUM_SEEDS]   = {600, 600, 100, 100};
const int16_t  SEED_X[NUM_SEEDS]   = {100, 250, 100, 250};
const int16_t  SEED_Y[NUM_SEEDS]   = {250, 250, 100, 100};

#define MIN_X 50
#define MIN_Y 50
#define MAX_X 300
#define MAX_Y 300

#define UNDF	((int16_t)0x8000)

#define BOT_MEAS_MSG_FLAG		'X'

#define POS_DEFINED(pos) ((((pos)->x)!=UNDF)&&(((pos)->y)!=UNDF)&&(((pos)->o)!=UNDF))

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
	char flag;
}BotMeasMsg;

typedef struct bot_meas_msg_node_struct{
	BotMeasMsg msg;
	id_t tgt;
	uint16_t range;
	uint8_t dirMask;
	uint8_t numTries;
}BotMeasMsgNode;

typedef struct bot_meas_struct{
	id_t id;
	uint16_t r;
	int16_t b;
	int16_t h;
} BotMeas;

BotPos myPos;
DensePosCovar myPosCovar;

uint8_t		seedFlag;
uint8_t		myState;
uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	loopID;
int16_t		xRange;
int16_t		yRange;
int16_t		maxRange;

void		init();
void		loop();

float		chooseOmega(Matrix* myPinv, Matrix* yourPinv);
void		covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
void		covarUnion(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
void		updatePos(BotPos* pos, Matrix* yourP);
void		processMeasurement(BotMeas* rawMeas);
void		getMeasCovar(Matrix* R, Vector* meas);
void		calcRelativePose(Vector* pose, Vector* meas);
void		populateGammaMatrix(Matrix* G, Vector* pos);
void		populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you);
void		compressP(Matrix* P, DensePosCovar* covar);
void		decompressP(Matrix* P, DensePosCovar* covar);

void		useNewRnbMeas(uint16_t id, uint16_t range, int16_t bearing, int16_t heading);

void		prepBotMeasMsg(BotPos* pos, BotMeas* meas, DensePosCovar* covar);
void		handleBotMeasMsg(BotMeasMsg* msg, id_t senderID);
void		sendBotMeasMsg(BotMeasMsgNode* mNode);

void		updateColor();
void		handle_msg			(ir_msg* msg_struct);

void		printPosCovar(DensePosCovar* P);
void		frameEndPrintout();

inline static void copyBotPos(BotPos* src, BotPos* dest){
	dest->x = src->x;
	dest->y = src->y;
	dest->o = src->o;
}

inline static uint8_t dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

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
			break;
		}
	}
}

static inline uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}

static float discreteTriangularPDF(float x, uint8_t max, uint16_t r){
	float c = ((r-50.0)/150.0)*max;
	float firstTerm;
	float xPlus = x+0.5;
	if(xPlus<=c){
		firstTerm = powf(xPlus,2)/(c*max);
	}else if( (xPlus<=max) && (c<xPlus) ){
		firstTerm = 1 - powf(xPlus-max,2)/(max*(max-c));
	}else if(xPlus > max){
		firstTerm = 1;
	}else{
		firstTerm = 0;
	}
	float secondTerm;
	float xMinus = x-0.5;
	if((0 < xMinus) && (xMinus<=c)){
		secondTerm = powf(xMinus,2)/(c*max);
	}else if( xMinus > c ){
		secondTerm = 1 - powf(max-xMinus,2)/(max*(max-c));
	}else{
		secondTerm = 0;
	}
	return firstTerm - secondTerm;
}

/*
 * This function computes the exponential-backoff time for CSMA.
 *
 * However, while a standard implementation would choose uniformly between 0 and randMax*16,
 * this implementation weights the random choice so that measurements of a smaller radius
 * are more likely to choose lower slots.
 */
static uint32_t getBackoffTime(uint8_t N, uint16_t r){
	uint8_t randMax = (1<<N) - 1;
	float totalValue = 0;
	float chooser = rand_real();
	//printf("Discrete Triangular:\r\n");
	for(uint8_t i=0;i<randMax;i++){
		totalValue += discreteTriangularPDF(i, randMax, r);
		//printf("\t%f\r\n", totalValue);
		if(chooser<=totalValue){
			return ((uint32_t)i)*16;
		}
	}
	return randMax*16;
}