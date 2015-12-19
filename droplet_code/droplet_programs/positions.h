#pragma once

#include "droplet_init.h"

#define BALL_BOUNCE_FLAG		'B'
#define BOT_POS_FLAG			'P'
#define LOOP_PERIOD_MS			500
#define LOOPS_PER_RNB			21
#define RNB_BC_PERIOD_MS		(LOOP_PERIOD_MS*LOOPS_PER_RNB)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000
#define NUM_TRACKED_BOTS		12
#define NUM_PACKED_BOTS			6
#define NUM_POSSIBLE_BOTS		120
#define NUM_TRACKED_BAYESIAN	12

#define BAYES_CLOSE_THRESH		2.5
#define BAYES_FAR_THRESH		6
#define NEIGHBORHOOD_SIZE		6
#define UNKNOWN_DIST			10

typedef struct ball_bounce{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallBounceMsg;

typedef struct packed_bot_pos{
	uint16_t id;
	uint8_t rangeMM;
	uint8_t bhcPacked[3];
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

typedef struct neighbor_neighbor_struct
{
	uint16_t id;
	BotPos pos;
} NeighborNeighbor;

typedef struct neighbor_struct{
	uint16_t id;
	BotPos pos;
	//NeighborNeighbor neighbs[NUM_PACKED_BOTS];
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

typedef struct bayes_bot_struct{
	float P;
	uint16_t id;
} BayesBot;
BayesBot neighbors[NEIGHBORHOOD_SIZE][NUM_TRACKED_BAYESIAN];
float neighbPos[NEIGHBORHOOD_SIZE][2] = {{2.5,4.33},{5.0,0.0},{2.5,-4.33},{-2.5,-4.33},{-5.0,0.0},{-2.5,4.33}};


uint32_t time_before;

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

uint8_t outwardDir;
uint16_t outwardDirID;
uint32_t lastGoodbye;
uint8_t firstLoop;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);


void calculateOutboundDir(uint16_t inID);
void sendBotPosMsg();
void processOtherBotData();
void useNewRnbMeas();
void useBotPosMsg(PackedBotPos* bots, uint16_t senderID);
void fuseData(OtherBot* currPos, float newR, float newB, float newH,float otherRvar, float otherBvar, float otherHvar);
void combineVars(float Rms, float Bms, float Hms, float vRms, float vBms, float vHms, float Rso, float Bso, float Hso, float vRso, float vBso, float vHso, float* vRmo, float* vBmo, float* vHmo);
void cleanOtherBot(OtherBot* other);
OtherBot* addOtherBot(uint16_t id, float conf);
OtherBot* getOtherBot(uint16_t id);
void removeOtherBot(uint16_t id);

static void inline getVarsFromConf(float conf, float* rVar, float* bVar, float* hVar){
	*rVar = 43.41/conf;
	*bVar = 12.17/conf;
	*hVar = 12.17/conf;
}

static float inline getConfFromVars(float rVar, float bVar, float hVar){
	float rConf, bConf, hConf;
	rConf =  43.41/rVar;
	bConf =  12.17/bVar;
	hConf =  12.17/hVar;
	float newConf = (rConf + bConf + hConf)/3.0;
	if(newConf>189) newConf=189.;
	return newConf;
}

static void inline packPackedBotPos(PackedBotPos* bot, float r, float b, float h, float rV, float bV, float hV){
	int16_t range = (int16_t)(r*10);
	if(range>255)		range = 255;
	else if(range<0)	range = 0;
	bot->rangeMM = (uint8_t)range;
	uint16_t bearing = ((int16_t)rad_to_deg(b))+180;
	uint16_t heading = ((int16_t)rad_to_deg(h))+180;
	
	uint8_t conf = (uint8_t)(getConfFromVars(rV, bV, hV)/3.0);
	
	bot->bhcPacked[0] = (uint8_t)(bearing&0xFF);
	bot->bhcPacked[1] = (uint8_t)(heading&0xFF);
	bot->bhcPacked[2] = (((uint8_t)(bearing>>1))&0x80) | (((uint8_t)(heading>>2))&0x40) | (conf&0x3F);
}

static void inline unpackPackedBotPos(PackedBotPos* bot, float* r, float* b, float* h, float* vR, float* vB, float* vH){
	*r = (float)(bot->rangeMM)/10.0;
	*b = pretty_angle(deg_to_rad((((((uint16_t)(bot->bhcPacked[2]))<<1)&0x0100) | ((uint16_t)(bot->bhcPacked[0])))-180));
	*h = pretty_angle(deg_to_rad((((((uint16_t)(bot->bhcPacked[2]))<<2)&0x0100) | ((uint16_t)(bot->bhcPacked[1])))-180));
	float conf = ((float)((bot->bhcPacked[2])&0x3F))*3.0;
	getVarsFromConf(conf, vR, vB, vH);	
}