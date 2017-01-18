#include "droplet_programs/positions.h"

void init(){
	if((RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR+BALL_MSG_DUR)>=(SLOT_LENGTH_MS)){
		printf_P(PSTR("You've got problems! SLOT_LENGTH_MS needs to be longer than all the things that take place during a slot!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	//initUsedBots();
	initPositions();
	myState = STATE_PIXEL;
	colorMode = POS;
	gameMode = BOUNCE;
	lastBallID = 0;
	lastBallMsg = 0;
	//lastPaddleMsg = 0;
	lastLightCheck = get_time();
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	//mySlot = get_droplet_ord(get_droplet_id());
	printf("mySlot: %u, frame_length: %lu\r\n", mySlot, FRAME_LENGTH_MS);
	frameEndPrintout();
	hardBotsList = NULL;
	isCovered = 0;
	xRange = MAX_X-MIN_X;
	yRange = MAX_Y-MIN_Y;
	maxRange = (int16_t)hypotf(xRange, yRange);
	useBlacklist = 1;
	//useNewInfo = 0;
	stdDevThreshold = 100;
	useMeasAveraging = 0;
	addedPosStdDev = 11;
	addedMeasStdDev = 22;
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
			handleMySlot();
		}else if(loopID==SLOTS_PER_FRAME-1){
			handleFrameEnd();
		}
		//if(loopID!=mySlot){
			//uint32_t curSlotTime = (get_time()-frameStart)%SLOT_LENGTH_MS;
			////if(NS_PIXEL(myState) && paddleChange>=1.0){
				//////schedule_task((RNB_DUR-curSlotTime),sendPaddleMsg(), NULL);
			////}else{
				////paddleChange = 0.0;
			//////}
			//if(/*myState<=2 &&*/ myDist!=UNDF && otherDist!=UNDF && myDist<otherDist){
				//uint32_t curSlotTime = (get_time()-frameStart)%SLOT_LENGTH_MS;
				//schedule_task(((RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR)-curSlotTime), sendBallMsg, NULL);
			//}
		//}
		updateBall();
		updateColor();
	}
	//These things happen every single loop: once every LOOP_DELAY_MS.
	if(NS_PIXEL(myState)){
		checkLightLevel();
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
	delay_ms(LOOP_DELAY_MS);
}

void handleMySlot(){
	broadcast_rnb_data();	
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR)
		delay_us(500);
	//printf("\tCollisions: ");
	//int16_t coll_vals[6];
	//check_collision_values(coll_vals);
	//for(uint8_t i=0;i<6;i++){
		//printf("%5d ", coll_vals[i]);
	//}
	//printf("\r\n\tColors: ");
	//int16_t r, g, b;
	//get_rgb(&r, &g, &b);
	//printf("%5d %5d %5d\r\n", r, g, b);
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<(RNB_DUR+PADDLE_MSG_DUR))
		delay_us(500);		
	sendNearBotsMsg();
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<(RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR))
		delay_us(500);			
	//if(myDist!=UNDF && otherDist!=UNDF && myDist<otherDist){
		//sendBallMsg();
	//}
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<(RNB_DUR+PADDLE_MSG_DUR+NEIGHB_MSG_DUR+BALL_MSG_DUR))
		delay_us(500);	
}

//Note! This function assumes that the meas is from the perspective of the bot at pos.
//		ie, not THIS bot.
void calcPosFromMeas(BotPos* calcPos, BotPos* pos, BotMeas* meas){
	float x		 = pos->x;
	float y		 = pos->y;
	int16_t o	 = pos->o;
	float posStdDev   = pos->stdDev;
	uint16_t r   = meas->r;
	int16_t b	 = meas->b;
	int16_t h    = meas->h;
	float measStdDev = meas->stdDev;
	float deltaX = (float)r * cos(deg_to_rad(b + o + 90));
	float deltaY = (float)r * sin(deg_to_rad(b + o + 90));
	calcPos->x = (int16_t)(x + deltaX + 0.5);
	calcPos->y = (int16_t)(y + deltaY + 0.5);
	calcPos->o = pretty_angle_deg(o + h);
	float newStdDev = (hypotf(posStdDev,measStdDev)+0.5);
	newStdDev = (newStdDev>255) ? 255 : newStdDev;
	calcPos->stdDev = (uint8_t)newStdDev;
}

void printPosFromMeas(BotPos* pos, BotMeas* meas){
	BotPos calcPos;
	calcPosFromMeas(&calcPos, pos, meas);
	printf("\t[%04X] {%4d, %4d, % 4d, %2hu, %4u, % 4d, % 4d, ", meas->id, pos->x, pos->y, pos->o, pos->stdDev, meas->r, meas->b, meas->h);
	printf("%4d, %4d, %4d, %3hu}\r\n", calcPos.x, calcPos.y, calcPos.o, calcPos.stdDev);
}

