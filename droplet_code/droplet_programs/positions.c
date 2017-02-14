#include "droplet_programs/positions.h"

void init(){
	if((RNB_DUR+PADDING_DUR+POS_MSG_DUR+PADDING_DUR+MEAS_MSG_DUR+PADDING_DUR+MEAS_MSG_DUR+PADDING_DUR)>=(SLOT_LENGTH_MS)){
		printf_P(PSTR("You've got problems! SLOT_LENGTH_MS needs to be longer than all the things that take place during a slot!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	for(uint8_t i=0;i<NUM_TRACKED_BOTS+1;i++){
		cleanOtherBot(&nearBots[i]);
	}
	initPositions();
	mySlot = getSlot(get_droplet_id());
	printf("mySlot: %u, frame_length: %lu\r\n", mySlot, FRAME_LENGTH_MS);
	frameEndPrintout();
	xRange = MAX_X-MIN_X;
	yRange = MAX_Y-MIN_Y;
	maxRange = (int16_t)hypotf(xRange, yRange);
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
		//preppedMsgTarget = 0;
		if(loopID==mySlot){
			handleMySlot();
		}else if(loopID==SLOTS_PER_FRAME-1){
			handleFrameEnd();
		}else{
			//for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
				//id_t id = nearBots[i].myMeas.id;
				//BotPos* pos = &(nearBots[i].posFromMe);
				//if(id!=0 && loopID==getSlot(id)){
					//if(pos->x != UNDF && pos->y != UNDF && pos->o != UNDF){
						//prepBotMeasMsg(i);
						//msgExtraDelay = (/*rand_short()%3*/0)*(MEAS_MSG_DUR+PADDING_DUR);
						//POS_MSG_DEBUG_PRINT("Prepped to send %04X an update with delay %u. (Slot T: %u)\r\n", id, msgExtraDelay, (uint16_t)(frameTime%SLOT_LENGTH_MS));
					//}
					//break;
				//}
			//}
		}
		updateColor();
	}
	//if(preppedMsgTarget && (frameTime%SLOT_LENGTH_MS)>=(RNB_DUR+PADDING_DUR+POS_MSG_DUR+PADDING_DUR+msgExtraDelay)){
		//set_all_ir_powers(150);
		//ir_targeted_send(preppedMsgDirMask, (char*)(&preppedMsg), sizeof(BotMeasMsg), preppedMsgTarget);
		//set_all_ir_powers(256);
		//findAndRemoveOtherBot(preppedMsgTarget);		
		//POS_MSG_DEBUG_PRINT("\t%04X sent pos msg in dirs %02hX. (Slot T: %u)\r\n", preppedMsgTarget, preppedMsgDirMask, (uint16_t)(frameTime%SLOT_LENGTH_MS));
		//preppedMsgTarget = 0;
	//}
	//These things happen every single loop: once every LOOP_DELAY_MS.
	if(rnb_updated){
		useNewRnbMeas();
	}
	delay_ms(LOOP_DELAY_MS);
}

void handleMySlot(){
	while(((get_time()-frameStart)&SLOT_LENGTH_MS)<PADDING_DUR)
		delay_us(500);
	broadcast_rnb_data();
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR+PADDING_DUR)
		delay_us(500);
	if(POS_DEFINED(&myPos)){
		sendBotPosMsg();
	}
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR+PADDING_DUR+POS_MSG_DUR)
		delay_us(500);
	uint8_t targetBotIdxs[2] = {0xFF, 0xFF};
	getWeightedRandOtherBots(targetBotIdxs, 2);
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR+PADDING_DUR+POS_MSG_DUR+PADDING_DUR)	
		delay_us(500);
	if(targetBotIdxs[0]!=0xFF){
		sendBotMeasMsg(targetBotIdxs[0]);
	}
	while(((get_time()-frameStart)%SLOT_LENGTH_MS)<RNB_DUR+PADDING_DUR+POS_MSG_DUR+PADDING_DUR+MEAS_MSG_DUR+PADDING_DUR)
		delay_us(500);
	if(targetBotIdxs[1]!=0xFF){
		sendBotMeasMsg(targetBotIdxs[1]);
	}
}

