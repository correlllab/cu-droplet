#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

#define BAYES_DEBUG_MODE
//#define POS_DEBUG_MODE

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

void init(){	
	numNeighbors = 0;
	loopCount = 0;
	outwardDir = 0;
	outwardDirID = 0;
	firstLoop = 1;	
	myState=NOT_BALL;	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbors[i].emptyProb=INIT_LIKELIHOOD_EMPTY;
		neighbors[i].untrackedProb=1.0-INIT_LIKELIHOOD_EMPTY;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN-1;j++){
			neighbors[i].candidates[j].P=0.0;
			neighbors[i].candidates[j].id=0;
		}
	}
	myRNBLoop = (get_droplet_id()%LOOPS_PER_RNB);
	printf("myRNBLoop: %d\r\n", myRNBLoop);
}

//TODO: Maybe have robots communicate their neighbors and if you think
// I'm your neighbor I add a good amount of probability to the idea that you are my neighbor?

void loop(){
	if((get_time()%LOOP_PERIOD_MS)<(LOOP_PERIOD_MS/50)){
		set_rgb(0,0,0);
		if((!firstLoop)&&(loopCount==myRNBLoop)){
			delay_ms(5);
			broadcast_rnb_data();
			//myRNBLoop=(myRNBLoop)%LOOPS_PER_RNB;
		}else if((!firstLoop)&&(loopCount==LOOPS_PER_RNB-1)){
			qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);				
			printf("\nT: %lu\r\n", get_time());
			printf("  ID  |    R    |   rV   |   B    |   bV   |   H    |   hV   |  CONF  |\r\n{\r\n");
			BotPos* pos;
			for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
				if(nearBots[i].id!=0){
					pos = &(nearBots[i].pos);
					printf("{{\"%04X\", % -7.2f , %6.2f , % -6.1f , %6.1f , % -6.1f , %6.1f , %6.1f },{\r\n", nearBots[i].id, pos->r, pos->rV, rad_to_deg(pos->b), rad_to_deg(pos->bV), rad_to_deg(pos->h), rad_to_deg(pos->hV), getConfFromVars(pos->rV, pos->bV, pos->hV));
					printf("}},\r\n");
				}
			}
			printf("}\r\n\n");
			processOtherBotData();			
		}
		if(myState==BALL){
			switch(ballSeqPos){
				case 0: set_rgb(50,50,50);		break;
				case 1: set_rgb(200,200,200);	break;
				case 2: set_rgb(50,50,50);		break;
				default:
				printf("Error! Unexpected seqPos.\r\n");
			}
			sendBallMsg();
			ballSeqPos++;
			if(ballSeqPos>2){
				myState=NOT_BALL;
				ballSeqPos=0;
			}
		}else{
			set_rgb(0,0,0);
		}
		
		if(loopCount==(LOOPS_PER_RNB-1)){ firstLoop = 0;}
		loopCount=((loopCount+1)%LOOPS_PER_RNB);
		delay_ms(LOOP_PERIOD_MS/50);
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
}

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]){
	BotPos* pos;
	float x, y;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id!=0){
			printf("Bot %04X:\r\n", nearBots[i].id);
			pos = &(nearBots[i].pos);
			x = (pos->r)*cosf(pos->b+M_PI_2);
			y = (pos->r)*sinf(pos->b+M_PI_2);
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				dists[j][i] = hypotf(x-neighbPos[j][0],y-neighbPos[j][1]);
				if(dists[j][i]==0.0){
					dists[j][i]=0.01;
				}
				printf("\tSlot %hu: %f\r\n",j,dists[j][i]);
			}
		}
	}
}

