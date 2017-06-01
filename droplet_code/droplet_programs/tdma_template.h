#pragma once

#include "droplet_init.h"

/*
 * SLOT_LENGTH_MS should be long enough for all of the communicating
 * and rnb_broadcasting the Droplets need to do in their slots.
 */
#define SLOT_LENGTH_MS			331
/*
 * This should be big enough for your density of robots that
 * the probability of two adjacent robots having the same slot
 * is very low.
 */
#define SLOTS_PER_FRAME			21 
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			10 //This number should stay small, but above 0.

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint16_t	loopID;

void		init(void);
void		loop(void);

void		handle_msg			(ir_msg* msg_struct);
