#pragma once

#include "droplet_init.h"

#define PING_MSG_FLAG 'P'
#define ACK_MSG_FLAG  'A'

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

#define FILLER_LENGTH 1

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	mySlot;
uint8_t	acksSincePing;
uint16_t	loopID;

typedef struct ping_message_struct{
	char flag;
	uint8_t filler[FILLER_LENGTH];
}PingMsg;

typedef struct ack_message_struct{
	char flag;
	id_t src;
}AckMsg;

void		init(void);
void		loop(void);

void		sendPingMsg(void);
void		handlePingMsg(PingMsg* msg, id_t src);

void		sendAckMsg(id_t src);
void		handleAckMsg(AckMsg* msg);

void		handle_msg			(ir_msg* msg_struct);