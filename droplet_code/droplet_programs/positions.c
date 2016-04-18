#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

#define BAYES_DEBUG_MODE
#define BAYES_DIST_DEBUG_MODE
#define NEIGHBS_DEBUG_MODE
//#define POS_DEBUG_MODE

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
	//int16_t BM[6][6] = {
		//{  18,  27,  27,   7,   7,  14},
		//{   6,  46,  26,  25,  20,   4},
		//{  13,   0,   0,   8,  21,   0},
		//{  30,   0,  46,   0,   0,   0},
		//{   0,   0,  35,   3,  26,  13},
		//{  38,  34,   0,   0,   4,   0}
	//};
	//printf("!\r\n%f\r\n!\r\n",calculate_innovation(18.8, deg_to_rad(-180.1),deg_to_rad(110.7),BM));
	//busy_delay_ms(5000);
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		//printf("{\r\n");
			//for(uint8_t e=0;e<6;e++){
				//printf("\t{");
					//for(uint8_t s=0;s<6;s++){
						//printf("%5.3f",expected_bright_mat(18.8, deg_to_rad(-180.1), deg_to_rad(110.7), e,s));
						//if(s<5) printf(",");
					//}
				//printf("}");
				//if(e<5) printf(",");
				//printf("\r\n");
			//}
		//printf("};");
		//while(1){
			//busy_delay_ms(20);
		//}
	//}
	
	set_all_ir_powers(256);
	set_sync_blink_duration(100);
	enable_sync_blink(FFSYNC_D+FFSYNC_W);
	numNearBots = 0;
	lastLoop = 0;
	outwardDir = 0;
	outwardDirID = 0;
	frameCount = 0;
	frameStart=get_time();
	myState=NOT_BALL;	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	myX = NAN;
	myY = NAN;
	seedFlag = 0;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(get_droplet_id()==SEED_IDS[i]){
			seedFlag = 1;
			myX = SEED_X[i];
			myY = SEED_Y[i];
			break;
		}
	}
	posColorMode = 0;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		reciprocationTracker[i] = 0;
		bayesSlots[i].emptyProb=INIT_LIKELIHOOD_EMPTY;
		bayesSlots[i].untrackedProb=1.0-INIT_LIKELIHOOD_EMPTY;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN-1;j++){
			bayesSlots[i].candidates[j].P=0.0;
			bayesSlots[i].candidates[j].id=0;
		}
		neighbsList[i].id = 0;
		neighbsList[i].x = 0x8000;
		neighbsList[i].y = 0x8000;
		for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
			neighbsList[i].neighbs[j] = 0;
		}		
	}
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	//mySlot = get_droplet_ord(get_droplet_id());
	//myMsgLoop = ((mySlot+(SLOTS_PER_FRAME/2))%(SLOTS_PER_FRAME-1));
	printf("mySlot: %u, frame_length: %lu\r\n", mySlot,FRAME_LENGTH_MS);
}

void loop(){

	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	uint16_t loopID = frameTime/SLOT_LENGTH_MS;
	//printf("frameTime: %lu, loop: %u\r\n", frameTime, loopID);	
	if(loopID!=lastLoop){
		if(loopID==mySlot){
			delay_ms(8+(rand_byte()>>3));
			broadcast_rnb_data();
			delay_ms(40);
			sendNeighbMsg();
		}else if(loopID==SLOTS_PER_FRAME-1){
			//get_ir_baselines();
			qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);
			printf("\nT: %lu\t(%f, %f)\r\n", get_time(), myX, myY);
			processNeighborData();
			processOtherBotData();
			updateNeighbsList();		
			printNeighbsList();
			if(!seedFlag){
				updatePos();
			}
			printf("\n\n\n");		
		}
		setColor();
		lastLoop = loopID;		
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
	delay_ms(LOOP_DELAY_MS);
}

void updatePos(){
	float xSum = 0.0;
	float ySum = 0.0;
	uint8_t numSummed=0;
	uint8_t opp_dir;
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		if(neighbsList[dir].id){
			if((neighbsList[dir].x!=0x8000)&&(neighbsList[dir].y!=0x8000)){
				opp_dir = getOppDir(dir);
				if(neighbsList[dir].neighbs[opp_dir]==get_droplet_id()){
					xSum += (neighbsList[dir].x + 10*neighbPos[opp_dir][0]);
					ySum += (neighbsList[dir].y + 10*neighbPos[opp_dir][1]);
					numSummed++;
				}

			}
		}
	}
	if(numSummed){
		myX = xSum/numSummed;
		myY = ySum/numSummed;
	}else{
		myX = NAN;
		myY = NAN;
	}
}

