#pragma once
#include "droplet_base.h"
#include "localization.h"
#include "random.h"

#define NUM_ROLES 8


#define ROLE_MASK_ARR_SIZE (1+(NUM_ROLES-1)/8)

#define NUM_MONTE_CARLO_SAMPLES 100

typedef uint8_t RoleMask[ROLE_MASK_ARR_SIZE];

typedef struct region_struct{
	Matrix invTfm;
	uint8_t (*shapePrimitiveFunc)(Vector*);
	RoleMask mappedRoles;
}Region;


static __attribute__((unused)) uint8_t isInUnitCircle(Vector* pt){
	float x = (*pt)[0];
	float y = (*pt)[1];
	return ((x*x + y*y) <= 1);
}

static __attribute__((unused)) uint8_t isInUnitSquare(Vector* pt){
	float x = (*pt)[0];
	float y = (*pt)[1];
	uint8_t xGood =  (0<=x) && (x<=1);
	uint8_t yGood =  (0<=y) && (y<=1);
	return (xGood && yGood);
}

typedef enum role_enum{
	ROLE_BODY,
	ROLE_EMTR,
	ROLE_POWER,
	ROLE_CHN_UP,
	ROLE_CHN_DN,
	ROLE_VOL_UP,
	ROLE_VOL_DN,
	ROLE_MUTE
}Role;

#define NUM_REGIONS 7
extern Region regionsList[NUM_REGIONS];

extern RoleMask myRoles;


//void getMultivariatePositionSample(Vector* dest);
void checkPosition(void);

uint8_t hasRole(RoleMask* mask, Role r);
void giveRole(RoleMask* mask, Role r);
void giveRoles(RoleMask* rxer, RoleMask* giver);
void removeRole(RoleMask* mask, Role r);
void clearRoles(RoleMask* mask);


static __attribute__((unused)) void printRoles(RoleMask* mask){
	uint8_t numRoles = 0;
	printf("\t%p Roles: ", mask);
	for(uint8_t i=0;i<NUM_ROLES;i++){
		if(hasRole(mask, i)){
			printf("%2hu ", i);
			numRoles++;
		}
	}
	if(numRoles>0){
		printf("\r\n");
	}else{
		printf("None\r\n");
	}
}

