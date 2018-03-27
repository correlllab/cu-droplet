#pragma once
#include "droplet_init.h"

#define RECEIVER_ID 0x3F9D

uint32_t MSG_PERIOD;
uint8_t dataCollecting;
uint8_t startSending;

uint16_t msgCount;
//static volatile uint16_t recvArray[500];
//static volatile uint16_t recvCount;

void		init(void);
void		loop(void);
void		handleMsg(irMsg* msgStruct);

void startListening(void);

void startTransmitting(void);