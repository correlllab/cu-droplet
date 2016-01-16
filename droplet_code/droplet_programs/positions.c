#include "droplet_programs/positions.h"
#include "stdio.h"
#include "stdarg.h"

//#define POS_DEBUG_MODE

#ifdef POS_DEBUG_MODE
#define POS_DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define POS_DEBUG_PRINT(format, ...)
#endif

int cmpFunc(const void* a, const void* b){
	OtherBot* aN = (OtherBot*)a;
	OtherBot* bN = (OtherBot*)b;
	float aR = aN->pos.r;
	float bR = bN->pos.r;
	if(aR==0){
		return 1;
	}else if(bR==0){
		return -1;
	}else if(fabsf(aR-bR)<=1.5){ 
		return (int)(aN->pos.rV - bN->pos.rV);
	}else{
		return (int)(aR-bR);
	}

}

void init(){	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	numNeighbors = 0;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbors[i].emptyProb=INIT_LIKELIHOOD_EMPTY;
		neighbors[i].untrackedProb=1.0-INIT_LIKELIHOOD_EMPTY;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN-1;j++){
			neighbors[i].candidates[j].P=0.0;
			neighbors[i].candidates[j].id=0;
		}
	}
	loopCount = 0;
	outwardDir = 0;
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
			//delay_ms(30);
			//sendBotPosMsg();
			myMsgLoop=(myMsgLoop)%LOOPS_PER_RNB;			
		}else if((!firstLoop)&&(loopCount==LOOPS_PER_RNB-1)){
			qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), cmpFunc);				
			printf("\nT: %lu\r\n", get_time());
			printf("  ID  |    R    |   rV   |   B    |   bV   |   H    |   hV   |  CONF  |\r\n{\r\n");
			BotPos* pos;
			for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++)
			{
				if(nearBots[i].id!=0){
					pos = &(nearBots[i].pos);
					printf("{{\"%04X\", % -7.2f , %6.2f , % -6.1f , %6.1f , % -6.1f , %6.1f , %6.1f },{\r\n", nearBots[i].id, pos->r, pos->rV, rad_to_deg(pos->b), rad_to_deg(pos->bV), rad_to_deg(pos->h), rad_to_deg(pos->hV), getConfFromVars(pos->rV, pos->bV, pos->hV));
					//for(uint8_t j=0;j<NUM_PACKED_BOTS;j++){
						//if(neighbors[i].neighbs[j].id!=0){
							//pos = &(neighbors[i].neighbs[j].pos);
							//printf("{   \"%04X\", % -7.2f , %6.2f , % -6.1f , %6.1f , % -6.1f , %6.1f , %6.1f },\r\n", neighbors[i].neighbs[j].id, pos->r, pos->rV, rad_to_deg(pos->b), rad_to_deg(pos->bV), rad_to_deg(pos->h), rad_to_deg(pos->hV), getConfFromVars(pos->rV, pos->bV, pos->hV));
						//}
					//}
					printf("}},\r\n");
				}
			}
			printf("}\r\n\n");
			processOtherBotData();			
		}
		
		if(loopCount==(LOOPS_PER_RNB-1)){ firstLoop = 0;}
		loopCount=((loopCount+1)%LOOPS_PER_RNB);
		delay_ms(LOOP_PERIOD_MS/50);
	}
	if(rnb_updated){
		useNewRnbMeas();
	}
}

int bayesCmpFunc(const void* a, const void* b){
	BayesBot* aN = (BayesBot*)a;
	BayesBot* bN = (BayesBot*)b;
	if((bN->P-aN->P)<0){ 
		return -1;
	}else if((bN->P-aN->P)>0){
		return 1;
	}else{
		return 0;
	}
}

