#include "DropletCustomTwo.h"

const uint8_t DropletCustomTwo::led_state_colors[NUM_STATES][3] = {
	{  0,   0, 200},
	{200,   0, 200},
	{200,   0,   0},
	{  0, 200, 200},
	{  0, 200,   0},
};

DropletCustomTwo::DropletCustomTwo(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) {}

DropletCustomTwo::~DropletCustomTwo() {}

void DropletCustomTwo::DropletInit()
{
	init_all_systems();	

	rqst_group_size = RQST_GROUP_SIZE;
	task_time = TASK_TIME;
	sigmoid_slope = SIGMOID_SLOPE;

	// TODO: Remove this later
	//char filename[256];
	//memset(filename, 0, 256);
	//sprintf(filename, "Droplet_%u_msg_buffer.txt", get_droplet_id());
	//fh = fopen(filename, "w");

	reset_values();
}

void DropletCustomTwo::DropletMainLoop()
{
	switch(state)
	{
	case SEARCH:
		searching();
		break;
	case DISCOVER_GROUP:
		discovering_group();
		break;
	case WAIT:
		waiting_at_object();
		break;
	case LEAD_GROUP:
		leading_group();
		break;
	case COLLABORATE:
		collaborating();
	}
}

void DropletCustomTwo::searching()
{
	if(!is_moving())
	{
		move_steps((rand_byte() % 6) + 1, 500+rand()%50);
	}

	if(check_timer(2))
	{
		get_rgb_sensor(&color_msg[RED], &color_msg[GREEN], &color_msg[BLUE]);
		if(color_msg[RED] >= RED_THRESHOLD)
			//|| color_msg[GREEN] >= GREEN_THRESHOLD
			//|| color_msg[BLUE] >= BLUE_THRESHOLD)
		{
			state = DISCOVER_GROUP;
			set_state_led();
			cancel_move();
		}
	}
}

void DropletCustomTwo::discovering_group()
{
	if(repeat_discover_msg == 0)
	{
		init_leader();
		return;
	}

	if(check_timer(0) && repeat_discover_msg > 0)
	{
		repeat_discover_msg--;
		set_timer(1000, 0);
		char msg[5];
		msg[0] = (uint8_t)RQST_DISCOVER_GROUP;
		memcpy(&msg[1], color_msg, 3);
		msg[4] = group_size;
		ir_broadcast(msg, 5);
	}

	while(check_for_new_messages())
	{
		//log_msg();
		if(global_rx_buffer.buf[0] == (uint8_t)RSP_DISCOVER_GROUP 
			&& strncmp((char *)&(global_rx_buffer.buf[1]), (char *)color_msg, 3) == 0)
		{
				group_size = global_rx_buffer.buf[4];
				global_rx_buffer.read = 1;

				state = WAIT;
				set_state_led();
				break;
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomTwo::leading_group()
{
	collaborators = 0;
	if(run_sigmoid())
		collaborators++;

	std::vector<droplet_id_type>::iterator pos;

	uint8_t state_changed = 0;
	while(check_for_new_messages())
	{
		//log_msg();
		// Make sure the msg originated from the same collaboration site.
		if(strncmp((char *)&(global_rx_buffer.buf[1]), (char *)color_msg, 3) == 0)
		{
			switch(global_rx_buffer.buf[0])
			{
			case RQST_DISCOVER_GROUP:
				// If a new droplet is requesting information on the group then
				// increment the group size by 1.
				pos = std::find(unique_ids.begin(), unique_ids.end(), global_rx_buffer.sender_ID);
				if(pos != unique_ids.end())
					group_size++;
					
				char msg[5];
				msg[0] = (uint8_t)RSP_DISCOVER_GROUP;
				memcpy(&msg[1], color_msg, 3);
				msg[4] = group_size;
				ir_broadcast(msg, 5);
				break;

			case RQST_UPDATE_COLLAB:
				collaborators++;

				// If half or more decide to collaborate in a single time-step
				// then send the collaborate msg to everyone at the site.
				if(collaborators >= std::max(2.0, (group_size / 2.0)))
				{
					char msg[5];
					msg[0] = (uint8_t)RSP_START_COLLAB;
					memcpy(&msg[1], color_msg, 3);
					msg[4] = group_size;
					ir_broadcast(msg, 5);
					state = COLLABORATE;
					set_state_led();
					state_changed = 1;
				}
				break;
			}
		}
		global_rx_buffer.read = 1;
		if(state_changed)
			break;
	}
}

void DropletCustomTwo::waiting_at_object()
{
	while(check_for_new_messages())
	{
		//log_msg();
		switch(global_rx_buffer.buf[0])
		{
		case RSP_START_COLLAB:
			state = COLLABORATE;
			set_state_led();
			break;
		case RSP_DISCOVER_GROUP:
			group_size = (uint8_t)global_rx_buffer.buf[4];
		}
		global_rx_buffer.read = 1;
	}

	// Run the sigmoid to see if you want to collaborate
	if(run_sigmoid())
	{
		char msg[5];
		msg[0] = (uint8_t)RQST_UPDATE_COLLAB;
		memcpy(&msg[1], color_msg, 3);
		msg[4] = group_size;
		ir_broadcast(msg, 5);
	}
}

void DropletCustomTwo::collaborating()
{
	if(rand() <= RAND_MAX / task_time)
	{
		reset_values();
	}
}

uint8_t DropletCustomTwo::run_sigmoid()
{
	double exp_val = pow(M_E, sigmoid_slope * (rqst_group_size - group_size));
	double sig_val = pow(1. + exp_val, -1);
	return (double)rand() <= (sig_val * RAND_MAX);
}


void DropletCustomTwo::init_leader()
{
		i_am_leader = 1;
		state = LEAD_GROUP;
		unique_ids.clear();
		unique_ids.push_back(get_droplet_id());
		set_state_led();
}

void DropletCustomTwo::reset_values()
{
	state = SEARCH;
	set_state_led();
	if(i_am_leader)
		unique_ids.clear();

	collaborators = 0;
	group_size = 1;
	i_am_leader = 0;
	repeat_discover_msg = (rand_byte() % 10) + 2;
	memset(color_msg, 0, 3);

	// Reset the message buffers
	for(unsigned int i = 0; i < NUM_COMM_CHANNELS; i++)
		reset_ir_sensor(i);

	set_timer(3000, 2);
}

void DropletCustomTwo::set_state_led()
{
	set_rgb_led(
		led_state_colors[state][RED],
		led_state_colors[state][GREEN],
		led_state_colors[state][BLUE]);
}

void DropletCustomTwo::log_msg()
{
	int cmpVal = strncmp((char *)&global_rx_buffer.buf[1], (char *)color_msg, 3);
	fprintf(fh, "Droplet[%u] State[%u] ColorCmp[%d] Color[%u %u %u] Msg[%u %u %u %u %u]\n", 
		get_droplet_id(), 
		state,
		cmpVal,
		color_msg[0],
		color_msg[1],
		color_msg[2],
		global_rx_buffer.buf[0],
		global_rx_buffer.buf[1],
		global_rx_buffer.buf[2],
		global_rx_buffer.buf[3],
		global_rx_buffer.buf[4]);
}