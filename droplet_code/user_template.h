#pragma once

#include "droplet_init.h"

#define MSG_FILLER_LENGTH 36
#define MSG_SEND_PERIOD 1000

typedef struct test_msg_struct{
	char filler[MSG_FILLER_LENGTH];
	uint32_t id;
}TestMsg;
TestMsg testMsg;

typedef struct test_msg_node_struct{
	TestMsg msg;
	uint8_t numTries;
}TestMsgNode;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msgStruct);

void prepTestMsg(void);
void sendTestMsg(TestMsgNode* msgNode);