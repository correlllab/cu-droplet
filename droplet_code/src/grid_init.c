#include "grid_init.h"

uint8_t init_finished;
uint8_t m;
uint8_t n;
id_t undf_id = 0xFFFF;
uint8_t state;
uint32_t start_time;
uint8_t nb_update;

cell me;
cell N;
cell NE;
cell E;
cell SE;
cell S;
cell SW;
cell W;
cell NW;
cell neighbors[8];



typedef struct pos_msg_struct{
	int16_t posx;
	int16_t posy;
} PosMsg;

void sendPosMsg(void){
	PosMsg msg;

	msg.posx = myPos.x;
	msg.posy = myPos.y;
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PosMsg));
}

void reset_neighbors(void){
	me.ID = get_droplet_id();
	N.ID = undf_id;
	NE.ID = undf_id;
	E.ID = undf_id;
	SE.ID = undf_id;
	S.ID = undf_id;
	SW.ID = undf_id;
	W.ID = undf_id;
	NW.ID = undf_id;
}

void update_neighbors(void){
	neighbors[0] = N;
	neighbors[1] = NE;
	neighbors[2] = E;
	neighbors[3] = SE;
	neighbors[4] = S;
	neighbors[5] = SW;
	neighbors[6] = W;
	neighbors[7] = NW;
}

void count_neighbors_expected(void){
	// Knowing how many neighbors to expect (am I a corner, border or center ?)
	
	reset_neighbors();
	
	if(POS_DEFINED(&myPos)){
		if (myPos.y >= 0-margin && myPos.y <= 0+margin)
		{
			N.ID = 0;
			NW.ID = 0;
			NE.ID = 0;
		}
		
		if (myPos.y >= -(n-1)*dist-margin && myPos.y <= -(n-1)*dist+margin)
		{
			S.ID = 0;
			SW.ID = 0;
			SE.ID = 0;
		}
		
		if (myPos.x >= 0-margin && myPos.x <= 0+margin)
		{
			W.ID = 0;
			NW.ID = 0;
			SW.ID = 0;
		}
		
		if (myPos.x >= (m-1)*dist-margin && myPos.x <= (m-1)*dist+margin)
		{
			E.ID = 0;
			NE.ID = 0;
			SE.ID = 0;
		}
	}
	
	update_neighbors();
}

void show_neighbors_expected(void){
	// Show if I am a corner, border or center
	uint8_t blank_neighbors = 0;
	for (int i=0; i<8; i++)
	{
		if (neighbors[i].ID == 0)
		{
			blank_neighbors += 1;
		}
	}
	
	if (blank_neighbors == 5)
	{
		set_rgb(255,0,0);
	}
	
	if (blank_neighbors == 3)
	{
		set_rgb(255,0,255);
	}
	
	if (blank_neighbors == 0)
	{
		set_rgb(0,0,255);
	}

}

void show_neighbors_filled(void){
	uint8_t neighbors_filled = 0;
	for (int i=0; i<8; i++)
	{
		if (neighbors[i].ID != undf_id)
		{
			neighbors_filled += 1;
		}
	}
	
	if (neighbors_filled == 8)
	{
		set_green_led(255);
		state = 2;
	}
	
}

void print_neighbors(void){
	
	printf("My ID is %04X \r\n", me.ID);
	printf("N ID is %04X \r\n", N.ID);
	printf("NE ID is %04X \r\n", NE.ID);
	printf("E ID is %04X \r\n", E.ID);
	printf("SE ID is %04X \r\n", SE.ID);
	printf("S ID is %04X \r\n", S.ID);
	printf("SW ID is %04X \r\n", SW.ID);
	printf("W ID is %04X \r\n", W.ID);
	printf("NW ID is %04X \r\n", NW.ID);
}


void grid_init(void){
	#ifndef SYNCHRONIZED
	printf("Error! It doesn't make sense to do TDMA without synchronizing the Droplets!\r\n");
	#endif
	grid_loopID = 0xFFFF;
	grid_frameCount = 0;
	grid_mySlot = (get_droplet_id()%(GRID_SLOTS_PER_FRAME-1));
	grid_frameStart = get_time();
	
	dist = 50;
	margin = 20;
	nb_update = 0;
	reset_neighbors();
	
	update_neighbors();
	
	state = 0;
	start_time = get_time();
	set_rgb(255,255,255);
}

void grid_loop(void){
	uint32_t grid_frameTime = get_time()-grid_frameStart;
	if(grid_frameTime>GRID_FRAME_LENGTH_MS){
		grid_frameTime = grid_frameTime - GRID_FRAME_LENGTH_MS;
		grid_frameStart += GRID_FRAME_LENGTH_MS;
		grid_frameCount++;
	}
	if(grid_loopID!=(grid_frameTime/GRID_SLOT_LENGTH_MS)){
		grid_loopID = grid_frameTime/GRID_SLOT_LENGTH_MS;
		if(grid_loopID==grid_mySlot){
			
			if (state == 0)
			{
				broadcast_rnb_data();
			}
			
			if (state >= 1)
			{
				if (POS_DEFINED(&myPos))
				{
					sendPosMsg();
				}
			}
			
		}else if(grid_loopID==(GRID_SLOTS_PER_FRAME-1)){
			//compute new state based on messages.
			
			if (state == 0)
			{
				count_neighbors_expected();
				show_neighbors_expected();
			}
			
			if (state == 1)
			{
				show_neighbors_filled();
			}
			
			if (state == 2)
			{
				print_neighbors();
				init_finished = 1;
								
			}
			
			printf("state is %hu \r\n", state);
			
		}
	
	}
	
	if (state == 0)
	{
		if (get_time()-start_time > 90000)
		{
			printf("time > 1mn30 \r\n");
			printf("changing to state = 1 \r\n");
			state = 1;
			start_time = get_time();
		}
	}
	
	if (state == 1)
	{
		if (get_time()-start_time > 90000)
		{
			printf("time > 1mn30 \r\n");
			printf("changing to state = 0 \r\n");
			state = 0;
			start_time = get_time();
		}
	}
	
	
	if(rnb_updated){
	//Handle rnb data in last_good_rnb struct.
	
		//if (nb_update > 10 || seedFlag == 1)
		//{
			useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		//}
	
		rnb_updated=0;
	
	}
	
	delay_ms(GRID_LOOP_DELAY_MS);
}

