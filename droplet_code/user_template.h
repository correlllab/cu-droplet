#pragma once

#include "droplet_init.h"
#include "keyboard.h"
#include "keypress_queue.h"

#define KEYPRESS_MSG_FLAG 'K'

//#define KEYBOARD_DEBUG_MODE
#ifdef KEYBOARD_DEBUG_MODE
#define KEYBOARD_DEBUG_MODE(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define KEYBOARD_DEBUG_MODE(format, ...)
#endif

//#define RNB_DEBUG_MODE
#ifdef RNB_DEBUG_MODE
#define RNB_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define RNB_DEBUG_PRINT(format, ...)
#endif

#define SLOT_LENGTH_MS			397
#define SLOTS_PER_FRAME			38
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

typedef enum droplet_role{
	UNKNOWN,
	KEYBOARD,
	MOUSE	
}DropletRole;

uint32_t	frameCount;
uint32_t	frameStart;
uint32_t	lastKeypress;
uint16_t	mySlot;
uint16_t	loopID;
uint8_t		isWired;
uint8_t		isShifted;
DropletRole myRole;
KeyboardKey myKey;
volatile Task_t* wireSleepTask;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msg_struct);

void handleKeypressMsg(KeypressMsg* msg);
void checkPosition(void);

void wireSleep(void);

static inline uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}

inline DropletRole getRoleFromPosition(BotPos* pos){
	myKey = getKeyFromPosition(pos);
	if(myKey != KEYBOARD_UNKNOWN){
		return KEYBOARD;
	}
	if( (pos->x > 652) && (pos->y <= 225 ) && (pos->y > 0)){
		return MOUSE;
	}
	return UNKNOWN;
}

inline void wireTxKeypress(KeyboardKey key){
	setRGB(50,0,0);
	scheduleTask(150, ledOff, NULL);
	printf("KeyboardKey ");
	printf(isprint(key) ? "   '%c'\r\n" : "'\\%3hu'\r\n", key);
}

inline void sendKeypressMsg(KeypressEvent* evt){
	KeypressMsg msg;
	msg.evt = *evt;
	msg.flag = KEYPRESS_MSG_FLAG;
	irSend(ALL_DIRS, (char*)(&msg), sizeof(KeypressMsg));
}

inline void buildKeypressEvent(KeypressEvent* evt){
	printf("PRESSED: ");
	printf(isprint(myKey) ? "   '%c'\r\n" : "'\\%3hu'\r\n", myKey);
	evt->time = getTime();
	evt->key = ( isShifted && isalpha(myKey) ) ? myKey : (myKey+32); //convert to lowercase if appropriate.
	evt->src = getDropletID();
}

inline uint8_t repeatKeypressMsg(KeypressMsg* msg){
	return irSend(ALL_DIRS, (char*)msg, sizeof(KeypressMsg));
}

