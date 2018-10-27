#pragma once

#include "droplet_init.h"
#include "matrix_utils.h"
#include "role_handler.h"
#include "nec_ir.h"

#define ANTI_FLOOD_DELAY		350
#define  TIMEOUT_THRESHOLD		2000
#define ANTI_DOUBLETAP_DELAY	50

#define SLOT_LENGTH_MS			397
#define SLOTS_PER_FRAME			38
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			11

#define BUTTON_PRESS_MSG_FLAG 'B'
typedef struct button_press_msg_struct{
	id_t		src;
	Role		pressType;
	uint8_t		hopLife;
	uint8_t		flag;
}ButtonPressMsg;
#define IS_BUTTON_PRESS_MSG(msgStruct) ( (msgStruct->length==sizeof(ButtonPressMsg)) && (((ButtonPressMsg*)(msgStruct->msg))->flag==BUTTON_PRESS_MSG_FLAG) )

void init(void);
void loop(void);
void handleMsg( __attribute__ ((unused)) irMsg* msg_struct);
void handleMeas( __attribute__ ((unused)) Rnb* meas);
void getMyColor(void);

uint32_t	frameStart;
uint32_t	lastTapDetectedTime;
uint32_t	lastMsgTime;
uint16_t	mySlot;
uint16_t	loopID;
uint16_t	prevLoopID;
uint8_t		isBlinking;

static inline uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}

void restoreRedLED(uint16_t val){
	setRedLED((uint8_t)val);
	isBlinking = 0;
}
void restoreGreenLED(uint16_t val){
	setGreenLED((uint8_t)val);
	isBlinking = 0;
}
void restoreBlueLED(uint16_t val){
	setBlueLED((uint8_t)val);
	isBlinking = 0;
}

void lessGeneralCheckPosition(void);

//colorSelect 0: red, 1: green, 2: blue
inline uint8_t blinkLED(uint8_t whichColor, uint8_t val){
	uint8_t currentValue;
	switch(whichColor){
		case 0: currentValue = getRedLED(); break;
		case 1: currentValue = getGreenLED(); break;
		case 2: currentValue = getBlueLED(); break;
		default: return 0;
	}
	if(currentValue == val){
		return 0;
	}
	switch(whichColor){
		case 0: setRedLED(val);    scheduleTask(100, (arg_func_t)restoreRedLED, (void*)((uint16_t)currentValue)); break;
		case 1: setGreenLED(val);  scheduleTask(100, (arg_func_t)restoreGreenLED, (void*)((uint16_t)currentValue)); break;
		case 2: setBlueLED(val);   scheduleTask(100, (arg_func_t)restoreBlueLED, (void*)((uint16_t)currentValue)); break;
		default: return 0;
	}
	isBlinking = 1;
	return 1;
}

Region regionsList[NUM_REGIONS] = {
	{//BODY: 0
		{{0.008, 0., 0.}, {0., 0.004, 0.}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_BODY}
	},	
	{//EMITTER: 1
		{{0.008, 0., 0.}, {0., 0.016, -3.}, {0., 0., 1.}},
		//{{0.01, 0., 0.}, {0., 0.005, 0.}, {0., 0., 1.}}, //(This is the body tfm, here for testing.)
		isInUnitSquare,
		{1<<ROLE_EMTR}
	},
	{//POWER: 2
		{{0.0106667, 0., -0.166667}, {0., 0.0106667, 0.333333}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_POWER}
	},
	{//CHN_UP: 3
		{{0.0106667, 0., -0.666667}, {0., 0.016, -2.}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_CHN_UP}
	},
	{//CHN_DN: 4
		{{0.0106667, 0., -0.666667}, {0., 0.016, -1.}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_CHN_DN}
	},
	{//VOL_UP: 5
		{{0.0106667, 0., 0.333333}, {0., 0.016, -2.}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_VOL_UP}
	},
	{//VOL_DOWN: 6
		{{0.0106667, 0., 0.333333}, {0., 0.016, -1.}, {0., 0., 1.}},
		isInUnitSquare,
		{1<<ROLE_VOL_DN}
	}
};

inline uint8_t conversionFunc(int16_t angleDeg){
	return (((angleDeg+180)/60))%6;
}