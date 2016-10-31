#include "user_template.h"

///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init(){
	//loopID = 0xFFFF;
	//frameCount = 0;
	//mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	//frameStart = get_time();
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop(){
	//uint32_t frameTime = get_time()-frameStart;
	//if(frameTime>FRAME_LENGTH_MS){
		//frameTime = frameTime - FRAME_LENGTH_MS;
		//frameStart += FRAME_LENGTH_MS;
		//frameCount++;
	//}
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct){
	////if(((NearBotsMsg*)(msg_struct->msg))->flag==NEAR_BOTS_MSG_FLAG){
		////handleNearBotsMsg((NearBotsMsg*)(msg_struct->msg), msg_struct->sender_ID);
	////}else{
		////printf_P(PSTR("%hu byte msg from %04X:\r\n\t"), msg_struct->length, msg_struct->sender_ID);
		////for(uint8_t i=0;i<msg_struct->length;i++){
			////printf("%02hX ", msg_struct->msg[i]);
		////}
		////printf("\r\n");
	////}
//}
