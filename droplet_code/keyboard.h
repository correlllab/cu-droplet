#pragma once

typedef enum keyboard_key{
	KEYBOARD_UNKNOWN	=    0,
	KEYBOARD_ESCAPE		=   27,
	KEYBOARD_TILDA		=   96,
	KEYBOARD_ONE		=   49,
	KEYBOARD_TWO		=   50,
	KEYBOARD_THREE		=   51,
	KEYBOARD_FOUR		=   52,
	KEYBOARD_FIVE		=   53,
	KEYBOARD_SIX		=   54,
	KEYBOARD_SEVEN		=   55,
	KEYBOARD_EIGHT		=   56,
	KEYBOARD_NINE		=   57,
	KEYBOARD_ZERO		=   48,
	KEYBOARD_MINUS		=   45,
	KEYBOARD_EQUALS		=   61,
	KEYBOARD_BACKSPACE	=    8,
	KEYBOARD_TAB		=    9,
	KEYBOARD_Q		    =  113,
	KEYBOARD_W		    =  119,
	KEYBOARD_E		    =  101,
	KEYBOARD_R		    =  114,
	KEYBOARD_T		    =  116,
	KEYBOARD_Y		    =  121,
	KEYBOARD_U		    =  117,
	KEYBOARD_I		    =  105,
	KEYBOARD_O		    =  111,
	KEYBOARD_P		    =  112,
	KEYBOARD_L_BRACKET	=   91,
	KEYBOARD_R_BRACKET	=   93,
	KEYBOARD_BACKSLASH	=   92,
	KEYBOARD_CAPSLOCK	=   20,
	KEYBOARD_A		    =   97,
	KEYBOARD_S		    =  115,
	KEYBOARD_D		    =  100,
	KEYBOARD_F		    =  102,
	KEYBOARD_G		    =  103,
	KEYBOARD_H		    =  104,
	KEYBOARD_J		    =  106,
	KEYBOARD_K		    =  107,
	KEYBOARD_L		    =  108,
	KEYBOARD_COLON		=   58,
	KEYBOARD_QUOTE		=   34,
	KEYBOARD_ENTER      =   13,
	KEYBOARD_Z		    =  122,
	KEYBOARD_X		    =  120,
	KEYBOARD_C		    =   99,
	KEYBOARD_V		    =  118,
	KEYBOARD_B		    =   98,
	KEYBOARD_N		    =  110,
	KEYBOARD_M		    =  109,
	KEYBOARD_COMMA		=   44,
	KEYBOARD_PERIOD		=   46,
	KEYBOARD_FWDSLASH	=   47,
	KEYBOARD_SHIFT		=   16,
	KEYBOARD_CTRL		=   17,
	KEYBOARD_OS			=   18,
	KEYBOARD_ALT		=   19,
	KEYBOARD_SPACE		=   32,
	KEYBOARD_UP		    =   38,
	KEYBOARD_LEFT		=   37,
	KEYBOARD_DOWN		=   40,
	KEYBOARD_RIGHT      =   39
}KeyboardKey;

#define KEY_WIDTH 50
#define KEY_HALFWIDTH (KEY_WIDTH/2)

inline KeyboardKey getKey_UPPERROW(int16_t x){
	uint8_t index = x/KEY_WIDTH;
	switch(index){
		case  0: return KEYBOARD_Q;
		case  1: return KEYBOARD_W;
		case  2: return KEYBOARD_E;
		case  3: return KEYBOARD_R;
		case  4: return KEYBOARD_T;
		case  5: return KEYBOARD_Y;
		case  6: return KEYBOARD_U;
		case  7: return KEYBOARD_I;
		case  8: return KEYBOARD_O;
		case  9: return KEYBOARD_P;
		case 10: return KEYBOARD_BACKSPACE;
		default: return KEYBOARD_UNKNOWN;			
	}
}

inline KeyboardKey getKey_HOMEROW(int16_t x){
	uint8_t index = (x-KEY_HALFWIDTH)/KEY_WIDTH;
	switch(index){
		case  0: return KEYBOARD_A;
		case  1: return KEYBOARD_S;
		case  2: return KEYBOARD_D;
		case  3: return KEYBOARD_F;
		case  4: return KEYBOARD_G;
		case  5: return KEYBOARD_H;
		case  6: return KEYBOARD_J;
		case  7: return KEYBOARD_K;
		case  8: return KEYBOARD_L;
		case  9: return KEYBOARD_ENTER;
		default: return KEYBOARD_UNKNOWN;	
	}
}

inline KeyboardKey getKey_LOWERROW(int16_t x){
	uint8_t index = x/KEY_WIDTH;
	switch(index){
		case  0: return KEYBOARD_SHIFT;
		case  1: return KEYBOARD_Z;
		case  2: return KEYBOARD_X;
		case  3: return KEYBOARD_C;
		case  4: return KEYBOARD_V;
		case  5: return KEYBOARD_B;
		case  6: return KEYBOARD_N;
		case  7: return KEYBOARD_M;
		case  8: return KEYBOARD_COMMA;
		case  9: return KEYBOARD_PERIOD;
		default: return KEYBOARD_UNKNOWN;
	}
}

inline KeyboardKey getKey_SPACEROW(int16_t x){
	if( x>(3*KEY_WIDTH + KEY_HALFWIDTH) &&
		x<(7*KEY_WIDTH + KEY_HALFWIDTH))
	{
		return KEYBOARD_SPACE;			
	}else{
		return KEYBOARD_UNKNOWN;
	}
}

inline KeyboardKey getKeyFromPosition(BotPos* pos){
	if( !POS_DEFINED(pos) ){
		return KEYBOARD_UNKNOWN;
	}
	uint8_t rowIndex = ((pos->y-10)/60); //KEY_HEIGHT
	switch(rowIndex){
		case 0: return getKey_SPACEROW(pos->x);
		case 1: return getKey_LOWERROW(pos->x);
		case 2: return getKey_HOMEROW(pos->x);
		case 3: return getKey_UPPERROW(pos->x);
		default: return KEYBOARD_UNKNOWN; 
	}
}
