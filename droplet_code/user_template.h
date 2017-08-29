#pragma once

#include "droplet_init.h"

typedef struct msg_struct{
	char text[3];
	uint16_t msgId;
}Msg;

uint16_t msgCount;

void		init(void);
void		loop(void);
void		handle_msg(ir_msg* msg_struct);