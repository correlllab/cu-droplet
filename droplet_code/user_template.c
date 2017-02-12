#include "user_template.h"

uint32_t last_rnb_broadcast;

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	last_rnb_broadcast = 0;
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
int i;
void loop(){
	
	if(get_time()-last_rnb_broadcast > 3000){
		broadcast_rnb_data();
		last_rnb_broadcast=get_time();
	}

	if(rnb_updated){
		            // new data in last_good_rnb, 	
		  //  printf("id_number %d\n\r",last_good_rnb.id_number);
		if((last_good_rnb.range) == 0)
			set_rgb(255,255,255);
		else if((last_good_rnb.range) != 0)
			set_rgb(255,0,0);
			printf("range %d\n\r", (uint16_t)(10*last_good_rnb.range));					 
		    printf("bearing %d\n\r", (int16_t)rad_to_deg(last_good_rnb.bearing));
		    printf("heading %d\n\r", (int16_t)rad_to_deg(last_good_rnb.heading));
			printf("-----------\n\r");
		    rnb_updated = 0; //Note! This line must be included for things to work properly.
	          }
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	//set_rgb(255,255,255);
	//printf("Time :%lu\n\r",msg_struct->arrival_time);
	//printf("Got message of length %hu from %04X:\r\n\t",msg_struct->length, msg_struct->sender_ID);
	//printf("RANGE%d",last_good_rnb.range);
	//for(uint8_t i=0;i<msg_struct->length;i++) printf("\n\r%c ", msg_struct->msg[i]);
	//printf("\r\n");
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. It should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}

