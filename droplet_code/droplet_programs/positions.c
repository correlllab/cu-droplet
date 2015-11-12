#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

//#define POS_DEBUG_MODE

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

#ifdef PARTICLE
float randFloat(){
	uint16_t uniformInt = ((uint16_t)rand_byte())<<8|((uint16_t)rand_byte());
	return ((float)uniformInt)/65535.0;;
}

float cauchyPDF(float theta, float mean, float var){
	var = 1.0+var;
	if(theta<-M_PI||theta>M_PI) return 0.0;
	return M_1_PI*fabsf(1-var*var)/(2*(1+var*var-(2*var)*cosf(theta-mean)));
}

float gaussianPDF(float x, float mean, float var){
	return expf(((-x+mean)*(x-mean))/(2*var))/sqrtf(2*var*M_PI);
}

float otherRand=NAN;
float getGaussRandVar(float mean, float var){
	float val;
	if(!isnanf(otherRand)){
		val = otherRand;
		otherRand = NAN;
	}else{
		float u1, u2, tmp;
		do{
			u1 = 2*randFloat()-1.0;
			u2 = 2*randFloat()-1.0;
			tmp = u1*u1+u2*u2;
		}while(tmp>=1.0);
		
		tmp = sqrtf((-2.0*logf(tmp))/tmp);
		val = u1*tmp;
		otherRand = u2*tmp;
	}
	return val*sqrtf(var) + mean;
}

float getCauchyRandVar(float mean, float var){
	if(mean<-M_PI||mean>M_PI||var<0||var>(2*M_PI)) return NAN;
	var = 1.0+var;	
	float sign;
	if(var*var-1>=0)		sign = 1;
	else if(var*var-1<0)	sign = -1;
	else					sign = 0;
	float val = mean+2*atan2f((var-1)*tanf(atan2f((var+1)/tanf(mean/2.0),var-1)+M_PI*randFloat()*sign),var+1);
	return pretty_angle(val);
}
#endif

void init(){	
	//float valA, valB;
	//printf("dat = {");	
	//for(uint16_t i=0;i<9999;i++){
		//if(rand()&0x1){
			//valA = getNormalRandVar(1.2, M_PI);
			//valB = getNormalRandVar(-3, 16);
		//}else{
			//valB = getNormalRandVar(-3, 16);	
			//valA = getNormalRandVar(1.2, M_PI);								
		//}
		//printf("{%f, %f},", valA, valB);
	//}
	//if(rand()&0x1){
		//valA = getNormalRandVar(1.2, M_PI);
		//valB = getNormalRandVar(-3, 16);
	//}else{
		//valB = getNormalRandVar(-3, 16);
		//valA = getNormalRandVar(1.2, M_PI);
	//}	
	//printf("{%f, %f}};\r\n", valA, valB);		
	//float val;
	//for(uint16_t i=0;i<999;i++){	
		//val = ( ((float)i)/250.0 ) - 2.0;	
		//printf("%f, ", gaussianPDF(val, 1.2,M_PI));
	//}
	//printf("%f};\r\n",gaussianPDF(2.0,1.2,M_PI));
	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++) cleanNeighbor(i);	
	loopCount = 0;
	outwardDir = 0;
	numNeighbors = 0;
	lastGoodbye = 0;	
	myRNBLoop = (get_droplet_id()%LOOPS_PER_RNB);
	myMsgLoop = ((get_droplet_id()+LOOPS_PER_RNB/2)%LOOPS_PER_RNB);
	myState=NOT_BALL;
	firstLoop = 1;
	printf("myRNBLoop: %d\r\n", myRNBLoop);
}

