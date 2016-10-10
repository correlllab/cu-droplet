//#include "user_template.h"
//
//uint32_t	frameCount;
//uint32_t	frameStart;
//uint16_t	loopID;
//uint16_t	mySlot;
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
//void handleNearBotsMsg(NearBotsMsg* msg, id_t senderID){
	////printf("%04X", senderID);
	////if(msg->x!=UNDF && msg->y!=UNDF) printf("\t%4d\t%4d\t%hd\r\n", msg->x, msg->y, msg->posConf);
	////else printf("\t -- \t -- \t -- \r\n");
	////id_t id;
	////uint16_t range;
	////int16_t bearing, heading;
	////int8_t conf;
	////for(uint8_t i=0;i<NUM_SHARED_BOTS;i++){
		////id = msg->shared[i].id;
		////if(id == 0) continue;
		////range = unpackRange(msg->shared[i].range);
		////bearing = unpackAngleMeas(msg->shared[i].b);
		////heading = unpackAngleMeas(msg->shared[i].h);
		////conf = msg->shared[i].conf;
		////printf("\t%04X\t%4u\t%4d\t%4d\t%4hd\r\n", id, range, bearing, heading, conf);
	////}
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