uint8_t countAvailableMeasurements(){
	uint8_t numBots=0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBotUseabilityCheck(i)){
			numBots++;
		}
	}
	return numBots;
}

/* 
	* If we use the same bot every frame, then the measurement errors between me and this other bot become 
	* very much not independent. Thus, we blacklist otherBots for a certain number of frames before using
	* them again. This blacklist/waiting period is intended to allow the other bot to incorporate new/different
	* information from other robots, and thus become more independent. Honestly it will still be somewhat dependent,
	* but the goal here is to mitigate our abuse of the independence assumption.
	*/
uint8_t nearBotUseabilityCheck(uint8_t i){
	if(!validNearBotIdx(i)){
		return 0;
	}
	if(nearBots[i].pos.stdDev > stdDevThreshold){
		return 0;
	}
	if(nearBots[i].occluded){
		return 0;
	}
	//if(!(nearBots[i].hasNewInfo)){
		//if(useNewInfo){
			//return 0;
		//}
	//}
	if((nearBots[i].theirMeas).id != get_droplet_id()){
		return 0;
	}
	if((frameCount - nearBots[i].lastUsed) < OTHERBOT_BLACKLIST_FRAME_COUNT){
		if(nearBots[i].lastUsed==0 && frameCount<OTHERBOT_BLACKLIST_FRAME_COUNT){
			return 1;
		}
		if(useBlacklist){
			return 0;
		}
	}
	return 1;
}


//Fused measurement is from other robot's perspective
void fuseMeasurements(BotMeas* fused, BotMeas* myMeas, BotMeas* theirMeas){
	fused->id = myMeas->id;
	fused->b = theirMeas->b;
	fused->h = pretty_angle_deg(180 - myMeas->b + theirMeas->b);
	fused->r = (myMeas->r + theirMeas->r)/2;
	float myStdDev = myMeas->stdDev;
	float theirStdDev = theirMeas->stdDev;
	float stdDev = hypotf(myStdDev, theirStdDev) + 0.5;
	stdDev = stdDev>255 ? 255 : stdDev;
	fused->stdDev = (uint8_t)(stdDev);
}

void extrapolateCovarFromScalar(Matrix* A, float stdDev){
	matrixCopy(A, &identity_matrix);
	float stdDevSq = stdDev*stdDev;
	float stdDevRed = stdDevSq/3.0;
	(*A)[0][0] = stdDevSq;
	(*A)[1][1] = stdDevSq;
	(*A)[2][2] = stdDevSq;
	(*A)[0][2] = stdDevRed;
	(*A)[1][2] = stdDevRed;
	(*A)[2][0] = stdDevRed;
	(*A)[2][1] = stdDevRed;
}

float distillCovarIntoScalar(){
	return powf(matrixDet(&pK), 0.166667)+1;
}

void prepExpectedPositions(BotPos* expPosArr){
	uint8_t botIdx=0;
	BotMeas fusedMeas;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBotUseabilityCheck(i)){
			fuseMeasurements(&fusedMeas, &(nearBots[i].myMeas), &(nearBots[i].theirMeas));
			calcPosFromMeas(&(expPosArr[botIdx]), &(nearBots[i].pos), &fusedMeas);
			printPosFromMeas(&(nearBots[i].pos), &fusedMeas);
			botIdx++;
		}
	}
}

void combineExpectedPositions(Vector* pos, float* stdDev, uint8_t numMeas, BotPos* expPosArr){
	float totX = 0;
	float totY = 0;
	float oTotX = 0;
	float oTotY = 0;
	float thisWeight;
	float totWeight = 0;

	for(uint8_t i=0 ; i<numMeas ; i++){
		thisWeight	 = 1.0/(expPosArr[i].stdDev);
		totX		+=  thisWeight*expPosArr[i].x;
		totY		+= thisWeight*expPosArr[i].y;
		oTotX		+= thisWeight*cosf(deg_to_rad(expPosArr[i].o));
		oTotY		+= thisWeight*sinf(deg_to_rad(expPosArr[i].o));
		totWeight	+= thisWeight;
		//addUsedBot(nearBots[i].myMeas.id);
		nearBots[i].lastUsed = frameCount;
	}
	(*pos)[0] = totX/totWeight;
	(*pos)[1] = totY/totWeight;
	(*pos)[2] = rad_to_deg(atan2f(oTotY, oTotX));

	float sD = 1.0/totWeight;
	if(numMeas>2){
		sD *= (float)(numMeas-1);
	}else{
		sD *= (float)(numMeas);
	}
	*stdDev = sD;
	printf("\t{%4d, %4d, %4d, %3hu}\r\n", (int16_t)(*pos)[0], (int16_t)(*pos)[1], (int16_t)(*pos)[2], (uint8_t)(*stdDev));
}