void loop(){
	if((get_time()%LOOP_PERIOD_MS)<(LOOP_PERIOD_MS/50)){
		set_rgb(0,0,0);
		if((!firstLoop)&&(loopCount==myRNBLoop)){
			delay_ms(5);
			broadcast_rnb_data();
			myRNBLoop=(myRNBLoop)%LOOPS_PER_RNB;
		}else if((!firstLoop)&&(loopCount==myMsgLoop)){
			delay_ms(30);
			//sendBotPosMsg();
			myMsgLoop=(myMsgLoop)%LOOPS_PER_RNB;			
		}else if((!firstLoop)&&(loopCount==LOOPS_PER_RNB-1)){
			printf("\nT: %lu\r\n", get_time());
			printf("  ID  |   R   |   rV   |   B   |   bV   |   H   |   hV   |\r\n");
			for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++)
			{
				if(neighbors[i].id!=0)
				printf(" %04X | % -6.2f| %6.3f | % -6.1f| %6.3f | % -6.1f| %6.3f |\r\n", neighbors[i].id, neighbors[i].r, neighbors[i].rV, rad_to_deg(neighbors[i].b), rad_to_deg(neighbors[i].bV), rad_to_deg(neighbors[i].h), rad_to_deg(neighbors[i].hV));
			}
			printf("\n\n");
		}
		
		if(loopCount==(LOOPS_PER_RNB-1)){ firstLoop = 0;}
		loopCount=((loopCount+1)%LOOPS_PER_RNB);
		delay_ms(LOOP_PERIOD_MS/50);
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
}

void useNewRnbMeas(){
	rnb_updated=0;	
	float rVar, bVar, hVar;
	getVarsFromConf(last_good_rnb.conf, &rVar, &bVar, &hVar);
	BotPos* botPos = addNeighbor(last_good_rnb.id_number, last_good_rnb.conf);
	POS_DEBUG_PRINT("(RNB) ID: %04X\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", last_good_rnb.id_number, last_good_rnb.range, rVar, rad_to_deg(last_good_rnb.bearing), rad_to_deg(bVar), rad_to_deg(last_good_rnb.heading), rad_to_deg(hVar), last_good_rnb.conf);		
	if(botPos==NULL) return;
	if(botPos->id == 0){ //We weren't tracking this ID before, so just add the new info.
		botPos->id		= last_good_rnb.id_number;
		botPos->r	= last_good_rnb.range;
		botPos->rV  = rVar;
		botPos->b	= last_good_rnb.bearing;
		botPos->bV  = bVar;
		botPos->h	= last_good_rnb.heading;
		botPos->hV  = hVar;
		#ifdef PARTICLE
		for(uint8_t i=0;i<NUM_PARTICLES;i++){
			botPos->particles[i].r = getGaussRandVar(botPos->r, botPos->rV);
			botPos->particles[i].b = getCauchyRandVar(botPos->b, botPos->bV);
			botPos->particles[i].h = getCauchyRandVar(botPos->h, botPos->hV);
		}
		#endif
	}else if(botPos->id == last_good_rnb.id_number){
		POS_DEBUG_PRINT("\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", botPos->r, botPos->rV, rad_to_deg(botPos->b), rad_to_deg(botPos->bV), rad_to_deg(botPos->h), rad_to_deg(botPos->hV));
		fuseData(botPos, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading, rVar, bVar, hVar);
	}else{
		printf("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n");
	}
}

int cmpFunc(const void* a, const void* b){
	BotPos* x = (BotPos*)a;
	BotPos* y = (BotPos*)b;
	float xConf = getConfFromVars(x->rV, x->bV, x->hV);
	float yConf = getConfFromVars(y->rV, y->bV, y->hV);
	if(xConf==INFINITY&&yConf==INFINITY)	return 0;
	else if(xConf==INFINITY)					return -1;
	else if(yConf==INFINITY)					return 1;
	return (yConf - xConf);
}

void sendBotPosMsg(){
	char msgHolder[sizeof(BotPosMsg)];
	BotPosMsg* msg = (BotPosMsg*)msgHolder;
	msg->flag = BOT_POS_FLAG;
	qsort(neighbors, NUM_TRACKED_BOTS, sizeof(BotPos), cmpFunc);
	for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		msg->bots[i].id = neighbors[i].id;
		packPackedBotPos(&(msg->bots[i]), neighbors[i].r, neighbors[i].b, neighbors[i].h, neighbors[i].rV, neighbors[i].bV, neighbors[i].hV);
	}
	ir_send(ALL_DIRS, msgHolder, sizeof(BotPosMsg));
}