void grid_handle_msg(ir_msg* msg_struct){
	
	if (state == 0)
	{
		if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
			handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
			nb_update += 1;
		}
	}
	
	if (state == 1)
	{
		if(msg_struct->length == sizeof(PosMsg))
		{
			PosMsg* posMsg = (PosMsg*)(msg_struct->msg);
			
			for (int i=0; i<8; i++)
			{
				if (neighbors[i].ID == undf_id)
				{
					if (posMsg->posx >= myPos.x-dist-margin && posMsg->posx <= myPos.x-dist+margin)
					{
						if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin )
						{
							if (i == 5)
							{
								SW.ID = msg_struct->sender_ID;
								printf("SW neighbor filled \r\n");
							}
							
						}
				
						if (posMsg->posy >= myPos.y-margin && posMsg->posy <= myPos.y+margin)
						{
							if (i == 6)
							{
								W.ID = msg_struct->sender_ID;
								printf("W neighbor filled \r\n");
							}
							
						}
				
						if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
						{
							if (i == 7)
							{
								NW.ID = msg_struct->sender_ID;
								printf("NW neighbor filled \r\n");
							}
							
						}
					}
			
					if (posMsg->posx >= myPos.x+dist-margin && posMsg->posx <= myPos.x+dist+margin)
					{
						if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin)
						{
							if (i == 3)
							{
								SE.ID = msg_struct->sender_ID;
								printf("SE neighbor filled \r\n");
							}
							
						}
				
						if (posMsg->posy >= myPos.y-margin && posMsg->posy <= myPos.y+margin )
						{
							if (i == 2)
							{
								E.ID = msg_struct->sender_ID;
								printf("E neighbor filled \r\n");
							}
							
						}
				
						if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
						{
							if (i == 1)
							{
								NE.ID = msg_struct->sender_ID;
								printf("NE neighbor filled \r\n");
							}
							
						}
					}
			
					if (posMsg->posx >= myPos.x-margin && posMsg->posx <= myPos.x+margin )
					{
						if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin)
						{
							if (i == 4)
							{
								S.ID = msg_struct->sender_ID;
								printf("S neighbor filled \r\n");
							}
							
						}
				
						if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
						{
							if (i == 0)
							{
								N.ID = msg_struct->sender_ID;
								printf("N neighbor filled \r\n");
							}
							
						}
					}
				}
			
				update_neighbors();
			}
			
			
			
			
		}
	}
	
}

void grid_check_messages(void){
	ir_msg* msg_struct;
	char actual_struct[sizeof(ir_msg)]; //It's like malloc, but on the stack.
	char actual_msg[IR_BUFFER_SIZE+1];
	msg_struct = (ir_msg*)actual_struct;
	msg_struct->msg = actual_msg;
	uint8_t i;
	
	if(user_facing_messages_ovf){
		num_waiting_msgs=MAX_USER_FACING_MESSAGES;
		user_facing_messages_ovf=0;
		printf_P(PSTR("Error: Messages overflow. Too many messages received. Try speeding up your loop if you see this a lot.\r\n"));
	}
	//if(num_waiting_msgs>0) printf("num_msgs: %hu\r\n",num_waiting_msgs);
	while(num_waiting_msgs>0){
		i=num_waiting_msgs-1;
		//We don't want this block to be interrupted by perform_ir_upkeep because the
		//list of messages could get corrupted.
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if(msg_node[i].msg_length==0){
				printf_P(PSTR("ERROR: Message length 0 for msg_node.\r\n"));
			}
			memcpy(msg_struct->msg, (const void*)msg_node[i].msg, msg_node[i].msg_length);
			msg_struct->arrival_time					= msg_node[i].arrival_time;
			msg_struct->sender_ID						= msg_node[i].sender_ID;
			msg_struct->dir_received					= msg_node[i].arrival_dir;
			msg_struct->length							= msg_node[i].msg_length;
			msg_struct->wasTargeted						= msg_node[i].wasTargeted;
			num_waiting_msgs--;
		}
		msg_struct->msg[msg_node[i].msg_length]	= '\0';



		grid_handle_msg(msg_struct);
	}
}

void initialize_grid(uint8_t nb_columns, uint8_t nb_rows){
	
	m = nb_columns;
	n = nb_rows;
	init_finished = 0; 

	grid_init();
	while (init_finished == 0)
	{
		grid_loop();
		grid_check_messages();
		if(task_list_check()){
			printf_P(PSTR("Error! We got ahead of the task list and now nothing will execute.\r\n"));
			task_list_cleanup();
		}
		delay_ms(1);
	}
}