void getWeightedRandOtherBots(uint8_t* botIdxs, uint8_t n){
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsCmpFunc);
	float totalRange = 0;
	float values[NUM_TRACKED_BOTS];
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(POS_DEFINED(&(nearBots[i].posFromMe))){
			values[i] = 1.0/(nearBots[i].myMeas.r);
			totalRange += values[i];
		}
	}
	float chooser;
	uint8_t numSoFar = 0;
	while( numSoFar<n ){
		chooser = rand_real()*totalRange;
		for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
			if(POS_DEFINED(&(nearBots[i].posFromMe))){
				if(chooser < values[i]){
					botIdxs[numSoFar] = i;
					totalRange -= values[i];
					values[i] = 0;
					break;
				}
				chooser -= values[i];
			}
		}
		if(botIdxs[numSoFar]==0xFF){
			break;	
		}else{
			numSoFar++;
		}
	}
}

uint8_t nearBotUseabilityCheck(uint8_t i){
	if(!(nearBots[i].myMeas.id)){
		return 0;
	}
	if(nearBots[i].occluded){
		return 0;
	}
	for(uint8_t j=0;j<NUM_SEEDS;j++){
		if(nearBots[i].myMeas.id==SEED_IDS[j]){
			return 0;
		}
	}
	return 1;
}

/************************************************************************/
/* omega: selected to minimize (omega*myPinv + (1-omega)*yourPinv)^{-1} */
/* but that's hard. So, insted we're doing a cheat, as described in		*/
/* Franken & Hupper's 2005 Information Fusion paper, "Improved Fast		*/
/* Covariance Intersection for Distributed Data Fusion"					*/
/* TODO (small):														*/
/* I am somewhat worried about the numerical stability of this cheat.   */
/************************************************************************/
float chooseOmega(Matrix* myPinv, Matrix* yourPinv){
	float omega;
	Matrix sum;
	matrixAdd(&sum, myPinv, yourPinv);
	float sumDet = matrixDet(&sum);
	float myInvDet = matrixDet(myPinv);
	float yourInvDet = matrixDet(yourPinv);
	omega = (sumDet - yourInvDet + myInvDet)/(2*sumDet);
	omega = omega>1.0 ? 1.0 : (omega<0.0 ? 0.0 : omega);
	return omega;
}

void ciUpdatePos(uint8_t idx, BotPos* pos, Matrix* yourP){
	Vector xMe = {perSeedPos[idx].x, perSeedPos[idx].y, perSeedPos[idx].o};
	if(!POS_DEFINED(pos)){
		MY_POS_DEBUG_PRINT(" sent me an undefined position.\r\n");
		return;
	}
	MY_POS_DEBUG_PRINT("thinks I'm at {%d, %d, %d}", pos->x, pos->y, pos->o);
	Vector xMeFromYou = {pos->x, pos->y, pos->o};
	Matrix myP;
	Matrix myPinv;
	decompressP(&myP, &(perSeedCovars[idx]));
	matrixInverse(&myPinv, &myP);
	Matrix yourPinv;
	matrixInverse(&yourPinv, yourP);
	Matrix myNewP;
	Vector myNewPos;
	float omega = chooseOmega(&myPinv, &yourPinv);
	//myNewP = (omega*myPinv + (1-omega)*yourPinv)^{-1}
	matrixScale(&myPinv, omega);
	matrixScale(&yourPinv, 1.0-omega);
	matrixAdd(&myNewP, &myPinv, &yourPinv);
	matrixInplaceInverse(&myNewP);
	//myNewPos = myNewP*(omega*myPinv*myPrevPos + (1-omega)*yourPinv*yourPos)
	matrixTimesVector(&myNewPos, &myPinv, &xMe);
	Vector tmp;
	matrixTimesVector(&tmp, &yourPinv, &xMeFromYou);
	vectorAdd(&tmp, &myNewPos, &tmp);
	matrixTimesVector(&myNewPos, &myNewP, &tmp);
	perSeedPos[idx].x = myNewPos[0]>1023 ? 1023 : (myNewPos[0]<-1023 ? -1023 : myNewPos[0]);
	perSeedPos[idx].y = myNewPos[1]>1023 ? 1023 : (myNewPos[1]<-1023 ? -1023 : myNewPos[1]);
	perSeedPos[idx].o = pretty_angle_deg(myNewPos[2]);
	MY_POS_DEBUG_PRINT(" giving pos {%d, %d, %d} (omega: %5.3f).\r\n", perSeedPos[idx].x, perSeedPos[idx].y, perSeedPos[idx].o, omega);
	//#ifdef MY_POS_DEBUG_MODE
		//POS_DEBUG_PRINT("His Est Covar:\r\n");
		//printMatrixMathematica(&yourP);
		//POS_DEBUG_PRINT("My new covar:\r\n");
		//printMatrixMathematica(&myNewP);
	//#endif
	compressP(&myNewP, &(perSeedCovars[idx]));		
}

