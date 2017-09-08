#include "user_template.h"

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	for(uint8_t i=0;i<MSG_FILLER_LENGTH;i++){
		testMsg.filler[i] = (char)(65+i);
	}
	testMsg.id = 0;
	lastMsgTime = getTime();
}



/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(getTime()-lastMsgTime > MSG_SEND_PERIOD){
		if(getDropletID()==0x6C66){
			irSend(ALL_DIRS, (char*)(&testMsg), sizeof(TestMsg));
			testMsg.id++;
			lastMsgTime = getTime();
		}
	}
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
	printf("Received %u\r\n", msg->id);
}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}