#include "user_template.h"

uint32_t last_time;

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	last_time=0;
	group_root = NULL;
	who_asked_me=0;
	msg_f = "F!";
	msg_q = "F?";
	msg_h = "here";
	
	is_end = 0;
	
	set_all_ir_powers(236);
	
	change_state ( INIT );
	delay_ms(200);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	if(rand_byte()<4) ir_send(ALL_DIRS,msg_h,4); // signals its presence to its neighbors 1/64th of the time.
	if(redSenseVal>RED_THRESH) change_state(FINAL);
	switch ( state )
	{
		case INIT:{
			if(get_time()>BUILD_NEIGHBOR_LIST_TIME) change_state(IDLE);
		}
		break;
	
		case IDLE:{
			if (greenSenseVal>GREEN_THRESH){ // if a droplet sees a green light above
				change_state(FRONTIER);
				send_query();
			}
		}
		break;
	
		case FRONTIER:{
			send_query(); // send a query to neighbors			
			change_state(WAIT);
		}
		break;

		case FINAL:{
			
		}
		break;

		case WAIT:{

		}
		break;

		case LIGHT_ON:{
			// do nothing
		}
		break;
	
	}
	if((state!=FRONTIER)||(state!=LIGHT_ON)||(state!=FINAL))
	{	
		redSenseVal = get_red_sensor();
		greenSenseVal = get_green_sensor();
	}
	delay_ms(20);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	if (strcmp(msg_struct->msg,"here") == 0){ // if a neighbor sends me a message, "I am here!"
		add_group_member(msg_struct->sender_ID); // add the neighbor to a list
	}
	else
	{
		if(state==IDLE) // added below
		{
			if (strcmp(msg_struct->msg,"F?") == 0){ // if got a query
				who_asked_me = msg_struct->sender_ID;    // memorize the one who put a query on me
				change_state(FRONTIER);      // change the state into FRONTIER
			}
		}
		else if(state==FINAL) // added below
		{
			if (strcmp(msg_struct->msg,"F?") == 0){ // if got a query
				who_asked_me = msg_struct->sender_ID;    // memorize the one who put a query on me
				ir_targeted_send(ALL_DIRS,msg_f,2,who_asked_me);				
			}
		}		
		else if(state==WAIT) // added below
		{
			if (strcmp(msg_struct->msg,"F!") == 0){ // if got an answer from your neighbor
				change_state(LIGHT_ON);     // change a state into LIGHT_ON
		
				if (who_asked_me != 0)// if I am the start, just turn on an LED
					ir_targeted_send(ALL_DIRS,msg_f,2,who_asked_me);// send a message "F!" (Found!) to the one who put a query on me
				}
		}
	}
}

// send queries to neighbors in the list
void send_query () {
	
	group_item* temp;
	temp = group_root;
	
	while (temp != NULL){ // walk along the list of neighbors
		ir_targeted_send(ALL_DIRS,msg_q,2,temp->ID); // send a query to a neighbor
		temp = temp->next;
	}
	
}

// If the senderID is already in our group, this function resets its age to 0.
// Otherwise, this function adds it to the group list.
void add_group_member ( uint16_t senderID )
{
	if ( group_root == NULL )
	{
		group_root			= (group_item *)malloc ( sizeof(group_item) );
		group_root->ID		= senderID;
		group_root->ms_age	= 0;
		group_root->next	= group_root;
		group_root->prev	= group_root;
		
		current_group_size++;
		return;
	}
	
	group_item* gi = group_root;
	do
	{
		if ( senderID == gi->ID )
		{
			gi->ms_age = 0;
			return;
		}
		gi = gi->next;
	}
	while(gi != group_root);

	group_item* new_node	= (group_item *)malloc ( sizeof(group_item) );
	new_node->ms_age		= 0;
	new_node->ID			= senderID;
	new_node->prev			= gi->prev;
	
	gi->prev->next	= new_node;
	gi->prev		= new_node;
	new_node->next	= gi;
	
	current_group_size++;
}

void change_state ( State new_state )
{
	state = new_state;
	switch ( state )
	{
		case INIT:
		break;
		
		case IDLE:
		//set_rgb(0,0,250); //blue
		break;
		
		case FRONTIER:
		set_rgb(0,250,0);//green
		break;
		
		case WAIT:
		set_rgb(0,0,0); //yellow
		break;
		
		case LIGHT_ON:
		set_rgb(0,0,250); //blue
		break;
		
		case FINAL:
		set_rgb(250,0,250);
		break;
		
		default:
		led_off ();
		
	}
}