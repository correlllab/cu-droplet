#pragma once

#include "droplet_init.h"

void		init(void);
void		loop(void);
void		handle_msg(irMsg* msgStruct);

void		lightsOn(void);
void		lightsOff(void);