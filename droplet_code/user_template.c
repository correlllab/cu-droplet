#include "user_template.h"

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	enable_sync_blink(500);
	set_sync_blink_color(0, 100, 0);
	set_sync_blink_duration(4000);
	set_rgb(0,0,150);
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	delay_ms(10);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){

}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}