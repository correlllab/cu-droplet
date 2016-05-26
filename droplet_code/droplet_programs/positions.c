#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

#define BAYES_DEBUG_MODE
//#define BAYES_DIST_DEBUG_MODE
#define NEIGHBS_DEBUG_MODE
//#define POS_DEBUG_MODE

#ifdef NEIGHBS_DEBUG_MODE
#define NEIGHBS_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define NEIGHBS_DEBUG_PRINT(format, ...)
#endif

#ifdef BAYES_DIST_DEBUG_MODE
#define BAYES_DIST_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define BAYES_DIST_DEBUG_PRINT(format, ...)
#endif

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

#ifdef BAYES_DEBUG_MODE
#define BAYES_DEBUG_PRINT(format, ...) printf_P(PSTR(format), ##__VA_ARGS__)
#else
#define BAYES_DEBUG_PRINT(format, ...)
#endif

void init(){
	set_all_ir_powers(256);
	//set_sync_blink_duration(100);
	//enable_sync_blink(FFSYNC_D+FFSYNC_W);
	numNearBots = 0;
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	myState = PIXEL; //default state
	colorMode = OFF;
	gameMode = PONG;
	lastBallID = 0;
	lastBallMsg = 0;
	lastPaddleMsg = 0;
	eastWestCount = 0;
	northSouthCount = 0;
	lastLightCheck = get_time();
	initPositions();
	initBayesDataStructs();
	//mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	mySlot = get_droplet_ord(get_droplet_id());
	//myMsgLoop = ((mySlot+(SLOTS_PER_FRAME/2))%(SLOTS_PER_FRAME-1));
	printf("mySlot: %u, frame_length: %lu\r\n", mySlot, FRAME_LENGTH_MS);
}

void handleMySlot(){
	uint32_t before = get_time();
	broadcast_rnb_data();
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR) delay_us(500);
	if(myState>=3 && myState!=CTRL_UNKNWN && paddleChange>=1.0){
		sendPaddleMsg();
	}else{
		paddleChange = 0.0;
	}
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<(RNB_DUR+PADDLE_MSG_DUR)) delay_us(500);
	sendNeighbMsg();
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<(RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR)) delay_us(500);
	if(myState<2 && myDist!=0x7FFF && myDist<30){
		sendBallMsg();
	}
	while((get_time()-before)<NEIGHB_MSG_DUR) delay_us(500);
}

void handleFrameEnd(){
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsCmpFunc);
	processOtherBotData();
	updateNeighbsList();
	updateState();
	processNeighborData(1);
	if(!seedFlag){
		updatePos();
	}
	frameEndPrintout();
	printNeighbsList();
	printf("\r\n");
}

void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){		
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){
			uint32_t before = get_time();
			handleMySlot();
			printf("My slot processing took %lu ms.\r\n", get_time()-before);
		}else if(loopID==SLOTS_PER_FRAME-1){
			uint32_t before = get_time();
			handleFrameEnd();
			printf("End of Frame Processing/Printing Took %lu ms.\r\n",get_time()-before);
		}
		if(loopID!=mySlot){
			uint32_t curSlotTime = (get_time()-frameStart)%SLOT_LENGTH_MS;
			if(myState>=3 && myState!=CTRL_UNKNWN && paddleChange>=1.0){	
				schedule_task((RNB_DUR-curSlotTime), sendPaddleMsg, NULL);
			}else{
				paddleChange = 0.0;
			}
			if(myState<=2 && myDist!=0x7FFF && myDist<30){
				schedule_task(((RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR)-curSlotTime), sendBallMsg, NULL);
			}
		}
		updateBall();
		updateColor();
	}
	//These things happen every single loop: once every LOOP_DELAY_MS.
	if(myState>=3){
		checkLightLevel();
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
	delay_ms(LOOP_DELAY_MS);
}

void processOtherBotData(){
	uint16_t dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS];
	char newBayesMemory[sizeof(NewBayesSlot)];
	NewBayesSlot* newBayes = (NewBayesSlot*)(&(newBayesMemory[0]));
	BayesSlot* slot;
	calculateDistsFromNeighborPos(dists); //Populates this array.
	BAYES_DEBUG_PRINT(" --- Processing otherBot Data --- \r\n");
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		slot = &(bayesSlots[dir]);
		//candidates = slot->candidates;
		BAYES_DEBUG_PRINT("Slot %hu:\r\n", dir);
		newBayes->emptyProb = 0.0;
		newBayes->untrackedProb = 0.0;
		addProbsForNeighbor(newBayes, dists[dir], slot, dir);
	}
	BAYES_DEBUG_PRINT("\n");
	//#ifdef BAYES_DEBUG_MODE
	//bayesDebugPrintout();
	//#endif
	for(uint8_t dir=0;dir<NUM_TRACKED_BOTS;dir++){
		cleanOtherBot(&nearBots[dir]);
	}
	eastWestCount   = (eastWestCount>0) ? 1 : ((eastWestCount<0) ? -1 : 0);
	northSouthCount = (northSouthCount>0) ? 1 : ((northSouthCount<0) ? -1 : 0);
	numNearBots = 0;
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
				neighbsList[i].x = 0x7FFF;
				neighbsList[i].y = 0x7FFF;
				noRecipPenalty[i] = 1.0;
			}
		}else{
			neighbsList[i].id = 0;
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				neighbsList[i].neighbs[j] = 0;
			}
			neighbsList[i].x = 0x7FFF;
			neighbsList[i].y = 0x7FFF;
		}
	}
}

