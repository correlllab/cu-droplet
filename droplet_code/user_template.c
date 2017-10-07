#include "user_template.h"


/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	schedulePeriodicTask(2000, lightsOn, NULL);
}



/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){

	delayMS(10);
}

void lightsOn(){
	setRGB(200,200,200);
	scheduleTask(1000, lightsOff, NULL);
}

void lightsOff(){
	setRGB(0,0,0);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msgStruct){

}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t userHandleCommand(char* command_word, char* command_args){
	//return 0;
//}