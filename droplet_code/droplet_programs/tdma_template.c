#include "droplet_programs/tdma_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	#ifndef SYNCHRONIZED
		printf("Error! It doesn't make sense to do TDMA without synchronizing the Droplets!\r\n");
	#endif
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == mySlot){
			//Code here will happen once per frame. Each Droplet has its own slot. 
			//This is where you would put message broadcasts and rnb_broadcasts.
		}else if(loopID == (SLOTS_PER_FRAME-1)){
			//Code here will happen once per frame: after every Droplet has had a slot.
		}
		//Code here will happen once per slot: every SLOT_LENGTH_MS
	}
	//Code here will happen every single loop: once every LOOP_DELAY_MS
	delay_ms(LOOP_DELAY_MS);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){

}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//return 0;
//}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}