void updateState(){
	const uint8_t eNeighbFound = ((neighbsList[0].id) || (neighbsList[1].id) || (neighbsList[2].id));
	//const uint8_t wNeighbFound = (neighbsList[3].id) || (neighbsList[4].id) || (neighbsList[5].id);
	const uint8_t sNeighbFound = (neighbsList[2].id) || (neighbsList[3].id) ||
									((neighbsList[1].neighbs[2])) || ((neighbsList[1].neighbs[3])) ||
									((neighbsList[4].neighbs[2])) || ((neighbsList[4].neighbs[3]));
	const uint8_t nNeighbFound = (neighbsList[5].id) || (neighbsList[0].id) ||
									((neighbsList[1].neighbs[5])) || ((neighbsList[1].neighbs[0])) ||
									((neighbsList[4].neighbs[5])) || ((neighbsList[4].neighbs[0]));
	uint8_t neighbCount = 0;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
			neighbCount += !!neighbsList[i].id;
	}
	
	if(neighbCount==0){
		#ifdef AUDIO_DROPLET //This feels like CHEATING.
			if(northSouthCount>0 && eastWestCount>0){
				myState = CTRL_SW;
			}else if(northSouthCount>0 && eastWestCount<0){
				myState = CTRL_SE;
			}else if(northSouthCount<0 && eastWestCount>0){
				myState = CTRL_NW;
			}else if(northSouthCount<0 && eastWestCount<0){
				myState = CTRL_NE;
			}else{
				myState = CTRL_UNKNWN;
			}
		#else
			myState = PIXEL;
		#endif
	}else if(!sNeighbFound && nNeighbFound){
		myState = PIXEL_S;
	}else if(sNeighbFound && !nNeighbFound){
		myState = PIXEL_N;
	}else{
		myState = PIXEL;
	}
}

int16_t processNeighborData(uint8_t update){
	uint8_t opp_dir;
	uint8_t dirP;
	uint8_t dirM;
	
	int16_t totalConsistency=0;
	if(update){
		for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			consistency[dir] = 0;
		}
	}
	
	uint16_t tgtID;
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE; dir++){
		opp_dir = getOppDir(dir);
		dirP = (dir+1)%NEIGHBORHOOD_SIZE;
		dirM = (dir+(NEIGHBORHOOD_SIZE-1))%NEIGHBORHOOD_SIZE;
		tgtID = neighbsList[dir].id;
		if(tgtID){
			for(uint8_t nDir=0;nDir<NEIGHBORHOOD_SIZE; nDir++){
				if(neighbsList[nDir].id){
					for(uint8_t nnDir=0;nnDir<NEIGHBORHOOD_SIZE; nnDir++){
						if(tgtID == neighbsList[nDir].neighbs[nnDir]){
							if(nDir==dirM && nnDir==dirP){
								consistency[dir] += (update);
								consistency[nDir] += (update);
								totalConsistency+=2;
							}else if(nDir==dirP && nnDir==dirM){
								consistency[dir] += (update);
								consistency[nDir] += (update);
								totalConsistency+=2;
							}else{
								consistency[dir] -= (update);
								consistency[nDir] -= (update);
								totalConsistency-=2;
							}
						}
					}
				}
			}
			if(neighbsList[dir].neighbs[opp_dir]){
				if(neighbsList[dir].neighbs[opp_dir]==get_droplet_id()){
					consistency[dir] += update + update;
					totalConsistency +=2;
					if(update){
						noRecipPenalty[dir] = 1.0;
					}
				}else{
					consistency[dir] -= (update + update);
					totalConsistency -=2;
				}
			}else{ //my neighbor hasn't added me as a neighbor back.
				if(update){
					noRecipPenalty[dir] *= 0.95;
				}
			}
		}
	}
	return totalConsistency;
}

void updateBall(){
	if(theBall.lastUpdate){
		uint32_t now = get_time();
		int32_t timePassed = now-theBall.lastUpdate;
		int8_t crossedBefore = checkBallCrossed();
		theBall.xPos += (((int32_t)(theBall.xVel))*timePassed)/1000.0;
		theBall.yPos += (((int32_t)(theBall.yVel))*timePassed)/1000.0;
		int8_t crossedAfter = checkBallCrossed();
		theBall.lastUpdate = now;
		if(!(0x7FFF==myX||0x7FFF==myY||isnanf(theBall.xPos)||isnanf(theBall.yPos))){
			myDist = (uint16_t)hypotf(theBall.xPos-myX,theBall.yPos-myY);
		}else{
			myDist = 0x7FFF;
		}
		if(myDist!=0x7FFF && myDist<=30 && crossedBefore!=crossedAfter){ //BOUNCE CHECK
			if((myState==PIXEL_S && theBall.yVel<0) || (myState==PIXEL_N && theBall.yVel>0)){
				if(gameMode==PONG && ((theBall.xPos+theBall.radius)>=paddleStart && (theBall.xPos-theBall.radius)<=paddleEnd)){
					check_bounce(theBall.xVel, theBall.yVel, &(theBall.xVel), &(theBall.yVel));
				}else{
					killBall();
				}
			}
		}else{
			if(theBall.xPos<(MIN_DIM-30)){
				theBall.xVel = abs(theBall.xVel);
				printf("Ball hit lower x boundary.\r\n");
			}else if(theBall.xPos>(X_MAX_DIM+30)){
				theBall.xVel = -abs(theBall.xVel);
				printf("Ball hit upper x boundary.\r\n");
			}
			if(theBall.yPos<(MIN_DIM-30)){
				theBall.yVel = abs(theBall.yVel);
				printf("Ball hit lower y boundary.\r\n");
			}else if(theBall.yPos>(Y_MAX_DIM+30)){
				theBall.yVel = -abs(theBall.yVel);
				printf("Ball hit upper y boundary.\r\n");
			}
		}
	}
}

