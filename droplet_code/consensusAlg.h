/*
 * consensusAlg.h
 *
 * Created: 5/25/2016 9:04:54 PM
 * Author : Yang Li and Joe Jackson
 */ 

#pragma once

#include "droplet_init.h"

#define HIST_MSG_FLAG 'H'
#define NUM_BINS 6

#define NUM_DROPLETS 5
#define SLOT_LENGTH_MS 300
#define SLOTS_PER_FRAME (NUM_DROPLETS+1)
#define FRAME_LENGTH_MS (((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS   17

const uint16_t dropletsID_set[NUM_DROPLETS] =
{0xA649, 0x4DB0, 0x43BA, 0xC24B, 0x8b46};
	
typedef struct histogram_struct{
	char flag;
	uint16_t dropletID;
	uint16_t hist[6];
	uint8_t degree;
}HistMsg;

void		init();
void		loop();
void		handle_msg	(ir_msg* msg_struct);

uint32_t frameStart;
uint16_t frameCount;
uint16_t mySlot;
uint16_t loopID;
float curHistogram[NUM_BINS];
float oriHistogram[NUM_BINS];
float preHistogram[NUM_BINS];
uint8_t myDegree; // update each Frame, the number includes itself
HistMsg neighbor_hists[NUM_DROPLETS];
uint8_t counter_neighbor;

uint8_t get_droplet_order_camouflage(uint16_t id)
{
	for(uint8_t i=0; i<NUM_DROPLETS; i++){
		if(dropletsID_set[i] == id) return i;
	}
	return 0xFF;
}

