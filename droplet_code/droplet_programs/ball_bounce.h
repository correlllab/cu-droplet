#pragma once

#include "droplet_init.h"

//#define GOODBYE_FLAG			'!'
#define BALL_BOUNCE_FLAG		'B'
#define BOT_POS_FLAG			'P'
#define LOOP_PERIOD_MS			200
#define RNB_BC_PERIOD_MS		8600
#define LOOPS_PER_RNB			(RNB_BC_PERIOD_MS/LOOP_PERIOD_MS)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000
#define NEIGHBORHOOD_SIZE		6

typedef struct ball_bounce{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallBounceMsg;

typedef struct packed_bot_pos{
	uint16_t id;
	uint8_t rangeMM;
	uint8_t rangeBearingPacked[3];
}PackedBotPos;

typedef struct bot_pos_msg{
	char flag;
	PackedBotPos bots[6];
}BotPosMsg;

typedef enum{
	NOT_BALL,
	NOT_BALL_ALERT,
	BALL
} State;

State myState;
uint8_t ballSeqPos;
uint8_t missedBroadcast;

typedef struct neighbor_struct
{
	float posError;
	uint32_t lastSeen;
	uint16_t ID;
	uint8_t noCollCount;
} OtherBot;
OtherBot nearBots[NEIGHBORHOOD_SIZE];

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