void updatePos(){
	int32_t xSum = 0;
	int32_t ySum = 0;
	uint8_t numSummed=0;
	uint8_t opp_dir;
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		if(neighbsList[dir].id){
			if((neighbsList[dir].x!=0x7FFF)&&(neighbsList[dir].y!=0x7FFF)){
				opp_dir = getOppDir(dir);
				if(neighbsList[dir].neighbs[opp_dir]==get_droplet_id()){
					xSum += (neighbsList[dir].x + neighbPos[opp_dir][0]);
					ySum += (neighbsList[dir].y + neighbPos[opp_dir][1]);
					numSummed++;
				}
				
			}
		}
	}
	if(numSummed){
		myX = xSum/numSummed;
		myY = ySum/numSummed;
	}else{
		myX = 0x7FFF;
		myY = 0x7FFF;
	}
}

void updateColor(){
	uint8_t newR = 0, newG = 0, newB = 0;
	if(colorMode==POS){
		if(myX==0x7FFF||myY==0x7FFF){
			newR = newG = newB = 50;
		}else{
			int16_t xColVal = (int16_t)(6.0*pow(41.0,(myX-30)/((X_MAX_DIM-60)*1.0))+9.0);
			int16_t yColVal = (int16_t)(3.0*pow(84.0,(myY-30)/((Y_MAX_DIM-60)*1.0))+3.0);
			newR = (uint8_t)(xColVal);
			newG = (uint8_t)(yColVal);
		}
	}else if(colorMode==NEIGHB){
		uint8_t neighbCount=0;
		for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			if(neighbsList[dir].id){
				neighbCount++;
			}
		}
		if(neighbCount>0){
			hsv_to_rgb(60*((int16_t)(neighbCount-1)),255,127,&newR,&newG,&newB);
		}else{
			newR = newG = newB = 0;
		}
	}else if(colorMode==SYNC_TEST){
		if((loopID/6)%2==0){
			hsv_to_rgb((uint16_t)(60*(loopID%6)), 200, 127, &newR, &newG, &newB);
		}else{
			float value = (frameCount%6)/6.0;
			uint8_t val = (uint8_t)(5.0*pow(51.0,value));
			hsv_to_rgb(0,5,(frameCount%3)*(frameCount%3)*60,&newR, &newG, &newB);
		}
	}
	if(!(0x7FFF==myX || 0x7FFF==myY)){
		float coverage = getBallCoverage() + getPaddleCoverage();
		coverage = (coverage > 1.0) ? 1.0 : coverage;
		uint8_t intensityIncrease = 0;
		if(coverage>0.01){
			intensityIncrease = (uint16_t)(5.0*pow(51.0,coverage));
		}
		uint16_t newRed		= newR + intensityIncrease;
		uint16_t newGreen	= newG + intensityIncrease;
		uint16_t newBlue	= newB + intensityIncrease;
		newR = newRed>255 ? 255 :  newRed;
		newG = newGreen>255 ? 255 : newGreen;
		newB = newBlue>255 ? 255 : newBlue;
	}
	set_rgb(newR, newG, newB);	
}

float getBallCoverage(){
	float ballCoveredRatio = 0.0;
	if(myDist!=0x7FFF && myDist<(HALF_BOT+theBall.radius) && theBall.id!=0x0F){
		if(theBall.radius<HALF_BOT){
			if(myDist>=(HALF_BOT-theBall.radius)){
				ballCoveredRatio = getCoverageRatioA(theBall.radius, myDist);
			}else{
				ballCoveredRatio = 1.0;
			}
		}else{
			if(myDist>=(theBall.radius-HALF_BOT)){
				ballCoveredRatio = getCoverageRatioB(theBall.radius, myDist);
			}else{
				ballCoveredRatio = 1.0;
			}
		}
	}
	return ballCoveredRatio;
	//printf("Ball Coverage:\t%f | me: (%5.1f, %5.1f) ball: (%5.1f, %5.1f)->%hu\r\n", ballCoveredRatio, myX, myY, theBall.xPos, theBall.yPos, theBall.radius);	
}

float getPaddleCoverage(){
	float paddleCoveredRatio = 0.0;
	if(gameMode==PONG && (myState==PIXEL_S || myState==PIXEL_N)){
		int16_t myStart = myX-X_OFFSET;
		int16_t myEnd   = myX+X_OFFSET;
		if(myEnd>paddleStart && paddleEnd>myStart){ //otherwise, no intersection
			if(myEnd>paddleEnd){
				if(myStart>paddleStart){
					paddleCoveredRatio = (1.0*(paddleEnd-myStart))/(1.0*(myEnd-myStart));
				}else{
					paddleCoveredRatio = (1.0*(paddleEnd-paddleStart))/(1.0*(myEnd-myStart));
				}
			}else{
				if(myStart>paddleStart){
					paddleCoveredRatio = 1.0; //my end-myStart
				}else{
					paddleCoveredRatio = (1.0*(myEnd-paddleStart))/(1.0*(myEnd-myStart));
				}
			}
		}
	}
	return paddleCoveredRatio;
	//printf("Paddle Coverage:\t%f | me: (%5.1f, %5.1f) ball: (%5.1f, %5.1f)->%hu\r\n", paddleCoveredRatio, myX, myY, theBall.xPos, theBall.yPos, theBall.radius);	
}

