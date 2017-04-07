#include "droplet_programs/positions.h"

void init(){
	if((PADDING_DUR+RNB_DUR+PADDING_DUR+POS_MSG_DUR+PADDING_DUR+MEAS_MSG_DUR+PADDING_DUR+MEAS_MSG_DUR)>=(SLOT_LENGTH_MS)){
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
			slotTasksSwitch = 0;
			targetBotIdxs[0] = 0xFF;
			targetBotIdxs[1] = 0xFF;
		}else if(loopID==SLOTS_PER_FRAME-1){
			handleFrameEnd();
		}
		updateColor();
	}
	if(loopID==mySlot){
		handleMySlot(frameTime);
	}
	if(rnb_updated){
		useNewRnbMeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	delay_ms(LOOP_DELAY_MS);
}

void handleMySlot(uint32_t frameTime){
	uint32_t loopTime = frameTime%SLOT_LENGTH_MS;
	if(!(slotTasksSwitch & SLOT_TASK_RNB_bm) && (loopTime>PADDING_DUR)){
		broadcast_rnb_data();
		slotTasksSwitch |= SLOT_TASK_RNB_bm;
	}else if(!(slotTasksSwitch & SLOT_TASK_POS_MSG_bm) && (loopTime>(PADDING_DUR + RNB_DUR + PADDING_DUR))){
		if(POS_DEFINED(&myPos)){
			if(rand_byte()%2==0){
				sendBotPosMsg();
			}
		}
		slotTasksSwitch |= SLOT_TASK_POS_MSG_bm;
	}else if(!(slotTasksSwitch & SLOT_TASK_MEAS_MSG0_bm) && (loopTime>(PADDING_DUR + RNB_DUR + PADDING_DUR + POS_MSG_DUR + PADDING_DUR))){
		getWeightedRandOtherBots(targetBotIdxs, 2);
		if(targetBotIdxs[0]!=0xFF){
			sendBotMeasMsg(targetBotIdxs[0]);
		}
		slotTasksSwitch |= SLOT_TASK_MEAS_MSG0_bm;
	}else if(!(slotTasksSwitch & SLOT_TASK_MEAS_MSG1_bm) && (loopTime>(PADDING_DUR + RNB_DUR + PADDING_DUR + POS_MSG_DUR + PADDING_DUR + MEAS_MSG_DUR + PADDING_DUR))){
		if(targetBotIdxs[1]!=0xFF){
			sendBotMeasMsg(targetBotIdxs[1]);
		}
		slotTasksSwitch |= SLOT_TASK_MEAS_MSG1_bm;
	}
}

void getWeightedRandOtherBots(uint8_t* botIdxs, uint8_t n){
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), smallRangeSorter);
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

float mahalanobisDistance(Vector* a, Matrix* A, Vector* b, Matrix* B){
	Vector a_sub_b;
	vectorSubtract(&a_sub_b, a, b);
	Matrix A_plus_B_inv;
	matrixAdd(&A_plus_B_inv, A, B);
	matrixInplaceInverse(&A_plus_B_inv);
	Vector a_sub_b_normed;
	matrixTimesVector(&a_sub_b_normed, &A_plus_B_inv, &a_sub_b);
	//Now, a_sub_b_normed = (A+B)^{-1} X (a-b)
	float distance = a_sub_b[0]*a_sub_b_normed[0] + a_sub_b[1]*a_sub_b_normed[1] + a_sub_b[2]*a_sub_b_normed[2];	
	//Now, distance = (a-b)^{tr} X (A+B)^{-1} X (a-b)
	return distance;
}

//P is covar matrix resulting from CI of A & B (with means a & b, respectively)
//x is resulting mean.
//CITE:
//"Decentralized Multi-robot Cooperative Localization using Covariance Intersection"
//by Luic C. Carillo-Arce et. al.
void covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B){
	Matrix A_inv;
	matrixInverse(&A_inv, A);
	Matrix B_inv;
	matrixInverse(&B_inv, B);
	float omega = chooseOmega(&A_inv, &B_inv);
	//myNewP = (omega*myPinv + (1-omega)*yourPinv)^{-1}
	matrixScale(&A_inv, omega);
	matrixScale(&B_inv, 1.0-omega);
	matrixAdd(P, &A_inv, &B_inv);
	matrixInplaceInverse(P);
	//myNewPos = myNewP*(omega*myPinv*myPrevPos + (1-omega)*yourPinv*yourPos)
	matrixTimesVector(x, &A_inv, a);
	Vector tmp;
	matrixTimesVector(&tmp, &B_inv, b);
	vectorAdd(&tmp, x, &tmp);
	matrixTimesVector(x, P, &tmp);
}