void fusePerSeedMeas(){
	// This function needs to intelligently combine the independent position estimates 
	// which we have from each seed.
	Vector posSum;
	Matrix infoMatSum;
	uint8_t lastSeed = 0xFF;
	for(uint8_t i=0;i<3;i++){
		posSum[i] = 0;
		for(uint8_t j=0;j<3;j++){
			infoMatSum[i][j] = 0;
		}
	}
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(POS_DEFINED(&(perSeedPos[i]))){
			lastSeed = (lastSeed==0xFF) ? i : 0x0F;
			Matrix thisInfoMat;
			decompressP(&thisInfoMat, &(perSeedCovars[i]));
			Vector thisPos = {perSeedPos[i].x, perSeedPos[i].y, perSeedPos[i].o};
			Vector tmp;
			matrixInplaceInverse(&thisInfoMat);
			matrixAdd(&infoMatSum, &thisInfoMat, &infoMatSum);
			matrixTimesVector(&tmp, &thisInfoMat, &thisPos);
			vectorAdd(&posSum, &tmp, &posSum);
		}
	}
	if(lastSeed==0xFF){
		myPos.x = myPos.y = myPos.o = UNDF;
	}else if(lastSeed<=NUM_SEEDS){
		myPos.x = perSeedPos[lastSeed].x;
		myPos.y = perSeedPos[lastSeed].y;
		myPos.o = perSeedPos[lastSeed].o;
	}else{
		Vector tmp;
		matrixInplaceInverse(&infoMatSum);
		matrixTimesVector(&tmp, &infoMatSum, &posSum);
		myPos.x = (int16_t)tmp[0];
		myPos.y = (int16_t)tmp[1];
		myPos.o = pretty_angle_deg(tmp[2]);
	}
}