void calculateDistsFromNeighborPos(uint16_t dists[NEIGHBORHOOD_SIZE][NUM_TRACKED_BOTS]){
	BAYES_DIST_DEBUG_PRINT(" --- Calculating Dists From Neighbor Pos --- \r\n");
	BotPos* pos;
	int16_t x, y;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id!=0){
			pos = &(nearBots[i].pos);
			BAYES_DIST_DEBUG_PRINT("Bot %04X (%f):\r\n", nearBots[i].id, nearBots[i].conf);
			x = (int16_t)((10.0*pos->r)*cosf(pos->b+M_PI_2));
			y = (int16_t)((10.0*pos->r)*sinf(pos->b+M_PI_2));

			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				dists[j][i] = (uint16_t)hypot(x-neighbPos[j][0],y-neighbPos[j][1]);
				if(dists[j][i]==0){
					dists[j][i]=1;
				}
				BAYES_DIST_DEBUG_PRINT("\tSlot %hu: %u mm\r\n",j,dists[j][i]);
			}
		}
	}
}

float calculateFactor(uint16_t dist, float conf, uint8_t dir, uint16_t id, float* dfP, float* nfP){
	float factor, distFactor, neighbFactor;
	
	distFactor = sqrtf(35.0/dist);
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
		if(consistency[dir]>0){
			neighbFactor=5.0;
		}else if(consistency[dir]<0){
			neighbFactor=0.1;
		}
	}
	factor = factor*neighbFactor;

	if(!(myX == 0x7FFF || myY == 0x7FFF) && (neighbPos)){
		int16_t otherX = myX + neighbPos[dir][0];
		int16_t otherY = myY + neighbPos[dir][1];
		if(otherX<MIN_DIM || otherX > X_MAX_DIM || otherY<MIN_DIM || otherY>Y_MAX_DIM){
			factor = factor/4;
		}
	}
	
	if(neighbsList[dir].id==id && noRecipPenalty[dir]<1.0){
		printf_P(PSTR("\tNo recip for %04X in dir %hu: %f\r\n"), id, dir, noRecipPenalty[dir]);
	}
	factor = factor*noRecipPenalty[dir];
	//factor = distFactor*neighbFactor*confFactor;
	(*dfP) =  distFactor;
	(*nfP) = neighbFactor;
	return factor;
}