void useBotPosMsg(PackedBotPos* bots, uint16_t senderID){
	BotPos* sender = getNeighbor(senderID);
	
	if(sender==NULL){
		//printf("Got botPosMsg from someone I don't have data for, so doing nothing.\r\n");
		return;
	}
	
	float Rso, Bso, Hso, vRso, vBso, vHso;

	uint8_t foundMyself=0;
	for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		if(bots[i].id!=get_droplet_id()) continue;
		foundMyself=1;
		unpackPackedBotPos(&(bots[i]), &Rso, &Bso, &Hso, &vRso, &vBso, &vHso);	
		POS_DEBUG_PRINT("ID: %04X\r\n\tRms: % 5.1fñ%-4.1f Bms: % 6.1fñ%-4.1f Hms: % 6.1fñ%-4.1f\r\n\tRsm: % 5.1fñ%-4.1f Bsm: % 6.1fñ%-4.1f Hsm: % 6.1fñ%-4.1f\r\n", sender->id, 
																						sender->r, sender->rV, rad_to_deg(sender->b), rad_to_deg(sender->bV), rad_to_deg(sender->h), rad_to_deg(sender->hV),
																						Rso, vRso, rad_to_deg(Bso), rad_to_deg(vBso), rad_to_deg(Hso), rad_to_deg(vHso));
		fuseData(sender, Rso, pretty_angle(Bso-Hso+M_PI), -Hso, vRso, vBso+vHso, vHso);
		break;
	}
	if(!foundMyself) return;
	
	float Rms = sender->r;
	float Bms = sender->b;
	float Hms = sender->h;
	float vRms = sender->rV;
	float vBms = sender->bV;
	float vHms = sender->hV;
		
	for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		if(bots[i].id==0||bots[i].id==get_droplet_id()) continue;
		BotPos* sharedNeighbor = getNeighbor(bots[i].id);
		if(sharedNeighbor==NULL) continue;//Someone in their packed bots not in my tracked bots.
		unpackPackedBotPos(&(bots[i]), &Rso, &Bso, &Hso, &vRso, &vBso, &vHso);
		//We have a loop closure. Need to combine my estimate of senders pos + senders estimate of otherPos
		float Xmo, Ymo, Rmo, Bmo, Hmo;
		Xmo = Rms*cos(Bms)+Rso*cos(Bso+Hms);
		Ymo = Rms*sin(Bms)+Rso*sin(Bso+Hms);
		Rmo = hypotf(Xmo,Ymo);
		Bmo = atan2(Ymo,Xmo);
		Hmo = pretty_angle(Hms + Hso);
		float vRmo, vBmo, vHmo;
		combineVars(Rms, Bms, Hms, vRms, vBms, vHms, Rso, Bso, Hso, vRso, vBso, vHso, &vRmo, &vBmo, &vHmo);
		//printf("{%8f,%8f,%8f,%8f,%8f,%8f,%8f,%8f,%8f}, Rms, Bms, Hms, Rso, Bso, Hso, Rmo, Bmo, Hmo);
		POS_DEBUG_PRINT("ID: %04X\r\n\tRms: % 5.1fñ%-4.1f Bms: % 6.1fñ%-4.1f Hms: % 6.1fñ%-6.3f\r\n\tRso: % 5.1fñ%-4.1f Bso: % 6.1fñ%-4.1f Hso: % 6.1fñ%-4.1f\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", bots[i].id, 
																			Rms, vRms, rad_to_deg(Bms), rad_to_deg(vBms), rad_to_deg(Hms), rad_to_deg(vHms),
																			Rso, vRso, rad_to_deg(Bso), rad_to_deg(vBso), rad_to_deg(Hso), rad_to_deg(vHso),
																			Rmo, vRmo, rad_to_deg(Bmo), rad_to_deg(vBmo), rad_to_deg(Hmo), rad_to_deg(vHmo));
		//Now combine that with existing information on that bot.
		fuseData(sharedNeighbor, Rmo, Bmo, Hmo, vRmo, vBmo, vHmo);
	}
}

