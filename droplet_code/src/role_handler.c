#include "role_handler.h"

RoleMask myRoles = {0};

void checkPosition(){
	Vector mean = {myPos.x, myPos.y, degToRad(myPos.o)};
	Matrix covar;	
	decompressP(&covar, &myPosCovar);
	Matrix multinormTfm;
	getMultinormalSampleTransform(&multinormTfm, &covar);	
	uint8_t regionCounts[NUM_REGIONS] = {0}; //this initializes everything to 0?	
	Vector sampledPos;
	Vector tfmdPoint;
	for(uint8_t i=0;i<NUM_MONTE_CARLO_SAMPLES;i++){
		Vector tmp = {randNorm(0,1), randNorm(0,1), randNorm(0,1)};
		matrixTimesVector(&sampledPos, &multinormTfm, &tmp);
		vectorAdd(&sampledPos, &sampledPos, &mean);
		// Now that point is sampled, we are switching to 2D homogeneous coordinates;
		// dropping the orientation. Now the third value in our vectors is fixed at 1
		// as is done with homogeneous coordinates.
		sampledPos[2]=1;
		for(uint8_t j=0;j<NUM_REGIONS;j++){
			matrixTimesVector(&tfmdPoint, &(regionsList[j].invTfm), &sampledPos);
			if(regionsList[j].shapePrimitiveFunc(&tfmdPoint)){
				regionCounts[j]++;
			}
		}
	}
	clearRoles(&myRoles);
	//printf("Region Counts:\r\n");
	for(uint8_t i=0;i<NUM_REGIONS;i++){
		float ratio = ((float)regionCounts[i])/NUM_MONTE_CARLO_SAMPLES;
		//printf("\t%hu: %hu %4.2f\r\n", i, regionCounts[i], ratio);
		if(ratio>0.50){
			giveRoles(&myRoles,&(regionsList[i].mappedRoles));
		}
	}
}

//void getMultivariatePositionSample(Vector* dest){
	//Matrix covar;
	//decompressP(&covar, &myPosCovar);
	//Matrix L;
	//choleskyDecomposition(&L, &covar);
	//printMatrixMathematica(&L);
	//Vector xMe = {myPos.x, myPos.y, degToRad(myPos.o)};
	//printf("\txMe: {%f, %f, %f}\r\n", xMe[0], xMe[1], xMe[2]);
	//Vector standardNormSample = {randNorm(0,1), randNorm(0,1), randNorm(0,1)};
	//printf("\tstdNorm: {%f, %f, %f}\r\n", standardNormSample[0], standardNormSample[1], standardNormSample[2]);
	//matrixTimesVector(dest, &L, &standardNormSample);
	//printf("\tscaled: {%f, %f, %f}\r\n", (*dest)[0], (*dest)[1], (*dest)[2]);
	//vectorAdd(dest, dest, &xMe);
	//printf("\tfinal: {%f, %f, %f}\r\n", (*dest)[0], (*dest)[1], (*dest)[2]);
//}

uint8_t hasRole(RoleMask* mask, Role r){
	uint8_t roleMaskArrIdx = r>>3;
	uint8_t bitPos = r&0x07;
	return ((*mask)[roleMaskArrIdx] >> bitPos)&0x01;
}

void giveRole(RoleMask* mask, Role r){
	uint8_t roleMaskArrIdx = r>>3;
	uint8_t bitPos = r&0x07;
	(*mask)[roleMaskArrIdx] |=	(1<<bitPos);
}

void removeRole(RoleMask* mask, Role r){
	uint8_t roleMaskArrIdx = r>>3;
	uint8_t bitPos = r&0x07;
	(*mask)[roleMaskArrIdx] &= ~(1<<bitPos);
}

void clearRoles(RoleMask* mask){
	for(uint8_t i=0;i<ROLE_MASK_ARR_SIZE;i++){
		(*mask)[i] = 0;
	}
}

void giveRoles(RoleMask* rxer, RoleMask* giver){
	for(uint8_t i=0;i<ROLE_MASK_ARR_SIZE;i++){
		(*rxer)[i] |= (*giver)[i];
	}
}