//CHANGED DISTS TO UINT16_T and MMs!!!!! finish propogating.
void addProbsForNeighbor(NewBayesSlot* newNeighb, uint16_t dists[NUM_TRACKED_BOTS], BayesSlot* slot, uint8_t dir){
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
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		newProbs[i].id = 0;
		newProbs[i].P  = 0.0;
		if(!nearBots[i].id){
			continue;
		}

		factor = calculateFactor(dists[i], nearBots[i].conf, dir, nearBots[i].id, &distFactor, &neighbFactor);
		//pos = &(nearBots[j].pos);
		found=0;
		for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
			if(candidates[k].id!=0){
				if(candidates[k].id==nearBots[i].id){
					found = 1;
					numTrackedAndMeasured++;
					if(dists[i]>(GRID_DIMENSION/2)) numNonadjacent++; //halving, and converting from mm.
					//A previously tracked robot was measured again.
					newProbs[i].id=candidates[k].id;
					BAYES_DEBUG_PRINT("\t%04X: %6.2f\t(%4u -> %6.2f, %3.1f, %6.2f)\t[ f]\r\n", nearBots[i].id, factor, dists[i], distFactor, neighbFactor, nearBots[i].conf);
					newProbs[i].P=factor*candidates[k].P; //bayes-y
					deltaTrackedProb += (newProbs[i].P - candidates[k].P);
				}
			}
		}
		if(!found && nearBots[i].id){
			//We measured a robot we weren't tracking.
			numMeasuredNotTracked++;
			if(dists[i]>(GRID_DIMENSION/2)) numNonadjacent++;
			newProbs[i].id=nearBots[i].id;
			//BAYES_DEBUG_PRINT("\t%04X: %6.2f\t(%4u -> %6.2f, %3.1f, %6.2f)\t[!f]\r\n", nearBots[i].id, factor, dists[i], distFactor, neighbFactor, nearBots[i].conf);
			newProbs[i].P = factor*((slot->untrackedProb)/(NUM_POSSIBLE_BOTS-(numTrackedAndMeasured+numTrackedNotMeasured)));
			deltaTrackedProb += newProbs[i].P*(1-(1.0/factor));
		}
	}
	for(uint8_t i=0 ; i < NUM_TRACKED_BAYESIAN ; i++){
		newProbs[NUM_TRACKED_BOTS+i].id=0;
		newProbs[NUM_TRACKED_BOTS+i].P=0.0;
		found = 0;
		for(uint8_t k=0;k<NUM_TRACKED_BOTS;k++){
			if(newProbs[k].id!=0 && newProbs[k].id==candidates[i].id){
				found=1;
				break;
			}
		}
		if(!found && candidates[i].id){
			//A robot we were tracking was not measured.
			numTrackedNotMeasured++;
			newProbs[NUM_TRACKED_BOTS+i].id=candidates[i].id;
			newProbs[NUM_TRACKED_BOTS+i].P=UNMEASURED_NEIGHBOR_LIKELIHOOD*candidates[i].P;
			deltaTrackedProb += (newProbs[NUM_TRACKED_BOTS+i].P - candidates[i].P);
			//BAYES_DEBUG_PRINT("\t%04X: %6.2f\t                         \t[ m]\r\n", candidates[i].id, UNMEASURED_NEIGHBOR_LIKELIHOOD);
		}
	}

	/* ~ ~ ~ ~ ~ ~ ~ ! ~ ~ ~ ~ ~ ~ ~ */
	
	float emptyUntrackedRatio = ((float)(numNonadjacent*numNonadjacent))/((float)(NUM_TRACKED_BOTS*NUM_TRACKED_BOTS));
	if(emptyUntrackedRatio<0.1)
		emptyUntrackedRatio = 0.1;
	else if(emptyUntrackedRatio>0.9)
		emptyUntrackedRatio = 0.9;
	
	NeighbsList tmpNL;
	tmpNL.x = 0x8000;
	tmpNL.y = 0x8000;
	
	for(uint8_t i=0;i<NEW_PROBS_SIZE;i++){
		if(newProbs[i].id && neighbsList[dir].id!=newProbs[i].id){
			if(newProbs[i].P>0.2){
				tmpNL.id = newProbs[i].id;

				if(potNeighbsList[dir].id == newProbs[i].id){
					for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
						tmpNL.neighbs[j] = potNeighbsList[dir].neighbs[j];
						potNeighbsList[dir].neighbs[j] = 0;
					}
					potNeighbsList[dir].id = 0;
				}
				int16_t deltaConsistency = checkNeighborChange(dir, &tmpNL);
				newProbs[i].P = newProbs[i].P*powf(1.2,deltaConsistency);
				//printf("DeltaC: %d, newProb: %f\r\n", deltaConsistency, newProbs[i].P);
			}
			if(potNeighbsList[dir].id==newProbs[i].id && (potNeighbsList[dir].x!=0x7FFF && potNeighbsList[dir].y!=0x7FFF) && (myX != 0x7FFF && myY != 0x7FFF)){
				float distDiff = hypotf(potNeighbsList[dir].x-(neighbPos[dir][0]+myX), potNeighbsList[dir].y-(neighbPos[dir][1]+myY));
				if(distDiff<20){
					newProbs[i].P = newProbs[i].P*2;
				}else{
					newProbs[i].P=newProbs[i].P*0.75;
				}
			}
		}
	}
	
	qsort(newProbs, NEW_PROBS_SIZE, sizeof(BayesBot), bayesCmpFunc);
	newNeighb->emptyProb = 0;
	newNeighb->untrackedProb = 0;
	newTrackedTotal=0.0;
	for(uint8_t i=0;i<NUM_TRACKED_BAYESIAN;i++){
		float totalPThisID;
		if(newProbs[i].id && newProbs[i].P>0.25){
			totalPThisID = newProbs[i].P;
			for(uint8_t k=i+1;k<NUM_TRACKED_BAYESIAN;k++){
				if(newProbs[i].id==newProbs[k].id && newProbs[k].P>0.25){
					totalPThisID += newProbs[k].P;
				}
			}
			newProbs[i].P = newProbs[i].P/totalPThisID;
			for(uint8_t k=i+1;k<NUM_TRACKED_BAYESIAN;k++){
				if(newProbs[i].id==newProbs[k].id && newProbs[k].P>0.25){
					newProbs[k].P = newProbs[k].P/(1+totalPThisID);
				}
			}
		}
	}

	//float tooSmallThresh = slot->untrackedProb/NUM_POSSIBLE_BOTS;
	for(uint8_t i=0;i<NUM_TRACKED_BAYESIAN;i++){
		if(newProbs[i].id!=0){
			if(newProbs[i].P<0.02){
				candidates[i].id=0;
				candidates[i].P=0.0;
				newNeighb->emptyProb += emptyUntrackedRatio*newProbs[i].P;
				newNeighb->untrackedProb += (1.0-emptyUntrackedRatio)*newProbs[i].P;
			}else{
				candidates[i].id=newProbs[i].id;
				candidates[i].P=newProbs[i].P;
			}
		}else{
			candidates[i].id=0;
			candidates[i].P=0.0;
		}
		newTrackedTotal+=candidates[i].P;
	}
	float prevTrackedProb = 1.0-(slot->emptyProb + slot->untrackedProb);
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
	
	for(uint8_t i=0;i<NUM_TRACKED_BAYESIAN;i++){
		candidates[i].P=candidates[i].P/newTrackedTotal;
	}
}


int16_t checkNeighborChange(uint8_t dir, NeighbsList* neighb){
	NeighbsList tmp = {0, 0x7FFF, 0x7FFF, {0, 0, 0, 0, 0, 0}};
	tmp.id = neighbsList[dir].id;
	tmp.x = neighbsList[dir].x;
	tmp.y = neighbsList[dir].y;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		tmp.neighbs[i] = neighbsList[dir].neighbs[i];
	}
	neighbsList[dir].id = neighb->id;
	neighbsList[dir].x = neighb->x;
	neighbsList[dir].y = neighb->y;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbsList[dir].neighbs[i] = neighb->neighbs[i];
	}
	//if(neighb->id){
		//NEIGHBS_DEBUG_PRINT("Considering adding %04X to %hu.\r\n", neighb->id, dir);
	//}else{
		//NEIGHBS_DEBUG_PRINT("Considering removing %04X from %hu.\r\n", neighbsList[dir].id, dir);
	//}
	int16_t consistencyAfter = processNeighborData(0);
	int16_t neighborConsistency = 0;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighborConsistency += consistency[i];
		//printf_P(PSTR("\t%hu: %04X %d\r\n"), i, neighbsList[i].id, consistency[i]);
	}
	//if(potNeighbsList[dir].id==neighb->id){
		//printf("\t");
		//for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
			//printf("%04X ", potNeighbsList[dir].neighbs[i]);
		//}
		//printf("\r\n");
	//}
	//NEIGHBS_DEBUG_PRINT("Checking change for %04X in dir %hu. Before: %d, After: %d\r\n", neighb->id, dir, neighborConsistency, consistencyAfter);

	
	neighbsList[dir].id = tmp.id;
	neighbsList[dir].x = tmp.x;
	neighbsList[dir].y = tmp.y;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbsList[dir].neighbs[i] = tmp.neighbs[i];
	}
	return consistencyAfter-neighborConsistency;
}

