#pragma once
#include "droplet_base.h"
#include "button.h"

#define NUM_LOGGED_EVENTS		  20
#define MIN_MULTIPRESS_DELAY	 150
#define MOUSE_EVENT_MARKER_FLAG	0xFF //Important that this not be a member of the Button enum.

typedef struct button_press_event_struct{
	uint32_t	time;
	id_t		src;
	Button		button;
}ButtonPressEvent;

typedef struct mouse_move_event_struct{
	uint32_t	time;
	int8_t		deltaX;
	int8_t		deltaY;
	uint8_t		mouseEventMarker;
}MouseMoveEvent;

typedef struct either_event_struct{
	uint32_t	time;
	uint16_t	idOrDelta;
	uint8_t		buttonOrMarker;
}EitherEvent;

typedef union generic_event_union{
	ButtonPressEvent	*buttonPress;
	MouseMoveEvent		*mouseMove;
	EitherEvent			*both;
}GenericEvent  __attribute__ ((__transparent_union__));

volatile EitherEvent eventLog[NUM_LOGGED_EVENTS];

void queueInit(void);
void checkForEvent(uint32_t minTime, uint32_t maxTime, GenericEvent evtPtr);
uint8_t addEvent(GenericEvent keypress);
void printEventLog(void);