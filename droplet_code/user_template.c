#include "user_template.h"

uint32_t last_rnb_time;
uint32_t last_red_time;
uint32_t last_green_time;
uint32_t last_blue_time;
uint32_t last_print;
uint32_t last_normal_msg;

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	last_rnb_time=get_time();
	last_normal_msg=get_time();
	last_red_time = 0;
	last_green_time = 0;
	last_blue_time = 0;
	set_all_ir_powers(250);
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(get_time()-last_rnb_time > 4850){
		printf("Performing RNB Broadcast.\r\n");
		set_green_led(100);
		last_green_time = get_time();
		broadcast_rnb_data();
		last_rnb_time = get_time();
	}
	if(get_time()-last_normal_msg > 2050){
		printf("Sending normal message: %02hX\r\n",ir_send(ALL_DIRS, "This is a normal message.", 25));
		last_normal_msg=get_time();
	}
	
	if(rnb_updated){
		set_blue_led(100);
		last_blue_time = get_time();
		printf("%04X | %3u  % 4d  % 4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated = 0;
	}
	
	if(get_time()-last_red_time > 500)
		set_red_led(0);
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
	printf("Got normal msg from %04X:\r\n\t", msg_struct->sender_ID);
	set_red_led(100);
	last_red_time = get_time();
	for(uint8_t i=0;i<msg_struct->length;i++){
		printf("%c",msg_struct->msg[i]);
	}
	printf("\r\n");
}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}