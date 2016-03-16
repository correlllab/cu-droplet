#include "droplet_programs/hopCount.h"
#include "stdio.h"
#include "stdarg.h"

#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(format, ...)
#endif

/*
 *  TODO
 *   --Choose seeds.
 *   --Infer location?
 */ 


void init(){
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		trackedHops[i].id=0;
		trackedHops[i].hopCount=0;
	}
	lastLoop = 0;	
	frameCount = 0;

	myMsgLoop = (get_droplet_id()%(SLOTS_PER_FRAME-1));;
	printf("MsgLoop: %d\r\n", myMsgLoop);
	frameStart = get_time();	
}

void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime -= FRAME_LENGTH_MS;
		frameStart = get_time()-frameTime;
		frameCount++;
	}
	uint8_t loopID = (SLOTS_PER_FRAME*frameTime)/FRAME_LENGTH_MS;
	if(loopID!=lastLoop){
		if(loopID==myMsgLoop){
			uint8_t numEmpty;
			uint8_t result;
			numEmpty = propagateAsNecessary();
			uint16_t thresh = numEmpty*(0xFFFF/EST_BOT_COUNT);
			if(rand_short()<thresh){
				result = addHop(get_droplet_id(), 0);
				if(result!=2){
					printf("ERROR\t numEmpty was>0 but addHop was unsuccessful or my ID was already present? (%hu)\r\n", result);
				}else{
					DEBUG_PRINT("I'm going to be a seed.\r\n");
				}
			}
		}
		if(loopID==SLOTS_PER_FRAME-1){
			DEBUG_PRINT("%lu [%lu]\r\n", frameCount, get_time());
			for(uint8_t i=0;i<NUM_SEEDS;i++){
				DEBUG_PRINT("\t%04X: %hu", trackedHops[i].id, trackedHops[i].hopCount);
				if(trackedHops[i].flag){
					DEBUG_PRINT(" [%lu]\r\n", trackedHops[i].time);
				}else{
					DEBUG_PRINT("\r\n");
				}
			}
		}
		setColor(loopID);
	}

	
	lastLoop = loopID;
	delay_ms(SLOT_LENGTH_MS/50);
}

void setColor(uint8_t loopID){
	uint8_t prevSeedFrame = (((loopID + (SLOTS_PER_FRAME-1))%SLOTS_PER_FRAME)*(NUM_SEEDS+1))/SLOTS_PER_FRAME;
	uint8_t seedFrame = (loopID*(NUM_SEEDS+1))/SLOTS_PER_FRAME;
	if(prevSeedFrame==seedFrame){
		if(seedFrame){
			if(trackedHops[seedFrame-1].hopCount>=MAX_HOP_COUNT){
				printf("WARNING: Hop count greater than max! (%hu>%hu)\r\n",trackedHops[seedFrame-1].hopCount, MAX_HOP_COUNT);
				set_rgb(255,255,255);
			}else{
				uint16_t hue = (360*((uint16_t)trackedHops[seedFrame-1].hopCount))/MAX_HOP_COUNT;
				set_hsv(hue, 255, 255);
			}
		}else{
			set_rgb(0,0,0);
		}
	}else{
		set_rgb(255,255,255);
	}
}

/* 
 * Returns number of empty seed slots, or '0' if we're sending a mesmsage. 
 * Either way, '0' means we definitely don't want to become a seed.
 */
uint8_t propagateAsNecessary(){
	uint32_t oldest = get_time();
	uint8_t oldestIdx = 0xFF;
	uint8_t numEmpty = 0;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(!trackedHops[i].id){
			numEmpty++;
		}else{
			if(trackedHops[i].flag){
				if(trackedHops[i].time<oldest){
					oldest = trackedHops[i].time;
					oldestIdx = i;
				}
			}
		}
	}
	if(oldestIdx==0xFF){
		return numEmpty;
	}else{
		propagateHop(oldestIdx);
		return 0;
	}
}

void sendHopMsg(uint16_t id, uint8_t hC){
	HopMsg msg;
	msg.flag = HOP_MSG_FLAG;
	msg.id = id;
	msg.hopCount = hC;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(HopMsg));
}

/*
 * Returns: 
 *		0 if the addition was unsuccessful,
 *		1 if successful and ID was already present.
 *		2 if successful and ID was not already present.
 */
uint8_t addHop(uint16_t id, uint8_t hopCount){
	uint8_t idIdx = 0xFF;
	uint8_t emptyIdx = 0xFF;
	for(uint8_t i=0 ; i<NUM_SEEDS ; i++){
		if(emptyIdx==0xFF && trackedHops[i].id == 0){
			emptyIdx = i;
		}
		if(trackedHops[i].id == id){
			idIdx = i;
			break;
		}
	}
	if(idIdx!=0xFF){ //ID found.
		if(hopCount < trackedHops[idIdx].hopCount){
			trackedHops[idIdx].hopCount = hopCount;
			trackedHops[idIdx].time = get_time();
			trackedHops[idIdx].flag = 1;
		}
		return 1;
	}else{ //ID not found
		if(emptyIdx==0xFF){
			return 0;
		}else{
			trackedHops[emptyIdx].id = id;
			trackedHops[emptyIdx].hopCount = hopCount;
			trackedHops[emptyIdx].time = get_time();
			trackedHops[emptyIdx].flag = 1;
			return 2;
		}
	}	
}

void handle_msg(ir_msg* msg_struct){
	HopMsg* msg;
	if((msg=((HopMsg*)(msg_struct->msg)))->flag==HOP_MSG_FLAG){
		if(msg->id==0){
			return;
		}	
		uint8_t result = addHop(msg->id, msg->hopCount);
		if(result==2){ //new ID added!
			for(uint8_t i=0;i<NUM_SEEDS;i++){
				if(trackedHops[i].id==get_droplet_id()){
					if(trackedHops[i].flag){ //We were going to be a seed, but hadn't broadcasted yet. So we can safely cancel.
						trackedHops[i].id = 0;
						trackedHops[i].hopCount = 0;
						trackedHops[i].time = 0;
						trackedHops[i].flag = 0;
						DEBUG_PRINT("Seed birth aborted.\r\n");
					}
					break;
				}
			}	
		}else if(result==0){
			printf("ERROR\tID not found and no room in trackedHops!\r\n");
		}
	}
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	if(strcmp_P(command_word,PSTR("ir_p"))==0){
		uint16_t power = atoi(command_args);
		if(power>256){
			printf("IR Power must be 0 to 256 inclusive. Yes, 256 is max. Not 255.\r\n");
		}else{
			printf("Setting IR Power to %u\r\n", power);
			set_all_ir_powers(power);
		}
		return 1;
	}
	return 0;
}