void updatePosition(){
	uint8_t numMeas = countAvailableMeasurements();
	if(!numMeas){ //no measurements to initialize with.
		printf("\tCan't update; no new measurements available.\r\n");
		return;
	}
	Vector zK;
	float measStdDev;
	BotPos expPos[numMeas];
	prepExpectedPositions(expPos);
	combineExpectedPositions(&zK, &measStdDev, numMeas, expPos);
	Vector yK;
	Matrix kK;
	Matrix sK;
	Matrix rK;

	extrapolateCovarFromScalar(&rK, measStdDev);
	Vector xK = {(float)(myPos.x), (float)(myPos.y), (float)(myPos.o)};
	if(myPos.x !=UNDF && myPos.y !=UNDF && myPos.o != UNDF){
		matrixAdd(&pK, &pK, &qK);							//pK = pK + qK
		vectorSubtract(&yK, &zK, &xK);					//yK = zK - xK
		matrixAdd(&kK, &pK, &rK);						//	(kK: scratch)
		matrixInverse(&sK, &kK);						//sK = (pK + rK)^(-1)
		matrixMultiply(&kK, &pK, &sK);					//kK = pK*(pK+rK)^(-1)
		matrixTimesVector((&sK)[0], &kK, &yK);			//	(sK[0]: scratch))
		vectorAdd(&xK, &xK, (&sK)[0]);						//xK = xK + kK*yK
		matrixSubtract(&kK, &identity_matrix, &kK);	//	(kK: scratch)
		matrixCopy(&sK, &pK);							//	(sK: scratch)
		matrixMultiply(&pK, &kK, &sK);					//pK = (I - kK)*pK
		
		myPos.x = (int16_t)(xK[0] + 0.5);
		myPos.y = (int16_t)(xK[1] + 0.5);
		myPos.o = pretty_angle_deg( (int16_t)(xK[2] + 0.5) );
		float stdDev = distillCovarIntoScalar() + 0.5;
		stdDev = stdDev>255 ? 255 : stdDev;
		myPos.stdDev = (uint8_t)stdDev;
	}else{
		myPos.x		 = (int16_t)(zK[0] + 0.5);
		myPos.y		 = (int16_t)(zK[1] + 0.5);
		myPos.o		 = pretty_angle_deg((int16_t)(zK[2] + 0.5));
		extrapolateCovarFromScalar(&pK, measStdDev*2);
		float stdDev = measStdDev*2 + 0.5;
		stdDev = stdDev>255 ? 255 : stdDev;
		myPos.stdDev = (uint8_t)stdDev;
	}
}

//Note: This function assumes that nearBots have been sorted by range!
void updateNearBotOcclusions(){
	OCC_DEBUG_PRINT("\tUpdating Occlusions\r\n");
	float range;
	float halfOccConeArc;
	float rForPadding;
	float halfPaddedConeArc;
	int16_t intervals[NUM_TRACKED_BOTS][2];
	uint8_t numIntervals=0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].myMeas.id == 0) continue;
		for(uint8_t j=0;j<numIntervals;j++){
			if(abs(intervals[j][0] - nearBots[i].myMeas.b)<=(intervals[j][1])){
				nearBots[i].occluded=1;
				OCC_DEBUG_PRINT("\t\t%04X occluded.\r\n",nearBots[i].myMeas.id);
				break;
			}
		}
		if(!(nearBots[i].occluded)){
			range = nearBots[i].myMeas.r;
			halfOccConeArc = asin(DROPLET_DIAMETER_MM/(2*range));
			rForPadding = DROPLET_DIAMETER_MM*cosf(halfOccConeArc);
			halfPaddedConeArc = asin(DROPLET_DIAMETER_MM/(range+rForPadding));
			intervals[numIntervals][0] = nearBots[i].myMeas.b;
			intervals[numIntervals][1] = rad_to_deg(halfPaddedConeArc);
			nearBots[i].occluded = 0;
			OCC_DEBUG_PRINT("\t\t%hu| %04X -> % 4d, % 3d\r\n", numIntervals, nearBots[i].myMeas.id, intervals[numIntervals][0], intervals[numIntervals][1]);
			numIntervals++;
		}
	}
}

void handleFrameEnd(){
	//printf("Frame End Calculations\r\n");
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsCmpFunc);
	//printNearBots();


	//Maybe we'll want to remove the N worst nearBots, here.
	if(!seedFlag){
		updateNearBotOcclusions();
		updatePosition();
	}
	
	////Removing the worst half of the nearBots.
	//for(uint8_t i=NUM_TRACKED_BOTS/2; i<NUM_TRACKED_BOTS;i++){
		//cleanOtherBot(&(nearBots[i]));
	//}

	updateHardBots();
	increaseStdDev();
	frameEndPrintout();
	printf("\r\n");
}

