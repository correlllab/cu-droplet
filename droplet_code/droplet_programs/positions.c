#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

//#define BAYES_DEBUG_MODE
//#define BAYES_DIST_DEBUG_MODE
//#define NEIGHBS_DEBUG_MODE
//#define POS_DEBUG_MODE
//#define KALMAN_DEBUG_MODE

#ifdef NEIGHBS_DEBUG_MODE
#define NEIGHBS_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define NEIGHBS_DEBUG_PRINT(format, ...)
#endif

#ifdef BAYES_DIST_DEBUG_MODE
#define BAYES_DIST_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define BAYES_DIST_DEBUG_PRINT(format, ...)
#endif

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

#ifdef BAYES_DEBUG_MODE
#define BAYES_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define BAYES_DEBUG_PRINT(format, ...)
#endif

void init(){	
	numNearBots = 0;
	loopCount = 0;
	outwardDir = 0;
	outwardDirID = 0;
	firstLoop = 0;	
	myState=NOT_BALL;	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		bayesSlots[i].emptyProb=INIT_LIKELIHOOD_EMPTY;
		bayesSlots[i].untrackedProb=1.0-INIT_LIKELIHOOD_EMPTY;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN-1;j++){
			bayesSlots[i].candidates[j].P=0.0;
			bayesSlots[i].candidates[j].id=0;
		}
		neighbsList[i].id = 0;
		for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
			neighbsList[i].neighbs[j] = 0;
		}		
	}
	myRNBLoop = (get_droplet_id()%(LOOPS_PER_RNB-1));
	myMsgLoop = ((myRNBLoop+(LOOPS_PER_RNB/2))%(LOOPS_PER_RNB-1));
	printf("RNBLoop: %d\tMsgLoop: %d\r\n", myRNBLoop, myMsgLoop);
}

void loop(){
	if((get_time()%LOOP_PERIOD_MS)<(LOOP_PERIOD_MS/50)){
		if((!firstLoop)&&(loopCount==myRNBLoop)){
			delay_ms(10);
			broadcast_rnb_data();
		}else if((!firstLoop)&&(loopCount==myMsgLoop)){
			sendNeighbMsg();
		}else if((!firstLoop)&&(loopCount==LOOPS_PER_RNB-1)){

			qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);
			printf("\nT: %lu\r\n", get_time());
			#ifdef KALMAN_DEBUG_MODE
				BotPos* pos;						
				printf("  ID  |    R    |   B    |   H    |  CONF  |\r\n{\r\n");

				for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
					if(nearBots[i].id!=0){
						pos = &(nearBots[i].pos);
						printf("{{\"%04X\", % -7.2f , %-6.1f , %-6.1f , %6.1f },{\r\n", nearBots[i].id, pos->r, rad_to_deg(pos->b), rad_to_deg(pos->h), nearBots[i].conf);
					printf("}},\r\n");
				}
			}
				printf("}\r\n\n");
			#endif
			
			processNeighborData();
			processOtherBotData();
			cleanupNeighbsList();		
			printNeighbsList();
			printf("\n\n\n");
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
			uint8_t neighbCount=0;
			for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
				if(neighbsList[dir].id){
					neighbCount++;
				}
			}
			if(neighbCount==0){
				set_rgb(0,0,0);
			}else{
				set_hsv(60*((int16_t)(neighbCount-1)),255,255);
			}

		}
		
		if(loopCount==(LOOPS_PER_RNB-1)){ firstLoop = 0;}
		loopCount=((loopCount+1)%LOOPS_PER_RNB);
		delay_ms(LOOP_PERIOD_MS/50);
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
}

void printNeighbsList(){
	uint16_t id;
	NEIGHBS_DEBUG_PRINT("Slot:  ID       |  0   |  1   |  2   |  3   |  4   |  5   |\r\n");
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		if(neighbsList[i].id){
			NEIGHBS_DEBUG_PRINT("   %hu: %04X      |", i, neighbsList[i].id);
			}else{
			NEIGHBS_DEBUG_PRINT("   %hu:  --       |", i, neighbsList[i].id);
		}
		for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
			id=neighbsList[i].neighbs[j];
			if(id==0){
				NEIGHBS_DEBUG_PRINT("  --  |");
				}else{
				NEIGHBS_DEBUG_PRINT(" %04X |", neighbsList[i].neighbs[j]);
			}
		}
		NEIGHBS_DEBUG_PRINT("\r\n");
	}
}