uint8_t addProbsForNeighbor(NewNeighbSlot* newNeighb, float dists[NUM_TRACKED_BOTS], NeighbSlot* neighbor){
	uint8_t numTrackedAndMeasured=0;
	uint8_t numMeasuredNotTracked=0;
	uint8_t numTrackedNotMeasured=0;
	uint8_t numDistsOverThresh=0;
	uint8_t found;
	BayesBot* candidates = neighbor->candidates;
	BayesBot* newProbs = (newNeighb->candidates);
	for(uint8_t j=0;j<NUM_TRACKED_BOTS;j++){
		found=0;
		newProbs[j].id=0;
		newProbs[j].P=0.0;
		for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
			if(candidates[k].id!=0){
				if(candidates[k].id==nearBots[j].id){
					found = 1;
					numTrackedAndMeasured++;
					if(dists[j]>5.0) numDistsOverThresh++;
					//A previously tracked robot was measured again.
					newProbs[j].id=candidates[k].id;
					newProbs[j].P=(4.0/dists[j])*candidates[k].P; //bayes-y
				}
			}
		}
		if(!found){
			//We measured a robot we weren't tracking.
			numMeasuredNotTracked++;
			if(dists[j]>5.0) numDistsOverThresh++;
			newProbs[j].id=nearBots[j].id;
			newProbs[j].P = (4.0/dists[j])*((neighbor->untrackedProb)/(NUM_POSSIBLE_BOTS-(numTrackedAndMeasured+numTrackedNotMeasured)));
		}
	}
	for(uint8_t j=0 ; j < NUM_TRACKED_BAYESIAN ; j++){
		newProbs[NUM_TRACKED_BOTS+j].id=0;
		newProbs[NUM_TRACKED_BOTS+j].P=0.0;
		uint8_t found=0;
		for(uint8_t k=0;k<NUM_TRACKED_BOTS;k++){
			if(newProbs[k].id!=0 && newProbs[k].id==candidates[j].id){
				found=1;
				break;
			}
		}
		if(!found){
			//A robot we were tracking was not measured.
			numTrackedNotMeasured++;
			newProbs[NUM_TRACKED_BOTS+j].id=candidates[j].id;
			newProbs[NUM_TRACKED_BOTS+j].P=UNMEASURED_NEIGHBOR_LIKELIHOOD*candidates[j].P;
		}
	}
	newNeighb->emptyProb = neighbor->emptyProb;
	newNeighb->untrackedProb = neighbor->untrackedProb;
	
	uint8_t numClose = numTrackedAndMeasured+numMeasuredNotTracked-numDistsOverThresh;	
	uint8_t diff;
	if(numMeasuredNotTracked>numTrackedNotMeasured){
		diff=numMeasuredNotTracked-numTrackedNotMeasured;
		newNeighb->emptyProb = newNeighb->emptyProb - 0.08*(diff+2*numClose);
		if(newNeighb->emptyProb<0.08) newNeighb->emptyProb = 0.08;
		newNeighb->untrackedProb = newNeighb->untrackedProb - 0.02*(diff+2*numClose);
		if(newNeighb->untrackedProb<0.02) newNeighb->untrackedProb = 0.02;		
	}else if(numMeasuredNotTracked<numTrackedNotMeasured){
		diff=numTrackedNotMeasured-numMeasuredNotTracked;
		newNeighb->emptyProb = newNeighb->emptyProb + 0.04*diff;
		newNeighb->untrackedProb = newNeighb->untrackedProb + 0.01*diff;
	}
	if( newNeighb->emptyProb <= 0.08 ) newNeighb->emptyProb = 0.08;
	if( newNeighb->untrackedProb <= 0.02) newNeighb->untrackedProb = 0.02;
	return (numTrackedAndMeasured+numTrackedNotMeasured);
}

void processOtherBotData(){
	float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS];
	NewNeighbSlot newNeighb;
	float total;
	uint8_t numTracked;
	BayesBot* newProbs = (newNeighb.candidates);
	NeighbSlot* neighbor;
	BayesBot* candidates;
	calculateDistsFromNeighborPos(dists); //Populates this array.
	
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbor = &(neighbors[i]);
		candidates = neighbor->candidates;
		numTracked = addProbsForNeighbor(&newNeighb, dists[i], neighbor);
		qsort(newNeighb.candidates, NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN,sizeof(BayesBot), bayesCmpFunc);

		total=0.0;
		float tooSmallThresh = newNeighb.untrackedProb/(NUM_POSSIBLE_BOTS-numTracked);
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			if(newProbs[j].id!=0){
				if(newProbs[j].P<=tooSmallThresh){
					newNeighb.untrackedProb+=newProbs[j].P;
					candidates[j].id=0;
					candidates[j].P=0.0;
				}else{
					candidates[j].id=newProbs[j].id;
					candidates[j].P=newProbs[j].P;
					total+=candidates[j].P;
				}

			}else{
				candidates[j].id=0;
				candidates[j].P=0.0;
			}
		}
		
		total += newNeighb.emptyProb + newNeighb.untrackedProb;
		neighbor->emptyProb=newNeighb.emptyProb/total;
		neighbor->untrackedProb=newNeighb.untrackedProb/total;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			if(newProbs[j].id!=0){
				candidates[j].P=candidates[j].P/total;
			}
		}
	}
	#ifdef BAYES_DEBUG_MODE
		bayesDebugPrintout();
	#endif
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	numNeighbors = 0;
}