void updateHardBots(){
	//First, making a copy of nearBots so we can sort it by a weird metric.
	BotMeas nearBotsMeas[NUM_TRACKED_BOTS];
	uint8_t numNearBots = 0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].myMeas.id==0) continue;
		nearBotsMeas[i].id   = nearBots[i].myMeas.id;
		nearBotsMeas[i].r    = nearBots[i].myMeas.r;
		nearBotsMeas[i].b    = nearBots[i].myMeas.b;
		nearBotsMeas[i].h    = nearBots[i].myMeas.h;
		nearBotsMeas[i].stdDev = nearBots[i].myMeas.stdDev;
		numNearBots++;
	}
	cleanHardBots(); //clean out the previous hardBots list -- we start fresh each farme.
	//sort nearBots according to their bearing.
	qsort(nearBotsMeas, numNearBots, sizeof(BotMeas), nearBotMeasBearingCmpFunc);
	//go through each near bot by bearing and add it to hardBots if the gap in bearings is above 120 degrees
	for(uint8_t i=0;i<numNearBots;i++){
		if(nearBotsMeas[i].id==0) continue;
		uint8_t nextI = (i+1)%numNearBots;
		uint8_t difference = abs((nearBotsMeas[i].b-nearBotsMeas[nextI].b + 540)%360 - 180) ;
		if(difference>120){
			addHardBot(nearBotsMeas[i].id);
			addHardBot(nearBotsMeas[nextI].id);
		}
	}
	//print out hard bots list.
	if(hardBotsList!=NULL){
		BALL_DEBUG_PRINT("Edges:");
		HardBot* tmp = hardBotsList;
		while(tmp!=NULL){
			BALL_DEBUG_PRINT("\t%04X", tmp->id);
			tmp = tmp->next;
		}
		BALL_DEBUG_PRINT("\r\n");
	}
}

void increaseStdDev(){
	float stdDev;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		stdDev = ((nearBots[i].myMeas.stdDev)*2.0);
		if(stdDev > 255){
			removeOtherBot(i);
		}else{
			nearBots[i].myMeas.stdDev = (uint8_t)stdDev;
		}
	}
}

void useNewRnbMeas(){
	//Pulling everything out of the global struct.
	uint16_t id = last_good_rnb.id;
	uint16_t range = last_good_rnb.range;
	int16_t bearing = last_good_rnb.bearing;
	rnb_updated=0;

	uint8_t stdDev = getStdDevFromRange(range);
	RNB_DEBUG_PRINT("            (RNB) ID: %04X | R: %4u B: %4d\r\n", id, range, bearing);
	OtherBot* measuredBot = addOtherBot(id);
	BotMeas* meas;
	if(measuredBot){
		meas = &(measuredBot->myMeas);
	}else{
		RNB_DEBUG_PRINT("No room in other bots, aborting.\r\n");
		return;
	}
	if(meas->id == 0 || (useMeasAveraging==0 && meas->id == id)){ //We weren't tracking this ID before, so just add the new info.
		if(meas->id == id){
			printf("\tG");
		}
		measuredBot->lastUsed = 0;
		meas->id	 = id;
		meas->r		 = range;
		meas->b		 = bearing;
		meas->h		 = UNDF;
		meas->stdDev = stdDev;
	}else if(useMeasAveraging==1 && meas->id == id){
		measuredBot->lastUsed = 0;
		meas->id	= id;
		float newWeight = 1.0/stdDev;
		float prevWeight = 1.0/meas->stdDev;
		meas->r		= (range*newWeight+meas->r*prevWeight)/(newWeight + prevWeight);
		float tempX = newWeight*cos(deg_to_rad(bearing))+prevWeight*cos(deg_to_rad(meas->b));
		float tempY = newWeight*sin(deg_to_rad(bearing))+prevWeight*sin(deg_to_rad(meas->b));
		meas->b		= rad_to_deg(atan2f(tempY, tempX));
		meas->h     = UNDF;
		meas->stdDev  = (uint8_t)(2.0/(newWeight+prevWeight)+0.5);
	}else{
		printf_P(PSTR("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n"));
		return;
	}
}

