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

void checkPosition(void);

void sendKeypressMsg(void);
void repeatKeypressMsg(KeypressMsg* msg);
void wireSleep(void);