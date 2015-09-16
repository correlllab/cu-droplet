#pragma once

#include "droplet_init.h"

#define BOT_DATA_FLAG			'D'
#define BALL_BOUNCE_FLAG		'B'

#define LOOP_PERIOD_MS			200
#define RNB_BC_PERIOD_MS		19600
#define LOOPS_PER_RNB			(RNB_BC_PERIOD_MS/LOOP_PERIOD_MS)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000

#define NUM_TRACKED_BOTS		12
#define NUM_SENT_BOTS			6

typedef struct ball_bounce{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallBounceMsg;

typedef struct packed_bot_datum{
	uint16_t id;
	uint8_t rangeMM;
	int8_t bearingOver2;
	int8_t headingOver2;
}PackedBotDatum;

typedef struct bot_datum{
	uint16_t id;
	uint16_t range;
	int16_t bearing;
	int16_t heading;
}BotDatum;

typedef struct pos_info{
	char			flag;
	uint8_t			numBots;
	PackedBotDatum	bots[NUM_SENT_BOTS];
}PosInfoMsg;

typedef enum{
	NOT_BALL,
	NOT_BALL_ALERT,
	BALL
} State;
	
BotDatum nearbyBotsData[NUM_TRACKED_BOTS];
State myState;
uint8_t ballSeqPos;
uint8_t missedBroadcast;

uint16_t myRNBLoop;
uint16_t loopCount;
uint8_t outwardDir;
uint16_t outwardDirID;
uint32_t lastGoodbye;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);

void use_new_rnb();
uint8_t check_bounce(uint8_t in_dir);