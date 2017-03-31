//#include "user_template.h"
//
///*
 //* any code in this function will be run once, when the robot starts.
 //*/
//void init(){
//
//}
//
///*
 //* the code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop(){
//
//}
//
//#define BOT_POS_MSG_FLAG		'P'
//typedef union flex_byte_union{
	//uint16_t u;
	//int16_t d;
//}FlexByte;
//
//typedef FlexByte DensePosCovar[6];
//
//typedef struct bot_pos_struct{
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
 //* after each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct){
	//if(((BotPosMsg*)(msg_struct->msg))->flag==BOT_POS_MSG_FLAG && msg_struct->length==sizeof(BotPosMsg)){
		//BotPosMsg* msg = (BotPosMsg*)(msg_struct->msg);
		//printf("\t%04X @ {%4d, %4d, % 4d}\r\n", msg_struct->sender_ID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
	//}else{
		//printf("Got message from %04X: ", msg_struct->sender_ID);
		//uint8_t isText = 1;
		//for(uint8_t i=0;i<(msg_struct->length);i++){
			//char c = msg_struct->msg[i];
			//if((' ' > c ) || (c > '~')){
				//isText = 0;
				//break;
			//}
		//}
		//for(uint8_t i=0;i<(msg_struct->length);i++){
			//if(isText){
				//printf("%c", msg_struct->msg[i]);	
			//}else{
				//printf("%02hx ", msg_struct->msg[i]);
			//}
		//}
		//printf("\r\n");
	//}
//}
//
/////*
 ////*	the function below is optional - commenting it in can be useful for debugging if you want to query
 ////*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 ////*  '0' otherwise.
 ////*/
////uint8_t user_handle_command(char* command_word, char* command_args){
	////return 0;
////}
//
