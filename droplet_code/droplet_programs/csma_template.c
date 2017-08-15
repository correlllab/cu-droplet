#include "csma_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	acksSincePing=0;
	pingMsgPending = 0;
	lastPingMsgSent = get_time() + (rand_short()>>1); //uniformly random number between 0 and ~32 seconds.
	recsSinceMess=0;
	messMsgPending = 0;
	lastMessMsgSent = get_time() + (rand_short()>>1); //uniformly random number between 0 and ~32 seconds.
	rec = 0;
	dropletListened = 0;
	
	// Add your init steps here
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	if(!pingMsgPending && !messMsgPending){
		if((get_time()-lastPingMsgSent) > PING_MSG_PERIOD){
			pingMsgPending = 1;
			prepPingMsg();
		}
	}
	
	// Add your loop steps here
		
	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
		rnb_updated=0;
	}
	
	delay_ms(LOOP_DELAY_MS);
}

void handlePingMsg(PingMsg* msg, id_t src){

	if (dropletListened == src || dropletListened == 0)
	{
		// You can optionnaly add conditions here to sort the messages you want to hear.
		printf("Ping received \r\n");
		dropletListened = src;
		prepAckMsg(src);
	}
	
	
}

void handleAckMsg(AckMsg* msg, id_t src){
	printf("Ack received \r\n");
	acksSincePing++;
	messMsgPending = 1;
	prepMessMsg(src);
}

void handleMessMsg(MessMsg* msg, id_t src){
	
	// Add here the treatment of your message.
	printf("Mess received \r\n");
	prepRecMsg(src);
}

void handleRecMsg(RecMsg* msg, id_t src){
	printf("Rec received \r\n");
	recsSinceMess++;
	rec = src;
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
			handleAckMsg(msg, msg_struct->sender_ID);
		}
	}
	if(msg_struct->length == sizeof(MessMsg)){
		MessMsg* msg = (MessMsg*)(msg_struct->msg);
		if(msg->flag == MESS_MSG_FLAG){
			handleMessMsg(msg, msg_struct->sender_ID);
		}
	}
	if(msg_struct->length == sizeof(RecMsg)){
		RecMsg* msg = (RecMsg*)(msg_struct->msg);
		if(msg->flag == REC_MSG_FLAG){
			handleRecMsg(msg, msg_struct->sender_ID);
		}
	}
}

uint32_t getBackoffTime(uint8_t n){
	uint8_t randMax = (1<<n) - 1;
	return (uint32_t)(rand_real()*randMax*16);
}

void prepPingMsg(){
	PingMsgNode* node = (PingMsgNode*)myMalloc(sizeof(PingMsgNode));
	node->numTries = 0;
	node->msg.flag = PING_MSG_FLAG;
	sendPingMsg(node);
}

void sendPingMsg(PingMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendPingMsg, node);
	}else{
		ir_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(PingMsg));
		if (acksSincePing != 0)
		{
			printf("Ping successful after %hu tries.\r\n", node->numTries);
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
		else
		{
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendPingMsg, node);
		}
		
	}
}


//Takes all of the information needed for a botMeasMsg and packs it up in to a BotMeasMsgNode,
//then calls sendBotMeasMsg with it.
void prepAckMsg(id_t id){
	AckMsgNode* node = (AckMsgNode*)myMalloc(sizeof(AckMsgNode));
	node->numTries = 0;
	node->tgt = id;
	node->msg.flag = ACK_MSG_FLAG;
	sendAckMsg(node);
}

//Sends an AckMsg using a poor man's CSMA protocol.
void sendAckMsg(AckMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendAckMsg, node);
	}else{
		ir_targeted_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(AckMsg), node->tgt);
		printf("Ack for %04X successful after %hu tries.\r\n", node->tgt, node->numTries);
		myFree(node);
	}
}

void prepMessMsg(id_t id){
	MessMsgNode* node = (MessMsgNode*)myMalloc(sizeof(MessMsgNode));
	node->numTries = 0;
	node->tgt = id;
	// Add here how to fill your message struct
	node->msg.flag = MESS_MSG_FLAG;
	sendMessMsg(node);
}

void sendMessMsg(MessMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendMessMsg, node);
	}else{
			ir_targeted_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(MessMsg), node->tgt);
			if (recsSinceMess != 0 && rec == node->tgt)
			{
				printf("Mess successful after %hu tries.\r\n", node->numTries);
				uint8_t numRecsReceived;
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					numRecsReceived = recsSinceMess;
					messMsgPending = 0;
					lastMessMsgSent=get_time();
					recsSinceMess=0;
				}
				printf("Recs Since Last Mess: %hu\r\n", numRecsReceived);
				myFree(node);
			} 
			else
			{
				schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendMessMsg, node);
			}
		
	}
}


//Takes all of the information needed for a botMeasMsg and packs it up in to a BotMeasMsgNode,
//then calls sendBotMeasMsg with it.
void prepRecMsg(id_t id){
	RecMsgNode* node = (RecMsgNode*)myMalloc(sizeof(RecMsgNode));
	node->numTries = 0;
	node->tgt = id;
	node->msg.flag = REC_MSG_FLAG;
	sendRecMsg(node);
}

//Sends an AckMsg using a poor man's CSMA protocol.
void sendRecMsg(RecMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendRecMsg, node);
	}else{
		ir_targeted_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(RecMsg), node->tgt);
		printf("Rec for %04X successful after %hu tries.\r\n", node->tgt, node->numTries);
		dropletListened = 0;
		myFree(node);
	}
}