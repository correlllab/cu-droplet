#pragma once

#include "droplet_init.h"

#define BALL_BOUNCE_FLAG		'B'
#define LOOP_PERIOD_MS			500
#define LOOPS_PER_RNB			26
#define RNB_BC_PERIOD_MS		(LOOP_PERIOD_MS*LOOPS_PER_RNB)
#define GROUP_TIMEOUT_MS		40000
#define MIN_GOODBYE_INTERVAL	10000
#define NUM_TRACKED_BOTS		12
#define NUM_PACKED_BOTS			6
#define NUM_POSSIBLE_BOTS		120
#define NUM_TRACKED_BAYESIAN	12
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
	
typedef struct ball_bounce{
	char flag;
	uint16_t id;
	uint8_t seqPos;
}BallBounceMsg;

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

typedef struct neighbor_struct{
	uint16_t id;
	BotPos pos;
} OtherBot;
OtherBot nearBots[NUM_TRACKED_BOTS];

typedef struct bayes_bot_struct{
	float P;
	uint16_t id;
} BayesBot;

typedef struct new_neighb_slot_struct{
	BayesBot candidates[NEW_PROBS_SIZE];
	float emptyProb;
	float untrackedProb;
} NewNeighbSlot;

typedef struct neighb_slot_struct{
	BayesBot candidates[NUM_TRACKED_BAYESIAN];
	float emptyProb;
	float untrackedProb;
} NeighbSlot;
NeighbSlot neighbors[NEIGHBORHOOD_SIZE];

uint32_t time_before;

State myState;
uint8_t ballSeqPos;

uint8_t		numNeighbors;

uint16_t	loopCount;
uint16_t	myRNBLoop;

uint8_t outwardDir;
uint16_t outwardDirID;
uint8_t firstLoop;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]);
uint8_t addProbsForNeighbor(NewNeighbSlot* newNeighb, float dists[NUM_TRACKED_BOTS], NeighbSlot* neighbor);
void processOtherBotData();
void bayesDebugPrintout();

//These three functions are the Kalman Filtering.
void useNewRnbMeas();
void combineVars(float Rms, float Bms, float Hms, float vRms, float vBms, float vHms, float Rso, float Bso, float Hso, float vRso, float vBso, float vHso, float* vRmo, float* vBmo, float* vHmo);
void fuseData(OtherBot* currPos, float newR, float newB, float newH,float otherRvar, float otherBvar, float otherHvar);

//These three functions are for the figuring out where the ball bounces to, 
//and communicating this information.
uint8_t check_bounce(uint8_t in_dir);
void calculateOutboundDir(uint16_t inID);
void sendBallMsg();

//These four functions are for interacting with the OtherBot data structure.
OtherBot* getOtherBot(uint16_t id);
void removeOtherBot(uint16_t id);
OtherBot* addOtherBot(uint16_t id, float conf);
void cleanOtherBot(OtherBot* other);

//Helper functions:
static int nearBotsCmpFunc(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	float aR = aN->pos.r;
	float bR = bN->pos.r;
	if(aR==0){
		return 1;
		}else if(bR==0){
		return -1;
		}else if(fabsf(aR-bR)<=1.5){
		return (int)(aN->pos.rV - bN->pos.rV);
		}else{
		return (int)(aR-bR);
	}
}

static int bayesCmpFunc(const void* a, const void* b){
	BayesBot* aN = (BayesBot*)a;
	BayesBot* bN = (BayesBot*)b;
	if((bN->P-aN->P)<0){
		return -1;
		}else if((bN->P-aN->P)>0){
		return 1;
		}else{
		return 0;
	}
}

static uint16_t inline getNeighborID(NeighbSlot* neighb){
	if(neighb->candidates[0].P > neighb->emptyProb){
		return neighb->candidates[0].id;
	}else{
		return 0;
	}
}

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