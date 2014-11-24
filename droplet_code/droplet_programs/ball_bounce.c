#include "ball_bounce.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	for(uint8_t i=0;i<6;i++) neighbors[i]=NULL;
	heartbeat_msg	= "<3";
	catch			= "catch";
	ack				= "caught";
	num_neighbors = 0;
	last_loop_time = get_time();
	last_heartbeat_time = get_time();
	set_all_ir_powers(90); // 238 -> 230
	
	change_state(NOT_BALL);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	//if(state==NOT_BALL)
	//{
		//if(num_neighbors==6)		set_rgb(0,0,0);
		//else if(num_neighbors>=4)	set_rgb(60,0,0);
		//else if(num_neighbors>=2)	set_rgb(0,60,0);
		//else						set_rgb(0,0,60);
	//}
	//else if(state==BALL)
	//{
		//if(rand_byte()<64)
		//{
			//ir_targeted_send(in_dir, ack, 6, neighbors[in_dir]->ID);
		//}
		//else if(rand_byte()<64)
		//{
			//uint8_t tgt_dir = check_bounce(in_dir);
			//ir_targeted_send(tgt_dir, catch, 5, neighbors[tgt_dir]->ID);
		//}
	//}
	//if((get_time()-last_heartbeat_time) > HEARTBEAT_PERIOD)
	//{ 
		//last_heartbeat_time = get_time();
		//ir_send(ALL_DIRS, heartbeat_msg, 2);
	//}
	//update_group(get_time()-last_loop_time);
	//last_loop_time = get_time();
	//delay_ms(50);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_s)
{
	//printf(msg_s->msg);
	//printf("\r\n");
	if(strcmp(msg_s->msg, heartbeat_msg)==0) add_group_member(msg_s->sender_ID, msg_s->dir_received);
	else if(strcmp(msg_s->msg, catch)==0) 
	{
		if(state==NOT_BALL)
		{
			in_dir = msg_s->dir_received;
			change_state(BALL);
		}
	}
	else if(strcmp(msg_s->msg, ack)==0)
	{
		if(state==BALL)
		{
			change_state(NOT_BALL);
		}
	}
	else
	{
		printf("Got unexpected message:\r\n\t");
		printf(msg_s->msg);
		printf("\r\n");
	}
}

uint8_t check_bounce(uint8_t dir_in)
{
	//note: directions below are relative to the direction from which the ball came in.
	neighbor* in		= neighbors[dir_in];
	neighbor* opp		= neighbors[(dir_in+3)%6];
	neighbor* left		= neighbors[(dir_in+1)%6];
	neighbor* right		= neighbors[(dir_in-1)%6];
	neighbor* far_left	= neighbors[(dir_in+2)%6];
	neighbor* far_right = neighbors[(dir_in-2)%6];
	
	if(opp!=NULL)					return (dir_in+3)%6;										//go straight.
	else if(right!=NULL && left!=NULL)													//both close neighbors exist
	{
		if((far_right!=NULL && far_left!=NULL)||(far_right==NULL && far_left==NULL))	//far neighbors symmetrical
									return  dir_in;			//in
		else if(far_right!=NULL)	return (dir_in-1)%6;	//right						//more neighbors on right
		else						return (dir_in+1)%6;	//left						//more neighbors on left
	}
	else if(right==NULL && left==NULL)													//neither close neighbor exists
	{
		if((far_right!=NULL && far_left!=NULL)||(far_right==NULL && far_left==NULL))	//far neighbors symmetrical
									return  dir_in;			//in
		else if(far_right!=NULL)	return (dir_in-2)%6;	//far_right
		else						return (dir_in+2)%6;	//far_left
	}
	else if(right!=NULL)
	{
		if(far_right!=NULL)			return (dir_in-2)%6;	//far_right
		else if(far_left!=NULL)		return (dir_in+2)%6;	//far_left
		else						return (dir_in-1)%6;	//right
	}
	else																				//left=!NULL
	{
		if(far_left!=NULL)			return (dir_in+2)%6;	//far_left
		else if(far_right!=NULL)	return (dir_in-2)%6;	//far_right
		else						return (dir_in+1)%6;	//left
	}
}

void update_group(uint32_t time_to_add)
{
	num_neighbors = 0;
	//uint8_t colls = check_collisions();	
	for(uint8_t i=0;i<6;i++)
	{
		if(neighbors[i]!=NULL)
		{
			neighbors[i]->ms_age+=time_to_add;
			if((neighbors[i]->ms_age > ((HEARTBEAT_PERIOD*5)/2)))
			{
				free(neighbors[i]);
				neighbors[i]=NULL;	
			}
			else
			{
				num_neighbors++;
			}
		}
	}
}

// If the senderID is already in our group, this function resets its age to 0.
// Otherwise, this function adds it to the group list.
void add_group_member ( uint16_t senderID, uint8_t dir)
{
	if(neighbors[dir]!=NULL)
	{
		if(neighbors[dir]->ID == senderID)
		{
			neighbors[dir]->ms_age = 0;
			return;
		}
		free(neighbors[dir]);
		num_neighbors--;
	} 
	
	neighbors[dir] = (neighbor*)malloc(sizeof(neighbor));
	neighbors[dir]->ID = senderID;
	neighbors[dir]->ms_age = 0;
	num_neighbors++;
}

void change_state ( State new_state )
{
	state = new_state;
	if(state==BALL) set_rgb(250,250,250);
	else			set_rgb(0,0,0);
}