void processOtherBotData(){
	float cartPos[NUM_TRACKED_BOTS][2];
	BotPos* pos;
	float dists[NUM_TRACKED_BOTS][NEIGHBORHOOD_SIZE];
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){

		if(nearBots[i].id!=0){
			//printf("Bot %04X:\r\n", nearBots[i].id);					
			pos = &(nearBots[i].pos);
			cartPos[i][0] = (pos->r)*cosf(pos->b+M_PI_2);
			cartPos[i][1] = (pos->r)*sinf(pos->b+M_PI_2);
			for(uint8_t j=0;j<NEIGHBORHOOD_SIZE;j++){
				dists[i][j] = hypotf(cartPos[i][0]-neighbPos[j][0],cartPos[i][1]-neighbPos[j][1]);
				//printf("\tSlot %hu: %f\r\n",j,dists[i][j]);				
			}
		}
	}
	BayesBot newProbs[NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN];
	uint8_t found, numTracked;
	
	
	NeighbSlot* neighbor;
	BayesBot* candidates;
	float newEmptyProb, newTotalTrackedProb, newUntrackedProb, total, probFound;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbor = &(neighbors[i]);
		candidates=neighbor->candidates;
		probFound = 0.0;
		numTracked=0;
		newEmptyProb=neighbor->emptyProb;				
		newUntrackedProb=neighbor->untrackedProb;
		for(uint8_t j=0;j<NUM_TRACKED_BOTS;j++){		
			found=0;
			newProbs[j].id=0;
			newProbs[j].P=0.0;
			for(uint8_t k=0;k<NUM_TRACKED_BAYESIAN;k++){
				if(candidates[k].id!=0){
					numTracked++;
					if(candidates[k].id==nearBots[j].id){
						found = 1;
						newProbs[j].id=candidates[k].id;
						newProbs[j].P=(4.0/dists[j][i])*candidates[k].P; //bayes-y
						probFound+=candidates[k].P;
					}
				}			
			}
			if(!found){
				newProbs[j].id=nearBots[j].id;
				newProbs[j].P = (2.0/dists[j][i])*((neighbor->untrackedProb)/(NUM_POSSIBLE_BOTS-numTracked));	
			}
			if(dists[j][i]>5.0){
				newEmptyProb+=0.05;
				newUntrackedProb+=0.01;
			}else{
				if(newEmptyProb>=0.2)	newEmptyProb+=-0.1;
				if(newUntrackedProb>=0.04) newUntrackedProb+=-0.02;
			}
		}
	//SO FAR, the above covers bots that were tracked and measured, and bots that were measured but not previously tracked.
	//need to also cover bots that were previously tracked, but not measured this time.
		for(uint8_t j=0 ; j < NUM_TRACKED_BAYESIAN ; j++){
			newProbs[NUM_TRACKED_BOTS+j].id=0;
			newProbs[NUM_TRACKED_BOTS+j].P=0.0;
			uint8_t found=0;
			for(uint8_t k=0;k<NUM_TRACKED_BOTS;k++){
				if(newProbs[k].id!=0 && newProbs[k].id==candidates[j].id){
					found=1;
					break;
				}
			}
			if(!found){
				//this previously tracked bot was not measured.
				newProbs[NUM_TRACKED_BOTS+j].id=candidates[j].id;
				newProbs[NUM_TRACKED_BOTS+j].P=UNMEASURED_NEIGHBOR_LIKELIHOOD*candidates[j].P;	
			}
		}
		//printf("\tLost prob: %f, new prob: %f\r\n",lostProb,newProb);
		if(newEmptyProb<0.1) newEmptyProb = 0.1;
		if(newUntrackedProb<0.1) newUntrackedProb = 0.1;
		//float emptyUntrackedDiff = fabsf(newUntrackedProb-2*newEmptyProb);		
		//newEmptyProb = newEmptyProb - newEmptyProb*emptyUntrackedDiff;		
		//newUntrackedProb = newUntrackedProb - emptyUntrackedDiff*newUntrackedProb;
		
		qsort(newProbs, NUM_TRACKED_BOTS+NUM_TRACKED_BAYESIAN,sizeof(BayesBot), bayesCmpFunc);
		newTotalTrackedProb=0.0;
		uint8_t tooSmallFlag = 0;
		float tooSmallThresh = newUntrackedProb/(NUM_POSSIBLE_BOTS-numTracked);
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			if(newProbs[j].id!=0){
				if(newProbs[j].P<=tooSmallThresh){
					newUntrackedProb+=newProbs[j].P;
					candidates[j].id=0;
					candidates[j].P=0.0;
				}else{
					candidates[j].id=newProbs[j].id;
					candidates[j].P=newProbs[j].P;
					newTotalTrackedProb+=candidates[j].P;
				}

			}else{
				candidates[j].id=0;
				candidates[j].P=0.0;
			}
		}
		


		
		total = newTotalTrackedProb+newEmptyProb+newUntrackedProb;
		//printf("\tnewEmptyProb: %f\tnewTotalProb: %f\t newUntrackedProb: %f\ttotal: %f\r\n",newEmptyProb, newTotalTrackedProb,newUntrackedProb, total);
		neighbor->emptyProb=newEmptyProb/total;
		neighbor->untrackedProb=newUntrackedProb/total;
		for(uint8_t j=0;j<NUM_TRACKED_BAYESIAN;j++){
			if(newProbs[j].id!=0){
				candidates[j].P=candidates[j].P/total;
			}
		}
	}

	uint8_t untrackedToggle, emptyToggle,emptyFirst;
	for(uint8_t i=0;i<NEIGHBORHOOD_SIZE;i++){
		neighbor = &(neighbors[i]);
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
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		cleanOtherBot(&nearBots[i]);
	}
	numNeighbors = 0;
	
	
	
	
	//uint8_t edgeMatrix[numNeighbors+1][numNeighbors+1];
	//edgeMatrix[0][0]=0;
	//for(uint8_t i=1;i<numNeighbors+1;i++){
		//edgeMatrix[0][i]=0xF;
		//edgeMatrix[i][0]=0;
		//for(uint8_t k=1;k<numNeighbors+1;k++){
			//edgeMatrix[i][k]=0;	
		//}
		//for(uint8_t j=0;j<NUM_PACKED_BOTS;j++){
			//if(neighbors[i-1].neighbs[j].id==0) continue;
			//if(neighbors[i-1].neighbs[j].id==get_droplet_id()){
				//edgeMatrix[i][0]=j+1;
				//continue;
			//}
			//for(uint8_t k=1;k<numNeighbors+1;k++){
				//if(neighbors[i-1].neighbs[j].id==neighbors[k-1].id){
					//edgeMatrix[i][k]=j+1;
					//break;
				//}
			//}			
		//}
	//}
	//printf("Adjacency Matrix (");
	//printf("%04X ", get_droplet_id());
	//for(uint8_t i=0;i<numNeighbors;i++){
		//if(neighbors[i].id==0) continue;
		//printf("%04X ", neighbors[i].id);
	//}
	//printf("):\r\n");
	//
	//for(uint8_t i=0;i<numNeighbors+1;i++){
		//if(i!=0 && neighbors[i-1].id==0) continue;
		//for(uint8_t j=0;j<numNeighbors+1;j++){
			//printf("%2X ",edgeMatrix[i][j]);
		//}
		//printf("\r\n");
	//}
	//printf("\r\n");
}