void updateBall(){
	if(theBall.lastUpdate){
		uint32_t now = get_time();
		int32_t timePassed = now-theBall.lastUpdate;
		if((myPos.x!=UNDF) && (myPos.y!=UNDF) && (theBall.xPos!=UNDF) && (theBall.yPos!=UNDF)){

			//int8_t crossedBefore = checkBallCrossedMe();

			//int8_t crossedAfter = checkBallCrossedMe();
			//myDist = (uint16_t)hypotf(theBall.xPos-myPos.x,theBall.yPos-myPos.y);
			//theBall.lastUpdate = now;
			//if(myDist<=30 && crossedBefore!=crossedAfter){ //BOUNCE CHECK
			//uint8_t ballInPaddle = ((theBall.xPos+theBall.radius)>=paddleStart && (theBall.xPos-theBall.radius)<=paddleEnd);
			//uint8_t ballLeaving = (NORTH_PIXEL(myState) && theBall.yVel<0) || (SOUTH_PIXEL(myState) && theBall.yVel>0);
			//if(gameMode==PONG && !ballInPaddle && ballLeaving){
			//killBall();
			//}else{
			//check_bounce(theBall.xVel, theBall.yVel, &(theBall.xVel), &(theBall.yVel));
			//}
			//}else{
			theBall.xPos += (int16_t)((((int32_t)(theBall.xVel))*timePassed)/1000.0);
			theBall.yPos += (int16_t)((((int32_t)(theBall.yVel))*timePassed)/1000.0);
			theBall.lastUpdate = now;
			BALL_DEBUG_PRINT("B[%hu]: %d, %d\r\n", theBall.id, theBall.xPos, theBall.yPos);
			uint8_t bounced = 0;
			HardBot* tmp = hardBotsList;
			myDist = (uint16_t)hypotf(myPos.x-theBall.xPos, myPos.y-theBall.yPos);
			while(tmp!=NULL){
				OtherBot* bot = getOtherBot(tmp->id);
				if(myDist<(((bot->myMeas).r*10)/6)){
					BALL_DEBUG_PRINT("\t%04X | ", tmp->id);
					if(checkBounceHard((bot->pos).x,(bot->pos).y, timePassed)){
						if(gameMode==PONG && ((SOUTH_PIXEL(myState) && theBall.yVel<=0) || (NORTH_PIXEL(myState) && theBall.yVel>=0))){
							if(!isCovered){
								//Other Side scores a point!
								killBall();
								set_rgb(255,0,0);
							}
						}
						calculateBounce((bot->pos).x,(bot->pos).y);
						BALL_DEBUG_PRINT("Ball bounced off boundary between me and %04X!\r\n", tmp->id);
						otherDist = (((bot->myMeas).r*10)/6);
						bounced = 1;
						break;
					}
				}
				tmp = tmp->next;
			}
			if(theBall.xPos<MIN_X || theBall.xPos>MAX_X || theBall.yPos<MIN_Y || theBall.yPos>MAX_Y){
				BALL_DEBUG_PRINT("Ball hit boundary, so we must have lost track.\r\n");
				theBall.xPos = UNDF;
				theBall.yPos = UNDF;
				myDist = UNDF;
				otherDist = UNDF;
			}
			}else{
			myDist = UNDF;
			otherDist = UNDF;
		}
	}
}

//void check_bounce(int8_t xVel, int8_t yVel, int8_t* newXvel, int8_t* newYvel){
//float inAngle = atan2(yVel, xVel)-M_PI_2;
////	float inVel = hypotf(xVel, yVel);
//uint8_t in_dir = dirFromAngle(inAngle+180);
//BALL_DEBUG_PRINT(PSTR("In check bounce:\r\n"));
//BALL_DEBUG_PRINT(PSTR("\tIn angle: %f, inDir: %hu, xVel: %hd, yVel: %hd\r\n"), rad_to_deg(inAngle), in_dir, xVel, yVel);
//BALL_DEBUG_PRINT("Note! check_bounce currently doesn't do anything! Eventually, it will look to see if there's a robot in direction 'inAngle'.\r\n");
//}

