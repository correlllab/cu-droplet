#pragma once

#include "droplet_init.h"

int comparison(uint8_t* aPtr, uint8_t* bPtr);

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);


