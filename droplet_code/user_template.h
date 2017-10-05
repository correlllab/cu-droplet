#pragma once

#include "droplet_init.h"

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msgStruct);

void		lightsOn(void);
void		lightsOff(void);