//x is the mean we will use.
//U is covar matrix, guaranteed to be consistent with (a,A) and (b,B)
//CITE:
//"Generalized Covariance Union: A Unified Approach to Hypothesis Merging in Tracking"
//by Steven Reece and Stephen Roberts
void covarUnion(Vector* x, Matrix* U, Vector* a, Matrix* A, Vector* b, Matrix* B){
	Vector a_dist;
	vectorSubtract(&a_dist, x, a);
	Matrix U_a;
	vectorSquare(&U_a, &a_dist);
	matrixAdd(&U_a, A, &U_a);

	Vector b_dist;
	vectorSubtract(&b_dist, x, b);
	Matrix U_b;
	vectorSquare(&U_b, &b_dist);
	matrixAdd(&U_b, B, &U_b);
	Matrix S;
	choleskyDecomposition(&S, &U_b);
	Matrix S_tr;
	matrixTranspose(&S_tr, &S);

	//printf("s = ");
	//printMatrixMathematica(&S);

	Matrix S_inv;
	matrixInverse(&S_inv, &S);

	Matrix TMP;
	matrixMultiply(&TMP, &U_a, &S_inv);
	matrixInplaceTranspose(&S_inv);
	matrixInplaceMultiply(&TMP, &S_inv, &TMP);
	//printf("SinvUSinv = ");
	//printMatrixMathematica(&TMP);

	//TMP = ((S^{-1})^{T}) x U_a x (S^{-1})
	Vector eigVals;
	Matrix V_tr; //eigenVectors
	eigensystem(&eigVals, &V_tr, &TMP);
	Matrix D = {{eigVals[0], 0, 0}, {0, eigVals[1], 0}, {0, 0, eigVals[2]}};

	Matrix V;
	matrixTranspose(&V, &V_tr);

	//printf("v = ");
	//printMatrixMathematica(&V);
	//printf("d = ");
	//printMatrixMathematica(&D);

	//matrixMultiply(&TMP, &V_tr, &S);
	//matrixInplaceMultiply(&TMP, &D, &TMP);
	//matrixInplaceMultiply(&TMP, &V, &TMP);
	//matrixInplaceMultiply(&TMP, &S_tr, &TMP);
//
	//printf("(*TEST ONE*)\r\n");
	//printf("uA = ");
	//printMatrixMathematica(&U_a);
	//printf("tstA = ");
	//printMatrixMathematica(&TMP);
//
	//matrixMultiply(&TMP, &V_tr, &S);
	//matrixInplaceMultiply(&TMP, &V, &TMP);
	//matrixInplaceMultiply(&TMP, &S_tr, &TMP);

	//printf("(*TEST TWO*)\r\n");
	//printf("uB = ");
	//printMatrixMathematica(&U_b);
	//printf("tstB = ");
	//printMatrixMathematica(&TMP);

	Matrix C = {{1,0,0},{0,1,0},{0,0,1}}; 
	for(uint8_t i=0;i<3;i++){
		C[i][i] = D[i][i]>1 ? D[i][i] : 1;
	}
	matrixMultiply(&TMP, &V_tr, &S);
	matrixMultiply(U, &V, &C);
	matrixInplaceMultiply(U, U, &TMP);
	matrixInplaceMultiply(U, &S_tr, U);
}