void bayesDebugPrintout(){
	NeighbSlot* neighbor;
	BayesBot* candidates;
	uint8_t untrackedToggle, emptyToggle,emptyFirst;	
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbor = &(neighbors[i]);
		candidates=neighbor->candidates;
		untrackedToggle=0;
		emptyToggle=0;
		printf("Slot %hu:\r\n", i);
		if(neighbor->emptyProb>neighbor->untrackedProb){
			emptyFirst=1;
			}else{
			emptyFirst=0;
		}
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			if(candidates[j].id!=0){
				if((emptyFirst || (!emptyFirst && untrackedToggle)) && !emptyToggle){
					if(candidates[j].P < neighbor->emptyProb){
						printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
						emptyToggle=1;
					}
				}
				if((!emptyFirst || (emptyFirst && emptyToggle)) && !untrackedToggle){
					if(candidates[j].P < neighbor->untrackedProb){
						printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
						untrackedToggle=1;
					}
					if(!emptyToggle && (candidates[j].P < neighbor->emptyProb)){
						printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
						emptyToggle=1;
					}
				}
				printf("\t%04X\t% 6f\r\n",candidates[j].id,candidates[j].P);
			}
		}
		if(emptyFirst){
			if(!emptyToggle){
				printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
				emptyToggle=1;
			}
			if(!untrackedToggle){
				printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
				untrackedToggle=1;
			}
			}else{
			if(!untrackedToggle){
				printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
				untrackedToggle=1;
			}
			if(!emptyToggle){
				printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
				emptyToggle=1;
			}
		}
	}
}


void useNewRnbMeas(){
	rnb_updated=0;
	float rVar, bVar, hVar;
	getVarsFromConf(last_good_rnb.conf, &rVar, &bVar, &hVar);
	OtherBot* neighbor = addOtherBot(last_good_rnb.id_number, last_good_rnb.conf);
	POS_DEBUG_PRINT("(RNB) ID: %04X\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", last_good_rnb.id_number, last_good_rnb.range, rVar, rad_to_deg(last_good_rnb.bearing), rad_to_deg(bVar), rad_to_deg(last_good_rnb.heading), rad_to_deg(hVar), last_good_rnb.conf);
	BotPos* pos = &(neighbor->pos);
	if(neighbor==NULL) return;
	if(neighbor->id == 0 || neighbor->id == last_good_rnb.id_number){ //We weren't tracking this ID before, so just add the new info.
		neighbor->id		= last_good_rnb.id_number;
		pos->r	= last_good_rnb.range;
		pos->rV  = rVar;
		pos->b	= last_good_rnb.bearing;
		pos->bV  = bVar;
		pos->h	= last_good_rnb.heading;
		pos->hV  = hVar;
		//}else if(neighbor->id == last_good_rnb.id_number){
		//POS_DEBUG_PRINT("\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", pos->r, pos->rV, rad_to_deg(pos->b), rad_to_deg(pos->bV), rad_to_deg(pos->h), rad_to_deg(pos->hV));
		//fuseData(neighbor, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading, rVar, bVar, hVar);
	}else{
		printf("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n");
	}
}

void combineVars(float Rms, float Bms, float Hms, float vRms, float vBms, float vHms, float Rso, float Bso, float Hso, float vRso, float vBso, float vHso, float* vRmo, float* vBmo, float* vHmo){
	float Rms2 = powf(Rms,2.0);
	float Rso2 = powf(Rso,2.0);
	
	float denominator = (Rms2+Rso2+2*Rms*Rso*cosf(Bms-Bso-Hms));
	
	float cosTerm = (-Rso2*vRms+Rms2*(Rso2*(vBms+vBso+vHms)-vRso))*cosf(2*Bms-2*Bso-2*Hms);
	
	*vRmo = (Rms2*(Rso2*(vBms+vBso+vHms)+2*vRms+vRso)+Rso2*(vRms+2*vRso)-cosTerm+4*Rms*Rso*(vRms+vRso)*cosf(Bms-Bso-Hms))/(2.0*denominator);
	*vBmo = (2*powf(Rms,4.0)*vBms+2*powf(Rso,4.0)*(vBso+vHms)+Rso2*vRms+Rms2*(Rso2*(vBms+vBso+vHms)+vRso)+cosTerm+4*Rms*Rso*(Rms2*vBms+Rso2*(vBso+vHms))*cosf(Bms-Bso-Hms))/(2.0*powf(denominator,2.0));
	*vHmo = vHms+vHso;
}

