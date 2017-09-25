#pragma once

#include "droplet_init.h"

#define MSG_FILLER_LENGTH 3
#define MSG_SEND_PERIOD 1000

typedef struct test_msg_struct{
	char filler[MSG_FILLER_LENGTH];
	uint32_t id;
}TestMsg;
TestMsg testMsg;

uint32_t lastMsgTime;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msg_struct);