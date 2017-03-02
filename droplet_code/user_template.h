#pragma once

#include "droplet_init.h"

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);
void		send_motor_settings();
char *custom_itoa(int value);
void custom_atoi(char*);
void edge_following(int16_t, int16_t);