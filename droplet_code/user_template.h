#pragma once

#include "droplet_init.h"

//#define RNB_DEBUG_MODE
#ifdef RNB_DEBUG_MODE
#define RNB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define RNB_DEBUG_PRINT(format, ...)
#endif

#define POS_MSG_FLAG		'P'

#define SLOT_LENGTH_MS			433
#define SLOTS_PER_FRAME			46
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

typedef struct bot_pos_msg_struct{
	BotPos pos; //6 bytes
	id_t id; //2 bytes
	char flag;
}PosMsg;
#define IS_POS_MSG(msgStruct) (((PosMsg*)(msgStruct->msg))->flag==POS_MSG_FLAG && msgStruct->length==sizeof(PosMsg))

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	loopID;

uint32_t lastFrameSeen[93];
uint32_t largestFrameGap;

void sendPosMsg(id_t id, BotPos pos);
void handlePosMsg(PosMsg* msg);
uint8_t isBlinking;


typedef struct color_field_struct{
	uint16_t r:5;
	uint16_t g:5;
	uint16_t b:5;
	uint16_t x:1;
}ColorF;

typedef union color_field_union{
	uint16_t val;
	ColorF   field;
}ColorField;


void restoreLED(ColorField colors){
	if(colors.field.x){
		setRGB(0,0,0);
	}else{
		setRedLED((colors.field.r)<<3);
		setGreenLED((colors.field.g)<<3);
		setBlueLED((colors.field.b)<<3);
	}
	isBlinking = 0;
}

//colorSelect 0: red, 1: green, 2: blue
inline void blinkLED(uint8_t rVal, uint8_t gVal, uint8_t bVal, uint32_t dur){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		rVal = ((rVal>>3)<<3);
		gVal = ((gVal>>3)<<3);
		bVal = ((bVal>>3)<<3);
		if(!(rVal || gVal || bVal)){
			return;
		}
		ColorField curColors;
		curColors.field.r = getRedLED()>>3;
		curColors.field.g = getGreenLED()>>3;
		curColors.field.b = getBlueLED()>>3;
		curColors.field.x = isBlinking;
		if(rVal) setRedLED(rVal);
		if(gVal) setGreenLED(gVal);
		if(bVal) setBlueLED(bVal);
		scheduleTask(dur, (arg_func_t)restoreLED, (void*)(curColors.val));
		isBlinking = 1;
	}
}

static inline uint16_t getSlot(id_t id){
	//return (id%(SLOTS_PER_FRAME-1));
	return ((getDropletOrd(id)+1)/2)-1;
}

static inline uint32_t getExponentialBackoff(uint8_t c){
	uint32_t k;
	uint32_t N;
	
	N= (((uint32_t)1)<<c);
	
	k = randQuad()%N;
	return ((k*16)+IR_MSG_TIMEOUT);
}