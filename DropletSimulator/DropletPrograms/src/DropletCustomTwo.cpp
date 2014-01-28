#include "DropletCustomTwo.h"

const uint8_t DropletCustomTwo::led_state_colors[NUM_STATES][3] = {
	{ 0,  0, 50},
	{50,  0, 50},
	{50,  0,  0},
	{ 0, 50, 50},
	{ 0, 50,  0},
};

DropletCustomTwo::DropletCustomTwo(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) {}

DropletCustomTwo::~DropletCustomTwo() {}

void DropletCustomTwo::DropletInit()
{
	init_all_systems();	
	
	group_size = 1;
	i_am_leader = 0;
	repeat_discover_msg = 3;

	state = SEARCH;
	set_state_led();
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
	color_msg[RED] = get_red_sensor();
	if(color_msg[RED] >= RED_THRESHOLD)
	{
		color_msg[GREEN] = get_green_sensor();
		color_msg[BLUE] = get_blue_sensor();
		state = DISCOVER_GROUP;
		set_state_led();
		cancel_move();
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
		ir_broadcast(msg, 4);
	}

	if(check_for_new_messages())
	{
		if(global_rx_buffer.buf[0] == RSP_DISCOVER_GROUP 
			&& strncmp((char *)&(global_rx_buffer.buf[1]), (char *)color_msg, 3) == 0)
				group_size = global_rx_buffer.buf[4];
		global_rx_buffer.read = 1;
		state = WAIT;
		set_state_led();
	}
}

void DropletCustomTwo::leading_group()
{
	uint8_t collaborators = 0;
	if(run_sigmoid())
		collaborators++;

	if(check_for_new_messages())
	{
		// Make sure the msg originated from the same collaboration site.
		if(strncmp((char *)&(global_rx_buffer.buf[1]), (char *)color_msg, 3) == 0)
		{
			switch(global_rx_buffer.buf[0])
			{
			case RQST_DISCOVER_GROUP:
				// If a new droplet is requesting information on the group then
				// increment the group size by 1.
				std::vector<droplet_id_type>::iterator pos;
				pos = std::find(unique_ids.begin(), unique_ids.end(), global_rx_buffer.sender_ID);
				if(pos != unique_ids.end())
					group_size++;
					
				char msg[5];
				msg[0] = (uint8_t)RSP_DISCOVER_GROUP;
				memcpy(&msg[1], color_msg, 3);
				msg[4] = group_size;
				ir_broadcast(msg, 4);
				break;

			case RQST_UPDATE_COLLAB:

				break;
			}
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomTwo::waiting_at_object()
{

}

void DropletCustomTwo::collaborating()
{

}

uint8_t DropletCustomTwo::run_sigmoid()
{
	double exp_val = pow(M_E, sigmoid_slope * (rqst_group_size - group_size));
}


void DropletCustomTwo::init_leader()
{
		i_am_leader = 1;
		state = LEAD_GROUP;
		unique_ids.clear();
		unique_ids.push_back(get_droplet_id());
		set_state_led();
}

void DropletCustomTwo::set_state_led()
{
	set_rgb_led(
		led_state_colors[state][RED],
		led_state_colors[state][GREEN],
		led_state_colors[state][BLUE]);
}