void combineVars(float Rms, float Bms, float Hms, float vRms, float vBms, float vHms, float Rso, float Bso, float Hso, float vRso, float vBso, float vHso, float* vRmo, float* vBmo, float* vHmo){
	//float vRms, vBms, vHms, vRso, vBso, vHso;
	//getVarsFromConf(Cms, &vRms, &vBms, &vHms);
	//getVarsFromConf(Cso, &vRso, &vBso, &vHso);
	//
	float Rms2 = powf(Rms,2.0);
	float Rso2 = powf(Rso,2.0);
	
	float denominator = (Rms2+Rso2+2*Rms*Rso*cosf(Bms-Bso-Hms));
	
	float cosTerm = (-Rso2*vRms+Rms2*(Rso2*(vBms+vBso+vHms)-vRso))*cosf(2*Bms-2*Bso-2*Hms);
	
	*vRmo = (Rms2*(Rso2*(vBms+vBso+vHms)+2*vRms+vRso)+Rso2*(vRms+2*vRso)-cosTerm+4*Rms*Rso*(vRms+vRso)*cosf(Bms-Bso-Hms))/(2.0*denominator);
	*vBmo = (2*powf(Rms,4.0)*vBms+2*powf(Rso,4.0)*(vBso+vHms)+Rso2*vRms+Rms2*(Rso2*(vBms+vBso+vHms)+vRso)+cosTerm+4*Rms*Rso*(Rms2*vBms+Rso2*(vBso+vHms))*cosf(Bms-Bso-Hms))/(2.0*powf(denominator,2.0));
	*vHmo = vHms+vHso;
}

//for each tracked robot, we have N points.
//whenever we get a new measurement of a tracked robot's position, we calculate how likely each point is, given the new measurement.
//These calculated likelihoods are put in an array of weights. We should also calculate the maximum weight (ie, most likely particle)

#ifdef PARTICLE
void fuseData(BotPos* currPos, float otherR, float otherB, float otherH, float otherRvar, float otherBvar, float otherHvar){
	//PARTICLE VERSION
	float otherVals[3] = {otherR, otherB, otherH};
	float otherVars[3] = {otherRvar, otherBvar, otherHvar};
		
	float weights[NUM_PARTICLES];
	for(uint8_t i=0;i<NUM_PARTICLES;i++) weights[i]=1.0;
	float maxWeight = -1;
	uint8_t maxWeightIdx = 0;
	
	//TODO: If we know how the bot is moving we can add that here.
	Particle resampledParticles[NUM_PARTICLES];
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
		resampledParticles[i].r = getGaussRandVar(currPos->particles[i].r, currPos->rV);
		resampledParticles[i].b = getCauchyRandVar(currPos->particles[i].b, currPos->bV);
		resampledParticles[i].h = getCauchyRandVar(currPos->particles[i].h, currPos->hV);
	}
	
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
		float vals[3] = {resampledParticles[i].r, resampledParticles[i].b, resampledParticles[i].h};
		for(uint8_t j=0;j<3;j++){
			if(j==0){
				weights[i] *= gaussianPDF(vals[j], otherVals[j], otherVars[j]);
				printf("\t\t\trW: %f\r\n",gaussianPDF(vals[j], otherVals[j], otherVars[j]));
			}else{
				weights[i] *= cauchyPDF(vals[j], otherVals[j], otherVars[j]);
				printf("\t\t\taW: %f\r\n",cauchyPDF(vals[j], otherVals[j], otherVars[j]));				
			}
		}
		if(weights[i]>maxWeight){
			maxWeight = weights[i];
			maxWeightIdx = i;
		}		
	}
	POS_DEBUG_PRINT("\tResampled Particles:\r\n");
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
		POS_DEBUG_PRINT("\t\tR: % 6.2f B: %6.1f H: %6.1f W: % 7.3f\r\n", resampledParticles[i].r, rad_to_deg(resampledParticles[i].b), rad_to_deg(resampledParticles[i].h), weights[i]);
	}	
	
	uint8_t idx = rand_byte()%NUM_PARTICLES;
	float beta = 0.0;
	float newVals[3] = {0.0,0.0,0.0};
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
			beta+=randFloat()*2.0*maxWeight;
			while(beta > weights[idx]){
				beta-=weights[idx];
				idx = (idx+1)%NUM_PARTICLES;
			}
			currPos->particles[i].r = resampledParticles[idx].r;
			currPos->particles[i].b = resampledParticles[idx].b;
			currPos->particles[i].h = resampledParticles[idx].h;
			newVals[0]+= currPos->particles[i].r;
			newVals[1]+= currPos->particles[i].b;
			newVals[2]+= currPos->particles[i].h;
	}
	for(uint8_t i=0;i<3;i++){
		newVals[i] = newVals[i]/((float)NUM_PARTICLES);
	}
	
	POS_DEBUG_PRINT("\tNew Particles:\r\n");	
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
		POS_DEBUG_PRINT("\t\tR: % 6.2f B: %6.1f H: %6.1f\r\n", currPos->particles[i].r, rad_to_deg(currPos->particles[i].b), rad_to_deg(currPos->particles[i].h));
	}	
	
	float newVars[3] = {0.0,0.0,0.0};	
	for(uint8_t i=0;i<NUM_PARTICLES;i++){
		newVars[0] += powf(currPos->particles[i].r-newVals[0],2.0);
		newVars[1] += powf(currPos->particles[i].b-newVals[1],2.0);
		newVars[2] += powf(currPos->particles[i].h-newVals[2],2.0);
	}
	for(uint8_t i=0;i<3;i++){
		newVars[i] = newVars[i]/((float)NUM_PARTICLES);	
	}
	

	
	currPos->r = newVals[0];
	currPos->b = newVals[1];
	currPos->h = newVals[2];
	currPos->rV = newVars[0];
	currPos->bV = newVars[1];
	currPos->hV = newVars[2];
	POS_DEBUG_PRINT("\tFUSED | R: % 6.2fñ%-6.3f B: % 6.1fñ%-6.3f H: % 6.1fñ%-6.3f\r\n", newVals[0], newVars[0], rad_to_deg(newVals[1]), rad_to_deg(newVars[1]), rad_to_deg(newVals[2]), rad_to_deg(newVars[2]));						
}
#endif