void updatePos(BotPos* pos, Matrix* yourP){
	Vector xMe = {myPos.x, myPos.y, deg_to_rad(myPos.o)};
	if(!POS_DEFINED(pos)){
		MY_POS_DEBUG_PRINT(" sent me an undefined position.\r\n");
		return;
	}
	MY_POS_DEBUG_PRINT("thinks I'm at {%d, %d, %d}", pos->x, pos->y, pos->o);
	Vector xMeFromYou = {pos->x, pos->y, deg_to_rad(pos->o)};
	Matrix myP;
	decompressP(&myP, &myPosCovar);
	Matrix myNewP;
	Vector myNewPos;

	float mDist = mahalanobisDistance(&xMe, &myP, &xMeFromYou, yourP);
	mDist = sqrtf(mDist); //The way this value scales, taking the square root seems reasonable.
	covarIntersection(&myNewPos, &myNewP, &xMe, &myP, &xMeFromYou, yourP);
	if(mDist>4.0){
		//This mDist corresponds to a likelihood (of consistency..?) of ~0.1%
		//Based on cumulative chi-squared distribution.
		MY_POS_DEBUG_PRINT(" but the mahalanobis distance (%5.2f) is too large.\r\n", mDist);
		return;
	}else if(mDist>1.0){ 
		//This mDist corresponds to a likelihood (of consistency..?) of ~80%
		//Based on cumulative chi-squared distribution.
		covarUnion(&myNewPos, &myNewP, &xMe, &myP, &xMeFromYou, yourP);
		if(!positiveDefiniteQ(&myNewP)){
			MY_POS_DEBUG_PRINT(" but covar union resulted in non-positive-definite P.\r\n");
			return;
		}
	}
	myPos.x = myNewPos[0]>8191 ? 8191 : (myNewPos[0]<-8192 ? -8192 : myNewPos[0]);
	myPos.y = myNewPos[1]>8191 ? 8191 : (myNewPos[1]<-8192 ? -8192 : myNewPos[1]);
	myPos.o = (rad_to_deg(myNewPos[2]) + 0.5);
	MY_POS_DEBUG_PRINT(" giving pos {%d, %d, %d}.\r\n", myPos.x, myPos.y, myPos.o);
	MY_POS_DEBUG_PRINT("\tMahalanobis Distance: %f\r\n", mDist);
	//#ifdef MY_POS_DEBUG_MODE
		//MY_POS_DEBUG_PRINT("Your Update Covar:\r\n");
		//printMatrixMathematica(yourP);
		//MY_POS_DEBUG_PRINT("My new covar=\r\n");
		//printMatrixMathematica(&myNewP);
	//#endif
	//resetEigValMax(&myNewP);
	compressP(&myNewP, &myPosCovar);		
}

void updatePositions(){
	if(!POS_DEFINED(&myPos)){
		printf("Can't adjust others' positions until I know where I am.\r\n");
		return;
	}
	POS_CALC_DEBUG_PRINT("Updating Positions!\r\n");
	Matrix myP;
	decompressP(&myP, &myPosCovar);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBotUseabilityCheck(i)){
			processMeasurement(i, &myP);
		}
	}
}

//void processMeasurement(uint8_t botIdx, uint8_t seedIdx, Matrix* myP){
void processMeasurement(uint8_t botIdx, Matrix* myP){
	//Vector x_me = {perSeedPos[seedIdx].x, perSeedPos[seedIdx].y, deg_to_rad(perSeedPos[seedIdx].o)};
	Vector x_me = {myPos.x, myPos.y, deg_to_rad(myPos.o)};
	Vector meas = {nearBots[botIdx].myMeas.r, deg_to_rad(nearBots[botIdx].myMeas.b+90), deg_to_rad(nearBots[botIdx].myMeas.h+90)};
	Matrix G;
	populateGammaMatrix(&G, &x_me);
	Matrix Gtp;
	matrixTranspose(&Gtp, &G);
	Vector z;
	calcRelativePose(&z, &meas);
	Matrix R;
	getMeasCovar(&R, &meas);
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
	matrixMultiply(&tmp, &H, myP);
	matrixInplaceTranspose(&H);
	matrixInplaceMultiply(&tmp, &tmp, &H); //now tmp is H.myP.tp(H)
	matrixAdd(&yourP, &tmp, &yourP);
			
	if(positiveDefiniteQ(&yourP)){
		POS_CALC_DEBUG_PRINT("\t%04X @ {%6.1f, %6.1f, % 5.0f} from {% 4d, % 4d, % 4d}\r\n", nearBots[botIdx].myMeas.id, x_you[0], x_you[1], (rad_to_deg(x_you[2]-M_PI_2)+0.5), nearBots[botIdx].myMeas.r, nearBots[botIdx].myMeas.b, nearBots[botIdx].myMeas.h);
		//#ifdef POS_CALC_DEBUG_MODE
			//POS_CALC_DEBUG_PRINT("Covar:\r\n");
			//printMatrixMathematica(&yourP);
		//#endif
		nearBots[botIdx].posFromMe.x = x_you[0]>8191 ? 8191 : (x_you[0]<-8192 ? -8192 : x_you[0]);
		nearBots[botIdx].posFromMe.y = x_you[1]>8191 ? 8191 : (x_you[1]<-8192 ? -8192 : x_you[1]);
		nearBots[botIdx].posFromMe.o = (rad_to_deg(x_you[2]-M_PI_2)+0.5);
		//nearBots[botIdx].seedIdx = seedIdx;
		compressP(&yourP, &(nearBots[botIdx].posCovar));
	}
}

