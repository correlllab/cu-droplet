#include "user_template.h"


/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	group_root = NULL;
	who_asked_me=0;
	msg_f = "F!";
	msg_q = "F?";
	msg_ack = "ACK";
	
	num_sent = 0;
	
	set_all_ir_powers(ir_power); // ir_power = 230
	
	change_state ( INIT );
	delay_ms(200);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	if(redSenseVal>RED_THRESH) change_state(FINAL);
	
	switch ( state )
	{
		case INIT:{
			if(get_time()>BUILD_NEIGHBOR_LIST_TIME) change_state(IDLE);
		}
		break;
	
		case IDLE:{
			if (greenSenseVal>GREEN_THRESH) change_state(FRONTIER);// if a droplet sees a green light above	
		}
		break;
	
		case FRONTIER:{
			if(rand_byte()<64) // 16 -> 64
			{
				send_query(); // send a query to neighbors 1/16 of the time (~160ms)
				num_sent++;
			}
			if(num_sent>5) change_state(WAIT); 
		}
		break;

		case FINAL:{
			if ((who_asked_me != 0) && !(got_ack)) // while got_ack = 0 (= msg_f is not properly sent back) 
			{
				if(rand_byte()<16)
				{
					ir_targeted_send(ALL_DIRS,msg_f,2,who_asked_me);// send a message "F!" (Found!) to the one who put a query on me
					ir_power_increment();
				}
			}
		}
		break;
		

		case WAIT:{
			// just handle messages
		}
		break;

		case LIGHT_ON:{
			if ((who_asked_me != 0) &&  !(got_ack) ) // while got_ack = 0 (= msg_f is not properly sent back) 
			{
			    if(rand_byte()<64){
					ir_targeted_send(ALL_DIRS,msg_f,2,who_asked_me);// send a message "F!" (Found!) to the one who put a query on me
					ir_power_increment();
				}
				
			}
		}
		break;
	
	}
	if((state!=FRONTIER)||(state!=LIGHT_ON)||(state!=FINAL)||(state!=INIT))
	{	
		redSenseVal = get_red_sensor();
		greenSenseVal = get_green_sensor();
	}
	delay_ms(40); // 20ms -> 40 ms, doing an experiment 
}

void ir_power_increment(){
	if(ir_power<255)
	{
		ir_power++;	                 // increase the ir_power
		set_all_ir_powers(ir_power); // change the ir_power
	}
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{

		if(state==IDLE) // added below
		{
			if (strcmp(msg_struct->msg, msg_q) == 0)				// if got a query
			{				
				who_asked_me = msg_struct->sender_ID;				// memorize the one who put a query on me
				change_state(FRONTIER);								// change the state into FRONTIER
			}
		}
		else if(state==FINAL||state==LIGHT_ON)
		{
			if ((strcmp(msg_struct->msg, msg_q) == 0)&&(state==FINAL))  // if got a query and the state is FINAL
			{	
				who_asked_me = msg_struct->sender_ID;				    // memorize the one who put a query on me		
			}
			else if ((strcmp(msg_struct->msg, msg_ack) == 0))			// if received an ACK message
			{
				got_ack = 1;											// set got_ack = 1  
			}
		}		
		else if(state==WAIT||state==FRONTIER)
		{
			if (strcmp(msg_struct->msg, msg_f) == 0){				// Got an answer from neighbor.
				who_asnwered = msg_struct->sender_ID;				// memorize the ID of the neighbor
				uint8_t i;
				for (i=0;i<5;i++)
				{
					ir_targeted_send(ALL_DIRS,msg_ack,3,who_asnwered);  // sends a message of ACKnowledge to the neighbor multiple times (5 times)
					delay_ms(40);
				}
				change_state(LIGHT_ON);								// change a state into LIGHT_ON
			}
		}

}

// send queries to neighbors 
void send_query () {
	ir_send(ALL_DIRS, msg_q, 2);
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
		//set_rgb(0,250,250); //cyan
		break;
		
		case IDLE:
		set_rgb(0,0,0); //blue
		break;
		
		case FRONTIER:
		set_rgb(0,250,0);//green
		num_sent = 0; 
		break;
		
		case WAIT:
		set_rgb(0,0,0);
		break;
		
		case LIGHT_ON:
		set_rgb(0,0,250); //blue
		num_sent = 0; 		
		break;
		
		case FINAL:
		set_rgb(250,0,250);
		num_sent = 0; 		
		break;
		
		default:
		led_off ();
		
	}
}