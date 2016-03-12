#include "droplet_programs/hopCount.h"
#include "stdio.h"
#include "stdarg.h"

//#define DEBUG_MODE

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
	loopCount = 0;	
	myMsgLoop = (get_droplet_id()%(SLOTS_PER_FRAME-1));;
	printf("MsgLoop: %d\r\n", myMsgLoop);
}

void loop(){
	if((get_time()%SLOT_LENGTH_MS)<(SLOT_LENGTH_MS/50)){
		if(loopCount==myMsgLoop){
			propagateAsNecessary();
		}else if((!firstLoop)&&(loopCount==SLOTS_PER_FRAME-1)){
			//end of frame.
		}
		
		setColor();
		
		loopCount=((loopCount+1)%SLOTS_PER_FRAME);
		delay_ms(SLOT_LENGTH_MS/50);
	}
}

void setColor(){
	uint8_t seedFrame = (loopCount*(NUM_SEEDS+1))/SLOTS_PER_FRAME;
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
}

void propagateAsNecessary(){
	uint32_t oldest = get_time();
	uint8_t oldestIdx = 0xFF;
	for(uint8_t i=0;i<NUM_SEEDS;i++){
		if(trackedHops[i].flag){
			if(trackedHops[i].time<oldest){
				oldest = trackedHops[i].time;
				oldestIdx = i;
			}
		}
	}
	if(oldestIdx==0xFF){
		//No flags were set.
	}else{
		propagateHop(oldestIdx);
	}
}

void sendHopMsg(uint16_t id, uint8_t hC){
	HopMsg msg;
	msg.flag = HOP_MSG_FLAG;
	msg.id = id;
	msg.hopCount = hC;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(HopMsg));
}

void handle_msg(ir_msg* msg_struct){
	HopMsg* msg;
	if((msg=((HopMsg*)(msg_struct->msg)))->flag==HOP_MSG_FLAG){
		if(msg->id==0){
			return;
		}
		uint8_t idIdx = 0xFF;
		uint8_t emptyIdx = 0xFF;
		for(uint8_t i ; i<NUM_SEEDS ; i++){
			if(emptyIdx==0xFF && trackedHops[i].id == 0){
				emptyIdx = i;
			}
			if(trackedHops[i].id == msg->id){
				idIdx = i;
				break;
			}
		}
		if(idIdx!=0xFF){ //ID found.
			if(msg->hopCount < trackedHops[idIdx].hopCount){
				trackedHops[idIdx].hopCount = msg->hopCount;
				trackedHops[idIdx].time = get_time();
				trackedHops[idIdx].flag = 1;
			}
		}else{ //ID not found
			if(emptyIdx==0xFF){
				printf("ERROR\tID not found and no room in trackedHops!\r\n");
			}else{
				trackedHops[emptyIdx].id = msg->id;
				trackedHops[emptyIdx].hopCount = msg->hopCount;
				trackedHops[emptyIdx].time = get_time();
				trackedHops[emptyIdx].flag = 1;
			}
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
			printf("IR Power must be 0 to 256 inclusive. Yes, 256 is max. Not 255.\r\n")
		}else{
			printf("Setting IR Power to %u\r\n", power);
			set_all_ir_powers(power);
		}
		return 1;
	}
	return 0;
}