void getMeasCovar(Matrix* R, Vector* meas){
	//Matrix* start = ((*meas)[0])<=130 ? &deltaPosCovarClose : ( ((*meas)[0])<=185 ? &deltaPosCovarMed : &deltaPosCovarFar );
	//matrixCopy(R, start);
	Matrix* start;
	if((*meas)[0]<=80){
		start = &measCovarClose;
	}else if((*meas)[0]<=140){
		start = &measCovarMed;
	}else{
		start = &measCovarFar;
	}
	matrixCopy(R, start);
	Matrix H; //This is the jacobian of the transformation from (r,b,h) to (deltaX, deltaY, deltaO)
	float cosO = cos((*meas)[1]);
	float sinO = sin((*meas)[1]);
	H[0][0] = cosO;
	H[0][1] = -((*meas)[0])*sinO;
	H[0][2] = 0;
	H[1][0] = sinO;
	H[1][1] = ((*meas)[0])*cosO;
	H[1][2] = 0;
	H[2][0] = 0;
	H[2][1] = 0;
	H[2][2] = 1;
	matrixInplaceMultiply(R, &H, R);
	matrixInplaceTranspose(&H);
	matrixInplaceMultiply(R, R, &H);
}

void calcRelativePose(Vector* pose, Vector* meas){
	(*pose)[0] = (*meas)[0] * cos((*meas)[1]);
	(*pose)[1] = (*meas)[0] * sin((*meas)[1]);
	(*pose)[2] = (*meas)[2];
}

void populateGammaMatrix(Matrix* G, Vector* pos){
	float cosO = cos((*pos)[2]);
	float sinO = sin((*pos)[2]);
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
	(*H)[0][0] = 1;
	(*H)[0][1] = 0;
	(*H)[0][2] = (*x_me)[1] - (*x_you)[1];
	(*H)[1][0] = 0;
	(*H)[1][1] = 1;
	(*H)[1][2] = (*x_you)[0] - (*x_me)[0];
	(*H)[2][0] = 0;
	(*H)[2][1] = 0;
	(*H)[2][2] = 1;
}

void compressP(Matrix* P, DensePosCovar* covar){
	(*P)[0][0] = (1/8.0)*(*P)[0][0];
	(*P)[0][1] = (1/16.0)*(*P)[0][1];
	(*P)[0][2] = 16.0*(*P)[0][2];
	(*P)[1][1] = (1/8.0)*(*P)[1][1];
	(*P)[1][2] = 16.0*(*P)[1][2];
	(*P)[2][2] = 256.0*(*P)[2][2];

	for(uint8_t i=0;i<3;i++){
		(*P)[i][i] = ((*P)[i][i] > 65535) ? 65535 : (*P)[i][i];
		for(uint8_t j=i+1;j<3;j++){
			(*P)[i][j] = ((*P)[i][j] > 32767) ? 32767 : ( ((*P)[i][j]<-32768) ? -32768 : (*P)[i][j] );
		}
	}
	(*covar)[0].u = (uint16_t)((*P)[0][0]);
	(*covar)[1].d =  (int16_t)((*P)[0][1]);
	(*covar)[2].d =  (int16_t)((*P)[0][2]);
	(*covar)[3].u = (uint16_t)((*P)[1][1]);
	(*covar)[4].d =  (int16_t)((*P)[1][2]);
	(*covar)[5].u = (uint16_t)((*P)[2][2]);
}