void fuseData(OtherBot* currPos, float otherR, float otherB, float otherH, float otherRvar, float otherBvar, float otherHvar){
	//KALMAN VERSION
	float vals[3] = {currPos->pos.r, currPos->pos.b, currPos->pos.h};
	float vars[3] = {currPos->pos.rV, currPos->pos.bV, currPos->pos.hV};
	float otherVals[3] = {otherR, otherB, otherH};
	float otherVars[3] = {otherRvar, otherBvar, otherHvar};	
	float newVals[3], newVars[3];
	float predVar, kalmanGain, newVal, newVar;		
	POS_DEBUG_PRINT("    FUSING:\r\n\tRmo': % 5.1fñ%-4.1f Bmo': % 6.1fñ%-4.1f Hmo': % 6.1fñ%-4.1f\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n",
				otherR, otherRvar, rad_to_deg(otherB), rad_to_deg(otherBvar), rad_to_deg(otherH), rad_to_deg(otherHvar),
				vals[0], vars[0], rad_to_deg(vals[1]), rad_to_deg(vars[1]), rad_to_deg(vals[2]), rad_to_deg(vars[2]));

	for(uint8_t i=0;i<3;i++){
		predVar = vars[i] + PROC_NOISE[i];
		kalmanGain = (predVar)/(predVar+MEAS_NOISE[i]);
		if(i>0){
			newVals[i] = pretty_angle(pretty_angle(kalmanGain*otherVals[i])+pretty_angle((1.0-kalmanGain)*vals[i]));
		}else{
			newVals[i] = kalmanGain*otherVals[i] + (1.0 - kalmanGain)*vals[i];
		}
		newVars[i] = predVar*(1.0-kalmanGain);
	}

	currPos->pos.r  = newVals[0];
	currPos->pos.b  = newVals[1];
	currPos->pos.h  = newVals[2];
	currPos->pos.rV = newVars[0];
	currPos->pos.bV = newVars[1];
	currPos->pos.hV = newVars[2];
	POS_DEBUG_PRINT("\tR: % 6.2fñ%-6.3f B: % 6.1fñ%-6.3f H: % 6.1fñ%-6.3f\r\n", newVals[0], newVars[0], rad_to_deg(newVals[1]), rad_to_deg(newVars[1]), rad_to_deg(newVals[2]), rad_to_deg(newVars[2]));						
}

uint8_t check_bounce(uint8_t in_dir){
	//note: directions below are relative to the direction from which the ball came in.
	uint8_t opp_dir			= (in_dir+3)%6;
	uint8_t left_dir		= (in_dir+1)%6;
	uint8_t right_dir		= (in_dir+5)%6; //it's like -1
	uint8_t far_left_dir	= (in_dir+2)%6;
	uint8_t far_right_dir	= (in_dir+4)%6; //it's like -2
	
	uint8_t opp			= !!getNeighborID(&neighbors[opp_dir]		);
	uint8_t left		= !!getNeighborID(&neighbors[left_dir]		);
	uint8_t right		= !!getNeighborID(&neighbors[right_dir]		);
	uint8_t far_left	= !!getNeighborID(&neighbors[far_left_dir]	);
	uint8_t far_right	= !!getNeighborID(&neighbors[far_right_dir]	);
	
	if(opp)				return opp_dir;							//go straight.
	else if(right && left){										//both close neighbors exist
		if((far_right && far_left)||(!far_right && !far_left))	//far neighbors symmetrical
							return in_dir;
		else if(far_right)	return right_dir;					//more neighbors on right
		else				return left_dir;					//more neighbors on left
	}else if(!right && !left){									//neither close neighbor exists
		if((far_right && far_left)||(!far_right && !far_left))	//far neighbors symmetrical
							return in_dir;
		else if(far_right)	return far_right_dir;
		else				return far_left_dir;
	}else if(right){
		if(far_right)		return far_right_dir;
		else if(far_left)	return far_left_dir;
		else				return right_dir;
	}else{
		if(far_left)		return far_left_dir;
		else if(far_right)	return far_right_dir;
		else				return left_dir;
	}
}

