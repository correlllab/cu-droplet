#pragma once

#include "droplet_init.h"

volatile uint8_t testData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
uint32_t tmpTime;
uint32_t tmpArray[4];

extern void	init(void);
//extern void init(void)	__attribute__ ((section (".usrtxt")));
void		loop(void);
void		handleMsg(irMsg* msgStruct);