void updatePositions(){
	fusePerSeedMeas();
	if(!POS_DEFINED(&myPos)){
		printf("Can't adjust others' positions until I know where I am.\r\n");
		return;
	}
	POS_CALC_DEBUG_PRINT("Updating Positions!\r\n");
	Matrix myP;
	float pDets[NUM_SEEDS];
	float pDetsTotal=0;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(POS_DEFINED(&(perSeedPos[i]))){
			decompressP(&myP, &(perSeedCovars[i]));
			pDets[i] = matrixDet(&myP);
			pDets[i] = powf(pDets[i],1.0/6.0);
			pDetsTotal+=1.0/pDets[i];
		}else{
			pDets[i]=NAN;
		}
	}
	uint8_t prev = 255;
	uint8_t thresh[NUM_SEEDS];
	uint8_t tmp;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(isnan(pDets[i])){
			thresh[i] = prev;
		}else{
			tmp = (255*((1.0/pDets[i])*(1.0/pDetsTotal))+0.5);
			if(tmp > prev){
				thresh[i] = 0;
			}else{
				thresh[i] = prev-tmp;
			}
		}
		prev = thresh[i];
	}

	if(pDetsTotal==0){
		printf("Not suffficiently confident for any seedIdxs.\r\n");
		return;
	}
	//printf("\tThresholds: {%hu, %hu, %hu, %hu}\r\n", thresh[0], thresh[1], thresh[2], thresh[3]);
	uint8_t thisSeedIdx;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		thisSeedIdx=0xFF;
		if(nearBotUseabilityCheck(i)){
			uint8_t chooser = (uint8_t)(rand_short()%255 + 1);
			for(uint8_t j=0;j<NUM_SEEDS;j++){
				if(chooser>thresh[j]){
					thisSeedIdx = j;
					break;
				}
			}
			if(thisSeedIdx==0xFF){
				printf("Something has gone very wrong with thisSeedIdx!\r\n");
			}
			Vector x_me = {perSeedPos[thisSeedIdx].x, perSeedPos[thisSeedIdx].y, perSeedPos[thisSeedIdx].o};
			Matrix G;
			populateGammaMatrix(&G, &x_me);
			Matrix Gtp;
			matrixTranspose(&Gtp, &G);
			Vector z;
			calcRelativePose(&z, &(nearBots[i].myMeas));
			Matrix R;
			getMeasCovar(&R, &(nearBots[i].myMeas));
			//yourX = myX + G.z
			Vector x_you;
			matrixTimesVector(&x_you, &G, &z);
			vectorAdd(&x_you, &x_me, &x_you);
			Matrix H;
			populateHMatrix(&H, &x_me, &x_you);
			Matrix tmp;
			Matrix yourP;
			//yourP = H.myP.tp(H) + G.R.tp(G)
			matrixMultiply(&tmp, &G, &R);
			matrixMultiply(&yourP, &tmp, &Gtp); //now yourP is G.R.tp(G)
			matrixMultiply(&tmp, &H, &myP);
			matrixInplaceTranspose(&H);
			matrixInplaceMultiply(&tmp, &tmp, &H); //now tmp is H.myP.tp(H)
			matrixAdd(&yourP, &tmp, &yourP);
			
			if(positiveDefiniteQ(&yourP)){
				POS_CALC_DEBUG_PRINT("\t%04X @ {%6.1f, %6.1f, % 5.0f} from {% 4d, % 4d, % 4d}\r\n", nearBots[i].myMeas.id, x_you[0], x_you[1], x_you[2], nearBots[i].myMeas.r, nearBots[i].myMeas.b, nearBots[i].myMeas.h);
				nearBots[i].posFromMe.x = x_you[0]>1023 ? 1023 : (x_you[0]<-1023 ? -1023 : x_you[0]);
				nearBots[i].posFromMe.y = x_you[1]>1023 ? 1023 : (x_you[1]<-1023 ? -1023 : x_you[1]);
				nearBots[i].posFromMe.o = pretty_angle_deg(x_you[2]);
				compressP(&yourP, &(nearBots[i].posCovar));
				nearBots[i].seedIdx = thisSeedIdx;
			}else{
				removeOtherBot(i);
			}
		}
	}
}

void getMeasCovar(Matrix* R, BotMeas* meas){
	//Matrix* start = (meas->r)<=80 ? &deltaPoseCovarClose : &deltaPoseCovarMed;
	Matrix* start = (meas->r)<=130 ? &measCovarClose : ( (meas->r)<=185 ? &measCovarMed : &measCovarFar );
	matrixCopy(R, start);
	Matrix H; //This is the jacobian of the transformation from (r,b,h) to (deltaX, deltaY, deltaO)
	float cosO = cos(deg_to_rad(meas->b+90));
	float sinO = sin(deg_to_rad(meas->b+90));
	float degToRad = M_PI/180.0;
	H[0][0] = cosO;
	H[0][1] = -degToRad*(meas->r)*sinO;
	H[0][2] = 0;
	H[1][0] = sinO;
	H[1][1] = degToRad*(meas->r)*cosO;
	H[1][2] = 0;
	H[2][0] = 0;
	H[2][1] = 0;
	H[2][2] = 1;
	matrixInplaceMultiply(R, &H, R);
	matrixInplaceTranspose(&H);
	matrixInplaceMultiply(R, R, &H);
}

