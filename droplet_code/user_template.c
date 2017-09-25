#include "user_template.h"


#define START_ID	0xDC9E
#define END_ID		0xCCD1

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	hopCount = 255;
	if(get_droplet_id()==START_ID){
		hopCount = 0;
		startTime = get_time();
		schedule_task(1000, prepSpeedMsg, NULL);
	}else{
		schedule_task(1000, noteStartTime, NULL);
	}
}

void noteStartTime(){
	startTime = get_time();
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(hopCount!=255){
		set_hsv((hopCount%6)*60, 255, 150);
	}
	delay_ms(100);
}

uint32_t getExponentialBackoff(uint8_t c){	
	volatile uint32_t k;
	volatile uint32_t N;
	
	N= (((uint32_t)1)<<c);
	
	k = rand_quad()%N;
	return ((k*16)+5);///20000000;
	
}

void prepSpeedMsg(){
	SpeedMsgNode* msgNode = (SpeedMsgNode*)myMalloc(sizeof(SpeedMsgNode));
	msgNode->numTries = 0;
	(msgNode->msg).flag = SPEED_MSG_FLAG;
	sendSpeedMsg(msgNode);
}

void sendSpeedMsg(SpeedMsgNode* msgNode){
	if(ir_is_busy(ALL_DIRS)){
		if(msgNode->numTries>6){
			myFree(msgNode);
		}else{
			schedule_task(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendSpeedMsg, msgNode);
		}
		msgNode->numTries++;
	}else{
		(msgNode->msg).hopCount = (hopCount+1);
		ir_send(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(SpeedMsg));
		myFree(msgNode);
	}
}

void handleSpeedMsg(SpeedMsg* msg){
	if(hopCount==255){
		hopCount = msg->hopCount;
		if(get_droplet_id()==END_ID){
			timeToCompletion = get_time() - startTime;
			printf("{%lu, %hu},\r\n", timeToCompletion, hopCount);
		}

		prepSpeedMsg();
		
	}
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	if( msg_struct->length == sizeof(SpeedMsg) && ((SpeedMsg*)(msg_struct->msg))->flag==SPEED_MSG_FLAG 	){
		 handleSpeedMsg((SpeedMsg*)(msg_struct->msg));
	}
}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}