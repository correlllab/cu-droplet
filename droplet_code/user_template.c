#include "user_template.h"
#include "sp_driver.h"
#include <stdio.h>

/*uint8_t data[512];
uint8_t read_data[512];
uint32_t address =0x0004;
uint8_t byte;
/*
 * any code in this function will be run once, when the robot starts.
 */




void init(){
		
	//delayMS(5000);
	//volatile int dummyArray[512];
	//memset(dummyArray, 0x55, 512);
	
	//for (int i= 0;i<512;i++)
	//dummyArray[i] = i;
	setRGB(255,0,0);
	
}


/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){

}

/*
 * after each pass through loop(), the robot checks for all mes sages it has 
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