void check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel){
	float inAngle = atan2(yVel, xVel)-M_PI_2;
	float inVel = hypotf(xVel, yVel);
	uint8_t in_dir = dirFromAngle(rad_to_deg(inAngle+M_PI));
	printf_P(PSTR("In check bounce:\r\n"));
	printf_P(PSTR("\tIn angle: %f, inDir: %hu, xVel: %hd, yVel: %hd\r\n"), rad_to_deg(inAngle), in_dir, xVel, yVel);
	float outAngle;
	//note: directions below are relative to the direction from which the ball came in.
	
	uint8_t opp_dir			= (in_dir+3)%6;
	uint8_t left_dir		= (in_dir+1)%6;
	uint8_t right_dir		= (in_dir+5)%6; //it's like -1
	uint8_t far_left_dir	= (in_dir+2)%6;
	uint8_t far_right_dir	= (in_dir+4)%6; //it's like -2
	
	//uint8_t in			= !!(neighbsList[in_dir].id);
	uint8_t opp			= !!(neighbsList[opp_dir].id);
	uint8_t left		= !!(neighbsList[left_dir].id);
	uint8_t right		= !!(neighbsList[right_dir].id);
	uint8_t far_left	= !!(neighbsList[far_left_dir].id);
	uint8_t far_right	= !!(neighbsList[far_right_dir].id);
	
	outAngle = inAngle;
	if(!opp){
		outAngle += M_PI;
		outAngle += (left		? -M_PI/3.0 : 0);
		outAngle += (right		?  M_PI/3.0 : 0);
		outAngle += (far_left	? -M_PI/6.0 : 0);
		outAngle += (far_right	?  M_PI/6.0 : 0);
		printf_P(PSTR("\tBall bounced!\r\n"));
		lastBallMsg = 0;
	}
	
	float newX = inVel*cosf(outAngle+M_PI_2);
	float newY = inVel*sinf(outAngle+M_PI_2);
	
	*newXvel = (int8_t)round(newX);
	*newYvel = (int8_t)round(newY);

	printf_P(PSTR("\toutAngle: %f, newXvel: %f, newYvel: %f\r\n"), rad_to_deg(outAngle), newX, newY);
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
		if(myState>=3 && last_good_rnb.conf>1.0){
			if(((pos->b)<=M_PI_2) && (pos->b>-M_PI_2)){
				northSouthCount++;
			}else{
				northSouthCount--;
			}
			if((pos->b)<=0){
				eastWestCount++;
			}else{
				eastWestCount--;
			}
		}
		POS_DEBUG_PRINT("\tNS: %hd, EW: %hd\r\n", northSouthCount, eastWestCount);	
		//POS_DEBUG_PRINT("\tRmo: % 5.1f Bmo: % 6.1f Hmo: % 6.1f\r\n", pos->r, rad_to_deg(pos->b), rad_to_deg(pos->h));
	}else{
		printf_P(PSTR("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n"));
	}
}

void checkLightLevel(){
	int16_t r, g, b;
	get_rgb(&r,&g,&b);
	int16_t sum = r+g+b;
	uint32_t now = get_time();
	if(sum<=25){
		paddleChange += ((now-lastLightCheck)*(3*PADDLE_VEL));
		//printf("Paddle ChangeX5: %f\r\n", paddleChange);
	}else if(sum<=40){
		paddleChange += ((now-lastLightCheck)*(2*PADDLE_VEL));
		//printf("Paddle ChangeX3: %f\r\n", paddleChange);
	}else if(sum<=60){
		paddleChange += ((now-lastLightCheck)*(PADDLE_VEL));
		//printf("Paddle ChangeX5: %f\r\n", paddleChange);
	}
	lastLightCheck = now;
	//printf("Light: %5d (%4d, %4d, %4d)\r\n",sum,r,g,b);
}

void sendBallMsg(){
	if(myState>2 || myDist==0x7FFF || myDist>=30){
		return;
	}
	BallMsg msg;
	msg.flag = BALL_MSG_FLAG;
	int16_t tempX = theBall.xPos;
	int16_t tempY = theBall.yPos;
	msg.xPos		= tempX&0xFF;
	msg.extraBits	= ((tempX & 0x0700)>>3)&0xE0;
	msg.yPos		= tempY&0xFF;
	msg.extraBits |= ((tempY & 0x0700)>>6)&0x1C;
	msg.extraBits |= theBall.id&0x03;
	msg.xVel = theBall.xVel;
	msg.yVel = theBall.yVel;
	msg.radius = (theBall.radius&0xFC) | ((theBall.id&0x0C)>>2);
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BallMsg));
	lastBallMsg=get_time();
}