void updateColor(){
	uint8_t newR = 0, newG = 0, newB = 0;
	if(colorMode==POS){
		if(myPos.x==UNDF||myPos.y==UNDF){
			newR = newG = newB = 50;
		}else{
			int16_t xColVal = (int16_t)(6.0*pow(41.0,(myPos.x-MIN_X)/((float)xRange))+9.0);
			int16_t yColVal = (int16_t)(3.0*pow(84.0,(myPos.y-MIN_Y)/((float)yRange))+3.0);
			newR = (uint8_t)(xColVal);
			newG = (uint8_t)(yColVal);
		}
	}else if(colorMode==SYNC_TEST){
		//printf("Frame: %2lu ; loopID: %2u (mod: %2u)", frameCount, loopID, loopID%30);
		if((loopID%36)<24){
			//printf(" || %hu", (uint8_t)((loopID%36)>>2));
			hsv_to_rgb((uint16_t)(60*((loopID%36)>>2)), 220, 127, &newR, &newG, &newB);
		}else{
			float value = (frameCount%6)/6.0;
			uint8_t val = (uint8_t)(5.0*pow(51.0,value));
			hsv_to_rgb(0,0, val,&newR, &newG, &newB);
		}
		//printf("\r\n");
	}
	if(myPos.x!=UNDF && myPos.y!=UNDF){
		float coverage = getBallCoverage() /*+ getPaddleCoverage()*/;
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
	if((((int16_t)myDist)!=UNDF) && (myDist<(DROPLET_RADIUS+theBall.radius)) && (theBall.id!=0x0F)){
		if(theBall.radius<DROPLET_RADIUS){
			if(myDist>=(DROPLET_RADIUS-theBall.radius)){
				ballCoveredRatio = getCoverageRatioA(theBall.radius, myDist);
			}else{
				ballCoveredRatio = 1.0;
			}
		}else{
			if(myDist>=(theBall.radius-DROPLET_RADIUS)){
				ballCoveredRatio = getCoverageRatioB(theBall.radius, myDist);
			}else{
				ballCoveredRatio = 1.0;
			}
		}
	}
	return ballCoveredRatio;
	//printf("Ball Coverage:\t%f | me: (%5.1f, %5.1f) ball: (%5.1f, %5.1f)->%hu\r\n", ballCoveredRatio, myX, myY, theBall.xPos, theBall.yPos, theBall.radius);	
}

void checkLightLevel(){
	int16_t r, g, b;
	get_rgb(&r,&g,&b);
	int16_t sum = r+g+b;
	uint32_t now = get_time();
	//if(sum<=25){
		//paddleChange += ((now-lastLightCheck)*(3*PADDLE_VEL));
		////printf("Paddle ChangeX5: %f\r\n", paddleChange);
	//}else if(sum<=40){
		//paddleChange += ((now-lastLightCheck)*(2*PADDLE_VEL));
		////printf("Paddle ChangeX3: %f\r\n", paddleChange);
	//}else if(sum<=60){
		//paddleChange += ((now-lastLightCheck)*(PADDLE_VEL));
		////printf("Paddle ChangeX5: %f\r\n", paddleChange);
	//}
	if(sum<=60){
		isCovered=1;
	}else{
		isCovered=0;
	}
	lastLightCheck = now;
	//printf("Light: %5d (%4d, %4d, %4d)\r\n",sum,r,g,b);
}

void sendBallMsg(){
	if((((int16_t)myDist)==UNDF) || (myDist>=30)){
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

void handleBallMsg(BallMsg* msg, uint32_t arrivalTime){
	BALL_DEBUG_PRINT("Got Ball! T: %lu\r\n\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", get_time(), theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
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
	BALL_DEBUG_PRINT("\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
}

void sendNearBotsMsg(){ 
	if(myPos.x==UNDF && myPos.y==UNDF && myPos.o==UNDF){
		return;
	}
	NearBotsMsg msg;
	msg.flag = NEAR_BOTS_MSG_FLAG;
	packPos(&(msg.pos));
	//qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsCmpFunc);
	for(uint8_t i=0;i<NUM_SHARED_BOTS;i++){
		msg.shared[i].id = nearBots[i].myMeas.id;		
		msg.shared[i].range = packRange(nearBots[i].myMeas.r);
		msg.shared[i].b = packAngleMeas(nearBots[i].myMeas.b);
	}
	//getUsedBots(msg.used);
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(NearBotsMsg));
}

void handleNearBotsMsg(NearBotsMsg* msg, id_t senderID){
	OtherBot* nearBot = getOtherBot(senderID);
	if(nearBot){
		NB_DEBUG_PRINT("    (NearBotsMsg) ID: %04X ", senderID);
		unpackPos(&(msg->pos), &(nearBot->pos));
		if(nearBot->pos.x!=UNDF && nearBot->pos.y!=UNDF && nearBot->pos.o!=UNDF){
			NB_DEBUG_PRINT("| X: %4d Y: %4d O: %3d C: %3hu", nearBot->pos.x, nearBot->pos.y, nearBot->pos.o, nearBot->pos.stdDev);
		}
		for(uint8_t i=0;i<NUM_SHARED_BOTS;i++){
			if(msg->shared[i].id == get_droplet_id()){
				nearBot->theirMeas.r = unpackRange(msg->shared[i].range);
				nearBot->theirMeas.b = unpackAngleMeas(msg->shared[i].b);
				nearBot->theirMeas.h = UNDF;
				nearBot->theirMeas.id = get_droplet_id();
				nearBot->theirMeas.stdDev = getStdDevFromRange(nearBot->theirMeas.r);
				NB_DEBUG_PRINT("| R: %4u, B: % 4d", nearBot->theirMeas.r, nearBot->theirMeas.b);
				break;
			}
		}
		NB_DEBUG_PRINT("\r\n");
		//nearBot->hasNewInfo = checkNearBotForNewInfo(msg->used);
	}
}

//uint8_t checkNearBotForNewInfo(id_t usedBots[NUM_USED_BOTS]){
	//NEW_INFO_DEBUG_PRINT("\tUsed Bots: ");
	//uint8_t found = 1;
	//for(uint8_t i=0 ; i < NUM_USED_BOTS ; i++){
		//NEW_INFO_DEBUG_PRINT("%04X ", usedBots[i]);
		//if(usedBots[i] == get_droplet_id()){
			//found = 0;
		//}
	//}
	//NEW_INFO_DEBUG_PRINT("\r\n");
	//return found;
//}

void handle_msg(ir_msg* msg_struct){
	if(((BallMsg*)(msg_struct->msg))->flag==BALL_MSG_FLAG){
		handleBallMsg((BallMsg*)(msg_struct->msg), msg_struct->arrival_time);
	}else if(((NearBotsMsg*)(msg_struct->msg))->flag==NEAR_BOTS_MSG_FLAG){
		handleNearBotsMsg((NearBotsMsg*)(msg_struct->msg), msg_struct->sender_ID);
	//}else if(flag==N_PADDLE_MSG_FLAG || flag==S_PADDLE_MSG_FLAG){
		//handlePaddleMsg(flag, ((PaddleMsg*)(msg_struct->msg))->deltaPos);
	}else{
		printf_P(PSTR("%hu byte msg from %04X:\r\n\t"), msg_struct->length, msg_struct->sender_ID);
		for(uint8_t i=0;i<msg_struct->length;i++){
			printf("%02hX ", msg_struct->msg[i]);
		}
		printf("\r\n");
	}
}

void frameEndPrintout(){
	printf_P(PSTR("\nID: %04X T: %lu [ "), get_droplet_id(), get_time());
	switch(myState){
		case STATE_PIXEL:					printf("Pixel");		break;
		case (STATE_PIXEL|STATE_NORTH):		printf("North Pixel");	break;
		case (STATE_PIXEL|STATE_SOUTH):		printf("South Pixel");	break;
		default:							printf("???");			break;
	}
	printf_P(PSTR(" ]\r\n"));
	if((myPos.x != UNDF) && (myPos.y != UNDF) && (myPos.o != UNDF)){
		printf_P(PSTR("\tMy Pos: {%d, %d, %d, %hu}\r\n"), myPos.x, myPos.y, myPos.o, myPos.stdDev);
	}
	if((theBall.xPos != UNDF) && (theBall.yPos != UNDF)){
		printf_P(PSTR("\tBall ID: %hu; radius: %hu; Pos: (%d, %d) @ vel (%hd, %hd)\r\n"), theBall.id, theBall.radius, theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel);
	}
}

OtherBot* getOtherBot(id_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].myMeas.id==id){
			return &(nearBots[i]);
		}
	}
	return NULL;
}

void findAndRemoveOtherBot(id_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].myMeas.id==id){
			cleanOtherBot(&nearBots[i]);
			break;
		}
	}
}

