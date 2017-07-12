#include "game_of_life.h"
#include <stdbool.h>


typedef struct gol_cell_struct
{
	id_t ID;			// ID of the Droplet representing this cell
	uint8_t state;		// Current state of the cell: 0 = dead, 1 = alive, 2 = unknown
	uint8_t	next_state;	// Next state of the cell: 0 = dead, 1 = alive, 2 = unknown
	bool next_state_calculated;		// Indicate that the cell calculated its next state.
	bool next_state_passed;		// Indicate that the cell passed to the next state.
	bool allow_next_step; // Indicate that you are ready to pass to the next step.
	uint8_t current_state; // The current state the cell is at.
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

typedef struct state_msg_struct{
	uint8_t state;
	uint8_t next_state_calculated;
	uint8_t next_state_passed;
	uint8_t allow_next_step;
	uint8_t current_state;
} StateMsg;

void sendStateMsg(void){
	StateMsg msg;

	msg.state = gol_me.state;
	msg.current_state = gol_me.current_state;
	
	if (gol_me.next_state_calculated == true)
	{
		msg.next_state_calculated = 1;
		//printf("sending next state calculated \r \n");
	}
	else
	{
		msg.next_state_calculated = 0;
	}
	
	
	if (gol_me.next_state_passed == true)
	{
		msg.next_state_passed = 1;
		//printf("sending next state passed \r \n");
	}
	else
	{
		msg.next_state_passed = 0;
	}
	
	if (gol_me.allow_next_step == true)
	{
		msg.allow_next_step = 1;
		//printf("sending next state allowed \r \n");
	}
	else
	{
		msg.allow_next_step = 0;
	}
	
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(StateMsg));
}

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
		}
		
	}
	
}

void init_gol_neighbors(void)
{
	for (int i=0; i<8; i++)
	{
		gol_neighbors[i].ID = neighbors[i].ID;
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
	#ifndef SYNCHRONIZED
	printf("Error! It doesn't make sense to do TDMA without synchronizing the Droplets!\r\n");
	#endif
	loopID = 0xFFFF;
	frameCount = 0;
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	frameStart = get_time();
	
	gol_me.allow_next_step = false;
	initialize_grid(4,4);
	init_gol_neighbors();
	init_gol_me();
	set_gol_beacon();
	//set_rgb(0,255,0);
	
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
			sendStateMsg();
			
			if (gol_me.current_state < 1)
			{
				delay_ms(500);
				sendPosMsg();
			}
			
		}else if(loopID==(SLOTS_PER_FRAME-1)){
			//compute new state based on messages.
			//print_neighbors();
			
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
				}
				
			}
			
		}
		
	}
	
	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
		useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	
	delay_ms(LOOP_DELAY_MS);
}
/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){

if(msg_struct->length == sizeof(StateMsg))
{
	StateMsg* stateMsg = (StateMsg*)(msg_struct->msg);
	
	if ( (stateMsg->current_state == gol_me.current_state) || (stateMsg->current_state == (gol_me.current_state + 1) ) )
	{
		for(int i=0; i<8; i++)
		{
			if (msg_struct->sender_ID == gol_neighbors[i].ID)
			{
				if (gol_me.next_state_calculated == false)
				{
					
					if (stateMsg->state == 1  && gol_neighbors[i].state == 2)
					{
						gol_neighbors[i].state = 1;
						//printf("Neighbor %u is alive \n \r", neighbors[i].ID);
					}
					
					if (stateMsg->state == 0 && gol_neighbors[i].state == 2)
					{
						gol_neighbors[i].state = 0;
						//printf("Neighbor %u is dead \n \r", neighbors[i].ID);
					}
					
					if (stateMsg->allow_next_step == 1 && gol_neighbors[i].allow_next_step == false)
					{
						gol_neighbors[i].allow_next_step = true;
						//printf("Neighbor %u allowed next step \n \r", neighbors[i].ID);
					}
				}
				
				if (gol_me.next_state_passed == false)
				{
					
					if (stateMsg->next_state_calculated == 1 && gol_neighbors[i].next_state_calculated == false)
					{
						gol_neighbors[i].next_state_calculated = true;
						//printf("Neighbor %u calculated next state \n \r", neighbors[i].ID);
					}
					
				}
				
				if (gol_me.allow_next_step == false)
				{
					
					if (stateMsg->next_state_passed == 1 && gol_neighbors[i].next_state_passed == false)
					{
						gol_neighbors[i].next_state_passed = true;
						//printf("Neighbor %u passed next state \n \r", neighbors[i].ID);
					}
					
				}
				
			}
		}
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
