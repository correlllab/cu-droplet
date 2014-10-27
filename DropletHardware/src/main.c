#include "main.h"

int main(void)
{
	init_all_systems ();
	
	// Set theta and tau initially
	theta	= 2.f;
	tau		= 10;

	group_root = NULL;
	clear_state();
	
	// Initialize timing values
	start_delay_time    = 0;
	heartbeat_time      = 0;
	voting_time         = 0;
		
	uint32_t last_time	= get_time();
	uint32_t cur_time	= get_time();
	uint16_t prev_gap	= 0;
	
    change_state ( START_DELAY );
	while (1)
	{
		cur_time	= get_time ();
		prev_gap	= (uint16_t)( cur_time - last_time );
		last_time	= cur_time;	
		
		switch ( state )
		{
			case COLLABORATING:
			if ( get_time() - collab_time > COLLABORATE_DURATION )
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
			if ( check_safe () )
			{
				change_state ( SEARCHING );
			}

			// If you start in the red region then try to get out before you die
			else
			{
				random_walk ();
			}
			break;

			case SEARCHING:
			random_walk ();
			if ( !check_safe() )
			{
				change_state ( WAITING );
			}
			break;

			case START_DELAY:
			{
				if ( cur_time - start_delay_time > START_DELAY_TIME )
				change_state ( SAFE );
			}
			break;

			case WAITING:
			if ( get_time() - heartbeat_time > HEART_RATE )
			{
				heartbeat_time = get_time ();
				send_heartbeat ();
			}

			// Checks incoming messages and updates group size.
			// There is a chance the state can be changed to COLLABORATING in
			// this function if the droplet sees a GO message.
			current_group_size = update_group_size ( prev_gap );

			if ( get_time() - voting_time > HEART_RATE && state == WAITING )
			{
				voting_time = get_time ();
				check_votes ();
			}
			break;

			default:
			break;
		}		
		
		check_messages ();	
	}
}


// Droplet Movement Helper Functions
uint8_t check_safe ()
{
    if ( get_red_sensor() > RED_THRESHOLD )
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
		move_steps ( last_move_dir, rand_byte() * MOVE_DIST_SCALAR );
    }	
}


// Task Allocation Helper Functions
void send_heartbeat ()
{
	set_rgb ( 80, 0, 120 );
	char* msg;
	if( roll_sigmoid(current_group_size) )
	{
		msg = "<3Y";
		yes_count += 1;
	}
	else
	{
		msg = "<3N";
	}
	
	ir_send ( ALL_DIRS, msg, 3 );
	set_rgb ( 0, 0, 0 );
}

/* 
 * This function traverses the group list, adding time_to_add to the age of each item.
 * If, after adding, the ms_age is greater than the timeout, that item is removed from the list.
 * Otherwise, we increase our group size by one.
 */
uint16_t update_group_size ( uint16_t time_to_add )
{
	group_item	*gi			= group_root;
	uint16_t	group_size	= 0;
	
	do 
	{
		if ( gi==group_root )
		{
			group_size++;
		}
		else
		{
			gi->ms_age += time_to_add;
			if( gi->ms_age > GROUP_MEMBERSHIP_TIMEOUT )
			{
				group_item* temp	= gi;
				gi->prev->next		= gi->next;
				gi->next->prev		= gi->prev;
				gi					= gi->next;
				free ( temp );
			}
			else
			{
				group_size++;
			}
		}
	} while ( gi != group_root );
	
	return group_size;
}

void check_votes ()
{
	if( yes_count*2 >= current_group_size )
	{
		char* msg = "GO";
		ir_send ( ALL_DIRS, msg, 2 );
		change_state ( COLLABORATING );
	}
}

uint8_t roll_sigmoid ( int16_t group_size )
{
	double		sig_value	= 1.0 / (1.0 + exp(theta*(tau-group_size)));
	uint8_t		rand_byte_h	= rand_byte();
	uint8_t		rand_byte_l	= rand_byte();
	uint16_t	rand_short  = ((uint16_t)rand_byte_l | (((uint16_t)rand_byte_h) << 8));
	
	if( rand_short <= (uint16_t)(sig_value * 0xffff) ) // Max Value of uint16_t = 0xffff 
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
		move_steps			( (last_move_dir + 3) % 6, WALKAWAY_TIME );

		case SAFE:
		set_rgb				( 250, 0, 0 );		// RED
		break;

		case START_DELAY:
		start_delay_time	= get_time ();
		break;

		case WAITING:
		stop				();
		led_off				();					// OFF
		clear_msg_buffers	();
		
		heartbeat_time		= get_time ();
		voting_time			= get_time ();
		send_heartbeat		();
		break;

		case SEARCHING:
		set_rgb				( 250, 250, 0 );	// YELLOW
		break;
			
		default:
		led_off				();
	}
}

void check_messages ()
{
	// If we are not in the wait state then discard all incoming messages
	if ( state != WAITING )
	{
		clear_msg_buffers ();			
		return;
	}
	
	uint8_t num_msgs = 0;
	while ( last_ir_msg != NULL )
	{
		// Add your code here
		char	tempStr [ IR_BUFFER_SIZE ];
		memset	( tempStr, 0, IR_BUFFER_SIZE );
		memcpy	( tempStr, last_ir_msg->msg, last_ir_msg->msg_length );

		uint16_t senderID = last_ir_msg->sender_ID;
		if ( strcmp(tempStr,"<3Y") == 0 || strcmp(tempStr, "<3N") == 0 )
		{
			add_group_member ( senderID );
			if ( tempStr[2]=='Y' ) 
			{
				yes_count++;
			}
		}
		else if ( strcmp(tempStr,"GO") == 0 )
		{
			char *msg = "GO";
			ir_send ( ALL_DIRS, msg, 2);
			change_state ( COLLABORATING );
			break;
		}
		
		// Keep this code here
		msg_node *temp	= last_ir_msg;
		last_ir_msg		= last_ir_msg->prev;
		free ( temp->msg );
		free ( temp );
		num_msgs++;
	}
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