void decompressP(Matrix* P, DensePosCovar* covar){
	(*P)[0][0] = (*covar)[0].u;
	(*P)[0][1] = (*covar)[1].d;
	(*P)[0][2] = (*covar)[2].d;
	(*P)[1][1] = (*covar)[3].u;
	(*P)[1][2] = (*covar)[4].d;
	(*P)[2][2] = (*covar)[5].u;

	(*P)[0][0] = 8*(*P)[0][0];
	(*P)[0][1] = 16*(*P)[0][1];
	(*P)[0][2] = (1/16.0)*(*P)[0][2];
	(*P)[1][0] = (*P)[0][1];
	(*P)[1][1] = 8*(*P)[1][1];
	(*P)[1][2] = (1/16.0)*(*P)[1][2];
	(*P)[2][0] = (*P)[0][2];
	(*P)[2][1] = (*P)[1][2];
	(*P)[2][2] = (1/256.0)*(*P)[2][2];
}

//Note: This function assumes that nearBots have been sorted by range!
void updateNearBotOcclusions(){
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), smallRangeSorter);
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
	//if(POS_DEFINED(&myPos) /*&& !seedFlag*/){
		//fusePerSeedMeas();
	//}	
	//printf("Frame End Calculations\r\n");
	//updateNearBotOcclusions();
	updatePositions();
	
	////Removing the worst half of the nearBots.
	//for(uint8_t i=NUM_TRACKED_BOTS/2; i<NUM_TRACKED_BOTS;i++){
		//cleanOtherBot(&(nearBots[i]));
	//}
	frameEndPrintout();
	printf("\r\n");
}

void useNewRnbMeas(uint16_t id, uint16_t range, int16_t bearing, int16_t heading){
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
	//if(frameCount%8==7){ //sync test color
		//hsv_to_rgb((uint16_t)(60*((loopID%36)>>2)), 220, 127, &newR, &newG, &newB);
	//}else{ //pos color
		if(POS_DEFINED(&myPos)){
			int16_t xColVal = (int16_t)(6.0*pow(41.0,(myPos.x-MIN_X)/((float)xRange))+9.0);
			int16_t yColVal = (int16_t)(3.0*pow(84.0,(myPos.y-MIN_Y)/((float)yRange))+3.0);
			newR = (uint8_t)(xColVal);
			newG = (uint8_t)(yColVal);
		}else{
			newR = newG = newB = 50;
		}
	//}

	set_rgb(newR, newG, newB);	
}

void sendBotMeasMsg(uint8_t i){ //i: index in nearBots array.
	BotMeasMsg msg;
	msg.flag = BOT_MEAS_MSG_FLAG;
	copyBotPos(&(nearBots[i].posFromMe), &(msg.pos));
	for(uint8_t j=0;j<6;j++){
		msg.covar[j].u = nearBots[i].posCovar[j].u;
	}
	uint8_t dir;
	uint8_t dirMask = 0;
	for(int8_t j=-45;j<90;j+=45){
		dir = dirFromAngle(pretty_angle_deg(nearBots[i].myMeas.b + j));
		dirMask |= 1<<dir;
	}
	//msg.seedIdx = nearBots[i].seedIdx;
	ir_targeted_send(dirMask, (char*)(&msg), sizeof(BotMeasMsg), nearBots[i].myMeas.id);
	POS_MSG_DEBUG_PRINT("%04X sent pos msg in dirs %02hX.\r\n", nearBots[i].myMeas.id, dirMask);
	removeOtherBot(i);
}

