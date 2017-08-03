<<<<<<< HEAD
#include "user_template.h"

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){

}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){

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
=======
//#include "user_template.h"
//
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
>>>>>>> 93d85f6fd1bc752e0bb4daaf2bb4b06a35c2a48a