void processNeighborData(){
	uint8_t opp_dir;
	uint8_t dirP;
	uint8_t dirM;
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE; dir++){
		failureCounts[dir] = 0;
		successCounts[dir] = 0;
		opp_dir = (dir+(NEIGHBORHOOD_SIZE/2))%NEIGHBORHOOD_SIZE;
		dirP = (dir+1)%NEIGHBORHOOD_SIZE;
		dirM = (dir+(NEIGHBORHOOD_SIZE-1))%NEIGHBORHOOD_SIZE;
		
		if(neighbsList[dir].neighbs[opp_dir]){
			if(get_droplet_id()!=neighbsList[dir].neighbs[opp_dir]){//Step 1
				failureCounts[dir]+=12;
			}else{
				successCounts[dir]+=12;
			}
		}
		if(neighbsList[dir].id && neighbsList[dirM].neighbs[dirP]){
			if(neighbsList[dir].id!=neighbsList[dirM].neighbs[dirP]){//step 2
				failureCounts[dirM]+=4;
				failureCounts[dir]+=4;
			}else{
				successCounts[dirM]+=4;
				successCounts[dir]+=4;
			}
		}
		if(neighbsList[dir].id && neighbsList[dirP].neighbs[dirM]){
			if(neighbsList[dir].id!=neighbsList[dirP].neighbs[dirM]){//step 2
				failureCounts[dirP]+=4;
				failureCounts[dir]+=4;
			}else{
				successCounts[dirP]+=4;
				successCounts[dir]+=4;
			}
		}
		if(neighbsList[dirP].neighbs[dirM] && neighbsList[dirM].neighbs[dirP]){
			if(neighbsList[dirP].neighbs[dirM]!=neighbsList[dirM].neighbs[dirP]){//step 3
				failureCounts[dirP]+=3;
				failureCounts[dirM]+=3;
			}else{
				successCounts[dirP]+=3;
				successCounts[dirM]+=3;
			}
		}
		if(neighbsList[dir].neighbs[dirP] && neighbsList[dirP].neighbs[dir]){
			if(neighbsList[dir].neighbs[dirP]!=neighbsList[dirP].neighbs[dir]){ //step 3
				failureCounts[dir]+=3;
				failureCounts[dirP]+=3;
			}else{
				successCounts[dir]+=3;
				successCounts[dirP]+=3;
			}
		}
	}
	NEIGHBS_DEBUG_PRINT(" --- Neighbor Failure/Success Counts --- \r\n");
	NEIGHBS_DEBUG_PRINT(" dir | Failure | Success |\r\n");
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		NEIGHBS_DEBUG_PRINT("   %1hu |   %2hu    |   %2hu    |\r\n", dir, failureCounts[dir], successCounts[dir]);
	}
	NEIGHBS_DEBUG_PRINT("\n");
}

void cleanupNeighbsList(){
	BayesSlot* slot;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		slot = &(bayesSlots[i]);
		if(slot->candidates[0].P > (slot->emptyProb + slot->untrackedProb)){
			if(neighbsList[i].id != slot->candidates[0].id){
				neighbsList[i].id = slot->candidates[0].id;
				for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
					neighbsList[i].neighbs[j] = 0;
				}
			}
		}else{
			neighbsList[i].id = 0;
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				neighbsList[i].neighbs[j] = 0;
			}
		}
	}
}

void calculateDistsFromNeighborPos(float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]){
	BAYES_DIST_DEBUG_PRINT(" --- Calculating Dists From Neighbor Pos --- \r\n");
	BotPos* pos;
	float x, y;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id!=0){
			pos = &(nearBots[i].pos);			
			BAYES_DIST_DEBUG_PRINT("Bot %04X (%f):\r\n", nearBots[i].id, nearBots[i].conf);
			x = (pos->r)*cosf(pos->b+M_PI_2);
			y = (pos->r)*sinf(pos->b+M_PI_2);

			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				dists[j][i] = hypotf(x-neighbPos[j][0],y-neighbPos[j][1]);
				if(dists[j][i]==0.0){
					dists[j][i]=0.01;
				}
				BAYES_DIST_DEBUG_PRINT("\tSlot %hu: %f\r\n",j,dists[j][i]);
			}
		}
	}
}

void addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* slot, uint8_t dir){
	uint8_t numTrackedAndMeasured=0;
	uint8_t numMeasuredNotTracked=0;
	uint8_t numTrackedNotMeasured=0;
	uint8_t numNonadjacent=0;
	uint8_t found;
	uint8_t oppDir = (dir+(NEIGHBORHOOD_SIZE/2))%NEIGHBORHOOD_SIZE;
	BayesBot* candidates = slot->candidates;
	BotPos* pos;
	float confFactor, neighbFactor;
	float constFactor = 3.0;
	float factor;
	BayesBot* newProbs = (newNeighb->candidates);
	float deltaTrackedProb = 0.0;
	float newTrackedTotal = 0.0;
	for(uint8_t j=0;j<NUM_TRACKED_BOTS;j++){
		newProbs[j].id = 0;
		newProbs[j].P  = 0.0;
		if(!nearBots[j].id){
			continue;
		}
		pos = &(nearBots[j].pos);
		found=0;
		
		confFactor = nearBots[j].conf;
		if(confFactor>1.0){
			confFactor = 1.0;
		}else if(confFactor<0.5){
			confFactor = 0.5;		
		}
		
		factor = sqrtf(constFactor/dists[j]);
		if(factor<0.5){
			factor=0.5;
		}else if(factor>5.0){
			factor=5.0;
		}
		
		neighbFactor=1.0;
		if(successCounts[dir]>failureCounts[dir]){
			neighbFactor=4.0;
		}else if(failureCounts[dir]>successCounts[dir]){
			neighbFactor=0.125;
		}
		factor = factor*neighbFactor*confFactor;
		for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
			if(candidates[k].id!=0){
				if(candidates[k].id==nearBots[j].id){
					found = 1;
					numTrackedAndMeasured++;
					if(dists[j]>5.0) numNonadjacent++;
					//A previously tracked robot was measured again.
					newProbs[j].id=candidates[k].id;
					BAYES_DEBUG_PRINT("\t%04X Factor [ f]:  %7.3f  |  %7.3f  |  %7.3f\r\n",  candidates[k].id, factor, neighbFactor, confFactor);
					newProbs[j].P=factor*candidates[k].P; //bayes-y
					deltaTrackedProb += (newProbs[j].P - candidates[k].P);
				}
			}
		}
		if(!found && nearBots[j].id){
			//We measured a robot we weren't tracking.
			numMeasuredNotTracked++;
			if(dists[j]>5.0) numNonadjacent++;
			newProbs[j].id=nearBots[j].id;		
			BAYES_DEBUG_PRINT("\t%04X Factor [!f]:  %7.3f  |  %7.3f  |  %7.3f\r\n", newProbs[j].id, factor, neighbFactor, confFactor);
			newProbs[j].P = factor*((slot->untrackedProb)/(NUM_POSSIBLE_BOTS-(numTrackedAndMeasured+numTrackedNotMeasured)));
			deltaTrackedProb += newProbs[j].P*(1-(1.0/factor));		
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
		if(!found && candidates[j].id){
			//A robot we were tracking was not measured.
			numTrackedNotMeasured++;
			newProbs[NUM_TRACKED_BOTS+j].id=candidates[j].id;
			newProbs[NUM_TRACKED_BOTS+j].P=UNMEASURED_NEIGHBOR_LIKELIHOOD*candidates[j].P;
			deltaTrackedProb += (newProbs[NUM_TRACKED_BOTS+j].P - candidates[j].P);		
			BAYES_DEBUG_PRINT("\t%04X Factor [!m]:  %7.3f  |  %7.3f  |  %7.3f\r\n", candidates[j].id, UNMEASURED_NEIGHBOR_LIKELIHOOD, neighbFactor, confFactor);
		}
	}
	
	float emptyUntrackedRatio = ((float)(numNonadjacent*numNonadjacent))/((float)(NUM_TRACKED_BOTS*NUM_TRACKED_BOTS));
	if(emptyUntrackedRatio<0.1) 
		emptyUntrackedRatio = 0.1;
	else if(emptyUntrackedRatio>0.9) 
		emptyUntrackedRatio = 0.9;
	qsort(newProbs, NEW_PROBS_SIZE,sizeof(BayesBot), bayesCmpFunc);
	newNeighb->emptyProb = 0;
	newNeighb->untrackedProb = 0;
	newTrackedTotal=0.0;
	//float tooSmallThresh = slot->untrackedProb/NUM_POSSIBLE_BOTS;
	for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
		if(newProbs[j].id!=0){
			if(newProbs[j].P<0.001){
				candidates[j].id=0;
				candidates[j].P=0.0;
				newNeighb->emptyProb += emptyUntrackedRatio*newProbs[j].P;
				newNeighb->untrackedProb += (1.0-emptyUntrackedRatio)*newProbs[j].P;
			}else{
				candidates[j].id=newProbs[j].id;
				candidates[j].P=newProbs[j].P;
			}
		}else{
			candidates[j].id=0;
			candidates[j].P=0.0;
		}
		newTrackedTotal+=candidates[j].P;
	}
	float prevTrackedProb = 1.0-(slot->emptyProb+slot->untrackedProb);
	if(prevTrackedProb<=0.05){		
		if(newTrackedTotal<0.3){
			prevTrackedProb = newTrackedTotal;
		}else{
			prevTrackedProb = 0.3;
		}
	}

	newNeighb->emptyProb += slot->emptyProb - emptyUntrackedRatio*deltaTrackedProb;
	newNeighb->untrackedProb += slot->untrackedProb - (1.0-emptyUntrackedRatio)*deltaTrackedProb;
	if(newNeighb->emptyProb<=0){
		newNeighb->emptyProb = 0.1*emptyUntrackedRatio;
	}
	if(newNeighb->untrackedProb<=0){
		newNeighb->untrackedProb = 0.1*(1.0-emptyUntrackedRatio);
	}
	BAYES_DEBUG_PRINT("\tTotal: %6.3f\r\n\tprevTrkdPrb: %6.3f | slotEmpty: %6.3f | slotUntrkd: %6.3f\r\n", 
						newTrackedTotal+newNeighb->emptyProb+newNeighb->untrackedProb, prevTrackedProb, slot->emptyProb, slot->untrackedProb);
	BAYES_DEBUG_PRINT("\tnewTrkdTot : %6.3f | newEmpty : %6.3f | newUntrkd : %6.3f\r\n", newTrackedTotal, newNeighb->emptyProb, newNeighb->untrackedProb);
	newTrackedTotal += newNeighb->emptyProb + newNeighb->untrackedProb;
	slot->emptyProb = newNeighb->emptyProb/newTrackedTotal;
	slot->untrackedProb = newNeighb->untrackedProb/newTrackedTotal;
	
	for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
		candidates[j].P=candidates[j].P/newTrackedTotal;	
	}
}

