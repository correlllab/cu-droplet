#include "user_template.h"

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
set_red_led(200);
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(rnb_updated){
		
		last_good_rnb.id;
		last_good_rnb.range;
		last_good_rnb.bearing;
		last_good_rnb.heading;
		rnb_updated = 0; //Note! This line must be included for things to work properly.
		
		
		//printf("\n\r%04x",last_good_rnb.id);
		//printf("\n\r%04x",last_good_rnb.id);
		//printf("\n\rRange-> %f,Bearing-> %f,Heading-> %f,ID-> %04x",
			//last_good_rnb.range), last_good_rnb.bearing*180/M_PI,
			//last_good_rnb.heading*180/M_PI, last_good_rnb.id);
		printf("\n\rRange-> %u",last_good_rnb.range);
		printf("\n\rBearing-> %f",last_good_rnb.bearing*180/M_PI);
		printf("\n\rHeading-> %f",last_good_rnb.heading*180/M_PI);
		printf("\n\rID-> %04x",last_good_rnb.id);
		//useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		//rnb_updated=0;
	}
	

}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	for(uint8_t l=0;l<msg_struct->length;l++){
		printf("%c", msg_struct->msg[l]);
	}
	printf("%s",msg_struct->msg);
	//int l = 0;
	while(*(msg_struct->msg)!=NULL){
		printf("%c", *(msg_struct->msg));
		msg_struct->msg += 1;
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