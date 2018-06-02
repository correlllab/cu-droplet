#pragma once

#include "droplet_init.h"

#define SPEED_MSG_FLAG 'S'


uint8_t hopCount;
volatile uint32_t startTime;
uint32_t timeToCompletion;

typedef struct speed_msg_struct{
	char flag;
	uint8_t expID;
	uint8_t hopCount;
}SpeedMsg;

typedef struct speed_msg_node_struct{
	SpeedMsg msg;
	uint8_t numTries;
}SpeedMsgNode;

void		init(void);
void		loop(void);

void		noteStartTime(void);
void		prepSpeedMsg(void);
void		sendSpeedMsg(SpeedMsgNode* msgNode);
uint32_t	getExponentialBackoff(uint8_t c);
void		handleSpeedMsg(SpeedMsg* msg);
void		handleMsg(irMsg* msg_struct);
void		handleMeas(Rnb* meas);