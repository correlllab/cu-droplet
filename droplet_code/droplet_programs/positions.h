#pragma once

#include "droplet_init.h"

#define BALL_MSG_FLAG			'B'
#define NEIGHB_MSG_FLAG			'N'
#define LOOP_PERIOD_MS			500
#define LOOPS_PER_RNB			38
#define RNB_BC_PERIOD_MS		(LOOP_PERIOD_MS*LOOPS_PER_RNB)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000
#define NUM_TRACKED_BOTS		8
#define 
#define NUM_POSSIBLE_BOTS		60
#define NUM_TRACKED_BAYESIAN	4
#define NEW_PROBS_SIZE			NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN

#define BAYES_CLOSE_THRESH		2.5
#define BAYES_FAR_THRESH		6
#define NEIGHBORHOOD_SIZE		6
#define UNKNOWN_DIST			10
#define INIT_LIKELIHOOD_EMPTY	0.2
#define UNMEASURED_NEIGHBOR_LIKELIHOOD	0.9

typedef enum{
	NOT_BALL,
	NOT_BALL_ALERT,
	BALL
} State;

//For each of the positions in our immediate 6-neighborhood, this array stores
//the x,y coordinate of that position, in cm.
const float neighbPos[NEIGHBORHOOD_SIZE][2] = {{2.5,4.33}, {5.0,0.0},
												{2.5,-4.33},{-2.5,-4.33},
												{-5.0,0.0}, {-2.5,4.33}  };
//The values below are for the Kalman Filter code.
const float PROC_NOISE[3] = {powf(1.0,2.0), powf(M_PI/30.0,2.0), powf(M_PI/22.5,2.0)};
const float MEAS_NOISE[3] = {powf(3.0,2.0), powf(M_PI/6.0,2.0), powf(M_PI/6.0,2.0)};
	
typedef struct ball_msg_struct{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallMsg;

typedef struct neighb_msg_struct{
	uint16_t ids[NEIGHBORHOOD_SIZE];
	char flag;
}NeighbMsg;

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

typedef struct other_bot_rnb_struct{
	uint16_t id;
	BotPos pos;
	float conf;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

typedef struct bayes_bot_struct{
	float P;
	uint16_t id;
} BayesBot;

typedef struct new_bayes_slot_struct{
	BayesBot candidates[NEW_PROBS_SIZE];
	float emptyProb;
	float untrackedProb;
} NewBayesSlot;

typedef struct bayes_slot_struct{
	BayesBot candidates[NUM_TRACKED_BAYESIAN];
	float emptyProb;
	float untrackedProb;
} BayesSlot;
BayesSlot bayesSlots[NEIGHBORHOOD_SIZE];

typedef struct neighbs_list_struct{
	uint16_t id;
	uint16_t neighbs[NEIGHBORHOOD_SIZE];
} NeighbsList;
NeighbsList neighbsList[NEIGHBORHOOD_SIZE];

uint32_t time_before;

State myState;
uint8_t ballSeqPos;

uint8_t		numNearBots;

uint16_t	loopCount;
uint16_t	myRNBLoop;
uint16_t	myMsgLoop;

uint8_t outwardDir;
uint16_t outwardDirID;
uint8_t firstLoop;


uint8_t failureCounts[NEIGHBORHOOD_SIZE];
uint8_t successCounts[NEIGHBORHOOD_SIZE];

void		init();
void		loop();
void		sendNeighbMsg();
void		sendBallMsg();
void		handle_msg			(ir_msg* msg_struct);

void printNeighbsList();
void processNeighborData();
void cleanupNeighbsList();

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]);
void addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* neighbor, uint8_t dir);
void processOtherBotData();
void bayesDebugPrintout();

//These three functions are the Kalman Filtering.
void useNewRnbMeas();

//These three functions are for the figuring out where the ball bounces to, 
//and communicating this information.
uint8_t check_bounce(uint8_t in_dir);
void calculateOutboundDir(uint16_t inID);

//These four functions are for interacting with the OtherBot data structure.
OtherBot* getOtherBot(uint16_t id);
void removeOtherBot(uint16_t id);
OtherBot* addOtherBot(uint16_t id, float conf);
void cleanOtherBot(OtherBot* other);

//int nearBotsCmpFunc(const void* a, const void* b);
//int bayesCmpFunc(const void* a, const void* b);

//Helper functions:
static void inline getVarsFromConf(float conf, float* rVar, float* bVar, float* hVar){
	*rVar = 43.41/conf;
	*bVar = 12.17/conf;
	*hVar = 12.17/conf;
}

static float getConfFromVars(float rVar, float bVar, float hVar){
	float rConf, bConf, hConf;
	rConf =  43.41/rVar;
	bConf =  12.17/bVar;
	hConf =  12.17/hVar;
	float newConf = (rConf + bConf + hConf)/3.0;
	if(newConf>600) newConf=600.; //189
	return newConf;
}

static int nearBotsCmpFunc(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	float aC = aN->conf;
	float bC = bN->conf;
	if(aN->id==0){
		return 1;
	}
	if(bN->id==0){
		return -1;
	}
	if(aC < bC){
		return 1;
	}else if(bC < aC){
		return -1;
	}else{
		return 0;
	}
}

static int bayesCmpFunc(const void* a, const void* b){
	BayesBot* aN = (BayesBot*)a;
	BayesBot* bN = (BayesBot*)b;
	float aP = aN->P;
	float bP = bN->P;
	//printf("\t%04X @(%p) : %10f || %04X @(%p) : %10f\r\n", aN->id, aN, aP, bN->id, bN, bP);
	if(bP<aP){
		return -1;
	}else if(bP>aP){
		return 1;
	}else{
		return 0;
	}
}