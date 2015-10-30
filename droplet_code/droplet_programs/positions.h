#pragma once

#include "droplet_init.h"

//#define GOODBYE_FLAG			'!'
#define BALL_BOUNCE_FLAG		'B'
#define BOT_POS_FLAG			'P'
#define LOOP_PERIOD_MS			400
#define RNB_BC_PERIOD_MS		20400
#define LOOPS_PER_RNB			(RNB_BC_PERIOD_MS/LOOP_PERIOD_MS)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000
#define NUM_TRACKED_BOTS		12
#define NUM_PACKED_BOTS			4

typedef struct ball_bounce{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallBounceMsg;

typedef struct packed_bot_pos{
	uint16_t id;
	uint8_t rangeMM;
	uint8_t bhvPacked[5];
}PackedBotPos;

typedef struct bot_pos_msg{
	PackedBotPos bots[NUM_PACKED_BOTS];
	char flag;	
}BotPosMsg;

typedef struct bot_pos_struct
{
	uint16_t id;	
	float r;
	float b;
	float h;
	float rV;
	float bV;
	float hV;
} BotPos;
BotPos neighbors[NUM_TRACKED_BOTS];

typedef enum{
	NOT_BALL,
	NOT_BALL_ALERT,
	BALL
} State;

State myState;
uint8_t ballSeqPos;


uint8_t		numNeighbors;

uint16_t	loopCount;
uint16_t	myRNBLoop;
uint16_t	myMsgLoop;
uint8_t		missedBroadcast;
uint8_t		missedMsg;

uint8_t outwardDir;
uint16_t outwardDirID;
uint32_t lastGoodbye;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);


void calculateOutboundDir(uint16_t inID);
void sendBotPosMsg();
void useNewRnbMeas();
void useBotPosMsg(PackedBotPos* bots, uint16_t senderID);
void fuseData(BotPos* currPos, float newR, float newB, float newH,float otherRvar, float otherBvar, float otherHvar);
void combineVars(float Rms, float Bms, float Hms, float vRms, float vBms, float vHms, float Rso, float Bso, float Hso, float vRso, float vBso, float vHso, float* vRmo, float* vBmo, float* vHmo);
void cleanNeighbor(uint8_t idx);
BotPos* addNeighbor(uint16_t id, float conf);
BotPos* getNeighbor(uint16_t id);
void removeNeighbor(uint16_t id);

static void inline getVarsFromConf(float conf, float* rVar, float* bVar, float* hVar){
	*rVar = powf( 43.41/conf,2.0);
	*bVar = powf( 12.17/conf,2.0);
	*hVar = powf( 12.17/conf,2.0);
}

static float inline getConfFromVars(float rVar, float bVar, float hVar){
	float rConf, bConf, hConf;
	rConf =  43.41/sqrtf(rVar);
	bConf =  12.17/sqrtf(bVar);
	hConf =  12.17/sqrtf(hVar);
	float newConf = (rConf + bConf + hConf)/3.0;
	if(newConf>3000) newConf=3000;
	return newConf;
}

static void inline packPackedBotPos(PackedBotPos* bot, float r, float b, float h, float rV, float bV, float hV){
	int16_t range = (int16_t)(r*10);
	if(range>255)		range = 255;
	else if(range<0)	range = 0;
	bot->rangeMM = (uint8_t)range;
	uint16_t bearing = ((int16_t)rad_to_deg(b))+180;
	uint16_t heading = ((int16_t)rad_to_deg(h))+180;
	
	float vars[3] = {rV, bV, hV};
	float scaledVar;
	for(uint8_t i=0;i<3;i++){
		if(i==0) scaledVar = vars[i]*12.0;
		else scaledVar	   = rad_to_deg(vars[i])*10.0;
		if(scaledVar>127) bot->bhvPacked[2+i]=127;
		else bot->bhvPacked[2+i] = ((uint8_t)scaledVar)&0x7F;
	}
	
	bot->bhvPacked[0] = (uint8_t)(bearing&0xFF);
	bot->bhvPacked[1] = (uint8_t)(heading&0xFF);
	bot->bhvPacked[2] |= ((uint8_t)(bearing>>8))&0x01;
	bot->bhvPacked[3] |= ((uint8_t)(heading>>8))&0x01;
}

static void inline unpackPackedBotPos(PackedBotPos* bot, float* r, float* b, float* h, float* vR, float* vB, float* vH){
	*r = (float)(bot->rangeMM)/10.0;
	*b = pretty_angle(deg_to_rad(((((uint16_t)(bot->bhvPacked[2]))<<8)&0x0100) | ((uint16_t)(bot->bhvPacked[0])))-M_PI);
	*h = pretty_angle(deg_to_rad(((((uint16_t)(bot->bhvPacked[3]))<<8)&0x0100) | ((uint16_t)(bot->bhvPacked[1])))-M_PI);
	*vR = ((float)(bot->bhvPacked[2]&0x7F))/12.0;
	*vB = deg_to_rad(((float)(bot->bhvPacked[2]&0x7F))/10.0);
	*vH = deg_to_rad(((float)(bot->bhvPacked[2]&0x7F))/10.0);		
}