void calculateOutboundDir(uint16_t inID){
	outwardDirID=0;
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		if(getNeighborID(&neighbors[dir])==inID){
			outwardDir = check_bounce(dir);
			outwardDirID = getNeighborID(&neighbors[outwardDir]);
			break;
		}
	}
	if(outwardDirID==0){
		printf("Got ball from a non-neighbor? But we must continue!\r\n");
		uint8_t randDir = rand_byte()%NEIGHBORHOOD_SIZE;
		uint8_t i;
		for(i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(getNeighborID(&neighbors[(randDir+i)%NEIGHBORHOOD_SIZE])!=0) break;
		}
		outwardDir = check_bounce(randDir+i);
		outwardDirID = getNeighborID(&neighbors[outwardDir]);
	}
}

void sendBallMsg(){
	BallBounceMsg msg;
	msg.flag = BALL_BOUNCE_FLAG;
	msg.id = outwardDirID;
	msg.seqPos = ballSeqPos;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BallBounceMsg));
}

void handle_msg(ir_msg* msg_struct){
	if(((BallBounceMsg*)(msg_struct->msg))->flag==BALL_BOUNCE_FLAG){
		BallBounceMsg msg = *((BallBounceMsg*)(msg_struct->msg));
		if(msg.id==get_droplet_id()&&myState!=BALL){
			//printf("Got the ball!\r\n");
			myState=BALL;
			calculateOutboundDir(msg_struct->sender_ID);
			ballSeqPos = msg.seqPos;
		}else{
		//printf("Someone else getting a ball. Could maybe use this for lost ball handling later.\r\n");
		}
	}
}

OtherBot* getOtherBot(uint16_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
	}
	return NULL;
}

void removeOtherBot(uint16_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			cleanOtherBot(&nearBots[i]);
			numNeighbors--;
			break;
		}
	}
}

OtherBot* addOtherBot(uint16_t id, float var){
	uint8_t emptyIdx=0xFF;
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
		if(nearBots[i].id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		numNeighbors++;
		return &(nearBots[emptyIdx]);
	}
	// No empty spot, so we'll remove the neighbor we're
	// least confident in. But only if we're more confident
	// in the new neighbor.
	BotPos* pos = &nearBots[NUM_TRACKED_BOTS-1].pos;
	float neighbConf = getConfFromVars(pos->rV, pos->bV, pos->hV);
	if(neighbConf<var){
		cleanOtherBot(&nearBots[NUM_TRACKED_BOTS-1]);
		return &(nearBots[NUM_TRACKED_BOTS-1]);
	}else{
		return NULL;
	}
}

void cleanOtherBot(OtherBot* other){
	if(other==NULL) return;
	other->id = 0;
	other->pos.r = 0;
	other->pos.b = 0;
	other->pos.h = 0;
	other->pos.rV = 0;
	other->pos.bV = 0;
	other->pos.hV = 0;
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	if(strcmp_P(command_word,PSTR("ball"))==0){
		uint8_t inDir=rand_byte()%NEIGHBORHOOD_SIZE;
		for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(getNeighborID(&neighbors[inDir+i])!=0){
				inDir+=i;
				break;
			}
		}
		outwardDir = check_bounce(inDir);
		outwardDirID = getNeighborID(&neighbors[outwardDir]);
		printf("GOT BALL COMMAND!\r\n\tInDir: %hu, outDir: %hu, outID: %X\r\n", inDir, outwardDir, outwardDirID);
		myState = BALL;	
		ballSeqPos = 0;
		return 1;
	}
	return 0;
}

