#pragma once
#include "droplet_base.h"
#include "button.h"

#define NUM_LOGGED_EVENTS		  10
#define MIN_MULTIPRESS_DELAY	 150
#define EVENT_MAX_ADD_AGE		2000
#define EVENT_MAX_TRACK_AGE		5000

volatile ButtonPressEvent keypressLog[NUM_LOGGED_EVENTS];

void queueInit(void);
uint8_t addEvent(ButtonPressEvent* keypress);
void printKeypressLog(void);