#ifdef MLE
void fuseData(BotPos* currPos, float otherR, float otherB, float otherH, float otherRvar, float otherBvar, float otherHvar){
	//MLE VERSION
	float vals[3] = {currPos->r, currPos->b, currPos->h};
	float vars[3] = {currPos->rV, currPos->bV, currPos->hV};
	float otherVals[3] = {otherR, otherB, otherH};
	float otherVars[3] = {otherRvar, otherBvar, otherHvar};
	
	float newVals[3];
	float newVars[3];
	
	for(uint8_t i=0;i<3;i++){
		newVars[i] = 1.0/((1.0/vars[i])+(1.0/otherVars[i]));
		if(i==0){
			newVals[i] = (vals[i]/vars[i]+otherVals[i]/otherVars[i])*newVars[i];
		}else{
			newVals[i] = atan2(sin(vals[i])/vars[i]+sin(otherVals[i])/otherVars[i], cos(vals[i])/vars[i]+cos(otherVals[i])/otherVars[i]);
		}
	}
	
	currPos->r = newVals[0];
	currPos->b = newVals[1];
	currPos->h = newVals[2];
	currPos->rV = newVars[0];
	currPos->bV = newVars[1];
	currPos->hV = newVars[2];
	POS_DEBUG_PRINT("\tFUSED | R: % 6.2fñ%-6.3f B: % 6.1fñ%-6.3f H: % 6.1fñ%-6.3f\r\n", newVals[0], newVars[0], rad_to_deg(newVals[1]), rad_to_deg(newVars[1]), rad_to_deg(newVals[2]), rad_to_deg(newVars[2]));						
}
#endif

#ifdef KALMAN
//If moving:
const float PROC_NOISE[3] = {powf(1.0,2.0), powf(M_PI/30.0,2.0), powf(M_PI/22.5,2.0)};
//If not moving:
//const float PROC_NOISE[3] = {0, 0, 0};
const float MEAS_NOISE[3] = {powf(3.0,2.0), powf(M_PI/6.0,2.0), powf(M_PI/6.0,2.0)};