void useNewRnbMeas(){
	rnb_updated=0;	
	float rVar, bVar, hVar;
	getVarsFromConf(last_good_rnb.conf, &rVar, &bVar, &hVar);
	OtherBot* neighbor = addOtherBot(last_good_rnb.id_number, last_good_rnb.conf);
	POS_DEBUG_PRINT("(RNB) ID: %04X\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", last_good_rnb.id_number, last_good_rnb.range, rVar, rad_to_deg(last_good_rnb.bearing), rad_to_deg(bVar), rad_to_deg(last_good_rnb.heading), rad_to_deg(hVar), last_good_rnb.conf);		
	BotPos* pos = &(neighbor->pos);
	if(neighbor==NULL) return;
	if(neighbor->id == 0 || neighbor->id == last_good_rnb.id_number){ //We weren't tracking this ID before, so just add the new info.
		neighbor->id		= last_good_rnb.id_number;
		pos->r	= last_good_rnb.range;
		pos->rV  = rVar;
		pos->b	= last_good_rnb.bearing;
		pos->bV  = bVar;
		pos->h	= last_good_rnb.heading;
		pos->hV  = hVar;
	//}else if(neighbor->id == last_good_rnb.id_number){
		//POS_DEBUG_PRINT("\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n", pos->r, pos->rV, rad_to_deg(pos->b), rad_to_deg(pos->bV), rad_to_deg(pos->h), rad_to_deg(pos->hV));
		//fuseData(neighbor, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading, rVar, bVar, hVar);
	}else{
		printf("Error: Unexpected botPos->ID in use_new_rnb_meas.\r\n");
	}
}

void sendBotPosMsg(){
	//char msgHolder[sizeof(BotPosMsg)];
	//BotPosMsg* msg = (BotPosMsg*)msgHolder;
	//msg->flag = BOT_POS_FLAG;
	//qsort(neighbors, NUM_TRACKED_BOTS, sizeof(Neighbor), cmpFunc);
	//BotPos* pos;
	//for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		//msg->bots[i].id = neighbors[i].id;
		//if(msg->bots[i].id){
			//pos=&(neighbors[i].pos);
			//packPackedBotPos(&(msg->bots[i]), pos->r, pos->b, pos->h, pos->rV, pos->bV, pos->hV);
		//}
	//}
	//ir_send(ALL_DIRS, msgHolder, sizeof(BotPosMsg));
}

