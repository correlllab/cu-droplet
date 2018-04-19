#pragma once

#include "droplet_init.h"

void		init(void);
void		loop(void);

void		handleMeas(Rnb* meas);
void		handleMsg(irMsg* msg_struct);
