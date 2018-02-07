#pragma once

typedef enum keyboard_key{
	KEYBOARD_UNKNOWN,
	KEYBOARD_ESCAPE,
	KEYBOARD_TILDA,
	KEYBOARD_ONE,
	KEYBOARD_TWO,
	KEYBOARD_THREE,
	KEYBOARD_FOUR,
	KEYBOARD_FIVE,
	KEYBOARD_SIX,
	KEYBOARD_SEVEN,
	KEYBOARD_EIGHT,
	KEYBOARD_NINE,
	KEYBOARD_ZERO,
	KEYBOARD_DASH,
	KEYBOARD_EQUALS,
	KEYBOARD_BACKSPACE,
	KEYBOARD_TAB,
	KEYBOARD_Q,
	KEYBOARD_W,
	KEYBOARD_E,
	KEYBOARD_R,
	KEYBOARD_T,
	KEYBOARD_Y,
	KEYBOARD_U,
	KEYBOARD_I,
	KEYBOARD_O,
	KEYBOARD_P,
	KEYBOARD_L_BRACKET,
	KEYBOARD_R_BRACKET,
	KEYBOARD_BCKSLASH,
	KEYBOARD_CAPSLOCK,
	KEYBOARD_A,
	KEYBOARD_S,
	KEYBOARD_D,
	KEYBOARD_F,
	KEYBOARD_G,
	KEYBOARD_H,
	KEYBOARD_J,
	KEYBOARD_K,
	KEYBOARD_L,
	KEYBOARD_COLON,
	KEYBOARD_QUOTE,
	KEYBOARD_ENTER,	
	KEYBOARD_L_SHIFT,
	KEYBOARD_Z,
	KEYBOARD_X,
	KEYBOARD_C,
	KEYBOARD_V,
	KEYBOARD_B,
	KEYBOARD_N,
	KEYBOARD_M,
	KEYBOARD_COMMA,
	KEYBOARD_PERIOD,
	KEYBOARD_FWDSLASH,
	KEYBOARD_R_SHIFT,
	KEYBOARD_L_CTRL,
	KEYBOARD_L_OS,
	KEYBOARD_L_ALT,
	KEYBOARD_SPACE,
	KEYBOARD_R_ALT,
	KEYBOARD_R_OS,
	KEYBOARD_R_CTRL,
	KEYBOARD_UP,
	KEYBOARD_LEFT,
	KEYBOARD_DOWN,
	KEYBOARD_RIGHT
}KeyboardKey;

inline KeyboardKey getKey_NUMBERROW(int16_t x){
	uint8_t index = x/45; //KEY_WIDTH
	switch(index){
		case  0: return KEYBOARD_TILDA;
		case  1: return KEYBOARD_ONE;
		case  2: return KEYBOARD_TWO;
		case  3: return KEYBOARD_THREE;
		case  4: return KEYBOARD_FOUR;
		case  5: return KEYBOARD_FIVE;
		case  6: return KEYBOARD_SIX;
		case  7: return KEYBOARD_SEVEN;
		case  8: return KEYBOARD_EIGHT;
		case  9: return KEYBOARD_NINE;
		case 10: return KEYBOARD_ZERO;
		case 11: return KEYBOARD_DASH;
		case 12: return KEYBOARD_EQUALS;
		case 13:
		case 14: return KEYBOARD_BACKSPACE;	
		default: return KEYBOARD_UNKNOWN;	
	}
}

inline KeyboardKey getKey_UPPERROW(int16_t x){
	if(x<68) return KEYBOARD_TAB; //1.5*KEY_WIDTH
	uint8_t index = (x-68)/45; //KEY_WIDTH
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
		case 10: return KEYBOARD_L_BRACKET;
		case 11: return KEYBOARD_R_BRACKET;
		case 12: return KEYBOARD_BCKSLASH;
		default: return KEYBOARD_UNKNOWN;			
	}
}

inline KeyboardKey getKey_HOMEROW(int16_t x){
	if(x<79) return KEYBOARD_CAPSLOCK; //1.75*KEY_WIDTH
	uint8_t index = (x-79)/45; //KEY_WIDTH
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
		case  9: return KEYBOARD_COLON;
		case 10: return KEYBOARD_QUOTE;
		case 11:
		case 12: return KEYBOARD_SPACE;
		default: return KEYBOARD_UNKNOWN;	
	}
}

inline KeyboardKey getKey_LOWERROW(int16_t x){
	
	if(x<102) return KEYBOARD_L_SHIFT; //2.25*KEY_WIDTH
	uint8_t index = (x-102)/45; //KEY_WIDTH
	switch(index){
		case  0: return KEYBOARD_Z;
		case  1: return KEYBOARD_X;
		case  2: return KEYBOARD_C;
		case  3: return KEYBOARD_V;
		case  4: return KEYBOARD_B;
		case  5: return KEYBOARD_N;
		case  6: return KEYBOARD_M;
		case  7: return KEYBOARD_COMMA;
		case  8: return KEYBOARD_PERIOD;
		case  9: return KEYBOARD_FWDSLASH;
		case 10: 
		case 11: return KEYBOARD_R_SHIFT;
		default: return KEYBOARD_UNKNOWN;
	}
}

inline KeyboardKey getKey_SPACEROW(int16_t x){
	if(x<169){ //3.75*KEY_WIDTH
		uint8_t index = x/56; //1.25*KEY_WIDTH
		switch(index){
			case  0: return KEYBOARD_L_CTRL;
			case  1: return KEYBOARD_L_OS;
			case  2: return KEYBOARD_L_ALT;
			default: return KEYBOARD_UNKNOWN;
		}
	}else if(x<484){ //(3.75+7)*KEY_WIDTH
		return KEYBOARD_SPACE;
	}else{
		uint8_t index = (x-485)/56; //1.25*KEY_WIDTH (-1, to prevent weirdness when x==652)
		switch(index){
			case  0: return KEYBOARD_R_ALT;
			case  1: return KEYBOARD_R_OS;
			case  2: return KEYBOARD_R_CTRL;
			default: return KEYBOARD_UNKNOWN;
		}
	}
}

inline KeyboardKey getKeyFromPosition(BotPos* pos){
	if( !POS_DEFINED(pos) ){
		return KEYBOARD_UNKNOWN;
	}
	//Line below has 14.5*KEY_WIDTH and 5*KEY_WIDTH
	if( (pos->x > 652) || (pos->y > 225) ){
		return KEYBOARD_UNKNOWN;
	}
	uint8_t rowIndex = ((pos->y - 1)/45); //KEY_WIDTH (The '-1' prevents weirdness when pos->y==225).
	switch(rowIndex){
		case 0: return getKey_SPACEROW(pos->x);
		case 1: return getKey_LOWERROW(pos->x);
		case 2: return getKey_HOMEROW(pos->x);
		case 3: return getKey_UPPERROW(pos->x);
		case 4: return getKey_NUMBERROW(pos->x);
		default: return KEYBOARD_UNKNOWN; 
	}
}
