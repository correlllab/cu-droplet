#pragma once
#include "droplet_base.h"
#include "matrix_utils.h"
#include "range_algs.h"

#define BOT_MEAS_MSG_DUR		72
#define LOCALIZATION_DUR (RNB_DUR + BOT_MEAS_MSG_DUR*2)

//#define POS_CALC_DEBUG_MODE
//#define POS_MSG_DEBUG_MODE
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

#ifdef MY_POS_DEBUG_MODE
#define MY_POS_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define MY_POS_DEBUG_PRINT(format, ...)
#endif

#define UNDF	((int16_t)0x8000)
#define POS_DEFINED(pos) ((((pos)->x)!=UNDF)&&(((pos)->y)!=UNDF)&&(((pos)->o)!=UNDF))

#define BOT_MEAS_MSG_FLAG		'X'

typedef union flex_byte_union{
	uint16_t u;
	int16_t d;
}FlexWord;

typedef FlexWord DensePosCovar[6];

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
#define IS_BOT_MEAS_MSG(msgStruct) (((BotMeasMsg*)(msgStruct->msg))->flag==BOT_MEAS_MSG_FLAG && msgStruct->length==sizeof(BotMeasMsg))

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

BotPos        myPos;
DensePosCovar myPosCovar;
uint8_t		  seedFlag;

void	localizationInit(void);
void	useRNBmeas(Rnb* meas);
uint8_t calcOtherBotPosFromMeas(BotPos* pos, DensePosCovar* covar, Rnb* newMeas);
void	handleBotMeasMsg(BotMeasMsg* msg, id_t senderID __attribute__ ((unused)));
float	updateDistance(Vector* a, Matrix* A, Vector* b, Matrix* B);
void	covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
void	covarUnion(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);

void		compressP(Matrix* P, DensePosCovar* covar);
void		decompressP(Matrix* P, DensePosCovar* covar);

//WARNING! This function hasn't yet been implemented; it's a stub to serve as a reminder/framework for future work.
void	updateForMovement(uint8_t dir, uint16_t mag);

/*
 * r, b, h  -  Should be a range and bearing measurement this robot took.
 * pos		-  Should point to the position (in global coordinate system) of the measured robot.
 * myPos    -  The resulting position will be stored in the Vector this points to.
 */
void	relativePosition(uint16_t r, int16_t b, int16_t h, BotPos* pos, Vector* myPos);

void	getPosColor(uint8_t* r, uint8_t* g, uint8_t* b);
float	posCovarSummary(DensePosCovar* denseP);
void	printPosCovar(DensePosCovar* denseP);
void	sendBotMeasMsg(BotMeasMsgNode* mNode);

inline uint8_t dirFromAngle(int16_t angle){
	return abs((angle - (angle>0 ? 360 : 0))/60);
}

//This function converts a measurement from the measuring robot's perspective to the measured robot's perspective.
inline void convertMeas(int16_t* newB, int16_t* newH, int16_t b, int16_t h){
	*newB = prettyAngleDeg(b-h-180);
	*newH = -h;
}