void processOtherBotData(){
	float dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS];
	char newBayesMemory[sizeof(NewBayesSlot)];
	NewBayesSlot* newBayes = (NewBayesSlot*)(&(newBayesMemory[0]));
	//NewBayesSlot* newBayes = &newBayesMem;
	float total;
	//uint8_t numTracked;
	BayesBot* newCandidates = newBayes->candidates;
	BayesSlot* slot;
	BayesBot* candidates;
	calculateDistsFromNeighborPos(dists); //Populates this array.
	BAYES_DEBUG_PRINT(" --- Processing otherBot Data --- \r\n");
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		slot = &(bayesSlots[i]);
		candidates = slot->candidates;
		BAYES_DEBUG_PRINT("Slot %hu:\r\n", i);
		newBayes->emptyProb = 0.0;
		newBayes->untrackedProb = 0.0;
		addProbsForNeighbor(newBayes, dists[i], slot, i);
	}
	BAYES_DEBUG_PRINT("\n");
	#ifdef BAYES_DEBUG_MODE
		bayesDebugPrintout();
	#endif
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	numNearBots = 0;
}

void bayesDebugPrintout(){
	printf(" --- Bayes Debug Printout --- \r\n");
	BayesSlot* neighbor;
	BayesBot* candidates;
	uint8_t untrackedToggle, emptyToggle,emptyFirst;	
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbor = &(bayesSlots[i]);
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
	printf("\n");
}


void useNewRnbMeas(){
	rnb_updated=0;
	float rVar, bVar, hVar;
	float conf = last_good_rnb.conf;
	if(conf<0.1) conf=0.1;
	getVarsFromConf(conf, &rVar, &bVar, &hVar);
	POS_DEBUG_PRINT("(RNB) ID: %04X\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\t%8.3f\r\n", last_good_rnb.id_number, last_good_rnb.range, rVar, rad_to_deg(last_good_rnb.bearing), bVar*(180./M_PI), rad_to_deg(last_good_rnb.heading), hVar*(180./M_PI), last_good_rnb.conf);	
	OtherBot* neighbor = addOtherBot(last_good_rnb.id_number, conf);
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
		neighbor->conf = getConfFromVars(rVar, bVar, hVar);
		POS_DEBUG_PRINT("\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", pos->r, pos->rV, rad_to_deg(pos->b), pos->bV*(180./M_PI), rad_to_deg(pos->h), pos->hV*(180./M_PI));
	}else{
		printf("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n");
	}
}