void handleBallMsg(BallMsg* msg, uint32_t arrivalTime){;
	//printf("Got Ball! T: %lu\r\n\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", get_time(), theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
	int16_t highX = (int16_t)(((int8_t)(msg->extraBits))>>5);
	int16_t highY = (int16_t)((((int8_t)(msg->extraBits))<<3)>>5);
	int16_t tempX = (int16_t)((highX<<8) | ((uint16_t)(msg->xPos)));
	int16_t tempY = (int16_t)((highY<<8) | ((uint16_t)(msg->yPos)));
	uint8_t id = ((msg->extraBits)&0x03) | (((msg->radius)&0x03)<<2);
	if(id == 0x0F && theBall.id!=0x0F){
		lastBallID = theBall.id;
		set_rgb(255,0,0);
	}else if(id == lastBallID && theBall.id==0x0F){
		return; //this is from someone who hasn't realized the ball is dead, yet.
	}
	theBall.xPos = tempX;
	theBall.yPos = tempY;
	theBall.id = id;
	theBall.xVel = msg->xVel;
	theBall.yVel = msg->yVel;
	theBall.radius = ((msg->radius)&0xFC);
	theBall.lastUpdate = arrivalTime-4;
	//printf("\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
}

void sendNeighbMsg(){
	NeighbMsg msg;
	msg.flag = NEIGHB_MSG_FLAG;
	msg.x = myX;
	msg.y = myY;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		msg.ords[i] = get_droplet_ord(neighbsList[i].id);
	}
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(NeighbMsg));
}

void handleNeighbMsg(NeighbMsg* msg, uint16_t sender){
	uint8_t found=0;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		if(sender==neighbsList[i].id){
			neighbsList[i].x = msg->x;
			neighbsList[i].y = msg->y;
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				neighbsList[i].neighbs[j] = get_id_from_ord(msg->ords[j]);
			}
			found=1;
			break;
		}
	}
	if(!found){
		for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			if(sender==bayesSlots[dir].candidates[0].P){
				potNeighbsList[dir].id = sender;
				potNeighbsList[dir].x = msg->x;
				potNeighbsList[dir].y = msg->y;
				for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
					potNeighbsList[dir].neighbs[i] = get_id_from_ord(msg->ords[i]);
				}
			}
		}
	}
}

void sendPaddleMsg(){
	PaddleMsg msg;
	switch(myState){
		case CTRL_NE:
			msg.flag = 'P';
			msg.deltaPos = ((int16_t)paddleChange);
			break;
		case CTRL_NW:
			msg.flag = 'P';
			msg.deltaPos = -1*((int16_t)paddleChange);
			break;
		case CTRL_SE:
			msg.flag = 'S';
			msg.deltaPos = ((int16_t)paddleChange);
			break;
		case CTRL_SW:
			msg.flag = 'S';
			msg.deltaPos = -1*((int16_t)paddleChange);
			break;
		default:
			paddleChange = 0.0;
			return;
	}
	paddleChange = 0.0;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PaddleMsg));
	printf_P(PSTR("Sent paddle msg with change: %d\r\n"), msg.deltaPos);
}

void handlePaddleMsg(char flag, int16_t delta){
	if((myState==PIXEL_N && flag=='P') || (myState==PIXEL_S && flag=='S')){
		paddleStart += delta;
		paddleEnd += delta;
		if(paddleStart<MIN_DIM){
			paddleStart = MIN_DIM;
			paddleEnd = MIN_DIM+ PADDLE_WIDTH;
		}
		if(paddleEnd>X_MAX_DIM){
			paddleEnd = X_MAX_DIM;
			paddleStart = X_MAX_DIM-PADDLE_WIDTH;
		}
	}
}

