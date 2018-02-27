#include "localization.h"

//#define NUM_SEEDS 4
//
//const BotPos SEED_POS[NUM_SEEDS] = {{50,50,0}, {200,50,0}, {50,200,0}, {200,200,0}};
//const id_t   SEED_IDS[NUM_SEEDS] = {0x7EDF, 0x1361, 0x6C66, 0x9669};
#define NUM_SEEDS 4

//const BotPos SEED_POS[NUM_SEEDS] = {{225,220,0}, {500,160,0},{350,40,0},{75,100,0}};
//const id_t   SEED_IDS[NUM_SEEDS] = {0xD913, 0xAF6A, 0x3D6C,0x9261};
	const BotPos SEED_POS[NUM_SEEDS] = {{225,220,0}, {500,160,0},{350,40,0},{75,100,0}};
	const id_t   SEED_IDS[NUM_SEEDS] = {0xD913, 0x3D6C, 0xAF6A, 0x9261};

//const BotPos SEEDS[NUM_SEEDS] = {{100, 600, 0}, {600, 600, 0}, {100, 100, 0}, {600, 100, 0}};

//The MIN and MAX values below are only needed for getPosColor.
#define MIN_X 0
#define MIN_Y 0
#define MAX_X 150
#define MAX_Y 150

static float	chooseOmega(Matrix* myPinv, Matrix* yourPinv);
static float	updateDistance(Vector* a, Matrix* A, Vector* b, Matrix* B);
static void		covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
static void		covarUnion(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B);
static void		updatePos(BotPos* pos, Matrix* yourP);
static void		getMeasCovar(Matrix* R, Vector* meas);
static void		calcRelativePose(Vector* pose, Vector* meas);
static void		populateGammaMatrix(Matrix* G, Vector* pos);
static void		populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you);

static void		prepBotMeasMsg(id_t id, uint16_t r, int16_t b, BotPos* pos, DensePosCovar* covar);
static uint32_t getBackoffTime(uint8_t N, uint16_t r);
static float	discreteTriangularPDF(float x, uint8_t max, uint16_t r);

static const Matrix measCovarClose  = {{ 100, -0.75,   0.1}, {-0.75, 0.03, 0.025}, { 0.1, 0.025,  0.1}}; //46-80mm
static const Matrix    measCovarMed = {{ 450,  0.15,  0.75}, { 0.15, 0.05,   0.1}, {0.75,   0.1, 0.25}}; //80-140mm
static const Matrix    measCovarFar = {{2000,    -5,  -0.1}, {   -5,  0.6,   0.5}, {-0.1,   0.5,  1.0}}; //140-200mm
//static const Matrix measCovarClose  = {{100, -0.75, 0.1}, {-0.75, 0.03, 0.025}, {0.1, 0.025, 0.1}}; //50-80mm
//static const Matrix measCovarMed = {{450, 0.15, 0.75}, {0.15, 0.05, 0.1}, {0.75, 0.1, 0.25}}; //80-140mm
//static const Matrix measCovarFar = {{2000, -5, -0.1}, {-5, 0.6, 0.5}, {-0.1, 0.5, 1.0}}; //140-210mm	

void localizationInit(){
	myPos.x = UNDF;
	myPos.y = UNDF;
	myPos.o = UNDF;
	for(uint8_t i=0;i<6;i++){
		myPosCovar[i].u = 0;
	}
	seedFlag = 0;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(getDropletID()==SEED_IDS[i]){
			seedFlag = 1;
			myPos.x = SEED_POS[i].x;
			myPos.y = SEED_POS[i].y;
			myPos.o = SEED_POS[i].o;
			myPosCovar[0].u = 1; //the actual value used will be this*8
			myPosCovar[3].u = 1; //the actual value used will be this*8
			myPosCovar[5].u = 16; //the actual value used will be this/256
			break;
		}
	}

}

