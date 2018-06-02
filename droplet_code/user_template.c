#include "user_template.h"


#define START_ID	0x5D61
#define END_ID		0x1361

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	hopCount = 255;
	if(getDropletID()==START_ID){
		hopCount = 0;
		startTime = getTime();
		scheduleTask(1000, prepSpeedMsg, NULL);
	}else{
		scheduleTask(1000, noteStartTime, NULL);
	}
}

void noteStartTime(){
	startTime = getTime();
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(hopCount!=255){
		setHSV((hopCount%6)*60, 255, 150);
	}
	delayMS(100);
}

void handleMeas(Rnb* meas){
	
}

uint32_t getExponentialBackoff(uint8_t c){	
	volatile uint32_t k;
	volatile uint32_t N;
	
	N= (((uint32_t)1)<<c);
	
	k = randQuad()%N;
	return ((k*16)+5);///20000000;
	
}

void prepSpeedMsg(){
	SpeedMsgNode* msgNode = (SpeedMsgNode*)myMalloc(sizeof(SpeedMsgNode));
	msgNode->numTries = 0;
	(msgNode->msg).flag = SPEED_MSG_FLAG;
	sendSpeedMsg(msgNode);
}

void sendSpeedMsg(SpeedMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if(msgNode->numTries>6){
			myFree(msgNode);
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendSpeedMsg, msgNode);
		}
		msgNode->numTries++;
	}else{
		(msgNode->msg).hopCount = (hopCount+1);
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(SpeedMsg));
		myFree(msgNode);
	}
}

void handleSpeedMsg(SpeedMsg* msg){
	if(hopCount==255){
		hopCount = msg->hopCount;
		if(getDropletID()==END_ID){
			timeToCompletion = getTime() - startTime;
			printf("!\r\n");
			printf("{%lu, %hu},\r\n", timeToCompletion, hopCount);
		}

		prepSpeedMsg();
		
	}
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msg_struct){
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