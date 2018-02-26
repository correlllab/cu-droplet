#pragma once
#include "localization.h"

#define LARGEST_KEYBOARD_KEY 96

typedef enum keyboard_key{
	BUTTON_UNKNOWN		=  0,
	BUTTON_ESCAPE		= 27,
	BUTTON_TILDA		= 96,
	BUTTON_ONE			= 49,
	BUTTON_TWO			= 50,
	BUTTON_THREE		= 51,
	BUTTON_FOUR			= 52,
	BUTTON_FIVE			= 53,
	BUTTON_SIX			= 54,
	BUTTON_SEVEN		= 55,
	BUTTON_EIGHT		= 56,
	BUTTON_NINE			= 57,
	BUTTON_ZERO			= 48,
	BUTTON_MINUS		= 45,
	BUTTON_EQUALS		= 61,
	BUTTON_BACKSPACE	=  8,
	BUTTON_TAB			=  9,
	BUTTON_Q		    = 81,
	BUTTON_W		    = 87,
	BUTTON_E		    = 69,
	BUTTON_R		    = 82,
	BUTTON_T		    = 84,
	BUTTON_Y		    = 89,
	BUTTON_U		    = 85,
	BUTTON_I		    = 73,
	BUTTON_O		    = 79,
	BUTTON_P		    = 80,
	BUTTON_L_BRACKET	= 91,
	BUTTON_R_BRACKET	= 93,
	BUTTON_BACKSLASH	= 92,
	BUTTON_CAPSLOCK		= 20,
	BUTTON_A		    = 65,
	BUTTON_S		    = 83,
	BUTTON_D		    = 68,
	BUTTON_F		    = 70,
	BUTTON_G		    = 71,
	BUTTON_H		    = 72,
	BUTTON_J		    = 74,
	BUTTON_K		    = 75,
	BUTTON_L		    = 76,
	BUTTON_COLON		= 58,
	BUTTON_QUOTE		= 34,
	BUTTON_ENTER		= 13,
	BUTTON_Z		    = 90,
	BUTTON_X		    = 88,
	BUTTON_C		    = 67,
	BUTTON_V		    = 86,
	BUTTON_B		    = 66,
	BUTTON_N		    = 78,
	BUTTON_M		    = 77,
	BUTTON_COMMA		= 44,
	BUTTON_PERIOD		= 46,
	BUTTON_FWDSLASH		= 47,
	BUTTON_SHIFT		= 16,
	BUTTON_CTRL			= 17,
	BUTTON_OS			= 18,
	BUTTON_ALT			= 19,
	BUTTON_SPACE		= 32,
	BUTTON_UP		    = 38,
	BUTTON_LEFT			= 37,
	BUTTON_DOWN			= 40,
	BUTTON_RIGHT		= 39,
	BUTTON_L_CLICK		= 60,
	BUTTON_R_CLICK		= 62
}Button;

typedef struct keypress_event_struct{
	uint32_t	time;
	id_t		src;
	Button		key;
}ButtonPressEvent;

typedef struct keypress_msg_struct{
	ButtonPressEvent	evt;
	uint8_t			flag;
}ButtonPressMsg;

#define IS_BUTTON_PRESS_MSG(msgStruct) ( (msgStruct->length==sizeof(ButtonPressMsg)) && (((ButtonPressMsg*)(msgStruct->msg))->flag==KEYPRESS_MSG_FLAG) )

#define BUTTON_WIDTH 50
#define BUTTON_HALFWIDTH (BUTTON_WIDTH/2)
#define BUTTON_HEIGHT 60

#define MOUSE_CIRCLE_X 610
#define MOUSE_CIRCLE_Y 130
#define MOUSE_CIRCLE_R (BUTTON_HEIGHT)

inline Button getButton_UPPERROW(int16_t x){
	uint8_t index = x/BUTTON_WIDTH;
	switch(index){
		case  0: return BUTTON_Q;
		case  1: return BUTTON_W;
		case  2: return BUTTON_E;
		case  3: return BUTTON_R;
		case  4: return BUTTON_T;
		case  5: return BUTTON_Y;
		case  6: return BUTTON_U;
		case  7: return BUTTON_I;
		case  8: return BUTTON_O;
		case  9: return BUTTON_P;
		case 10: return BUTTON_BACKSPACE;
		default: return BUTTON_UNKNOWN;			
	}
}

inline Button getButton_HOMEROW(int16_t x){
	uint8_t index = (x-BUTTON_HALFWIDTH)/BUTTON_WIDTH;
	switch(index){
		case  0: return BUTTON_A;
		case  1: return BUTTON_S;
		case  2: return BUTTON_D;
		case  3: return BUTTON_F;
		case  4: return BUTTON_G;
		case  5: return BUTTON_H;
		case  6: return BUTTON_J;
		case  7: return BUTTON_K;
		case  8: return BUTTON_L;
		case  9: return BUTTON_ENTER;
		default: return BUTTON_UNKNOWN;	
	}
}

inline Button getButton_LOWERROW(int16_t x){
	uint8_t index = x/BUTTON_WIDTH;
	switch(index){
		case  0: return BUTTON_SHIFT;
		case  1: return BUTTON_Z;
		case  2: return BUTTON_X;
		case  3: return BUTTON_C;
		case  4: return BUTTON_V;
		case  5: return BUTTON_B;
		case  6: return BUTTON_N;
		case  7: return BUTTON_M;
		case  8: return BUTTON_COMMA;
		case  9: return BUTTON_PERIOD;
		default: return BUTTON_UNKNOWN;
	}
}

inline Button getButton_SPACEROW(int16_t x){
	if( x>(3*BUTTON_WIDTH + BUTTON_HALFWIDTH) &&
		x<(7*BUTTON_WIDTH + BUTTON_HALFWIDTH))
	{
		return BUTTON_SPACE;			
	}else{
		return BUTTON_UNKNOWN;
	}
}

inline Button checkMouseCircle(BotPos* pos){
	int16_t xDiff = pos->x - MOUSE_CIRCLE_X;
	int16_t yDiff = pos->y - MOUSE_CIRCLE_Y;
	int32_t distSquared = xDiff*xDiff + yDiff*yDiff;
	if(distSquared > (MOUSE_CIRCLE_R*MOUSE_CIRCLE_R)){
		return BUTTON_UNKNOWN;
	}else{
		return (xDiff>0) ? BUTTON_R_CLICK : BUTTON_L_CLICK;
	}
}

inline Button getKeyFromPosition(BotPos* pos){
	uint8_t rowIndex = ((pos->y-10)/BUTTON_HEIGHT);
	switch(rowIndex){
		case 0: return getButton_SPACEROW(pos->x);
		case 1: return getButton_LOWERROW(pos->x);
		case 2: return getButton_HOMEROW(pos->x);
		case 3: return getButton_UPPERROW(pos->x);
		default: return BUTTON_UNKNOWN;
	}
}

inline Button getButtonFromPosition(BotPos* pos){
	if(POS_DEFINED(pos)){
		if(pos->x < 550){
			return getKeyFromPosition(pos);
		}else{
			return checkMouseCircle(pos);
		}
	}else{
		return BUTTON_UNKNOWN;	
	}
}