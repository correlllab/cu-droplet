#include "user_template.h"
#include <stdbool.h>

typedef struct gol_cell_struct
{
	id_t ID;			// ID of the Droplet representing this cell
	bool neighb_state_received;
	uint8_t state;		// Current state of the cell: 0 = dead, 1 = alive, 2 = unknown
	uint8_t	next_state;	// Next state of the cell: 0 = dead, 1 = alive, 2 = unknown
	bool next_state_calculated;		// Indicate that the cell calculated its next state.
	bool next_state_passed;		// Indicate that the cell passed to the next state.
	bool allow_next_step; // Indicate that you are ready to pass to the next step.
	uint16_t current_state; // The current state the cell is at.
}gol_cell;

gol_cell gol_me;
gol_cell gol_N;
gol_cell gol_NE;
gol_cell gol_E;
gol_cell gol_SE;
gol_cell gol_S;
gol_cell gol_SW;
gol_cell gol_W;
gol_cell gol_NW;
gol_cell gol_neighbors[8];

void calculate_next_state(void){
	
	uint16_t number_neighbors_alive = 0;
	
	for (int i=0; i<8; i++)
	{
		number_neighbors_alive += gol_neighbors[i].state;
	}
	
	if ( gol_me.state == 1)
	{
		if (number_neighbors_alive == 2 || number_neighbors_alive == 3)
		{
			gol_me.next_state = 1;
		}
		else
		{
			gol_me.next_state = 0;
		}
	}
	
	else
	{
		if (number_neighbors_alive == 3)
		{
			gol_me.next_state = 1;
		}
		else
		{
			gol_me.next_state = 0;
		}
	}
	
	gol_me.next_state_calculated = true;
	gol_me.next_state_passed = false;
}

void show_next_state_calculated(void){
	
	if (gol_me.next_state == 1)
	{
		set_rgb(0,255,255);
	}
	else
	{
		set_rgb(255,255,0);
	}
}

void pass_next_state(void){
	gol_me.state = gol_me.next_state;
	gol_me.next_state = 2;
	gol_me.allow_next_step = false;
	
	if (gol_me.state == 1)
	{
		set_rgb(0,0,255);
	}
	else
	{
		set_rgb(255,0,0);
	}
	
	gol_me.next_state_passed = true;
}

void allow_step(void){
	
	gol_me.allow_next_step = true;
	gol_me.next_state_calculated = false;
}

void reset_gol_neighbors(void)
{

	for (int i=0; i<8; i++)
	{
		if (gol_neighbors[i].ID == 0)
		{
			gol_neighbors[i].state =  0;
			gol_neighbors[i].allow_next_step = true;
			gol_neighbors[i].next_state_passed = true;
			gol_neighbors[i].next_state_calculated = true;
		}
		else
		{
			gol_neighbors[i].state = 2;
			gol_neighbors[i].allow_next_step = false;
			gol_neighbors[i].next_state_passed = false;
			gol_neighbors[i].next_state_calculated = false;
			gol_neighbors[i].neighb_state_received = false;
		}
		
	}
	
}

void init_gol_neighbors(void)
{
	for (int i=0; i<8; i++)
	{
		gol_neighbors[i].ID = neighbors[i].ID;
		gol_neighbors[i].neighb_state_received = false;
	}
	
	reset_gol_neighbors();
}

void init_gol_me(void){
	gol_me.ID = get_droplet_id();
	gol_me.state = 0;
	gol_me.next_state = 2;
	gol_me.next_state_calculated = false;
	gol_me.next_state_passed = false;
	gol_me.allow_next_step = true;
	gol_me.current_state = 0;
}

void set_gol_blinker(void){
	gol_me.state = 0;
	set_rgb(255,255,0);
	if (myPos.x >= dist-margin && myPos.x <= dist+margin)
	{
		gol_me.state = 1;
		set_rgb(0,255,255);
	}
}

