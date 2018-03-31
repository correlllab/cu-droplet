#pragma once
#include "droplet_init.h"

#define RECEIVER_ID 0x3F9D

#define MSG_MAX_TIME 8192 
#define HISTOGRAM_BINS 256
#define HISTOGRAM_BIN_WIDTH (MSG_MAX_TIME/HISTOGRAM_BINS)

uint32_t MSG_PERIOD;
uint8_t dataCollecting;
uint8_t startSending;
uint8_t histogram[HISTOGRAM_BINS];
uint16_t msgCount;
//static volatile uint16_t recvArray[500];
//static volatile uint16_t recvCount;

void		init(void);
void		loop(void);

void		handleMsg(irMsg* msgStruct);

void startListening(void);

void startTransmitting(void);

void		handleMeas(Rnb* meas);
void		handleMsg(irMsg* msg_struct);

