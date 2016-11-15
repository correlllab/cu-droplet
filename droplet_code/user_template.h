#pragma once

#include "droplet_init.h"

#define SLOT_LENGTH_MS			173
#define SLOTS_PER_FRAME			43
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	loopID;
uint16_t	mySlot;

void		init();
void		loop();
void		handle_msg(ir_msg* msg_struct);