void set_gol_beacon(void){
	gol_me.state = 0;
	set_rgb(255,255,0);
	if (myPos.x >= 2*dist-margin && myPos.y >= -dist-margin)
	{
		gol_me.state = 1;
		set_rgb(0,255,255);
	}
	if (myPos.x <= dist+margin && myPos.y <= -2*dist+margin)
	{
		gol_me.state = 1;
		set_rgb(0,255,255);
	}
}

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	set_rgb(255,255,255);
	acksSincePing=0;
	pingMsgPending = 0;
	lastPingMsgSent = get_time() + (rand_short()>>1); //uniformly random number between 0 and ~32 seconds.
	recsSinceMess=0;
	messMsgPending = 0;
	lastMessMsgSent = get_time() + (rand_short()>>1); //uniformly random number between 0 and ~32 seconds.
	rec = 0;
	dropletListened = 0;
	printf("ID %04X",get_droplet_id());
	gol_me.allow_next_step = false;
	initialize_grid(3,3);
	init_gol_neighbors();
	init_gol_me();
	set_gol_blinker();
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
	
	if (gol_me.current_state < 1)
	{
		sendPosMsg();
		delay_ms(500);
	}
	
	if (gol_neighbors[0].state != 2 && gol_neighbors[1].state != 2 && gol_neighbors[2].state != 2 && gol_neighbors[3].state != 2 && gol_neighbors[4].state != 2 && gol_neighbors[5].state != 2 && gol_neighbors[6].state != 2 && gol_neighbors[7].state != 2)
	{
		if (gol_neighbors[0].allow_next_step == true && gol_neighbors[1].allow_next_step == true && gol_neighbors[2].allow_next_step == true && gol_neighbors[3].allow_next_step == true && gol_neighbors[4].allow_next_step == true && gol_neighbors[5].allow_next_step == true && gol_neighbors[6].allow_next_step == true && gol_neighbors[7].allow_next_step == true)
		{
			if (gol_me.next_state_calculated == false)
			{
				calculate_next_state();
				show_next_state_calculated();
				reset_gol_neighbors();
				gol_me.current_state += 1;
				printf("Current state is %u , Current time is %lu ms\r\n",gol_me.current_state,get_time());
			}
			
		}
		
	}
	
	if (gol_neighbors[0].next_state_calculated == true && gol_neighbors[1].next_state_calculated == true && gol_neighbors[2].next_state_calculated == true && gol_neighbors[3].next_state_calculated == true && gol_neighbors[4].next_state_calculated == true && gol_neighbors[5].next_state_calculated == true && gol_neighbors[6].next_state_calculated == true && gol_neighbors[7].next_state_calculated == true)
	{
		if (gol_me.next_state_passed == false)
		{
			pass_next_state();
			reset_gol_neighbors();
			gol_me.current_state += 1;
			printf("Current state is %u , Current time is %lu ms\r\n",gol_me.current_state,get_time());
		}
		
	}
	
	if (gol_neighbors[0].next_state_passed == true && gol_neighbors[1].next_state_passed == true && gol_neighbors[2].next_state_passed == true && gol_neighbors[3].next_state_passed == true && gol_neighbors[4].next_state_passed == true && gol_neighbors[5].next_state_passed == true && gol_neighbors[6].next_state_passed == true && gol_neighbors[7].next_state_passed == true)
	{
		if (gol_me.allow_next_step == false)
		{
			allow_step();
			reset_gol_neighbors();
			gol_me.current_state += 1;
			set_rgb(255,255,255);
			printf("Current state is %u , Current time is %lu ms\r\n",gol_me.current_state,get_time());
		}
		
	}
	
	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
		useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	
	delay_ms(LOOP_DELAY_MS);
}

void handlePingMsg(PingMsg* msg, id_t src){

	if (dropletListened == src || dropletListened == 0)
	{
		for(int i=0; i<8; i++)
		{
			if (src == gol_neighbors[i].ID)
			{
				if (gol_neighbors[i].neighb_state_received == false)
				{
					printf("Ping received \r\n");
					dropletListened = src;
					prepAckMsg(src);
				}
			}
		}
	}
	
	
}

void handleAckMsg(AckMsg* msg, id_t src){
	printf("Ack received \r\n");
	acksSincePing++;
	messMsgPending = 1;
	prepMessMsg(src);
}

