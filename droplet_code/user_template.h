#pragma once

#include "droplet_init.h"

extern uint32_t return_value;

uint8_t rxEnabled;
uint8_t dataCollecting;
uint8_t startSending;

#define BIT_MASK_MSG_FLAG 'M'
#define DURATION_MSG_FLAG 'D'

typedef uint32_t BotIdMask[4];

typedef struct bit_mask_msg_struct{
	BotIdMask mask;
	char flag; 
}BitMaskMsg;

typedef struct duration_msg{
	uint32_t dur;
	char flag;	
}DurationMsg;

BotIdMask allIDsMask;

//typedef struct msg_struct{
	//char text[3];
	//uint16_t msgId;
//}testMsg;

#define HISTOGRAM_SLOTS 32
#define NUM_HISTOGRAM_BINS (HISTOGRAM_SLOTS+2)
#define HISTOGRAM_BIN_WIDTH ((IR_MAX_MSG_ATTEMPT_DUR+(2*IR_MSG_TIMEOUT))/HISTOGRAM_SLOTS)

uint32_t maxThroughputDuration;
uint32_t throughputMsgStart;
uint16_t histogram[NUM_HISTOGRAM_BINS];
uint16_t numSenders;
uint32_t lastThroughputMsgSent;
uint8_t senderThisTime;
uint16_t msgCount;
uint8_t throughputStarted;
//static volatile uint16_t recvArray[500];
//static volatile uint16_t recvCount;

void		handleMsgTime(uint32_t time);
void		printHistogram(void);
void		processResults(void);
void		printResults(void);

void sendDurationMsg(void);
void handleDurationMsg(DurationMsg* msg);
void startThroughputMessaging(void);

void startTransmitting(void);
void setMsgPeriod(uint32_t value);
void sendThroughputMsg();

void		lightsOn(void);
void		lightsOff(void);

const uint32_t m1  = 0x55555555; //binary: 0101...
const uint32_t m2  = 0x33333333; //binary: 00110011..
const uint32_t m4  = 0x0f0f0f0f; //binary:  4 zeros,  4 ones ...

static inline uint8_t hammingWeight(uint32_t x){
	x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
	x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
	x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
	return (uint8_t)x;
}

static inline uint8_t idMaskHammingWeight(BotIdMask* mask){
	return hammingWeight((*mask)[0])+hammingWeight((*mask)[1])+
	hammingWeight((*mask)[2])+hammingWeight((*mask)[3]);
}

//sets a = b;
static inline void idMaskCopy(BotIdMask* a, BotIdMask* b){
	for(uint8_t i=0;i<4;i++){
		(*a)[i] = (*b)[i];
	}
}

//sets a = a | b;
static inline void bitwiseIdMaskOR(BotIdMask* a, BotIdMask* b){
	for(uint8_t i=0;i<4;i++){
		(*a)[i] = (*a)[i] | (*b)[i];
	}
}

//sets a = a ^ b;
static inline void bitwiseIdMaskXOR(BotIdMask* a, BotIdMask* b){
	for(uint8_t i=0;i<4;i++){
		(*a)[i] = (*a)[i] ^ (*b)[i];
	}
} 

//returns !a;
static inline uint8_t idMaskLogicalNegation(BotIdMask* a){
	return ( ( !((*a)[0]) && !((*a)[1]) ) && ( !((*a)[2]) && !((*a)[3]) ) );
}

static inline void setIdMaskBit(BotIdMask* mask, id_t id){
	uint8_t bitPos = getDropletOrd(id);
	(*mask)[bitPos>>5] |= (((uint32_t)1)<<(bitPos&0x1F));
}

static inline void printIdMask(BotIdMask* mask){
	printf("\t");
	for(uint8_t i=0;i<4;i++){
		printf("%08lx", (*mask)[i]);
	}
	printf("\r\n");
}