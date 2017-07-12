#include "user_template.h"

void set_gol_blinker(void){
	set_rgb(255,255,0);
	if (myPos.x >= dist-margin && myPos.x <= dist+margin)
	{
		set_rgb(0,255,255);
	}
}

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	//initialize_grid(3,3);
	//set_gol_blinker();
	set_rgb(0,255,0);
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	//sendPosMsg();
	//print_neighbors();
	//delay_ms(500);
	//printf(" %04X", neighbors[0].ID);

	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
		
		//useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
		
	}

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
