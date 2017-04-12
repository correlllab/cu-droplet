#include "droplet_programs/positions.h"

void init(){
	if((RNB_DUR+PADDING_DUR + MEAS_MSG_DUR*2)>=SLOT_LENGTH_MS){
		printf_P(PSTR("You've got problems! SLOT_LENGTH_MS needs to be longer than all the things that take place during a slot!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	initPositions();
	mySlot = getSlot(get_droplet_id());
	printf("mySlot: %u, frame_length: %lu\r\n", mySlot, FRAME_LENGTH_MS);
	frameEndPrintout();
	xRange = MAX_X-MIN_X;
	yRange = MAX_Y-MIN_Y;
	maxRange = (int16_t)hypotf(xRange, yRange);
	set_all_ir_powers(200);
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
			broadcast_rnb_data();
		}else if(loopID==SLOTS_PER_FRAME-1){
			frameEndPrintout();
			printf("\r\n");
		}
		updateColor();
	}
	if(rnb_updated){
		useNewRnbMeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	delay_ms(LOOP_DELAY_MS);
}


/************************************************************************/
/* omega: selected to minimize (omega*myPinv + (1-omega)*yourPinv)^{-1} */
/* but that's hard. So, insted we're doing a cheat, as described in		*/
/* Franken & Hupper's 2005 Information Fusion paper, "Improved Fast		*/
/* Covariance Intersection for Distributed Data Fusion"					*/
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

	Matrix S_inv;
	matrixInverse(&S_inv, &S);

	Matrix TMP;
	matrixMultiply(&TMP, &U_a, &S_inv);
	matrixInplaceTranspose(&S_inv);
	matrixInplaceMultiply(&TMP, &S_inv, &TMP);

	Vector eigVals;
	Matrix V_tr; //eigenVectors
	eigensystem(&eigVals, &V_tr, &TMP);
	Matrix D = {{eigVals[0], 0, 0}, {0, eigVals[1], 0}, {0, 0, eigVals[2]}};

	Matrix V;
	matrixTranspose(&V, &V_tr);

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
	#if defined(MY_POS_DEBUG_MODE) && defined(COVAR_DEBUG_MODE)
		MY_POS_DEBUG_PRINT("Your Update Covar:\r\n");
		printMatrixMathematica(yourP);
		MY_POS_DEBUG_PRINT("My new covar=\r\n");
		printMatrixMathematica(&myNewP);
	#endif
	compressP(&myNewP, &myPosCovar);		
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
	H[0][0] = cosO;    H[0][1] = -((*meas)[0])*sinO;    H[0][2] = 0;
	H[1][0] = sinO;    H[1][1] =  ((*meas)[0])*cosO;    H[1][2] = 0;
	H[2][0] =    0;    H[2][1] =                  0;    H[2][2] = 1;
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
	(*G)[0][0] = cosO;    (*G)[0][1] = -sinO;    (*G)[0][2] = 0;
	(*G)[1][0] = sinO;    (*G)[1][1] =  cosO;    (*G)[1][2] = 0;
	(*G)[2][0] = 0;       (*G)[2][1] = 0;        (*G)[2][2] = 1;
}

void populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you){
	(*H)[0][0] = 1;    (*H)[0][1] = 0;    (*H)[0][2] = (*x_me)[1] - (*x_you)[1];
	(*H)[1][0] = 0;    (*H)[1][1] = 1;    (*H)[1][2] = (*x_you)[0] - (*x_me)[0];
	(*H)[2][0] = 0;    (*H)[2][1] = 0;    (*H)[2][2] = 1;
}

void compressP(Matrix* P, DensePosCovar* covar){
	(*P)[0][0] = (1/8.0)*(*P)[0][0];    (*P)[0][1] = (1/16.0)*(*P)[0][1];    (*P)[0][2] =  16.0*(*P)[0][2];
	                                    (*P)[1][1] = (1/8.0)*(*P)[1][1];     (*P)[1][2] =  16.0*(*P)[1][2];
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
	(*P)[0][0] = (*covar)[0].u;    (*P)[0][1] = (*covar)[1].d;    (*P)[0][2] = (*covar)[2].d;
                                   (*P)[1][1] = (*covar)[3].u;    (*P)[1][2] = (*covar)[4].d;
								                                  (*P)[2][2] = (*covar)[5].u;

	(*P)[0][0] = 8*(*P)[0][0];    (*P)[0][1] = 16*(*P)[0][1];    (*P)[0][2] = (1/16.0 )*(*P)[0][2];
	(*P)[1][0] =   (*P)[0][1];    (*P)[1][1] =  8*(*P)[1][1];    (*P)[1][2] = (1/16.0 )*(*P)[1][2];
	(*P)[2][0] =   (*P)[0][2];    (*P)[2][1] =    (*P)[1][2];    (*P)[2][2] = (1/256.0)*(*P)[2][2];
}

void useNewRnbMeas(uint16_t id, uint16_t range, int16_t bearing, int16_t heading){
	if(range>200){
		return;
	}
	RNB_DEBUG_PRINT("            (RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", id, range, bearing, heading);
	if(!POS_DEFINED(&myPos)){
		POS_CALC_DEBUG_PRINT("Can't adjust others' positions until I know where I am.\r\n");
		return;
	}
	BotMeas meas;
	meas.id = id;
	meas.r = range;
	meas.b = bearing;
	meas.h = heading;
	processMeasurement(&meas);
}