void setColor(){
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
		if(posColorMode){
			if(isnanf(myX)||isnanf(myY)){
				set_rgb(50,50,50);
			}else{
				int16_t xColVal = 255.0*myX/(X_MAX_DIM*1.0);
				int16_t yColVal = 255.0*myY/(Y_MAX_DIM*1.0);
				uint8_t rColor = (uint8_t)(xColVal);
				uint8_t gColor = (uint8_t)(yColVal);
				set_rgb(rColor,gColor,0);
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
				set_hsv(60*((int16_t)(neighbCount-1)),255,127);
			}
		}

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
		if(neighbsList[i].x!=0x8000){
			NEIGHBS_DEBUG_PRINT(" (%d,", neighbsList[i].x);
		}else{
			NEIGHBS_DEBUG_PRINT(" ( --- ,");
		}
		if(neighbsList[i].y!=0x8000){
			NEIGHBS_DEBUG_PRINT(" %d)", neighbsList[i].y);
			}else{
			NEIGHBS_DEBUG_PRINT(" --- )");
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
		opp_dir = getOppDir(dir);
		dirP = (dir+1)%NEIGHBORHOOD_SIZE;
		dirM = (dir+(NEIGHBORHOOD_SIZE-1))%NEIGHBORHOOD_SIZE;
		
		if(neighbsList[dir].neighbs[opp_dir]){
			if(get_droplet_id()==neighbsList[dir].neighbs[opp_dir]){//Step 1
				successCounts[dir]+=12;
				//increase probability that dir is occupied by neighbsList[dir].neighbs[opp_dir]
			}else{
				failureCounts[dir]+=12;
				//decrease probability that dir is occupied by neighbsList[dir].neighbs[opp_dir]
			}
		}
		if(neighbsList[dirM].neighbs[dirP]){
			if(neighbsList[dir].id){
				if(neighbsList[dir].id!=neighbsList[dirM].neighbs[dirP]){//step 2
					failureCounts[dirM]+=4;
					failureCounts[dir]+=4;
				}else{
					successCounts[dirM]+=4;
					successCounts[dir]+=4;
				}
			}else{
				//increase probability that dir is occupied by neighbsList[dirM].neighbs[dirP]
			}
		}
		if(neighbsList[dirP].neighbs[dirM]){
			if(neighbsList[dir].id){
				if(neighbsList[dir].id!=neighbsList[dirP].neighbs[dirM]){//step 2
					failureCounts[dirP]+=4;
					failureCounts[dir]+=4;
				}else{
					successCounts[dirP]+=4;
					successCounts[dir]+=4;
				}
			}else{
				//increase probability that dir is occupied by neighbsList[dirP].neighbs[dirM]
			}
		}
		if(neighbsList[dirP].neighbs[dirM] && neighbsList[dirM].neighbs[dirP]){
			if(neighbsList[dirP].neighbs[dirM]!=neighbsList[dirM].neighbs[dirP]){//step 3
				failureCounts[dirP]+=3;
				failureCounts[dirM]+=3;
				//nerf probability back down (as if unaffected by previous two outer 'if's)
			}else{
				successCounts[dirP]+=3;
				successCounts[dirM]+=3;
				//Further increase probability that dir is occupied by neighbsList[dirP].neighbs[dirM]
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
	#ifdef NEIGHBS_DEBUG_MODE
		printNeighborCountResults();
	#endif
}

void printNeighborCountResults(){
	NEIGHBS_DEBUG_PRINT(" --- Neighbor Failure/Success Counts --- \r\n");
	NEIGHBS_DEBUG_PRINT(" dir | Failure | Success |\r\n");
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		NEIGHBS_DEBUG_PRINT("   %1hu |   %2hu    |   %2hu    |\r\n", dir, failureCounts[dir], successCounts[dir]);
	}
	NEIGHBS_DEBUG_PRINT("\n");
}

void updateNeighbsList(){
	BayesSlot* slot;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		slot = &(bayesSlots[i]);
		if(slot->candidates[0].P > (slot->emptyProb + slot->untrackedProb)){
			if(neighbsList[i].id != slot->candidates[0].id){
				neighbsList[i].id = slot->candidates[0].id;
				for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
					neighbsList[i].neighbs[j] = 0;
				}
				neighbsList[i].x = 0x8000;
				neighbsList[i].y = 0x8000;
			}
		}else{
			neighbsList[i].id = 0;
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				neighbsList[i].neighbs[j] = 0;
			}
			neighbsList[i].x = 0x8000;
			neighbsList[i].y = 0x8000;			
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

float calculateFactor(float dist, float conf, uint8_t dir, uint16_t id, float* dfP, float* nfP){
	float factor, distFactor, neighbFactor;
	
	distFactor = sqrtf(4.0/dist);
	if(conf<=5.0){
		if(distFactor<0.5){
			distFactor = 0.5;
		}else if(distFactor>1.5){
			distFactor = 1.5;
		}
	}else if(conf>100.0){
		if(distFactor<0.25){
			distFactor=0.25;
		}else if(distFactor>8.0){
			distFactor=8.0;
		}
	}else{
		if(distFactor<0.4){
			distFactor=0.4;
		}else if(distFactor>4.0){
			distFactor=4.0;
		}
	}
	
	if(conf>300){
		factor = distFactor*distFactor;
	}else{
		factor = distFactor;
	}
	
	neighbFactor=1.0;
	if(id==(neighbsList[dir].id)){
		if(successCounts[dir]>failureCounts[dir]){
			neighbFactor=5.0;
		}else if(failureCounts[dir]>successCounts[dir]){
			neighbFactor=0.1;
		}
	}
	factor = factor*neighbFactor;
	
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		if(reciprocationTracker[i] == id){
			factor = factor*2.0;
			reciprocationTracker[i] = 0;
			break;
		}
	}
	
	//factor = distFactor*neighbFactor*confFactor;
	//BAYES_DEBUG_PRINT("%7.3f\t(%7.3f, %7.3f, %7.3f)",  factor, distFactor, neighbFactor, conf);
	(*dfP) =  distFactor;
	(*nfP) = neighbFactor;
	return factor;
}

void addProbsForNeighbor(NewBayesSlot* newNeighb, float dists[NUM_TRACKED_BOTS], BayesSlot* slot, uint8_t dir){
	uint8_t numTrackedAndMeasured=0;
	uint8_t numMeasuredNotTracked=0;
	uint8_t numTrackedNotMeasured=0;
	uint8_t numNonadjacent=0;
	uint8_t found;
	BayesBot* candidates = slot->candidates;
	//BotPos* pos;
	BayesBot* newProbs = (newNeighb->candidates);
	float deltaTrackedProb = 0.0;
	float newTrackedTotal = 0.0;
	float factor, distFactor, neighbFactor;
	for(uint8_t j=0;j<NUM_TRACKED_BOTS;j++){
		newProbs[j].id = 0;
		newProbs[j].P  = 0.0;
		if(!nearBots[j].id){
			continue;
		}

		factor = calculateFactor(dists[j], nearBots[j].conf, dir, nearBots[j].id, &distFactor, &neighbFactor);
		//pos = &(nearBots[j].pos);
		found=0;
		for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
			if(candidates[k].id!=0){
				if(candidates[k].id==nearBots[j].id){
					found = 1;
					numTrackedAndMeasured++;
					if(dists[j]>(GRID_DIMENSION/2)) numNonadjacent++;
					//A previously tracked robot was measured again.
					newProbs[j].id=candidates[k].id;
					BAYES_DEBUG_PRINT("\t%04X: %7.3f\t(%7.3f, %7.3f) -> [ f] <-\r\n", nearBots[j].id, factor, distFactor, neighbFactor);
					newProbs[j].P=factor*candidates[k].P; //bayes-y
					deltaTrackedProb += (newProbs[j].P - candidates[k].P);
				}
			}
		}
		if(!found && nearBots[j].id){
			//We measured a robot we weren't tracking.
			numMeasuredNotTracked++;
			if(dists[j]>(GRID_DIMENSION/2)) numNonadjacent++;
			newProbs[j].id=nearBots[j].id;		
			BAYES_DEBUG_PRINT("\t%04X: %7.3f\t(%7.3f, %7.3f) -> [!f] <-\r\n", nearBots[j].id, factor, distFactor, neighbFactor);
			newProbs[j].P = factor*((slot->untrackedProb)/(NUM_POSSIBLE_BOTS-(numTrackedAndMeasured+numTrackedNotMeasured)));
			deltaTrackedProb += newProbs[j].P*(1-(1.0/factor));		
		}
	}
	for(uint8_t j=0 ; j < NUM_TRACKED_BAYESIAN ; j++){
		newProbs[NUM_TRACKED_BOTS+j].id=0;
		newProbs[NUM_TRACKED_BOTS+j].P=0.0;
		found = 0;
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
			BAYES_DEBUG_PRINT("\t%04X: %7.3f\t -> [ m] <-\r\n", candidates[j].id, UNMEASURED_NEIGHBOR_LIKELIHOOD);
		}
	}

	/* ~ ~ ~ ~ ~ ~ ~ ! ~ ~ ~ ~ ~ ~ ~ */
	
	float emptyUntrackedRatio = ((float)(numNonadjacent*numNonadjacent))/((float)(NUM_TRACKED_BOTS*NUM_TRACKED_BOTS));
	if(emptyUntrackedRatio<0.1) 
		emptyUntrackedRatio = 0.1;
	else if(emptyUntrackedRatio>0.9) 
		emptyUntrackedRatio = 0.9;
	qsort(newProbs, NEW_PROBS_SIZE, sizeof(BayesBot), bayesCmpFunc);
	newNeighb->emptyProb = 0;
	newNeighb->untrackedProb = 0;
	newTrackedTotal=0.0;
	for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
		float totalPThisID;
		if(newProbs[j].id && newProbs[j].P>0.25){
			totalPThisID = newProbs[j].P;
			for(uint8_t k=j+1;k<NUM_TRACKED_BAYESIAN;k++){
				if(newProbs[j].id==newProbs[k].id && newProbs[k].P>0.25){
					totalPThisID += newProbs[k].P;
				}
			}
			newProbs[j].P = newProbs[j].P/totalPThisID;
			for(uint8_t k=j+1;k<NUM_TRACKED_BAYESIAN;k++){
				if(newProbs[j].id==newProbs[k].id && newProbs[k].P>0.25){
					newProbs[k].P = newProbs[k].P/(1+totalPThisID);
				}
			}
		}
	}
	//float tooSmallThresh = slot->untrackedProb/NUM_POSSIBLE_BOTS;
	for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
		if(newProbs[j].id!=0){
			if(newProbs[j].P<0.02){
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
	BayesSlot* slot;
	calculateDistsFromNeighborPos(dists); //Populates this array.
	BAYES_DEBUG_PRINT(" --- Processing otherBot Data --- \r\n");
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		slot = &(bayesSlots[i]);
		//candidates = slot->candidates;
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
	float conf = last_good_rnb.conf;
	if(conf<0.1) conf=0.1;
	POS_DEBUG_PRINT("(RNB) ID: %04X\r\n\tRmo: % 5.1f Bmo: % 6.1f Hmo: % 6.1f | %8.3f\r\n", last_good_rnb.id_number, last_good_rnb.range, rad_to_deg(last_good_rnb.bearing), rad_to_deg(last_good_rnb.heading), last_good_rnb.conf);	
	OtherBot* neighbor = addOtherBot(last_good_rnb.id_number, conf);
	BotPos* pos = &(neighbor->pos);
	if(neighbor==NULL) return;
	if(neighbor->id == 0 || neighbor->id == last_good_rnb.id_number){ //We weren't tracking this ID before, so just add the new info.
		neighbor->id		= last_good_rnb.id_number;
		pos->r	= last_good_rnb.range;
		pos->b	= last_good_rnb.bearing;
		pos->h	= last_good_rnb.heading;
		neighbor->conf = last_good_rnb.conf;
		//POS_DEBUG_PRINT("\tRmo: % 5.1f Bmo: % 6.1f Hmo: % 6.1f\r\n", pos->r, rad_to_deg(pos->b), rad_to_deg(pos->h));
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
	if(!isnanf(myX)){
		msg.x = (int16_t)myX;		
	}else{
		msg.x = 0x8000;
	}
	if(!isnanf(myY)){
		msg.y = (int16_t)myY;
	}else{
		msg.y = 0x8000;
	}
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
		for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			if(reciprocationTracker[dir]==msg_struct->sender_ID){
				reciprocationTracker[dir] = 0;
				break;
			}
		}
		uint8_t found=0;
		for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(msg_struct->sender_ID==neighbsList[i].id){
				neighbsList[i].x = msg.x;
				neighbsList[i].y = msg.y;
				for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
					neighbsList[i].neighbs[j] = msg.ids[j];
				}
				found=1;
				break;
			}
		}
		if(!found){
			for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
				if(msg.ids[dir]==get_droplet_id()){
					reciprocationTracker[getOppDir(dir)] = msg_struct->sender_ID;
					found = 1;
					break;
				}
			}
		}
	}else{
		printf("From %04X:\r\n\t",msg_struct->sender_ID);
		for(uint8_t i=0;i<msg_struct->length;i++){
			printf("%c",msg_struct->msg[i]);
		}
		printf("\r\n");
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
	//BotPos* pos = &(nearBots[NUM_TRACKED_BOTS-1].pos);
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
	}else if(strcmp_P(command_word,PSTR("pcm"))==0){
		if(command_args[0]=='e'){
			posColorMode = 1;
		}else if(command_args[0]=='d'){
			posColorMode = 0;
		}else{
			posColorMode = !posColorMode;
		}
		if(posColorMode){
			printf("posColorMode enabled.\r\n");
		}else{
			printf("posColorMode disabled.\r\n");
		}
		return 1;
	}
	return 0;
}