void removeOtherBot(uint8_t idx){
	cleanOtherBot(&nearBots[idx]);
}

OtherBot* addOtherBot(id_t id){
	uint8_t emptyIdx=0xFF;
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsCmpFunc);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].myMeas.id==id){
			return &(nearBots[i]);
		}
		if(emptyIdx==0xFF && nearBots[i].myMeas.id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		return &(nearBots[emptyIdx]);
	}

	//BotPos* pos = &(nearBots[NUM_TRACKED_BOTS-1].pos);
	cleanOtherBot(&nearBots[NUM_TRACKED_BOTS]);
	return &(nearBots[NUM_TRACKED_BOTS]);
}

void cleanOtherBot(OtherBot* other){
	if(other==NULL) return;
	other->pos.x = UNDF;
	other->pos.y = UNDF;
	other->pos.o = UNDF;
	other->pos.stdDev = 255;
	other->myMeas.id = 0;
	other->myMeas.r = UNDF;
	other->myMeas.b = UNDF;
	other->myMeas.h = UNDF;
	other->myMeas.stdDev = 255;
	other->theirMeas.id = 0;
	other->theirMeas.r = UNDF;
	other->theirMeas.b = UNDF;
	other->theirMeas.h = UNDF;
	other->theirMeas.stdDev =255;
	other->lastUsed = 0;
	other->occluded = 0;
	//other->hasNewInfo = 0;
}

void printNearBots(){
	printf_P(PSTR("\t                         Near Bots                        \r\n"));
	printf_P(PSTR("\t ID  |   x  |   y  |   o  |  c  |  myR |  myB | thR |  thB \r\n"));
	for(uint8_t i=0; i<NUM_TRACKED_BOTS+1; i++){
		printOtherBot(&nearBots[i]);
	}
	printf("\r\n");
}

