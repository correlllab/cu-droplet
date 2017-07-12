#pragma once

#include "droplet_init.h"

/*
 * GRID_SLOT_LENGTH_MS should be long enough for all of the communicating
 * and rnb_broadcasting the Droplets need to do in their slots.
 */
#define GRID_SLOT_LENGTH_MS			331
/*
 * This should be big enough for your density of robots that
 * the probability of two adjacent robots having the same slot
 * is very low.
 */
#define GRID_SLOTS_PER_FRAME			21 
#define GRID_FRAME_LENGTH_MS			(((uint32_t)GRID_SLOT_LENGTH_MS)*((uint32_t)GRID_SLOTS_PER_FRAME))
#define GRID_LOOP_DELAY_MS			10 //This number should stay small, but above 0.

uint32_t	grid_frameCount;
uint32_t	grid_frameStart;
uint16_t	grid_mySlot;
uint16_t	grid_loopID;

uint8_t dist;
uint8_t margin;

typedef struct cell_struct
{
	id_t ID;			// ID of the Droplet representing this cell
}cell;

cell neighbors[8];

void initialize_grid(uint8_t nb_columns, uint8_t nb_rows);
void sendPosMsg(void);
void print_neighbors(void);
