#pragma once

#include "droplet_init.h"

//void		init(void);
extern void		init(void) __attribute__ ((section (".USERCODE")));
void		loop(void);
void		handleMsg(irMsg* msgStruct);
