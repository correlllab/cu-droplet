#pragma once

#include "droplet_init.h"
#include "grid_init.h"

#define PING_MSG_FLAG 'P'
#define ACK_MSG_FLAG  'A'
#define MESS_MSG_FLAG 'M'
#define REC_MSG_FLAG 'R'

#define PING_MSG_PERIOD 32768

#define LOOP_DELAY_MS			17 //This number should stay small, but above 0.

uint8_t	acksSincePing;
uint32_t lastPingMsgSent;
uint8_t pingMsgPending;
uint8_t	recsSinceMess;
uint32_t lastMessMsgSent;
uint8_t messMsgPending;
id_t rec;
id_t dropletListened;

typedef struct ping_message_struct{
	char flag;
}PingMsg;

typedef struct ping_msg_node_struct{
	PingMsg msg;
	uint8_t numTries;
}PingMsgNode;

typedef struct ack_message_struct{
	char flag;
}AckMsg;

typedef struct ack_msg_node_struct{
	AckMsg msg;
	id_t tgt;
	uint8_t numTries;
}AckMsgNode;

typedef struct mess_message_struct{
	char flag;
// Insert here the message struct to be send by CSMA
}MessMsg;

typedef struct mess_msg_node_struct{
	MessMsg msg;
	id_t tgt;
	uint8_t numTries;
}MessMsgNode;

typedef struct rec_message_struct{
	char flag;
}RecMsg;

typedef struct rec_msg_node_struct{
	RecMsg msg;
	id_t tgt;
	uint8_t numTries;
}RecMsgNode;

void		init(void);
void		loop(void);

void		handlePingMsg(PingMsg* msg, id_t src);
void		handleAckMsg(AckMsg* msg, id_t src);
void		handleMessMsg(MessMsg* msg, id_t src);
void		handleRecMsg(RecMsg* msg, id_t src);

void		handle_msg(ir_msg* msg_struct);

uint32_t	getBackoffTime(uint8_t n);
void		prepPingMsg(void);
void		sendPingMsg(PingMsgNode* node);
void		prepAckMsg(id_t id);
void		sendAckMsg(AckMsgNode* node);
void		prepMessMsg(id_t id);
void		sendMessMsg(MessMsgNode* node);
void		prepRecMsg(id_t id);
void		sendRecMsg(RecMsgNode* node);