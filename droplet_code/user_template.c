#include "user_template.h"

uint32_t last_rnb_time;
uint32_t last_green_time;
uint32_t last_blue_time;
uint32_t last_print;

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	set_rgb(0,0,0);
	delay_ms(rand_short()%2000);
	last_rnb_time=get_time();
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(get_time()-last_rnb_time > 2000){
		printf("Performing RNB Broadcast...\r\n");
		set_green_led(100);
		last_green_time = get_time();
		broadcast_rnb_data();
		last_rnb_time = get_time();
	}
	
	if(rnb_updated){
		set_blue_led(100);
		last_blue_time = get_time();
		printf("%04X | %3u  % 4d  % 4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated = 0;
	}
	
	if(get_time()-last_green_time > 500)
		set_green_led(0);
	if(get_time()-last_blue_time > 500)
		set_blue_led(0);
		
	//if(get_time()-last_print > 50){
		//last_print = get_time();
		//printf("%hX %02hX\r\n", processing_rnb, hp_ir_block_bm);
	//}
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