void processMeasurement(BotMeas* rawMeas){
	Vector x_me = {myPos.x, myPos.y, deg_to_rad(myPos.o)};
	Vector meas = {rawMeas->r, deg_to_rad(rawMeas->b+90), deg_to_rad(rawMeas->h+90)};
	Matrix myP;
	decompressP(&myP, &myPosCovar);
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
	matrixMultiply(&tmp, &H, &myP);
	matrixInplaceTranspose(&H);
	matrixInplaceMultiply(&tmp, &tmp, &H); //now tmp is H.myP.tp(H)
	matrixAdd(&yourP, &tmp, &yourP);
	
	if(positiveDefiniteQ(&yourP)){
		POS_CALC_DEBUG_PRINT("\t%04X @ {%6.1f, %6.1f, % 5.0f} from {% 4d, % 4d, % 4d}\r\n", rawMeas->id, x_you[0], x_you[1], (rad_to_deg(x_you[2]-M_PI_2)+0.5), rawMeas->r, (int16_t)rawMeas->b, (int16_t)rawMeas->h);
		#if defined(POS_CALC_DEBUG_MODE) && defined(COVAR_DEBUG_MODE)
			POS_CALC_DEBUG_PRINT("Calc'd Covar:\r\n");
			printMatrixMathematica(&yourP);
		#endif
		BotPos pos;
		pos.x = x_you[0]>8191 ? 8191 : (x_you[0]<-8192 ? -8192 : x_you[0]);
		pos.y = x_you[1]>8191 ? 8191 : (x_you[1]<-8192 ? -8192 : x_you[1]);
		pos.o = (rad_to_deg(x_you[2]-M_PI_2)+0.5);
		DensePosCovar covar;
		compressP(&yourP, &covar);
		prepBotMeasMsg(&pos, rawMeas, &covar);
	}
}

void updateColor(){
	uint8_t newR = 0, newG = 0, newB = 0;
	if(POS_DEFINED(&myPos)){
		int16_t xColVal = (int16_t)(6.0*pow(41.0,(myPos.x-MIN_X)/((float)xRange))+9.0);
		int16_t yColVal = (int16_t)(3.0*pow(84.0,(myPos.y-MIN_Y)/((float)yRange))+3.0);
		newR = (uint8_t)(xColVal);
		newG = (uint8_t)(yColVal);
	}else{
		newR = newG = newB = 50;
	}
	set_rgb(newR, newG, newB);	
}

void prepBotMeasMsg(BotPos* pos, BotMeas* meas, DensePosCovar* covar){ //i: index in nearBots array.
	BotMeasMsgNode* mNode = (BotMeasMsgNode*)myMalloc(sizeof(BotMeasMsgNode));
	mNode->numTries = 0;
	mNode->tgt = meas->id;
	mNode->range = meas->r;
	mNode->dirMask = 0;
	for(int8_t j=-45;j<90;j+=45){
		mNode->dirMask |= 1<<dirFromAngle(pretty_angle_deg(meas->b + j));
	}
	mNode->msg.flag = BOT_MEAS_MSG_FLAG;
	copyBotPos(pos, &(mNode->msg.pos));
	for(uint8_t j=0;j<6;j++){
		(mNode->msg).covar[j].u =  (*covar)[j].u;
	}
	sendBotMeasMsg(mNode);
}

void sendBotMeasMsg(BotMeasMsgNode* mNode){
	if(!ir_is_busy(ALL_DIRS)){
		ir_targeted_send(mNode->dirMask, (char*)(&(mNode->msg)), sizeof(BotMeasMsg), mNode->tgt);
		POS_MSG_DEBUG_PRINT("%04X sent pos msg in dirs %02hX after %hu tries.\r\n", mNode->tgt, mNode->dirMask, mNode->numTries);
		myFree(mNode);
	}else{
		if(mNode->numTries>5){
			POS_MSG_DEBUG_PRINT("Giving up on msg to %04X after %hu tries.\r\n", mNode->tgt, mNode->numTries);
			myFree(mNode);
		}else{
			schedule_task(getBackoffTime(mNode->numTries, mNode->range), sendBotMeasMsg, mNode);
		}
		mNode->numTries++;
	}

}

void handleBotMeasMsg(BotMeasMsg* msg, id_t senderID __attribute__ ((unused))){
	if(seedFlag) return;
	if(!POS_DEFINED(&(msg->pos))){
		MY_POS_DEBUG_PRINT("%04X sent me an undefined position.\r\n", senderID);
		return;
	}
	Matrix covar;
	decompressP(&covar, &(msg->covar));
	if(!POS_DEFINED(&myPos)){
		MY_POS_DEBUG_PRINT("%04X initialized me to {%d, %d, %d}.\r\n", senderID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
		myPos.x = (msg->pos).x;
		myPos.y = (msg->pos).y;
		myPos.o = (msg->pos).o;
		for(uint8_t i=0;i<6;i++){
			myPosCovar[i].u = msg->covar[i].u;
		}
		#if defined(MY_POS_DEBUG_MODE) && defined(COVAR_DEBUG_MODE)
			MY_POS_DEBUG_PRINT("His Est Covar:\r\n");
			printMatrixMathematica(&covar);
		#endif
	}else{
		MY_POS_DEBUG_PRINT("%04X ", senderID);
		updatePos(&(msg->pos), &covar);
	}
}



void handle_msg(ir_msg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
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
	}else{
		printf("\r\n");
	}
}

void printPosCovar(DensePosCovar* denseP){
	Matrix P;
	decompressP(&P, denseP);
	printf("\t{\r\n\t  {%7.1f, %7.1f, %7.3f},\r\n", P[0][0], P[0][1], P[0][2]);
	printf("\t  {%7.1f, %7.1f, %7.3f},\r\n",	    P[1][0], P[1][1], P[1][2]);
	printf("\t  {%7.3f, %7.3f, %7.5f}\r\n\t}\r\n",  P[2][0], P[2][1], P[2][2]);
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){	
	//return 0;
//}
