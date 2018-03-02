#include "button_press_queue.h"

static inline uint8_t sameEvent(GenericEvent a, GenericEvent b){
	EitherEvent* evtA = a.both;
	EitherEvent* evtB = b.both;
	uint8_t concurrent			= (evtA->time - evtB->time) < MIN_MULTIPRESS_DELAY;
	uint8_t sameIDorDelta		= evtA->idOrDelta == evtB->idOrDelta;
	uint8_t sameButtonOrMarker	= evtA->buttonOrMarker == evtB->buttonOrMarker;
	return concurrent && sameIDorDelta && sameButtonOrMarker;
}

static inline uint8_t isEventInit(GenericEvent e){
	EitherEvent* evt = e.both;
	return !((evt->buttonOrMarker == BUTTON_UNKNOWN) && (evt->idOrDelta == 0xFFFF));
}

void queueInit(){
	for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
		eventLog[i].time = 0;
		eventLog[i].idOrDelta = 0xFFFF;
		eventLog[i].buttonOrMarker = BUTTON_UNKNOWN;
	}
}

//Returns '1' if an event was added (meaning that the Droplet should follow up)
//Returns '0' if an event wasn't added (meaning that the Droplet shouldn't follow up)
uint8_t  addEvent(GenericEvent evt){
	uint8_t openIdx = 0xFF;
	uint32_t smallestEventTime = 0xFFFFFFFF;
	uint8_t smallestEventIdx = 0xFF;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
			EitherEvent* thisEvt = &(eventLog[i]);
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
			eventLog[openIdx] = *(evt.both);
			return 1;
		}else if(smallestEventIdx != 0xFF){ //otherwise, put the new event over the oldest existing event.
			eventLog[smallestEventIdx] = *(evt.both);
			return 1;
		}
	}
	printf("Event log error!\r\n");
	printEventLog();
	return 0;
}

void printEventLog(){
	printf("Keypress Log:\r\n");
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){	
		for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
			GenericEvent thisEvt;
			thisEvt.both = &(eventLog[i]);
			if(isEventInit(thisEvt)){		
				if(eventLog[i].buttonOrMarker==0xFF){ //This is a mouse move event.
					MouseMoveEvent* evt = thisEvt.mouseMove;
					printf("\t(%hu) % 4hd, % 4hd @ %lu\r\n", i, evt->deltaX, evt->deltaY, evt->time);
				}else{ //This is a button press event.
					ButtonPressEvent* evt = thisEvt.buttonPress;
					printf("\t(%hu) %04X: '", i, evt->src);
					printf((isprint(evt->button) ? "   %c" : "\\%3hu"), evt->button);
					printf("' @ %lu\r\n", evt->time);
				}
			}
		}
	}
	printf("\r\n");
}