void calcRelativePose(Vector* pose, BotMeas* meas){
	uint16_t r   = meas->r;
	int16_t b	 = meas->b;
	int16_t h    = meas->h;
	(*pose)[0] = r * cos(deg_to_rad(b + 90));
	(*pose)[1] = r * sin(deg_to_rad(b + 90));
	(*pose)[2] = h;
}

void populateGammaMatrix(Matrix* G, Vector* pos){
	float cosO = cos(deg_to_rad((*pos)[2]));
	float sinO = sin(deg_to_rad((*pos)[2]));
	(*G)[0][0] = cosO;
	(*G)[0][1] = -sinO;
	(*G)[0][2] = 0;
	(*G)[1][0] = sinO;
	(*G)[1][1] = cosO;
	(*G)[1][2] = 0;
	(*G)[2][0] = 0;
	(*G)[2][1] = 0;
	(*G)[2][2] = 1;
}

void populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you){
	float degToRad = M_PI/180.0;
	(*H)[0][0] = 1;
	(*H)[0][1] = 0;
	(*H)[0][2] = degToRad*((*x_me)[1] - (*x_you)[1]);
	(*H)[1][0] = 0;
	(*H)[1][1] = 1;
	(*H)[1][2] = degToRad*((*x_you)[0] - (*x_me)[0]);
	(*H)[2][0] = 0;
	(*H)[2][1] = 0;
	(*H)[2][2] = 1;
}

void compressP(Matrix* P, DensePosCovar* covar){
	for(uint8_t i=0;i<3;i++){
		(*P)[i][i] = ((*P)[i][i])/4 + 0.5;
		(*P)[i][i] = ((*P)[i][i] > 65535) ? 65535 : (*P)[i][i];
		for(uint8_t j=i+1;j<3;j++){
			(*P)[i][j] = ((*P)[i][j])/8 + 0.5;
			(*P)[i][j] = ((*P)[i][j] > 32767) ? 32767 : ( ((*P)[i][j]<-32768) ? -32768 : (*P)[i][j] );
		}
	}
	(*covar)[0].u = (uint16_t)(*P)[0][0];
	(*covar)[1].d =  (int16_t)(*P)[0][1];
	(*covar)[2].d =  (int16_t)(*P)[0][2];
	(*covar)[3].u = (uint16_t)(*P)[1][1];
	(*covar)[4].d =  (int16_t)(*P)[1][2];
	(*covar)[5].u = (uint16_t)(*P)[2][2];
}