void handleBotMeasMsg(BotMeasMsg* msg, id_t senderID __attribute__ ((unused))){
	if(seedFlag) return;
	if(!POS_DEFINED(&(msg->pos))){
		MY_POS_DEBUG_PRINT("%04X sent me an undefined position.\r\n", senderID);
		return;
	}
	Matrix covar;
	decompressP(&covar, &(msg->covar));
	//Vector eigVals;
	//eigenvalues(&eigVals, &covar);
	//if(!eigValToleranceQ(&eigVals)){
		//MY_POS_DEBUG_PRINT("Eigenvalues exceeded tolerance. (%f, %f, %f)\r\n", eigVals[0], eigVals[1], eigVals[2]);
		//return;
	//}
	if(!POS_DEFINED(&myPos)){
		MY_POS_DEBUG_PRINT("%04X initialized me to {%d, %d, %d}.\r\n", senderID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
		myPos.x = (msg->pos).x;
		myPos.y = (msg->pos).y;
		myPos.o = (msg->pos).o;
		for(uint8_t i=0;i<6;i++){
			myPosCovar[i].u = msg->covar[i].u;
		}
		//#ifdef MY_POS_DEBUG_MODE
			//MY_POS_DEBUG_PRINT("His Est Covar:\r\n");
			//printMatrixMathematica(&covar);
		//#endif
	}else{
		MY_POS_DEBUG_PRINT("%04X ", senderID);
		updatePos(&(msg->pos), &covar);
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
		printPosCovar(&myPosCovar);
		//if(POS_DEFINED(&(perSeedPos[0]))){
			//printf("\tNW Pos: {%4d, % 4d, % 4d}", perSeedPos[0].x, perSeedPos[0].y, perSeedPos[0].o);
			//}else{
			//printf("\tNW Pos: { -- ,  -- ,  -- }");
		//}
		//if(POS_DEFINED(&(perSeedPos[1]))){
			//printf("           NE Pos: {%4d, % 4d, % 4d}\r\n",	perSeedPos[1].x, perSeedPos[1].y, perSeedPos[1].o);
			//}else{
			//printf("           NE Pos: { -- ,  -- ,  -- }\r\n");
		//}
		//printTwoPosCovar(&(perSeedCovars[0]), &(perSeedCovars[1]));
		//if(POS_DEFINED(&(perSeedPos[2]))){
			//printf("\tSW Pos: {%4d, % 4d, % 4d}", perSeedPos[2].x, perSeedPos[2].y, perSeedPos[2].o);
			//}else{
			//printf("\tSW Pos: { -- ,  -- ,  -- }");
		//}
		//if(POS_DEFINED(&(perSeedPos[3]))){
			//printf("           SE Pos: {%4d, % 4d, % 4d}\r\n",	perSeedPos[3].x, perSeedPos[3].y, perSeedPos[3].o);
			//}else{
			//printf("           SE Pos: { -- ,  -- ,  -- }\r\n");
		//}
		//printTwoPosCovar(&(perSeedCovars[2]), &(perSeedCovars[3]));
	}else{
		printf("\r\n");
	}
}

//void printTwoPosCovar(DensePosCovar* denseP1, DensePosCovar* denseP2){
	//Matrix P1;
	//decompressP(&P1, denseP1);
	//Matrix P2;
	//decompressP(&P2, denseP2);
	//printf("\t{                                    {\r\n");
		//printf("\t  {%7.1f, %7.1f, %7.3f},          {%7.1f, %7.1f, %7.3f},\r\n", P1[0][0], P1[0][1], P1[0][2], P2[0][0], P2[0][1], P2[0][2]);
		//printf("\t  {%7.1f, %7.1f, %7.3f},          {%7.1f, %7.1f, %7.3f},\r\n", P1[1][0], P1[1][1], P1[1][2], P2[1][0], P2[1][1], P2[1][2]);
		//printf("\t  {%7.3f, %7.3f, %7.5f}           {%7.3f, %7.3f, %7.5f},\r\n", P1[2][0], P1[2][1], P1[2][2], P2[2][0], P2[2][1], P2[2][2]);
	//printf("\t}                                    }\r\n");
//}

void printPosCovar(DensePosCovar* denseP){
	Matrix P;
	decompressP(&P, denseP);
	printf("\t{\r\n\t  {%7.1f, %7.1f, %7.3f},\r\n", P[0][0], P[0][1], P[0][2]);
	printf("\t  {%7.1f, %7.1f, %7.3f},\r\n",	    P[1][0], P[1][1], P[1][2]);
	printf("\t  {%7.3f, %7.3f, %7.5f}\r\n\t}\r\n",  P[2][0], P[2][1], P[2][2]);
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
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), smallRangeSorter);
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
	//other->seedIdx = 255;
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