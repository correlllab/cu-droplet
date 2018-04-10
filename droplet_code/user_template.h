#pragma once

#include "droplet_init.h"

extern uint32_t return_value;

uint8_t rxEnabled;


uint8_t dataCollecting;
uint8_t startSending;

//typedef struct msg_struct{
	//char text[3];
	//uint16_t msgId;
//}testMsg;

#define HISTOGRAM_SLOTS 32
#define NUM_HISTOGRAM_BINS (HISTOGRAM_SLOTS+2)
#define HISTOGRAM_BIN_WIDTH ((IR_MAX_MSG_ATTEMPT_DUR+(2*IR_MSG_TIMEOUT))/HISTOGRAM_SLOTS)

uint16_t histogram[NUM_HISTOGRAM_BINS];
uint16_t numSenders;
uint8_t senderThisTime;
uint16_t msgCount;
//static volatile uint16_t recvArray[500];
//static volatile uint16_t recvCount;

void		init(void);
void		loop(void);
void		handleMsgTime(uint32_t time);
void		handleMsg(irMsg* msgStruct);
void		printHistogram(void);
void		processResults(void);
void		printResults(void);

void startTransmitting(void);
void setMsgPeriod(uint32_t value);