void decompressP(Matrix* P, DensePosCovar* covar){
	(*P)[0][0] = (*covar)[0].u;
	(*P)[0][1] = (*covar)[1].d;
	(*P)[0][2] = (*covar)[2].d;
	(*P)[1][1] = (*covar)[3].u;
	(*P)[1][2] = (*covar)[4].d;
	(*P)[2][2] = (*covar)[5].u;
	for(uint8_t i=0;i<3;i++){
		(*P)[i][i] = 4*((*P)[i][i]);
		for(uint8_t j=i+1;j<3;j++){
			(*P)[i][j] = 8*((*P)[i][j]);
			(*P)[j][i] = (*P)[i][j];
		}
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
	updateNearBotOcclusions();
	updatePositions();
	
	////Removing the worst half of the nearBots.
	//for(uint8_t i=NUM_TRACKED_BOTS/2; i<NUM_TRACKED_BOTS;i++){
		//cleanOtherBot(&(nearBots[i]));
	//}
	frameEndPrintout();
	printf("\r\n");
}

void useNewRnbMeas(){
	//Pulling everything out of the global struct.
	uint16_t id = last_good_rnb.id;
	uint16_t range = last_good_rnb.range;
	int16_t bearing = last_good_rnb.bearing;
	int16_t heading = last_good_rnb.heading;
	rnb_updated=0;
	if(range>200){
		return;
	}
	RNB_DEBUG_PRINT("            (RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", id, range, bearing, heading);
	OtherBot* measuredBot = addOtherBot(id);
	BotMeas* meas = &(measuredBot->myMeas);
	if(meas->id == 0 || meas->id == id){ 
		meas->id	 = id;
		meas->r		 = range;
		meas->b		 = bearing;
		meas->h		 = heading;
	}else{
		printf_P(PSTR("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n"));
	}
}

void updateColor(){
	uint8_t newR = 0, newG = 0, newB = 0;
	if(frameCount%8==7){ //sync test color
		hsv_to_rgb((uint16_t)(60*((loopID%36)>>2)), 220, 127, &newR, &newG, &newB);
	}else{ //pos color
		if(POS_DEFINED(&myPos)){
			int16_t xColVal = (int16_t)(6.0*pow(41.0,(myPos.x-MIN_X)/((float)xRange))+9.0);
			int16_t yColVal = (int16_t)(3.0*pow(84.0,(myPos.y-MIN_Y)/((float)yRange))+3.0);
			newR = (uint8_t)(xColVal);
			newG = (uint8_t)(yColVal);
		}else{
			newR = newG = newB = 50;
		}
	}

	set_rgb(newR, newG, newB);	
}

void sendBotMeasMsg(uint8_t i){ //i: index in nearBots array.
	BotMeasMsg msg;
	msg.flag = BOT_MEAS_MSG_FLAG;
	copyBotPos(&(nearBots[i].posFromMe), &(msg.pos));
	for(uint8_t j=0;j<6;j++){
		msg.covar[j].u = nearBots[i].posCovar[j].u;
	}
	msg.seedIdx = nearBots[i].seedIdx;
	uint8_t dir;
	uint8_t dirMask = 0;
	for(int8_t j=-45;j<90;j+=45){
		dir = dirFromAngle(pretty_angle_deg(nearBots[i].myMeas.b + j));
		dirMask |= 1<<dir;
	}
	ir_targeted_send(dirMask, (char*)(&msg), sizeof(BotMeasMsg), nearBots[i].myMeas.id);
	POS_MSG_DEBUG_PRINT("%04X sent pos msg in dirs %02hX.\r\n", nearBots[i].myMeas.id, dirMask);
}

//void prepBotMeasMsg(uint8_t i){ //i: index in nearBots array.
	//preppedMsg.flag = BOT_MEAS_MSG_FLAG;
	//copyBotPos(&(nearBots[i].posFromMe), &(preppedMsg.pos));
	//for(uint8_t j=0;j<6;j++){
		//preppedMsg.covar[j].u = nearBots[i].posCovar[j].u;
	//}
	//preppedMsg.seedIdx = nearBots[i].seedIdx;
	//uint8_t dir;
	//preppedMsgDirMask = 0;
	//for(int8_t j=-45;j<90;j+=45){
		 //dir = dirFromAngle(pretty_angle_deg(nearBots[i].myMeas.b + j));
		 //preppedMsgDirMask |= 1<<dir;
	//}
	////preppedMsgDirMask = 1<<dirFromAngle(nearBots[i].myMeas.b);
	//preppedMsgTarget = nearBots[i].myMeas.id;
//}

void handleBotMeasMsg(BotMeasMsg* msg, id_t senderID __attribute__ ((unused))){
	uint8_t seedIdx = msg->seedIdx;
	if(!POS_DEFINED(&(msg->pos))){
		MY_POS_DEBUG_PRINT("%04X sent me an undefined position.\r\n", senderID);
		return;
	}
	Matrix covar;
	decompressP(&covar, &(msg->covar));
	if(!POS_DEFINED(&(perSeedPos[seedIdx]))){
		float yourTrace = covar[0][0]+covar[1][1] + covar[2][2];
		//if(powf(yourDet,1.0/6.0)>80){
			//MY_POS_DEBUG_PRINT("%04X's 'StdDev' (%f) to high for me to initialize.\r\n", senderID, powf(yourDet,1.0/6.0));
			//return;
		//}else{
			MY_POS_DEBUG_PRINT("%04X initialized seed %hu to {%d, %d, %d}. (Covar Trace: %f)\r\n", senderID, seedIdx, (msg->pos).x, (msg->pos).y, (msg->pos).o, yourTrace);
		//}
		perSeedPos[seedIdx].x = (msg->pos).x;
		perSeedPos[seedIdx].y = (msg->pos).y;
		perSeedPos[seedIdx].o = (msg->pos).o;
		for(uint8_t i=0;i<6;i++){
			perSeedCovars[seedIdx][i].u = msg->covar[i].u;
		}
	}else{
		MY_POS_DEBUG_PRINT("%04X ", senderID);
		ciUpdatePos(seedIdx, &(msg->pos), &covar);
	}
}

void sendBotPosMsg(){
	BotPosMsg msg;
	copyBotPos(&myPos, &(msg.pos));
	msg.flag = BOT_POS_MSG_FLAG;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BotPosMsg));
}

