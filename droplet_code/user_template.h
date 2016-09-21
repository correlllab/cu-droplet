#pragma once

#include "droplet_init.h"

#define SLOT_LENGTH_MS			163
#define SLOTS_PER_FRAME			29
#define FRAME_LENGTH_MS			(((uint32_t)SLOT_LENGTH_MS)*((uint32_t)SLOTS_PER_FRAME))
#define LOOP_DELAY_MS			17

void		init();
void		loop();

#define NEAR_BOTS_MSG_FLAG		'N'
#define NUM_SHARED_BOTS 4
#define UNDF	0x8000

typedef struct packed_bot_meas_struct{
	id_t id;
	uint8_t range;
	int8_t conf;
	int8_t b;
	int8_t h;
}PackedBotMeas;

typedef struct near_bots_msg_struct{
	int16_t minX;
	int16_t minY;
	int16_t maxX;
	int16_t maxY;
	int16_t x;
	int16_t y;
	int8_t posConf;
	PackedBotMeas shared[NUM_SHARED_BOTS];
	char flag;
}NearBotsMsg;

void handleNearBotsMsg(NearBotsMsg* msg, id_t senderID);

void		handle_msg			(ir_msg* msg_struct);

static int16_t inline unpackAngleMeas(int8_t angle){
	return (((int16_t)angle)<<1);
}

static uint16_t inline unpackRange(uint8_t packedRange){
	return (((uint16_t)packedRange)+2)>>2; //dividing by four, rounding towards closest.
}