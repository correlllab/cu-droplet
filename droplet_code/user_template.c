//#include "user_template.h"
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init(){
//
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop(){
//
//}
//
//typedef struct bot_pos_struct
//{
	//int16_t x;
	//int16_t y;
	//int16_t o;
//} BotPos;
//
//typedef struct bot_pos_msg_struct{
	//BotPos pos; //6 bytes
	//char flag;
//}BotPosMsg;
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct){
	//if(((BotPosMsg*)(msg_struct->msg))->flag=='P' && msg_struct->length==sizeof(BotPosMsg)){
		//BotPosMsg* msg = (BotPosMsg*)(msg_struct->msg);
		//printf("%04X @ {%4d, %4d, % 4d}\r\n", msg_struct->sender_ID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
	//}else{
		//printf_P(PSTR("%hu byte msg from %04X:\r\n\t"), msg_struct->length, msg_struct->sender_ID);
		//for(uint8_t i=0;i<msg_struct->length;i++){
			//printf("%02hX ", msg_struct->msg[i]);
		//}
		//printf("\r\n");
	//}
//}
//
/////*
 ////*	The function below is optional - commenting it in can be useful for debugging if you want to query
 ////*	user variables over a serial connection. It should return '1' if command_word was a valid command,
 ////*  '0' otherwise.
 ////*/
////uint8_t user_handle_command(char* command_word, char* command_args){
	////return 0;
////}
//