void handleMessMsg(MessMsg* msg, id_t src){
	printf("Mess received \r\n");
	
	if ( (msg->current_state == gol_me.current_state) || (msg->current_state == (gol_me.current_state + 1) ) )
	{
		for(int i=0; i<8; i++)
		{
			if (src == gol_neighbors[i].ID)
			{
				gol_neighbors[i].neighb_state_received = true;
				
				if (gol_me.next_state_calculated == false)
				{
					
					if (msg->state == 1  && gol_neighbors[i].state == 2)
					{
						gol_neighbors[i].state = 1;
						//printf("Neighbor %u is alive \n \r", neighbors[i].ID);
					}
					
					if (msg->state == 0 && gol_neighbors[i].state == 2)
					{
						gol_neighbors[i].state = 0;
						//printf("Neighbor %u is dead \n \r", neighbors[i].ID);
					}
					
					if (msg->allow_next_step == 1 && gol_neighbors[i].allow_next_step == false)
					{
						gol_neighbors[i].allow_next_step = true;
						//printf("Neighbor %u allowed next step \n \r", neighbors[i].ID);
					}
				}
				
				if (gol_me.next_state_passed == false)
				{
					
					if (msg->next_state_calculated == 1 && gol_neighbors[i].next_state_calculated == false)
					{
						gol_neighbors[i].next_state_calculated = true;
						//printf("Neighbor %u calculated next state \n \r", neighbors[i].ID);
					}
					
				}
				
				if (gol_me.allow_next_step == false)
				{
					
					if (msg->next_state_passed == 1 && gol_neighbors[i].next_state_passed == false)
					{
						gol_neighbors[i].next_state_passed = true;
						//printf("Neighbor %u passed next state \n \r", neighbors[i].ID);
					}
					
				}
				
			}
		}
	}
	
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
	//for(uint8_t i=0;i<9;i++){
	//	node->msg.filler[i] = rand_norm(0,1);
	//}
	node->msg.flag = PING_MSG_FLAG;
	sendPingMsg(node);
}

void sendPingMsg(PingMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		/*if(node->numTries>7){
			printf("Giving up on ping.\r\n");
			myFree(node);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				lastPingMsgSent=get_time();
				pingMsgPending=0;
			}
		}else{*/
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendPingMsg, node);
		//}
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
	//for(uint8_t i=0;i<9;i++){
	//	node->msg.filler[i] = rand_norm(0,1);
	//}
	node->msg.flag = ACK_MSG_FLAG;
	sendAckMsg(node);
}

//Sends an AckMsg using a poor man's CSMA protocol.
void sendAckMsg(AckMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		/*if(node->numTries>7){
			printf("Giving up on ack for %04X.\r\n", node->tgt);
			myFree(node);
		}else{*/
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendAckMsg, node);
		//}
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
	/*for(uint8_t i=0;i<9;i++){
		node->msg.filler[i] = rand_norm(0,1);
	}*/
	
	node->msg.state = gol_me.state;
	node->msg.current_state = gol_me.current_state;
	
	if (gol_me.next_state_calculated == true)
	{
		node->msg.next_state_calculated = 1;
		//printf("sending next state calculated \r \n");
	}
	else
	{
		node->msg.next_state_calculated = 0;
	}
	
	
	if (gol_me.next_state_passed == true)
	{
		node->msg.next_state_passed = 1;
		//printf("sending next state passed \r \n");
	}
	else
	{
		node->msg.next_state_passed = 0;
	}
	
	if (gol_me.allow_next_step == true)
	{
		node->msg.allow_next_step = 1;
		//printf("sending next state allowed \r \n");
	}
	else
	{
		node->msg.allow_next_step = 0;
	}
	
	node->msg.flag = MESS_MSG_FLAG;
	sendMessMsg(node);
}

void sendMessMsg(MessMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		/*if(node->numTries>7){
			printf("Giving up on mess.\r\n");
			myFree(node);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				lastMessMsgSent=get_time();
				messMsgPending=0;
			}
			}else{*/
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendMessMsg, node);
		//}
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
	//for(uint8_t i=0;i<9;i++){
	//	node->msg.filler[i] = rand_norm(0,1);
	//}
	node->msg.flag = REC_MSG_FLAG;
	sendRecMsg(node);
}

//Sends an AckMsg using a poor man's CSMA protocol.
void sendRecMsg(RecMsgNode* node){
	if(ir_is_busy(ALL_DIRS)){
		(node->numTries)++;
		/*if(node->numTries>7){
			printf("Giving up on rec for %04X.\r\n", node->tgt);
			myFree(node);
			}else{*/
			schedule_task(getBackoffTime(node->numTries), (arg_func_t)sendRecMsg, node);
		//}
		}else{
		ir_targeted_send(ALL_DIRS, (char*)(&(node->msg)), sizeof(RecMsg), node->tgt);
		printf("Rec for %04X successful after %hu tries.\r\n", node->tgt, node->numTries);
		dropletListened = 0;
		myFree(node);
	}
}