//void processOtherBotData(){
	//float dists[NUM_TRACKED_BOTS][NEIGHBORHOOD_SIZE];
	//calculateDistsFromNeighborPos(dists); //Populates this array.
	//
	//NeighbSlot* neighbor;
	//BayesBot* candidates;
	//BayesBot newProbs[NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN];
	//float newEmptyProb, newTotalTrackedProb, newUntrackedProb, total, probFound;
	//uint8_t found, numTracked;
	//for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		//neighbor = &(neighbors[i]);
		//candidates=neighbor->candidates;
		//probFound = 0.0;
		//numTracked=0;
		//newEmptyProb=neighbor->emptyProb;
		//newUntrackedProb=neighbor->untrackedProb;
		//for(uint8_t j=0;j<NUM_TRACKED_BOTS;j++){
			//found=0;
			//newProbs[j].id=0;
			//newProbs[j].P=0.0;
			//for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
				//if(candidates[k].id!=0){
					//numTracked++;
					//if(candidates[k].id==nearBots[j].id){
						//found = 1;
						//newProbs[j].id=candidates[k].id;
						//newProbs[j].P=(4.0/dists[j][i])*candidates[k].P; //bayes-y
						//probFound+=candidates[k].P;
					//}
				//}
			//}
			//if(!found){
				//newProbs[j].id=nearBots[j].id;
				//newProbs[j].P = (2.0/dists[j][i])*((neighbor->untrackedProb)/(NUM_POSSIBLE_BOTS-numTracked));
			//}
			//if(dists[j][i]>5.0){
				//newEmptyProb+=0.05;
				//newUntrackedProb+=0.01;
				//}else{
				//if(newEmptyProb>=0.2)	newEmptyProb+=-0.1;
				//if(newUntrackedProb>=0.04) newUntrackedProb+=-0.02;
			//}
		//}
		////SO FAR, the above covers bots that were tracked and measured, and bots that were measured but not previously tracked.
		////need to also cover bots that were previously tracked, but not measured this time.
		//for(uint8_t j=0 ; j < NUM_TRACKED_BAYESIAN ; j++){
			//newProbs[NUM_TRACKED_BOTS+j].id=0;
			//newProbs[NUM_TRACKED_BOTS+j].P=0.0;
			//uint8_t found=0;
			//for(uint8_t k=0;k<NUM_TRACKED_BOTS;k++){
				//if(newProbs[k].id!=0 && newProbs[k].id==candidates[j].id){
					//found=1;
					//break;
				//}
			//}
			//if(!found){
				////this previously tracked bot was not measured.
				//newProbs[NUM_TRACKED_BOTS+j].id=candidates[j].id;
				//newProbs[NUM_TRACKED_BOTS+j].P=UNMEASURED_NEIGHBOR_LIKELIHOOD*candidates[j].P;
			//}
		//}
		////printf("\tLost prob: %f, new prob: %f\r\n",lostProb,newProb);
		//if(newEmptyProb<0.1) newEmptyProb = 0.1;
		//if(newUntrackedProb<0.1) newUntrackedProb = 0.1;
		//
		//qsort(newProbs, NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN,sizeof(BayesBot), bayesCmpFunc);
		//newTotalTrackedProb=0.0;
		//uint8_t tooSmallFlag = 0;
		//float tooSmallThresh = newUntrackedProb/(NUM_POSSIBLE_BOTS-numTracked);
		//for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			//if(newProbs[j].id!=0){
				//if(newProbs[j].P<=tooSmallThresh){
					//newUntrackedProb+=newProbs[j].P;
					//candidates[j].id=0;
					//candidates[j].P=0.0;
					//}else{
					//candidates[j].id=newProbs[j].id;
					//candidates[j].P=newProbs[j].P;
					//newTotalTrackedProb+=candidates[j].P;
				//}
//
				//}else{
				//candidates[j].id=0;
				//candidates[j].P=0.0;
			//}
		//}
		//
		//total = newTotalTrackedProb+newEmptyProb+newUntrackedProb;
		////printf("\tnewEmptyProb: %f\tnewTotalProb: %f\t newUntrackedProb: %f\ttotal: %f\r\n",newEmptyProb, newTotalTrackedProb,newUntrackedProb, total);
		//neighbor->emptyProb=newEmptyProb/total;
		//neighbor->untrackedProb=newUntrackedProb/total;
		//for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			//if(newProbs[j].id!=0){
				//candidates[j].P=candidates[j].P/total;
			//}
		//}
	//}
	//#ifdef BAYES_DEBUG_MODE
	//bayesDebugPrintout();
	//#endif
	//for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		//cleanOtherBot(&nearBots[i]);
	//}
	//numNeighbors = 0;
//}