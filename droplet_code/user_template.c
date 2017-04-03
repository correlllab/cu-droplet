#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	set_rgb(0,255,0);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){

	delay_ms(5000);
	broadcast_rnb_data();
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){

}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. It should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}