void handle_msg(ir_msg* msg_struct){
	char flag = msg_struct->msg[0];
	if(flag==BALL_MSG_FLAG){
		handleBallMsg((BallMsg*)(msg_struct->msg), msg_struct->arrival_time);
	}else if(flag==NEIGHB_MSG_FLAG){
		handleNeighbMsg((NeighbMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else if(flag==N_PADDLE_MSG_FLAG || flag==S_PADDLE_MSG_FLAG){
		handlePaddleMsg(flag, ((PaddleMsg*)(msg_struct->msg))->deltaPos);
	}else{
		printf_P(PSTR("From %04X:\r\n\t"),msg_struct->sender_ID);
		for(uint8_t i=0;i<msg_struct->length;i++){
			printf("%c",msg_struct->msg[i]);
		}
		printf("\r\n");
	}
}


//void bayesDebugPrintout(){
	//printf(" --- Bayes Debug Printout --- \r\n");
	//BayesSlot* neighbor;
	//BayesBot* candidates;
	//uint8_t untrackedToggle, emptyToggle,emptyFirst;	
	//for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		//neighbor = &(bayesSlots[i]);
		//candidates=neighbor->candidates;
		//untrackedToggle=0;
		//emptyToggle=0;
		//printf("Slot %hu:\r\n", i);
		//if(neighbor->emptyProb>neighbor->untrackedProb){
			//emptyFirst=1;
		//}else{
			//emptyFirst=0;
		//}
		//for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			//if(candidates[j].id!=0){
				//if((emptyFirst || (!emptyFirst && untrackedToggle)) && !emptyToggle){
					//if(candidates[j].P < neighbor->emptyProb){
						//printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
						//emptyToggle=1;
					//}
				//}
				//if((!emptyFirst || (emptyFirst && emptyToggle)) && !untrackedToggle){
					//if(candidates[j].P < neighbor->untrackedProb){
						//printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
						//untrackedToggle=1;
					//}
					//if(!emptyToggle && (candidates[j].P < neighbor->emptyProb)){
						//printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
						//emptyToggle=1;
					//}
				//}
				//printf("\t%04X\t% 6f\r\n",candidates[j].id,candidates[j].P);
			//}
		//}
		//if(emptyFirst){
			//if(!emptyToggle){
				//printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
				//emptyToggle=1;
			//}
			//if(!untrackedToggle){
				//printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
				//untrackedToggle=1;
			//}
		//}else{
			//if(!untrackedToggle){
				//printf("   Untracked\t% 6f\r\n",neighbor->untrackedProb);
				//untrackedToggle=1;
			//}
			//if(!emptyToggle){
				//printf("       Empty\t% 6f\r\n",neighbor->emptyProb);
				//emptyToggle=1;
			//}
		//}
	//}
	//printf("\n");
//}

void printNeighbsList(){
	uint16_t id;
	NEIGHBS_DEBUG_PRINT("\tSlot:  ID       |  0   |  1   |  2   |  3   |  4   |  5   |\r\n");
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		if(neighbsList[i].id){
			NEIGHBS_DEBUG_PRINT("\t   %hu: %04X      |", i, neighbsList[i].id);
		}else{
			NEIGHBS_DEBUG_PRINT("\t   %hu:  --       |", i);
		}
		for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
			id=neighbsList[i].neighbs[j];
			if(id==0){
				NEIGHBS_DEBUG_PRINT("  --  |");
			}else{
				NEIGHBS_DEBUG_PRINT(" %04X |", neighbsList[i].neighbs[j]);
			}
		}
		if(neighbsList[i].x!=0x7FFF){
			NEIGHBS_DEBUG_PRINT(" (%d,", neighbsList[i].x);
		}else{
			NEIGHBS_DEBUG_PRINT(" ( --- ,");
		}
		if(neighbsList[i].y!=0x7FFF){
			NEIGHBS_DEBUG_PRINT(" %d)", neighbsList[i].y);
		}else{
			NEIGHBS_DEBUG_PRINT(" --- )");
		}
		NEIGHBS_DEBUG_PRINT("\r\n");
	}
}

void frameEndPrintout(){
	printf_P(PSTR("\nID: %04X T: %lu [ "), get_droplet_id(), 
	get_time());
	switch(myState){
		case PIXEL:			printf("Pixel");			break;
		case PIXEL_N:		printf("North Pixel");		break;
		case PIXEL_S:		printf("South Pixel");		break;
		case CTRL_NE:		printf("NE Control");		break;
		case CTRL_NW:		printf("NW Control");		break;
		case CTRL_SE:		printf("SE Control");		break;
		case CTRL_SW:		printf("SW Control");		break;
		case CTRL_UNKNWN:	printf("?? Control");		break;
	}
	printf_P(PSTR(" ]\r\n"));
	if(myX!=0x7FFF && myY!=0x7FFF){
		printf_P(PSTR("\tMy Pos: (%d, %d)\r\n"), myX, myY);
	}
	if(!isnan(theBall.xPos) && !isnan(theBall.yPos)){
		printf_P(PSTR("\tBall ID: %hu; radius: %hu; Pos: (%5.1f, %5.1f) @ vel (%hd, %hd)\r\n"), theBall.id, theBall.radius, theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel);
	}
	if(myState==PIXEL_N || myState==PIXEL_S){
		printf_P(PSTR("\tPaddle: %d <-> %d.\r\n"), paddleStart, paddleEnd);
	}else{
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
	printf("%04X",get_id_from_ord(rand_byte()));
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
uint8_t user_handle_command(char* command_word, char* command_args){
	if(strcmp_P(command_word,PSTR("ball"))==0){
		if(0x7FFF!=myX && 0x7FFF!=myY){
			const char delim[2] = " ";
			char* token = strtok(command_args, delim);
			int8_t vel = (token!=NULL) ? (int8_t)atoi(token) : 10;
			token = strtok(NULL, delim);
			uint8_t size = (token!=NULL) ? (uint8_t)atoi(token) : 60;
			theBall.xPos = myX;
			theBall.yPos = myY;
			int16_t randomDir = rand_short()%360;
			theBall.xVel = vel*cos(deg_to_rad(randomDir));
			theBall.yVel = vel*sin(deg_to_rad(randomDir));
			theBall.id = (lastBallID+1)%(0x0E);
			theBall.radius = size&0xFC;
			theBall.lastUpdate = get_time();
			printf_P(PSTR("Got ball command. Velocity: %hd\r\n"), vel);
		}else{
			uint8_t r = get_red_led();
			set_red_led(255);
			printf_P(PSTR("Got ball command, but I don't know where I am yet.\r\n"));
			set_red_led(r);
		}
		return 1;
	}else if(strcmp_P(command_word,PSTR("mode"))==0){
		switch(command_args[0]){
			case 'p': colorMode = POS;			break;
			case 'n': colorMode = NEIGHB;		break;
			case 's': colorMode = SYNC_TEST;	break;
			case 'o': colorMode = OFF;			break;
		}
		switch(command_args[1]){
			case 'p': gameMode = PONG;			break;
			case 'b': gameMode = BOUNCE;		break;
			default:  gameMode = BOUNCE;		break;
		}
		return 1;
	}else if(strcmp_P(command_word,PSTR("ball_kill"))==0){
		killBall();
		return 1;
	}
	return 0;
}