void handleBotPosMsg(BotPosMsg* msg, id_t senderID){
	printf("\t%04X @ {%4d, %4d, % 4d}\r\n", senderID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
}

void handle_msg(ir_msg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else if(((BotPosMsg*)(msg_struct->msg))->flag==BOT_POS_MSG_FLAG && msg_struct->length==sizeof(BotPosMsg)){
		handleBotPosMsg((BotPosMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else{
		printf_P(PSTR("%hu byte msg from %04X:\r\n\t"), msg_struct->length, msg_struct->sender_ID);
		for(uint8_t i=0;i<msg_struct->length;i++){
			printf("%02hX ", msg_struct->msg[i]);
		}
		printf("\r\n");
	}
}

void frameEndPrintout(){
	printf_P(PSTR("\nID: %04X T: %lu "), get_droplet_id(), get_time());
	if(POS_DEFINED(&myPos)){
		printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
		if(POS_DEFINED(&(perSeedPos[0]))){
			printf("\tNW Pos: {%4d, % 4d, % 4d}", perSeedPos[0].x, perSeedPos[0].y, perSeedPos[0].o);
		}else{
			printf("\tNW Pos: { -- ,  -- ,  -- }");
		}
		if(POS_DEFINED(&(perSeedPos[1]))){
			printf("           NE Pos: {%4d, % 4d, % 4d}\r\n",	perSeedPos[1].x, perSeedPos[1].y, perSeedPos[1].o);
		}else{
			printf("           NE Pos: { -- ,  -- ,  -- }\r\n");
		}
		printTwoPosCovar(&(perSeedCovars[0]), &(perSeedCovars[1]));
		if(POS_DEFINED(&(perSeedPos[2]))){
			printf("\tSW Pos: {%4d, % 4d, % 4d}", perSeedPos[2].x, perSeedPos[2].y, perSeedPos[2].o);
		}else{
			printf("\tSW Pos: { -- ,  -- ,  -- }");			
		}
		if(POS_DEFINED(&(perSeedPos[3]))){
			printf("           SE Pos: {%4d, % 4d, % 4d}\r\n",	perSeedPos[3].x, perSeedPos[3].y, perSeedPos[3].o);
		}else{
			printf("           SE Pos: { -- ,  -- ,  -- }\r\n");
		}
		printTwoPosCovar(&(perSeedCovars[2]), &(perSeedCovars[3]));
	}else{
		printf("\r\n");
	}
}

void printTwoPosCovar(DensePosCovar* P1, DensePosCovar* P2){
	float xx1 = (*P1)[0].u;
	float xy1 = (*P1)[1].d;
	float xo1 = (*P1)[2].d;
	float yy1 = (*P1)[3].u;
	float yo1 = (*P1)[4].d;
	float oo1 = (*P1)[5].u;
	float xx2 = (*P2)[0].u;
	float xy2 = (*P2)[1].d;
	float xo2 = (*P2)[2].d;
	float yy2 = (*P2)[3].u;
	float yo2 = (*P2)[4].d;
	float oo2 = (*P2)[5].u;
	xx1 = xx1*4;
	xy1 = xy1*8;
	xo1 = xo1*8;
	yy1 = yy1*4;
	yo1 = yo1*8;
	oo1 = oo1*4;
	xx2 = xx2*4;
	xy2 = xy2*8;
	xo2 = xo2*8;
	yy2 = yy2*4;
	yo2 = yo2*8;
	oo2 = oo2*4;
	printf("\t{                                    {\r\n");
	printf("\t  {%7.1f, %7.1f, %7.1f},          {%7.1f, %7.1f, %7.1f},\r\n", xx1, xy1, xo1, xx2, xy2, xo2);
	printf("\t  {%7.1f, %7.1f, %7.1f},          {%7.1f, %7.1f, %7.1f},\r\n", xy1, yy1, yo1, xy2, yy2, yo2);
	printf("\t  {%7.1f, %7.1f, %7.1f}           {%7.1f, %7.1f, %7.1f},\r\n", xo1, yo1, oo1, xo2, yo2, oo2);
	printf("\t}                                    }\r\n");
}

void printPosCovar(DensePosCovar* P){
	float xx = (*P)[0].u;
	float xy = (*P)[1].d;
	float xo = (*P)[2].d;
	float yy = (*P)[3].u;
	float yo = (*P)[4].d;
	float oo = (*P)[5].u;
	xx = xx*4;
	xy = xy*8;
	xo = xo*8;
	yy = yy*4;
	yo = yo*8;
	oo = oo*4;
	printf("\t{\r\n\t  {%7.1f, %7.1f, %7.1f},\r\n", xx, xy, xo);
	printf("\t  {%7.1f, %7.1f, %7.1f},\r\n",	    xy, yy, yo);
	printf("\t  {%7.1f, %7.1f, %7.1f}\r\n\t}\r\n",  xo, yo, oo);
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
	other->posFromMe.x = UNDF;
	other->posFromMe.y = UNDF;
	other->posFromMe.o = UNDF;
	other->myMeas.id = 0;
	other->myMeas.r = UNDF;
	other->myMeas.b = UNDF;
	other->myMeas.h = UNDF;
	for(uint8_t i=0;i<6;i++){
		other->posCovar[i].u = 0;
	}
	other->occluded = 0;
	other->seedIdx = 255;
	//other->hasNewInfo = 0;
}

//void printNearBots(){
	//printf_P(PSTR("\t                         Near Bots                        \r\n"));
	//printf_P(PSTR("\t ID  |   x  |   y  |   o  |  myR |  myB | thR |  thB \r\n"));
	//for(uint8_t i=0; i<NUM_TRACKED_BOTS+1; i++){
		//printOtherBot(&nearBots[i]);
	//}
	//printf("\r\n");
//}

//void printOtherBot(OtherBot* bot){
	//if(bot==NULL) return;
	//BotPos*  pos = &(bot->posFromMe);
	//BotMeas* myM = &(bot->myMeas);
	//BotMeas* thM = &(bot->theirMeas);
	//if(myM->id == 0) return;
	//printf("\t%04X ", myM->id);
	//if(pos->x!=UNDF && pos->y!=UNDF && pos->o!=UNDF){
		//printf_P(PSTR("| %4d | %4d | % 4d "), pos->x, pos->y, pos->o);
	//}else{
		//printf_P(PSTR("|  --  |  --  |  --  "));
	//}
	//printf_P(PSTR("| %4u | % 4d "), myM->r, myM->b);
	//if(thM->id==get_droplet_id()){
		//printf_P(PSTR("| %4u | % 4d "), thM->r, thM->b);
	//}else{
		//printf_P(PSTR("|  --  |  --  "));
	//}
	//printf("\r\n");
//}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){	
	//return 0;
//}

void printNearBots(){
	printf_P(PSTR("\t        Near Bots        \r\n"));
	printf_P(PSTR("\t ID  |   r  |   b  |   h  \r\n"));
	for(uint8_t i=0; i<NUM_TRACKED_BOTS+1; i++){
		printOtherBot(&nearBots[i]);
	}
	printf("\r\n");
}

void printOtherBot(OtherBot* bot){
	if(bot==NULL) return;
	//BotPos*  pos = &(bot->pos);
	BotMeas* myM = &(bot->myMeas);
	if(myM->id == 0) return;
	printf("\t%04X ", myM->id);
	printf("| %4d | %4d ", myM->r, myM->b);
	if(myM->h==UNDF){
		printf("|  --  \r\n");
	}else{
		printf("| %4d \r\n", myM->h);
	}
}