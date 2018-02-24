#include "button_press_queue.h"

static inline uint8_t sameEvent(ButtonPressEvent* evtA, ButtonPressEvent* evtB){
	return ((evtA->src == evtB->src) && (evtA->key == evtB->key) && ((evtA->time - evtB->time) < MIN_MULTIPRESS_DELAY));
}

static inline uint8_t isEventInit(ButtonPressEvent* evt){
	return !((evt->key == BUTTON_UNKNOWN) && (evt->src == 0xFFFF));
}

void queueInit(){
	for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
		keypressLog[i].time = 0;
		keypressLog[i].src = 0xFFFF;
		keypressLog[i].key = BUTTON_UNKNOWN;
	}
}

//Returns '1' if an event was added (meaning that the Droplet should follow up)
//Returns '0' if an event wasn't added (meaning that the Droplet shouldn't follow up)
uint8_t  addEvent(ButtonPressEvent* evt){
	uint8_t openIdx = 0xFF;
	uint32_t smallestEventTime = 0xFFFFFFFF;
	uint8_t smallestEventIdx = 0xFF;
	for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
		ButtonPressEvent* thisEvt = &(keypressLog[i]);
		if(!isEventInit(thisEvt)){ //This spot is available.
			openIdx = i;
		}else if(sameEvent(evt, thisEvt)){
			return 0;	
		}else if(thisEvt->time < smallestEventTime){
				smallestEventTime = thisEvt->time;
				smallestEventIdx = i;
		}	
	}
	//if we got here, then this new event isn't already queued.
	if(openIdx != 0xFF){ //This means that our array still has an open slot, so we can put the new event here.
		keypressLog[openIdx] = *evt;
		return 1;
	}else if(smallestEventIdx != 0xFF){ //otherwise, put the new event over the oldest existing event.
		keypressLog[smallestEventIdx] = *evt;
		return 1;
	}else{
		printf("Event log error!\r\n");
		printKeypressLog();
		return 0;
	}
}

void printKeypressLog(){
	printf("Keypress Log:\r\n");
	for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
		ButtonPressEvent* thisEvt = &(keypressLog[i]);
		if(isEventInit(thisEvt)){
			printf("\t(%hu) %04X: '", i, thisEvt->src);
			printf((isprint(thisEvt->key) ? "   %c" : "\\%3hu"), thisEvt->key);
			printf("' @ %lu\r\n", thisEvt->time);			
		}
	}
	printf("\r\n");
}