void useBotPosMsg(PackedBotPos* bots, uint16_t senderID){
	//Neighbor* sender = getNeighbor(senderID);
	//
	//if(sender==NULL){
		////printf("Got botPosMsg from someone I don't have data for, so doing nothing.\r\n");
		//return;
	//}
	//
	//float Rso, Bso, Hso, vRso, vBso, vHso;
//
	////uint8_t foundMyself=0;
	////for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		////if(bots[i].id==get_droplet_id()){
			////foundMyself=1;
			////break;
		////}
	////}
	////if(!foundMyself) return;
		//
	//for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		//if(bots[i].id==0) continue;
		//sender->neighbs[i].id=bots[i].id;
		//BotPos* pos = &(sender->neighbs[i].pos);
		//unpackPackedBotPos(&(bots[i]), &(pos->r), &(pos->b), &(pos->h), &(pos->rV), &(pos->bV), &(pos->hV));
	//}
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

//If moving:
const float PROC_NOISE[3] = {powf(1.0,2.0), powf(M_PI/30.0,2.0), powf(M_PI/22.5,2.0)};
//If not moving:
//const float PROC_NOISE[3] = {0, 0, 0};
const float MEAS_NOISE[3] = {powf(3.0,2.0), powf(M_PI/6.0,2.0), powf(M_PI/6.0,2.0)};

void fuseData(OtherBot* currPos, float otherR, float otherB, float otherH, float otherRvar, float otherBvar, float otherHvar){
	//KALMAN VERSION
	float vals[3] = {currPos->pos.r, currPos->pos.b, currPos->pos.h};
	float vars[3] = {currPos->pos.rV, currPos->pos.bV, currPos->pos.hV};
	float otherVals[3] = {otherR, otherB, otherH};
	float otherVars[3] = {otherRvar, otherBvar, otherHvar};	
	POS_DEBUG_PRINT("    FUSING:\r\n\tRmo': % 5.1fñ%-4.1f Bmo': % 6.1fñ%-4.1f Hmo': % 6.1fñ%-4.1f\r\n\tRmo: % 5.1fñ%-4.1f Bmo: % 6.1fñ%-4.1f Hmo: % 6.1fñ%-4.1f\r\n",
															otherR, otherRvar, rad_to_deg(otherB), rad_to_deg(otherBvar), rad_to_deg(otherH), rad_to_deg(otherHvar),
															vals[0], vars[0], rad_to_deg(vals[1]), rad_to_deg(vars[1]), rad_to_deg(vals[2]), rad_to_deg(vars[2]));

	
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

	currPos->pos.r  = newVals[0];
	currPos->pos.b  = newVals[1];
	currPos->pos.h  = newVals[2];
	currPos->pos.rV = newVars[0];
	currPos->pos.bV = newVars[1];
	currPos->pos.hV = newVars[2];
	POS_DEBUG_PRINT("\tR: % 6.2fñ%-6.3f B: % 6.1fñ%-6.3f H: % 6.1fñ%-6.3f\r\n", newVals[0], newVars[0], rad_to_deg(newVals[1]), rad_to_deg(newVars[1]), rad_to_deg(newVals[2]), rad_to_deg(newVars[2]));						
}

void handle_msg(ir_msg* msg_struct){
	if(((BotPosMsg*)(msg_struct->msg))->flag==BOT_POS_FLAG){
		useBotPosMsg(((BotPosMsg*)(msg_struct->msg))->bots, msg_struct->sender_ID);
	}
}

void cleanOtherBot(OtherBot* other){
	if(other==NULL) return;
	other->id = 0;
	other->pos.r = 0;
	other->pos.b = 0;
	other->pos.h = 0;
	other->pos.rV = 0;
	other->pos.bV = 0;
	other->pos.hV = 0;
	//BotPos* pos;
	//for(uint8_t i=0;i<NUM_PACKED_BOTS;i++){
		//neighbors[idx].neighbs[i].id=0;
		//pos = &(neighbors[idx].neighbs[i].pos);
		//pos->r = 0;
		//pos->b = 0;
		//pos->h = 0;
		//pos->rV = 0;
		//pos->bV = 0;
		//pos->hV = 0;
	//}
}

OtherBot* addOtherBot(uint16_t id, float var){
	uint8_t emptyIdx=0xFF;
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), cmpFunc);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
		if(nearBots[i].id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		numNeighbors++;
		return &(nearBots[emptyIdx]);
	}
	// No empty spot, so we'll remove the neighbor we're
	// least confident in. But only if we're more confident
	// in the new neighbor.
	
	BotPos* pos = &nearBots[NUM_TRACKED_BOTS-1].pos;
	float neighbConf = getConfFromVars(pos->rV, pos->bV, pos->hV);
	if(neighbConf<var){
		cleanOtherBot(&nearBots[NUM_TRACKED_BOTS-1]);
		return &(nearBots[NUM_TRACKED_BOTS-1]);
	}else{
		return NULL;
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
