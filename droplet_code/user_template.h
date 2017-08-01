#pragma once

#include "droplet_init.h"

#define PING_MSG_FLAG 'P'
#define ACK_MSG_FLAG  'A'

#define PING_MSG_PERIOD 32768

#define LOOP_DELAY_MS			17 //This number should stay small, but above 0.

uint8_t	acksSincePing;
uint32_t lastPingMsgSent;
uint8_t pingMsgPending;

typedef struct ping_message_struct{
	char flag;
	float filler[9];
}PingMsg;

typedef struct ping_msg_node_struct{
	PingMsg msg;
	uint8_t numTries;
}PingMsgNode;

typedef struct ack_message_struct{
	char flag;
	float filler[9];
}AckMsg;

typedef struct ack_msg_node_struct{
	AckMsg msg;
	id_t tgt;
	uint8_t numTries;
}AckMsgNode;

void		init(void);
void		loop(void);

void		handlePingMsg(PingMsg* msg, id_t src);
void		handleAckMsg(AckMsg* msg);

void		handle_msg(ir_msg* msg_struct);

uint32_t	getBackoffTime(uint8_t N);
void		prepPingMsg(void);
void		sendPingMsg(PingMsgNode* node);
void		prepAckMsg(id_t id);
void		sendAckMsg(AckMsgNode* node);