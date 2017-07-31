#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	#ifndef SYNCHRONIZED
	printf("Error! It doesn't make sense to do TDMA without synchronizing the Droplets!\r\n");
	#endif
	loopID = 0xFFFF;
	frameCount = 0;
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	frameStart = get_time();
	acksSincePing=0;
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
		if(loopID==mySlot){
			sendPingMsg();
		}else if(loopID==(SLOTS_PER_FRAME-1)){
			//compute new state based on messages.
		}
		
	}
	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
	}
	delay_ms(LOOP_DELAY_MS);
}


void sendPingMsg(){
	PingMsg msg;
	msg.flag = PING_MSG_FLAG;
	for(uint8_t i=0;i<FILLER_LENGTH;i++){
		msg.filler[i] = 0xAB;
	}
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PingMsg));
	printf("Acks Since Last Ping: %hu\r\n", acksSincePing);
	acksSincePing=0;
}

void handlePingMsg(PingMsg* msg, id_t src){
	sendAckMsg(src);
}

void sendAckMsg(id_t src){
	AckMsg msg;
	msg.flag = ACK_MSG_FLAG;
	ir_targeted_send(ALL_DIRS, (char*)(&msg), sizeof(AckMsg), src);
}

void handleAckMsg(AckMsg* msg){
	if(msg->src == get_droplet_id()){
		acksSincePing++;
	}
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	if(msg_struct->length == sizeof(PingMsg)){
		PingMsg* msg = (PingMsg*)(msg_struct->msg);
		if(msg->flag == PING_MSG_FLAG){
			handlePingMsg(msg, msg_struct->sender_ID);
		}
	}else if(msg_struct->length == sizeof(AckMsg)){
	AckMsg* msg = (AckMsg*)(msg_struct->msg);
	if(msg->flag == ACK_MSG_FLAG){
		handleAckMsg(msg);
	}
}
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. It should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}
