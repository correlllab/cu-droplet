#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	acksSincePing=0;
	pingMsgPending = 0;
	lastPingMsgSent = get_time() + (rand_short()>>1); //uniformly random number between 0 and ~32 seconds.
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(!pingMsgPending){
		if((get_time()-lastPingMsgSent) > PING_MSG_PERIOD){
			pingMsgPending = 1;
			prepPingMsg();
		}
	}
	delay_ms(LOOP_DELAY_MS);
}

void handlePingMsg(PingMsg* msg, id_t src){
	prepAckMsg(src);
}

void handleAckMsg(AckMsg* msg){
	acksSincePing++;
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
	}
	if(msg_struct->length == sizeof(AckMsg)){
		AckMsg* msg = (AckMsg*)(msg_struct->msg);
		if(msg->flag == ACK_MSG_FLAG){
			handleAckMsg(msg);
		}
	}
}

uint32_t getBackoffTime(uint8_t N){
	uint8_t randMax = (1<<N) - 1;
	return (uint32_t)(rand_real()*randMax*16);
}

void prepPingMsg(){
	PingMsgNode* node = (PingMsgNode*)myMalloc(sizeof(PingMsgNode));
	node->numTries = 0;
	for(uint8_t i=0;i<9;i++){
		node->msg.filler[i] = rand_norm(0,1);
	}
	node->msg.flag = PING_MSG_FLAG;
	sendPingMsg(node);
}

void sendPingMsg(PingMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		if(node->numTries>7){
			//printf("Giving up on ping.\r\n");
			myFree(node);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				lastPingMsgSent=get_time();
				pingMsgPending=0;
			}
		}else{
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendPingMsg, node);
		}
	}else{
		ir_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(PingMsg));
		//printf("Ping successful after %hu tries.\r\n", node->numTries);
		uint8_t numAcksReceived;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			numAcksReceived = acksSincePing;
			pingMsgPending = 0;
			lastPingMsgSent=get_time();
			acksSincePing=0;
		}
		printf("Acks Since Last Ping: %hu\r\n", numAcksReceived);
		myFree(node);
	}
}


//Takes all of the information needed for a botMeasMsg and packs it up in to a BotMeasMsgNode,
//then calls sendBotMeasMsg with it.
void prepAckMsg(id_t id){
	AckMsgNode* node = (AckMsgNode*)myMalloc(sizeof(AckMsgNode));
	node->numTries = 0;
	node->tgt = id;
	for(uint8_t i=0;i<9;i++){
		node->msg.filler[i] = rand_norm(0,1);
	}
	node->msg.flag = ACK_MSG_FLAG;
	sendAckMsg(node);
}

//Sends an AckMsg using a poor man's CSMA protocol.
void sendAckMsg(AckMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		if(node->numTries>7){
			//printf("Giving up on ack for %04X.\r\n", node->tgt);
			myFree(node);
		}else{
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendAckMsg, node);
		}
	}else{
		ir_targeted_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(AckMsg), node->tgt);
		//printf("Ack for %04X successful after %hu tries.\r\n", node->tgt, node->numTries);
		myFree(node);
	}
}