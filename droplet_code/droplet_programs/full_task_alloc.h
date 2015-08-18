#pragma once

#include "droplet_init.h"

enum States
{
    NOTHING,
    WALK_FORWARD,
    WALK_BACKWARD,
	TURN_RIGHT,
	TURN_LEFT
};

enum Adjusts
{
	NONE,
	ONE_OVER_TWO,
	TWO_OVER_ONE,
	WRITE_SETTINGS
};

int8_t prev_move_dir;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);