void fuseData(BotPos* currPos, float otherR, float otherB, float otherH, float otherRvar, float otherBvar, float otherHvar){
	//KALMAN VERSION
	float vals[3] = {currPos->r, currPos->b, currPos->h};
	float vars[3] = {currPos->rV, currPos->bV, currPos->hV};
	float otherVals[3] = {otherR, otherB, otherH};
	float otherVars[3] = {otherRvar, otherBvar, otherHvar};
	
	float newVals[3], newVars[3];
	float predVar, kalmanGain, newVal, newVar;
	
	for(uint8_t i=0;i<3;i++){
		predVar = vars[i] + PROC_NOISE[i];
		kalmanGain = (predVar)/(predVar+MEAS_NOISE[i]);
		if(i>0){
			//if(abs(vals[i]-otherVals[i])>deg_to_rad(75)) kalmanGain*=0.2;
			newVals[i] = pretty_angle(pretty_angle(kalmanGain*otherVals[i])+pretty_angle((1.0-kalmanGain)*vals[i]));
			/*
			atan2(kalmanGain*sin(otherVals[i])+(1.0-kalmanGain)*sin(vals[i]),kalmanGain*cos(otherVals[i])+(1.0-kalmanGain)*cos(vals[i]));*/
		}else{
			//if(abs(vals[i]-otherVals[i]>4*DROPLET_RADIUS)) kalmanGain*=0.2;
			newVals[i] = kalmanGain*otherVals[i] + (1.0 - kalmanGain)*vals[i];
		}
		newVars[i] = predVar*(1.0-kalmanGain);
	}

	currPos->r = newVals[0];
	currPos->b = newVals[1];
	currPos->h = newVals[2];
	currPos->rV = newVars[0];
	currPos->bV = newVars[1];
	currPos->hV = newVars[2];
	POS_DEBUG_PRINT("\tFUSED | R: % 6.2fñ%-6.3f B: % 6.1fñ%-6.3f H: % 6.1fñ%-6.3f\r\n", newVals[0], newVars[0], rad_to_deg(newVals[1]), rad_to_deg(newVars[1]), rad_to_deg(newVals[2]), rad_to_deg(newVars[2]));						
}
#endif

void handle_msg(ir_msg* msg_struct){
	if(((BotPosMsg*)(msg_struct->msg))->flag==BOT_POS_FLAG){
		useBotPosMsg(((BotPosMsg*)(msg_struct->msg))->bots, msg_struct->sender_ID);
	}
}

void cleanNeighbor(uint8_t idx){
	neighbors[idx].id = 0;
	neighbors[idx].r = 0;
	neighbors[idx].b = 0;
	neighbors[idx].h = 0;
	neighbors[idx].rV = 0;
	neighbors[idx].bV = 0;
	neighbors[idx].hV = 0;
}

BotPos* addNeighbor(uint16_t id, float var){
	uint8_t emptyIdx=0xFF;
	qsort(neighbors, NUM_TRACKED_BOTS, sizeof(BotPos), cmpFunc);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(neighbors[i].id==id){
			return &(neighbors[i]);
		}
		if(neighbors[i].id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		numNeighbors++;
		return &(neighbors[emptyIdx]);
	}
	// No empty spot, so we'll remove the neighbor we're
	// least confident in. But only if we're more confident
	// in the new neighbor.
	
	float neighbConf = getConfFromVars(neighbors[NUM_TRACKED_BOTS-1].rV, neighbors[NUM_TRACKED_BOTS-1].bV, neighbors[NUM_TRACKED_BOTS-1].hV);
	if(neighbConf<var){
		cleanNeighbor(NUM_TRACKED_BOTS-1);
		return &(neighbors[NUM_TRACKED_BOTS-1]);
		}else{
		return NULL;
	}
	
}

BotPos* getNeighbor(uint16_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(neighbors[i].id==id){
			return &(neighbors[i]);
		}
	}
	return NULL;
}

void removeNeighbor(uint16_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(neighbors[i].id==id){
			cleanNeighbor(i);
			numNeighbors--;
			break;
		}
	}
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	return 0;
}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