void printOtherBot(OtherBot* bot){
	if(bot==NULL) return;
	BotPos*  pos = &(bot->pos);
	BotMeas* myM = &(bot->myMeas);
	BotMeas* thM = &(bot->theirMeas);
	if(myM->id == 0) return;
	printf("\t%04X ", myM->id);
	if(pos->x!=UNDF && pos->y!=UNDF && pos->o!=UNDF){
		printf_P(PSTR("| %4d | %4d | % 4d | %3hu "), pos->x, pos->y, pos->o, pos->stdDev);
	}else{
		printf_P(PSTR("|  --  |  --  |  --  |  -- "));
	}
	printf_P(PSTR("| %4u | % 4d "), myM->r, myM->b);
	if(thM->id==get_droplet_id()){
		printf_P(PSTR("| %4u | % 4d "), thM->r, thM->b);
	}else{
		printf_P(PSTR("|  --  |  --  "));
	}
	printf("\r\n");
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args){	
	if(strcmp_P(command_word,PSTR("ball"))==0){
		if((UNDF!=myPos.x) && (UNDF!=myPos.y)){
			const char delim[2] = " ";
			char* token = strtok(command_args, delim);
			int8_t vel = (token!=NULL) ? (int8_t)atoi(token) : 10;
			token = strtok(NULL, delim);
			uint8_t size = (token!=NULL) ? (uint8_t)atoi(token) : 60;
			theBall.xPos = myPos.x;
			theBall.yPos = myPos.y;
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
			case 'p':	colorMode = POS;		break;
			case 's':	colorMode = SYNC_TEST;	break;
			case 'o':	colorMode = OFF;		break;
		}
		switch(command_args[1]){
			case 'p':	gameMode = PONG;		break;
			case 'b':	gameMode = BOUNCE;		break;
			default:	gameMode = BOUNCE;		break;
		}
		return 1;
	}else if(strcmp_P(command_word,PSTR("ball_kill"))==0){
		killBall();
		return 1;
	}else if(strcmp_P(command_word,PSTR("uB"))==0){
		switch(command_args[0]){
			case '1':	useBlacklist = 1; break;
			case '0':	useBlacklist = 0; break;
			default:	useBlacklist = !useBlacklist;
		}
		printf("Use blacklist: %hu\r\n",useBlacklist);
		return 1;
	}else if(strcmp_P(command_word, PSTR("uA"))==0){
		switch(command_args[0]){
			case '1':	useMeasAveraging = 1; break;
			case '0':	useMeasAveraging = 0; break;
			default:	useMeasAveraging = !useMeasAveraging;
		}
		printf("Use meas averaging: %hu\r\n",useMeasAveraging);
		return 1;
	}else if(strcmp_P(command_word, PSTR("sCT"))==0){
		stdDevThreshold = (uint8_t)atoi(command_args);
		printf("stdDevThreshold: %hu\r\n", stdDevThreshold);
		return 1;
	}else if(strcmp_P(command_word, PSTR("aPSD"))==0){
		addedPosStdDev = (uint8_t)atoi(command_args);
		printf("addedPosStdDev: %hu\r\n", addedPosStdDev);
		return 1;
	}else if(strcmp_P(command_word, PSTR("aMSD"))==0){
		addedMeasStdDev = (uint8_t)atoi(command_args);
		printf("addedMeasStdDev: %hu\r\n", addedMeasStdDev);
		return 1;
	}
	//else if(strcmp_P(command_word,PSTR("uN"))==0){
		//switch(command_args[0]){
			//case '1': useNewInfo = 1; break;
			//case '0': useNewInfo = 0; break;
			//default: useNewInfo = !useNewInfo;
		//}
		//printf("Use newInfo: %hu\r\n",useNewInfo);
		//return 1;
	//}

	return 0;
}


void addHardBot(id_t id){
	if(hardBotsList==NULL){
		hardBotsList = (HardBot*)myMalloc(sizeof(HardBot));
		hardBotsList->id = id;
		hardBotsList->next = NULL;
	}else{
		HardBot* temp = hardBotsList;
		while(temp->next!=NULL){
			if(temp->id==id) return; //requested ID is already added.
			temp = temp->next;
		}
		temp->next = (HardBot*)myMalloc(sizeof(HardBot));
		temp->next->id = id;
		temp->next->next = NULL;
	}
}

void cleanHardBots(){
	HardBot* temp;
	while(hardBotsList!=NULL){
		temp = hardBotsList->next;
		myFree(hardBotsList);
		hardBotsList = temp;
	}
}
//
//void getUsedBots(id_t dest[NUM_USED_BOTS]){
	//NEW_INFO_DEBUG_PRINT("My Used bots: ");
	//for(uint8_t i=0;i<NUM_USED_BOTS;i++){
		//NEW_INFO_DEBUG_PRINT("%04X ", lastUsedBots[i]);
		//dest[i] = lastUsedBots[i];
	//}
	//NEW_INFO_DEBUG_PRINT("\r\n");
//}
//
//void addUsedBot(id_t bot){
	//uint8_t curIdx = (NUM_USED_BOTS-1);
	//for(uint8_t i=0;i<NUM_USED_BOTS;i++){
		//if(lastUsedBots[i] == bot){
			//curIdx = i;
			//break;
		//}
	//}
	//for(uint8_t i=curIdx;i>=1;i--){
		//lastUsedBots[i] = lastUsedBots[i-1];
	//}
	//lastUsedBots[0] = bot;
//}
//
//void initUsedBots(){
	//for(uint8_t i=0;i<NUM_USED_BOTS;i++){
		//lastUsedBots[i] = 0;
	//}
//}