uint8_t check_bounce(uint8_t in_dir){
	//note: directions below are relative to the direction from which the ball came in.
	uint8_t opp_dir			= (in_dir+3)%6;
	uint8_t left_dir		= (in_dir+1)%6;
	uint8_t right_dir		= (in_dir+5)%6; //it's like -1
	uint8_t far_left_dir	= (in_dir+2)%6;
	uint8_t far_right_dir	= (in_dir+4)%6; //it's like -2
	
	uint8_t opp			= !!(neighbsList[opp_dir].id);
	uint8_t left		= !!(neighbsList[left_dir].id);
	uint8_t right		= !!(neighbsList[right_dir].id);
	uint8_t far_left	= !!(neighbsList[far_left_dir].id);
	uint8_t far_right	= !!(neighbsList[far_right_dir].id);
	
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
		if(neighbsList[dir].id==inID){
			outwardDir = check_bounce(dir);
			outwardDirID = neighbsList[outwardDir].id;
			break;
		}
	}
	if(outwardDirID==0){
		printf("Got ball from a non-neighbor? But we must continue!\r\n");
		uint8_t randDir = rand_byte()%NEIGHBORHOOD_SIZE;
		uint8_t i;
		for(i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(neighbsList[(randDir+i)%NEIGHBORHOOD_SIZE].id!=0) break;
		}
		outwardDir = check_bounce(randDir+i);
		outwardDirID = neighbsList[outwardDir].id;
	}
}

void sendNeighbMsg(){
	NeighbMsg msg;
	msg.flag = NEIGHB_MSG_FLAG;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		msg.ids[i] = neighbsList[i].id;
	}
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(NeighbMsg));
}

void sendBallMsg(){
	BallMsg msg;
	msg.flag = BALL_MSG_FLAG;
	msg.id = outwardDirID;
	msg.seqPos = ballSeqPos;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BallMsg));
}

void handle_msg(ir_msg* msg_struct){
	if(((BallMsg*)(msg_struct->msg))->flag==BALL_MSG_FLAG){
		BallMsg msg = *((BallMsg*)(msg_struct->msg));
		printf("%04X threw a ball!\r\n", msg_struct->sender_ID);		
		if(msg.id==get_droplet_id()){
			printf("\tIt's for me! (%hu)\r\n", msg.seqPos);
			if(myState!=BALL || msg.seqPos<ballSeqPos){
				if(myState==BALL){
					printf("\tI was a ball, but this is a new ball (%hu) for ", ballSeqPos);
				}else{
					printf("\tNew Ball for ");
				}
				myState=BALL;
				calculateOutboundDir(msg_struct->sender_ID);
				printf("%04X.\r\n", outwardDirID);
				ballSeqPos = msg.seqPos;		
			}
		}else{
			printf("\tIt's for %04X, not me. (%hu)\r\n", msg.id, msg.seqPos);
		//printf("Someone else getting a ball. Could maybe use this for lost ball handling later.\r\n");
		}
	}else if(((NeighbMsg*)(msg_struct->msg))->flag==NEIGHB_MSG_FLAG){
		NeighbMsg msg = *((NeighbMsg*)(msg_struct->msg));
		for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(msg_struct->sender_ID==neighbsList[i].id){
				for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
					neighbsList[i].neighbs[j] = msg.ids[j];
				}
				break;
			}
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
			numNearBots--;
			break;
		}
	}
}

OtherBot* addOtherBot(uint16_t id, float conf){
	uint8_t emptyIdx=0xFF;
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
		if(emptyIdx==0xFF && nearBots[i].id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		numNearBots++;
		return &(nearBots[emptyIdx]);
	}
	// No empty spot, so we'll remove the neighbor we're
	// least confident in. But only if we're more confident
	// in the new neighbor.
	BotPos* pos = &(nearBots[NUM_TRACKED_BOTS-1].pos);
	if(nearBots[NUM_TRACKED_BOTS-1].conf<conf){
		POS_DEBUG_PRINT("No empty spot, but higher conf.\r\n");
		cleanOtherBot(&nearBots[NUM_TRACKED_BOTS-1]);
		return &(nearBots[NUM_TRACKED_BOTS-1]);
	}else{
		POS_DEBUG_PRINT("No empty spot, and conf too low.\r\n");
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
	other->conf = 0.0;
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
			if(neighbsList[inDir+i].id!=0){
				inDir+=i;
				break;
			}
		}
		outwardDir = check_bounce(inDir);
		outwardDirID = neighbsList[outwardDir].id;
		printf("GOT BALL COMMAND!\r\n\tInDir: %hu, outDir: %hu, outID: %X\r\n", inDir, outwardDir, outwardDirID);
		myState = BALL;	
		ballSeqPos = 0;
		return 1;
	}
	return 0;
}