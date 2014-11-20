#include "user_template.h"

uint32_t last_time;

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	// Set theta and tau initially
	theta	= 10.f;
	tau		= 6;

	group_root = NULL;
	clear_state();
	
	// Initialize timing values
	heartbeat_time      = 0;
	light_check_time	= 0;
	last_update_time	= 0;

	uint32_t cur_time	= get_time();
	uint32_t prev_gap	= 0;
	
	change_state ( NOTHING );
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
		cur_time = get_time ();
		switch ( state )
		{
			case COLLABORATING:
			if ( cur_time - collab_time > COLLABORATE_DURATION )
			{
				change_state ( LEAVING );
			}
			break;

			case LEAVING:
			if ( !is_moving() )
			{
				change_state ( SAFE );
			}
			break;

			case SAFE:
			if ( cur_time - light_check_time > LIGHT_CHECK_RATE )
			{
				light_check_time = cur_time;
				if(check_safe()) change_state ( SEARCHING );
			}
			else // If you start in the red region then try to get out before you die
			{
				random_walk ();
			}
			break;
			case SEARCHING:
			random_walk ();
			if ( cur_time - light_check_time > LIGHT_CHECK_RATE )
			{
				light_check_time = cur_time;
				if(!check_safe()) change_state ( WAITING );
			}
			break;
			case WAITING:

			prev_gap	= ( cur_time - last_update_time );
			last_update_time	= cur_time;
			current_group_size = update_group_size ( prev_gap );
			
			if ( cur_time - heartbeat_time > HEART_RATE )
			{
				heartbeat_time = cur_time;
				send_heartbeat ();
				check_votes ();
			}
			break;
		}
		
		delay_ms(10);
}


// Droplet Movement Helper Functions
uint8_t check_safe ()
{
	int8_t r;
	get_rgb_sensors(&r, NULL, NULL);
    if ( r > RED_THRESHOLD )
    {
	    return 0;
    }

    return 1;	
}

void random_walk ()
{
    if ( !is_moving() )
    {
		last_move_dir = rand_byte() % 8;
		uint16_t num_steps = (rand_byte()%57)+8;
		if(last_move_dir<6) num_steps*= MOVE_DIST_SCALAR;
		move_steps ( last_move_dir, num_steps);
    }	
}


// Task Allocation Helper Functions
void send_heartbeat ()
{
	//set_rgb ( 80, 0, 120 );
	char* msg;
	if( roll_sigmoid(current_group_size) )
	{
		msg = "<3Y";
		yes_count++;
	}
	else
	{
		msg = "<3N";
	}
	
	ir_send	( ALL_DIRS, msg, 3 );
	add_group_member ( droplet_ID );
	//delay_ms(200);
	//set_rgb ( 0, 0, 0 );
}

/* 
 * This function traverses the group list, adding time_to_add to the age of each item.
 * If, after adding, the ms_age is greater than the timeout, that item is removed from the list.
 * Otherwise, we increase our group size by one.
 */
uint16_t update_group_size ( uint32_t time_to_add )
{
	group_item	*gi			= group_root;
	uint16_t	group_size	= 0;
	
	if(gi==NULL) return 0; //This shouldn't happen, but just in case.
	do 
	{
		gi->ms_age += time_to_add;
		if( gi->ms_age > GROUP_MEMBERSHIP_TIMEOUT )
		{
			group_item* temp	= gi;
			gi->prev->next		= gi->next;
			gi->next->prev		= gi->prev;
			gi					= gi->next;
			free ( temp );
			continue;
		}
		else
		{
			group_size++;
		}
		
		gi = gi->next;
	} while ( gi != group_root );
	
	return group_size;
}

void check_votes ()
{
	if( (current_group_size > 1) && (yes_count*2 >= current_group_size) )
	{
		char* msg = "GO";
		ir_send ( ALL_DIRS, msg, 2 );
		change_state ( COLLABORATING );
	}
}

uint8_t roll_sigmoid ( int16_t group_size )
{
	double		sig_value	= 1.0 / (1.0 + exp(theta*(tau-group_size)));
	
	if( rand_short() <= (uint16_t)(sig_value * 0xffff) ) // Max Value of uint16_t = 0xffff 
	{
		return 1;
	}
	
	return 0;
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

void clear_msg_buffers ()
{
	while ( last_ir_msg != NULL )
	{
		// Keep this code here
		msg_node *temp	= last_ir_msg;
		last_ir_msg		= last_ir_msg->prev;
		free ( temp->msg );
		free ( temp );
	}
}

void clear_state ()
{
	
	group_item *gi = group_root;
	while ( gi != NULL )
	{
		group_item *tmp = gi;
		gi = gi->next;
		free ( tmp );
		tmp = NULL;
	}
	
	yes_count			= 0;
	current_group_size	= 0;
}

void change_state ( State new_state )
{
	state = new_state;
	switch ( state )
	{
		case COLLABORATING:
		set_rgb				( 0, 0, 250 );		// BLUE
		collab_time			= get_time ();
		break;

		case LEAVING:
		set_rgb				( 0, 250, 0 );		// GREEN
		move_steps			( (last_move_dir + 3) % 6, WALKAWAY_STEPS );

		case SAFE:
		light_check_time	= 0;
		set_rgb				( 250, 0, 0 );		// RED
		break;

		case WAITING:
		stop				();
		led_off				();					// OFF
		clear_state			();	
		last_update_time	= get_time();
		heartbeat_time		= get_time ();
		send_heartbeat		();
		break;

		case SEARCHING:
		set_rgb				( 250, 250, 250 );	// YELLOW
		light_check_time	= 0;
		break;

		default:
		led_off				();
	}
}


/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	if ( state!=WAITING ) return;
	
	if ( strcmp(msg_struct->msg,"<3Y") == 0 || strcmp(msg_struct->msg, "<3N") == 0 )
	{
		add_group_member ( senderID );
		if ( msg_struct->msg[2]=='Y' )
		{
			yes_count++;
		}
	}
	else if ( strcmp(msg_struct->msg,"GO") == 0 )
	{
		char *msg = "GO";
		set_green_led ( 250 );
		wait_for_ir ( ALL_DIRS );
		ir_send ( ALL_DIRS, msg, 2 );
		change_state ( COLLABORATING );
		set_green_led ( 0 );
		break;
	}
}