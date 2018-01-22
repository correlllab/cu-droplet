#pragma once

#include "droplet_init.h"

extern uint32_t return_value;

typedef struct msg_struct{
	char text[3];
	uint16_t msgId;
}Msg;

uint16_t msgCount;
//static volatile uint16_t recvArray[500];
//static volatile uint16_t recvCount;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msgStruct);
