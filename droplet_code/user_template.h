#pragma once

#include "droplet_init.h"

extern void init(void);//_attribute__ ((section (".usrtxt")));
//void		init(void);
void		handleMeas(Rnb* meas);
void		handleMsg(irMsg* msg_struct);

