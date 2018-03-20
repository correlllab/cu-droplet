#pragma once

#include "droplet_init.h"
#include "button.h"
#include "button_press_queue.h"

#define KEYPRESS_MSG_FLAG 'K'
#define MOUSE_MOVE_MSG_FLAG 'M'
#define ACK_MSG_FLAG 'A'


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
#define LOOP_DELAY_MS			11


#define MK_SLOT_LENGTH 300
#define MK_FRAME_LENGTH 3000
#define MK_SLOTS_PER_FRAME (MK_FRAME_LENGTH/MK_SLOT_LENGTH)

typedef enum droplet_role{
	UNKNOWN,
	KEYBOARD,
	MOUSE	
}DropletRole;

typedef struct ack_msg_struct{
	uint32_t time;
	uint8_t flag;
}AckMsg;
#define IS_ACK_MSG(msgStruct) ( (msgStruct->length==sizeof(AckMsg)) && (((AckMsg*)(msgStruct->msg))->flag==ACK_MSG_FLAG) )
typedef struct ack_msg_node_struct{
	AckMsg msg;
	id_t target;
	uint8_t numTries;
}AckMsgNode;

typedef struct mouse_move_msg_struct{
	uint32_t	  time;
	int8_t		deltaX;
	int8_t		deltaY;
	uint8_t	      flag;
}MouseMoveMsg;
#define IS_MOUSE_MOVE_MSG(msgStruct) ( (msgStruct->length==sizeof(MouseMoveMsg)) && (((MouseMoveMsg*)(msgStruct->msg))->flag==MOUSE_MOVE_MSG_FLAG) )
typedef struct mouse_move_msg_node_struct{
	MouseMoveMsg msg;
	uint8_t numTries;
}MouseMoveMsgNode;

typedef struct button_press_msg_struct{
	ButtonPressEvent	evt;
	uint8_t			flag;
}ButtonPressMsg;
#define IS_BUTTON_PRESS_MSG(msgStruct) ( (msgStruct->length==sizeof(ButtonPressMsg)) && (((ButtonPressMsg*)(msgStruct->msg))->flag==KEYPRESS_MSG_FLAG) )
typedef struct button_press_msg_node_struct{
	ButtonPressMsg msg;
	uint8_t numTries;
}ButtonPressMsgNode;

uint32_t	frameStart;
uint32_t	lastKeypress;
uint16_t	mySlot;
uint16_t	loopID;
uint16_t	prevLoopID;
uint8_t		isWired;
uint8_t		isShifted;

id_t		leftMouseID;
DropletRole myRole;
Button myButton;
BotPos newPos;
DensePosCovar newPosCovar;
volatile Task_t* wireSleepTask;
uint8_t periodicMouseBroadcast;
uint8_t isBlinking;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msg_struct);

uint8_t combineBotMeasEvents(uint32_t);
void prepMouseMoveMsg(MouseMoveEvent* evt);
void sendMouseMoveMsg(MouseMoveMsgNode* msgNode);
void prepButtonPressMsg(ButtonPressEvent* evt);
void sendButtonPressMsg(ButtonPressMsgNode* msgNode);
void mouseHandleBotMeasMsg(BotMeasMsg* msg);
void handleButtonPressMsg(ButtonPressMsg* msg);
void handleMouseMoveMsg(MouseMoveMsg* msg);
void checkPosition(void);
void rnbBroadcastDebugWrapper(void);
void wireSleep(void);

uint32_t getExponentialBackoff(uint8_t c);

static inline uint16_t getSlot(id_t id){
	return (id%(SLOTS_PER_FRAME-1));
}

inline void setRoleAndButton(Button button){
	if(button == BUTTON_UNKNOWN){
		myRole = UNKNOWN;	
	}else if( (button!=BUTTON_L_CLICK) && (button!=BUTTON_R_CLICK) ){
		myRole = KEYBOARD;
	}else{
		myRole = MOUSE;
	}
	myButton = button;
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

inline uint8_t rebroadcastButton(Button key){
	if(key == BUTTON_SHIFT){
		return 0;
	}else{
		return 1;
	}
}

inline void wireTxButtonPress(Button key){
	blinkLED(0,200);
	printf("ButtonPress ");
	printf(isprint(key) ? "   '%c'\r\n" : "'\\%03hu'\r\n", key);
}

inline void wireMouseMove(int8_t deltaX, int8_t deltaY){
	blinkLED(0,200);
	blinkLED(1,200);
	printf("MouseMove % 4hd % 4hd\r\n", deltaX, deltaY);
}

inline void buildButtonPressEvent(ButtonPressEvent* evt){
	printf("PRESSED: ");
	printf(isprint(myButton) ? "   '%c'\r\n" : "'\\%03hu'\r\n", myButton);
	evt->time = getTime();
	evt->button = ( !isShifted && isupper(myButton) ) ? (myButton+32) : myButton; //convert to lowercase if appropriate.
	evt->src = getDropletID();
}

inline uint8_t handleShiftPressed(void){
	printf("KeyboardShift ");
	if(isShifted){
		printf("Off\r\n");
		setGreenLED(0);
		return 0;
	}else{
		printf("On\r\n");
		setGreenLED(5);
		return 1;
	}
}