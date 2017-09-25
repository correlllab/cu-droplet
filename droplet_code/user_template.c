#include "user_template.h"

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	for(uint8_t i=0;i<MSG_FILLER_LENGTH;i++){
		testMsg.filler[i] = (char)(65+i);
	}
	if(getDropletID()==0x3B61 || getDropletID() == 0x2826){
		if(getDropletID()==0x3B61){
			testMsg.id = 0xFFFF0000;
		}else if(getDropletID()==0x2826){
			testMsg.id = 0x00000000;
		}
		scheduleTask(5000, prepTestMsg, NULL);
	}
}



/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	delayMS(10);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msg_struct){
	TestMsg* msg = (TestMsg*)(msg_struct->msg);
	for(uint8_t i=0;i<MSG_FILLER_LENGTH;i++){
		if(msg->filler[i] != testMsg.filler[i]){
			printf("ERROR! Unexpected message received.\r\n");
			return;
		}
	}
	printf("Received %08lX\r\n", msg->id);
}

uint32_t getExponentialBackoff(uint8_t c){
	volatile uint32_t k;
	volatile uint32_t N;
	
	N= (((uint32_t)1)<<c);
	
	k = randQuad()%N;
	return ((k*16)+5);///20000000;
	
}

void prepTestMsg(){
	TestMsgNode* msgNode = (TestMsgNode*)myMalloc(sizeof(TestMsgNode));
	msgNode->numTries = 0;
	for(uint8_t i=0;i<MSG_FILLER_LENGTH;i++){
		(msgNode->msg).filler[i] = testMsg.filler[i];
	}
	(msgNode->msg).id = testMsg.id;
	testMsg.id++;
	sendTestMsg(msgNode);
	scheduleTask(MSG_SEND_PERIOD, prepTestMsg, NULL);
}

void sendTestMsg(TestMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if(msgNode->numTries>6){
			myFree(msgNode);
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendTestMsg, msgNode);
		}
		msgNode->numTries++;
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(TestMsg));
		myFree(msgNode);
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