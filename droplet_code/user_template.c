//#include "user_template.h"
//
//uint8_t getFixedIndex(uint16_t id){
	//switch(id){
		//case 0xA0D8: return 0;
		//case 0xCBAB: return 1;
		//case 0xDF64: return 2;
		//case 0xBC63: return 3;
		//case 0xC32D: return 4;
		//case 0xB561: return 5;
		//case 0x896F: return 6;
		//case 0xDD21: return 7;
	//}
	//printf("ERROR: Unexpected RNB ID: %04X\r\n",id);
	////set_rgb(255,0,255);
	////delay_ms(2000);
	//return 255;
//}
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init()
//{
	//set_rgb(30,0,35);
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop()
//{
	//uint32_t time_floor = (get_time()/300);
	//if(((time_floor+6*getFixedIndex(get_droplet_id()))%50)==0){
		//set_rgb(30,0,35);
		//broadcast_rnb_data();
	//}else{
		//delay_ms(300);	
	//}
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct)
//{
//
//}
//
/////*
 ////*	The function below is optional - commenting it in can be useful for debugging if you want to query
 ////*	user variables over a serial connection.
 ////*/
////uint8_t user_handle_command(char* command_word, char* command_args)
////{
	////return 0;
////}
//
/////*
 ////*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 ////*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 ////*/	
////void	user_leg_status_interrupt()
////{
	////
////}
//
