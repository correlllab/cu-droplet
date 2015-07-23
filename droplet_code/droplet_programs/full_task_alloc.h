#pragma once

#include "droplet_init.h"

enum States
{
    NOTHING,
    WALK_FORWARD,
    WALK_BACKWARD,
    TURN_LEFT_SHORT,
    TURN_RIGHT_SHORT,
    TURN_LEFT_LONG,
    TURN_RIGHT_LONG,
    LED_ON
};

enum States state;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);