/************************************************************************/
/* omega: selected to minimize (omega*myPinv + (1-omega)*yourPinv)^{-1} */
/* but that's hard. So, insted we're doing a cheat, as described in		*/
/* Franken & Hupper's 2005 Information Fusion paper, "Improved Fast		*/
/* Covariance Intersection for Distributed Data Fusion"					*/
/************************************************************************/
static float chooseOmega(Matrix* myPinv, Matrix* yourPinv){
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

/*
 * NOT Mahalanobis Distance
 * But.. closer to that than something else?
 * The Bhattacharyya distance might be more 'accurate' in some sense, but
 * has the property that, given one  distribution with very large covariance
 * and one distribution with much smaller covariance, somewhere within the
 * region of the first distribution, the Bhattacharyya distance is quite large.
 * This makes sense, because the distributions are, indeed, quite different.
 * In that scenario, however, the not-Mahalabonis distance metric used below
 * gives a small distance. THIS makes sense, too, becaus the two distributions
 * are consistent, ie. not mutually incompatible.
 * I had problems with covariance matrices blowing up because in the above
 * scenario, the large distance means my method uses covariance union to fuse
 * the two measurements, which grows the overall covariance. A small distance,
 * on the other hand, results in covariance intersection, shrinking the overall
 * covariance. 
 */
static float updateDistance(Vector* a, Matrix* A, Vector* b, Matrix* B){
	Vector a_sub_b;
	vectorSubtract(&a_sub_b, a, b);
	a_sub_b[2] = prettyAngle(a_sub_b[2]);
	Matrix A_plus_B_inv;
	matrixAdd(&A_plus_B_inv, A, B);
	matrixInplaceInverse(&A_plus_B_inv);
	Vector a_sub_b_normed;
	matrixTimesVector(&a_sub_b_normed, &A_plus_B_inv, &a_sub_b);
	//Now, a_sub_b_normed = (A+B)^{-1} X (a-b)
	float distance = a_sub_b[0]*a_sub_b_normed[0] + a_sub_b[1]*a_sub_b_normed[1] + a_sub_b[2]*a_sub_b_normed[2];
	//Now, distance = (a-b)^{tr} X (A+B)^{-1} X (a-b)
	return sqrtf(distance);
}

//Bhattacharyya Distance
//static float updateDistance(Vector* a, Matrix* A, Vector* b, Matrix* B){
	//Vector a_sub_b;
	//vectorSubtract(&a_sub_b, a, b);
	//a_sub_b[2] = prettyAngle(a_sub_b[2]);
	//Matrix Sigma;
	//matrixAdd(&Sigma, A, B);
	//matrixScale(&Sigma, 0.5);
	//float detSigma = matrixDet(&Sigma);
	//float detA     = matrixDet(A);
	//float detB	   = matrixDet(B);
	//matrixInplaceInverse(&Sigma);
	//Vector a_sub_b_normed;
	//matrixTimesVector(&a_sub_b_normed, &Sigma, &a_sub_b);
	////Now, a_sub_b_normed = (A+B)^{-1} X (a-b)
	//float distance = a_sub_b[0]*a_sub_b_normed[0] + a_sub_b[1]*a_sub_b_normed[1] + a_sub_b[2]*a_sub_b_normed[2];
	//distance = distance/8.0;
	////Now, distance = (1/8) * ( (a-b)^{tr} X (A+B)^{-1} X (a-b) );
	//float logTerm = detSigma/sqrtf(detA*detB);
	//distance = distance + 0.5*log(logTerm);
	//return distance;
//}

/*
 * P is covar matrix resulting from CI of A & B (with means a & b, respectively)
 * x is resulting mean.
 * CITE:
 * "Decentralized Multi-robot Cooperative Localization using Covariance Intersection"
 * by Luic C. Carillo-Arce et. al.
 */
static void covarIntersection(Vector* x, Matrix* P, Vector* a, Matrix* A, Vector* b, Matrix* B){
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

/*
 * x is the mean we will use.
 * U is the computed covar matrix, guaranteed to be consistent with (a,A) and (b,B)
 * CITE:
 * "Generalized Covariance Union: A Unified Approach to Hypothesis Merging in Tracking"
 * by Steven Reece and Stephen Roberts
 */
static void covarUnion(Vector* x, Matrix* U, Vector* a, Matrix* A, Vector* b, Matrix* B){
	Vector a_dist;
	vectorSubtract(&a_dist, x, a);
	a_dist[2]=prettyAngle(a_dist[2]);
	Matrix U_a;
	vectorSquare(&U_a, &a_dist);
	matrixAdd(&U_a, A, &U_a);

	Vector b_dist;
	vectorSubtract(&b_dist, x, b);
	b_dist[2]=prettyAngle(b_dist[2]);
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

/*
 * TODO:
 * Experiment with Mahalanobis Distance Thresholds for intersection vs. union vs. outright rejection.
 */

/*
 * This function updates my position estimate based on a new position estimate.
 * pos is the new position estimate.
 * yourP is the new position estimate's covariance matrix.
 */
static void updatePos(BotPos* pos, Matrix* yourP){
	Vector xMe = {myPos.x, myPos.y, degToRad(myPos.o)};
	if(!POS_DEFINED(pos)){
		MY_POS_DEBUG_PRINT(" sent me an undefined position.\r\n");
		return;
	}
	MY_POS_DEBUG_PRINT("thinks I'm at {%d, %d, %d}", pos->x, pos->y, pos->o);
	Vector xMeFromYou = {pos->x, pos->y, degToRad(pos->o)};
	Matrix myP;
	decompressP(&myP, &myPosCovar);
	Matrix myNewP;
	Vector myNewPos;


	covarIntersection(&myNewPos, &myNewP, &xMe, &myP, &xMeFromYou, yourP);
	if(!positiveDefiniteQ(&myNewP)){
		MY_POS_DEBUG_PRINT(" but covar intersection resulted in non-positive-definite P.\r\n");
		return;
	}
	float updateDist = updateDistance(&xMe, &myP, &xMeFromYou, yourP);
	if(updateDist>4.0){
		//This mDist corresponds to a likelihood (of consistency..?) of ~0.1%
		//Based on cumulative chi-squared distribution.
		MY_POS_DEBUG_PRINT(" but the update distance (%5.2f) is too large.\r\n", updateDist);
		return;
	}else if(updateDist>1.0){
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
	myPos.o = (radToDeg(myNewPos[2]) + 0.5);
	MY_POS_DEBUG_PRINT(" giving pos {%d, %d, %d}.\r\n", myPos.x, myPos.y, myPos.o);
	MY_POS_DEBUG_PRINT("\tUpdate Distance: %f\r\n", updateDist);
	#if defined(MY_POS_DEBUG_MODE) && defined(COVAR_DEBUG_MODE)
		MY_POS_DEBUG_PRINT("Your Update Covar:\r\n");
		printMatrixMathematica(yourP);
		MY_POS_DEBUG_PRINT("My new covar=\r\n");
		printMatrixMathematica(&myNewP);
	#endif
	compressP(&myNewP, &myPosCovar);
}

//Leaving this here for reference:
//void multinormalSample(Vector* result, Vector* mean, Matrix* covar){
	//Vector eigValues;
	//Matrix eigVectors;
	//eigensystem(&eigValues, &eigVectors, covar);
	//Vector randNormSample = {randNorm(0,1), randNorm(0,1), randNorm(0,1)};
	//Matrix diagSqrtEigValues = {{sqrt(eigValues[0]), 0, 0}, {0, sqrt(eigValues[1]), 0}, {0, 0, sqrt(eigValues[2])}};
	//matrixTimesVector(result, &diagSqrtEigValues, &randNormSample);
	//Vector tmp;
	//matrixTimesVector(&tmp, &eigVectors, result);
	//vectorAdd(result, mean, &tmp);
//}

/*
 * This function uses H, the Jacobian of the transformation matrix from an (r,b,h) measurement to 
 * a relative position estimate (deltaX, deltaY, deltaO).
 * This is used to compute the estimated covariance for a relative position estimate, based on a
 * hard-coded covariance for an (r,b,h) measurement.
 * The hard-coded covariances are based on empirical data. Three such matrices are stored and chosen
 * between based on the measured distance from the other robot.
 *
 * NOTE/TODO: It would be simpler to just calculate and use a hard-coded covariance for the relative
 * position estimate. This approach feels more accurate to me, but I haven't empirically determined
 * that it makes a difference.
 */
static void getMeasCovar(Matrix* R, Vector* meas){
	static const Matrix* start;
	if((*meas)[0]<=80){
		start = &measCovarClose;
	}else if((*meas)[0]<=140){
		start = &measCovarMed;
	}else{
		start = &measCovarFar;
	}
	matrixCopy(R, (Matrix*)start);
	Matrix H;
	float cosO = cos((*meas)[1]);
	float sinO = sin((*meas)[1]);
	H[0][0] = cosO;    H[0][1] = -((*meas)[0])*sinO;    H[0][2] = 0;
	H[1][0] = sinO;    H[1][1] =  ((*meas)[0])*cosO;    H[1][2] = 0;
	H[2][0] =    0;    H[2][1] =                  0;    H[2][2] = 1;
	matrixInplaceMultiply(R, &H, R);
	matrixInplaceTranspose(&H);
	matrixInplaceMultiply(R, R, &H);
}

static void calcRelativePose(Vector* pose, Vector* meas){
	(*pose)[0] = (*meas)[0] * cos((*meas)[1]);
	(*pose)[1] = (*meas)[0] * sin((*meas)[1]);
	(*pose)[2] = (*meas)[2];
}

void relativePosition(uint16_t r, int16_t b, int16_t h, BotPos* pos, Vector* myPos){
	Vector x_you = {pos->x, pos->y, degToRad(pos->o)};
	int16_t otherBotB;
	int16_t otherBotH;
	convertMeas(&otherBotB, &otherBotH, b, h);
	Vector meas = {r, degToRad(otherBotB+90), degToRad(otherBotH+90)};
	Matrix G;
	populateGammaMatrix(&G, &x_you);
	Vector z;
	calcRelativePose(&z, &meas);
	Matrix R;
	getMeasCovar(&R, &meas);
	
	matrixTimesVector(myPos, &G, &z);
	vectorAdd(myPos, &x_you, myPos);		
}

static void populateGammaMatrix(Matrix* G, Vector* pos){
	float cosO = cos((*pos)[2]);
	float sinO = sin((*pos)[2]);
	(*G)[0][0] = cosO;    (*G)[0][1] = -sinO;    (*G)[0][2] = 0;
	(*G)[1][0] = sinO;    (*G)[1][1] =  cosO;    (*G)[1][2] = 0;
	(*G)[2][0] = 0;       (*G)[2][1] = 0;        (*G)[2][2] = 1;
}

static void populateHMatrix(Matrix* H, Vector* x_me, Vector* x_you){
	(*H)[0][0] = 1;    (*H)[0][1] = 0;    (*H)[0][2] = (*x_me)[1] - (*x_you)[1];
	(*H)[1][0] = 0;    (*H)[1][1] = 1;    (*H)[1][2] = (*x_you)[0] - (*x_me)[0];
	(*H)[2][0] = 0;    (*H)[2][1] = 0;    (*H)[2][2] = 1;
}

//Takes covariance matrix P and packs it to a DensePosCovar covar.
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

//Takes DensePosCovar covar and unpacks it to a covariance matrix P.
void decompressP(Matrix* P, DensePosCovar* covar){
	(*P)[0][0] = (*covar)[0].u;    (*P)[0][1] = (*covar)[1].d;    (*P)[0][2] = (*covar)[2].d;
	(*P)[1][1] = (*covar)[3].u;    (*P)[1][2] = (*covar)[4].d;
	(*P)[2][2] = (*covar)[5].u;

	(*P)[0][0] = 8*(*P)[0][0];    (*P)[0][1] = 16*(*P)[0][1];    (*P)[0][2] = (1/16.0 )*(*P)[0][2];
	(*P)[1][0] =   (*P)[0][1];    (*P)[1][1] =  8*(*P)[1][1];    (*P)[1][2] = (1/16.0 )*(*P)[1][2];
	(*P)[2][0] =   (*P)[0][2];    (*P)[2][1] =    (*P)[1][2];    (*P)[2][2] = (1/256.0)*(*P)[2][2];
}

/*
 * dir is movement direction, as used in the 'walk' function.
 * mag is movement magnitude, as used in the 'walk' function.
 *         (units of mm for dirs 0-5; degrees for dirs 6-7)
 * CITE:
 * "Decentralized Multi-robot Cooperative Localization using Covariance Intersection"
 * by Luic C. Carillo-Arce et. al.
 */
void updateForMovement(__attribute__((unused)) uint8_t dir, __attribute__((unused)) uint16_t mag){
	__attribute__((unused)) Vector curX = {myPos.x, myPos.y, degToRad(myPos.o)};
	__attribute__((unused)) Matrix curP;
	decompressP(&curP, &myPosCovar);
	__attribute__((unused)) Vector newX;
	//TODO: Implement function 'f', which calculates newX based on curX, movement dir, and movement mag.
	__attribute__((unused)) Matrix Phi;
	//TODO: Calculate Phi, the gradient of 'f' w.r.t. changes in the robot's current position.
	__attribute__((unused)) Matrix G;
	//TODO: Calculate G, the gradient of 'f' w.r.t. errors in the robot's motion.
	__attribute__((unused)) Matrix Q;
	//TODO: Hard-Code Q, our movement's covariance. Probably separately for each direction???
	__attribute__((unused)) Matrix newP;
	//newP = Phi.curP.(tr(Phi)) + G.Q.(tr(G))
}

/*
 * This function takes a measurement of another droplet and computes an estimated position for that
 * droplet based on this droplet's position and position covariance, and the measurement.
 * It then prepares a message to be sent to the measured droplet, conveying this information.
 */
void useRNBmeas(rnb* meas){
	BotPos pos;
	DensePosCovar covar;
	uint8_t result = calcOtherBotPosFromMeas(&pos, &covar, meas);
	if(result){
		prepBotMeasMsg(meas->id, meas->range, meas->bearing, &pos, &covar);	
	}
}

//returns '1' if successful, '0' otherwise.
uint8_t calcOtherBotPosFromMeas(BotPos* pos, DensePosCovar* covar, rnb* measStruct){
	if(!POS_DEFINED(&myPos)){
		POS_CALC_DEBUG_PRINT("Can't adjust others' positions until I know where I am.\r\n");
		return 0;
	}
	Vector x_me = {myPos.x, myPos.y, degToRad(myPos.o)};
	Vector meas = {measStruct->range, degToRad(measStruct->bearing+90), degToRad(measStruct->heading+90)};
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
		POS_CALC_DEBUG_PRINT("\t%04X @ {%6.1f, %6.1f, % 5.0f} from {% 4d, % 4d, % 4d}\r\n", measStruct->id, x_you[0], x_you[1], (radToDeg(x_you[2]-M_PI_2)+0.5), r, b, h);
		#if defined(POS_CALC_DEBUG_MODE) && defined(COVAR_DEBUG_MODE)
		POS_CALC_DEBUG_PRINT("Calc'd Covar:\r\n");
		printMatrixMathematica(&yourP);
		#endif
		BotPos pos;
		pos.x = x_you[0]>8191 ? 8191 : (x_you[0]<-8192 ? -8192 : x_you[0]);
		pos.y = x_you[1]>8191 ? 8191 : (x_you[1]<-8192 ? -8192 : x_you[1]);
		pos.o = (radToDeg(x_you[2]-M_PI_2)+0.5);
		DensePosCovar covar;
		compressP(&yourP, &covar);
		return 1;
	}else{
		return 0;
	}
}

//Takes all of the information needed for a botMeasMsg and packs it up in to a BotMeasMsgNode,
//then calls sendBotMeasMsg with it.
static void prepBotMeasMsg(id_t id, uint16_t r, int16_t b, BotPos* pos, DensePosCovar* covar){ 
	BotMeasMsgNode* mNode = (BotMeasMsgNode*)myMalloc(sizeof(BotMeasMsgNode));
	mNode->numTries = 0;
	mNode->tgt = id;
	mNode->range = r;
	mNode->dirMask = 0;
	for(int8_t j=-45;j<90;j+=45){
		mNode->dirMask |= 1<<dirFromAngle(prettyAngleDeg(b + j));
	}
	mNode->msg.flag = BOT_MEAS_MSG_FLAG;
	(mNode->msg).pos.x = pos->x;
	(mNode->msg).pos.y = pos->y;
	(mNode->msg).pos.o = pos->o;
	for(uint8_t j=0;j<6;j++){
		(mNode->msg).covar[j].u =  (*covar)[j].u;
	}
	sendBotMeasMsg(mNode);
}

//Sends a BotMeasMsg using a poor man's CSMA protocol. Close-range measurements are biased to wait less 
//long in exponential backoff.
void sendBotMeasMsg(BotMeasMsgNode* mNode){
	if(irIsBusy(ALL_DIRS)){
		if(mNode->numTries>5){
			POS_MSG_DEBUG_PRINT("Giving up on msg to %04X after %hu tries.\r\n", mNode->tgt, mNode->numTries);
			myFree(mNode);
		}else{
			scheduleTask(getBackoffTime(mNode->numTries, mNode->range), (arg_func_t)sendBotMeasMsg, mNode);
		}
		mNode->numTries++;
	}else{
		irTargetedSend(mNode->dirMask, (char*)(&(mNode->msg)), sizeof(BotMeasMsg), mNode->tgt);
		POS_MSG_DEBUG_PRINT("%04X sent pos msg in dirs %02hX after %hu tries.\r\n", mNode->tgt, mNode->dirMask, mNode->numTries);
		myFree(mNode);
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

/*
 * This function computes the exponential-backoff time for CSMA.
 *
 * However, while a standard implementation would choose uniformly between 0 and randMax*16,
 * this implementation weights the random choice so that measurements of a smaller radius
 * are more likely to choose lower slots.
 */
static uint32_t getBackoffTime(uint8_t N, uint16_t r){
	uint8_t randMax = (1<<N) - 1;
	float totalValue = 0;
	float chooser = randReal();
	//printf("Discrete Triangular:\r\n");
	for(uint8_t i=0;i<randMax;i++){
		totalValue += discreteTriangularPDF(i, randMax, r);
		//printf("\t%f\r\n", totalValue);
		if(chooser<=totalValue){
			return ((uint32_t)i)*16;
		}
	}
	return randMax*16;
}

static float discreteTriangularPDF(float x, uint8_t max, uint16_t r){
	float c = ((r-50.0)/150.0)*max;
	float firstTerm;
	float xPlus = x+0.5;
	if(xPlus<=c){
		firstTerm = powf(xPlus,2)/(c*max);
	}else if( (xPlus<=max) && (c<xPlus) ){
		firstTerm = 1 - powf(xPlus-max,2)/(max*(max-c));
	}else if(xPlus > max){
		firstTerm = 1;
	}else{
		firstTerm = 0;
	}
	float secondTerm;
	float xMinus = x-0.5;
	if((0 < xMinus) && (xMinus<=c)){
		secondTerm = powf(xMinus,2)/(c*max);
	}else if( xMinus > c ){
		secondTerm = 1 - powf(max-xMinus,2)/(max*(max-c));
	}else{
		secondTerm = 0;
	}
	return firstTerm - secondTerm;
}

void getPosColor(uint8_t* r, uint8_t* g, uint8_t* b){
	if(POS_DEFINED(&myPos)){
		int16_t xRange = MAX_X-MIN_X;
		int16_t yRange = MAX_Y-MIN_Y;
		float effX = (myPos.x-MIN_X)/(2.0*xRange);
	    effX = 2.0*fabsf(floor(effX+0.5)-effX);
		int16_t xColVal = (int16_t)(26*(pow(10.0,effX)-1)+1);
		float effY = (myPos.y-MIN_Y)/(2.0*yRange);
		effY = 2.0*fabsf(floor(effY+0.5)-effY);
		int16_t yColVal = (int16_t)(20.0*(pow(10.0,effY)-1)+1);
		//printf("{% 4d, %4d}->{%4.2f, %4.2f}->{%3d, %3d}\r\n", myPos.x, myPos.y, effX, effY, xColVal, yColVal);
		*r = (uint8_t)(xColVal);
		*g = (uint8_t)(yColVal);
		*b = 0;
	}else{
		*r = 10;
		*g = 10;
		*b = 10;
	}
}

void printPosCovar(DensePosCovar* denseP){
	Matrix P;
	decompressP(&P, denseP);
	printf("\t{\r\n\t  {%7.1f, %7.1f, %7.3f},\r\n", P[0][0], P[0][1], P[0][2]);
	printf("\t  {%7.1f, %7.1f, %7.3f},\r\n",	    P[1][0], P[1][1], P[1][2]);
	printf("\t  {%7.3f, %7.3f, %7.5f}\r\n\t}\r\n",  P[2][0], P[2][1], P[2][2]);
}