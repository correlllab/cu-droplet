#include "DropletCustomSix.h"

const int guitar_rows[] =  {
		/* Body */ 3, 4, 5, 6, 5, 4, 5, 4, 3,
        /* Neck */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
        /* Head */ 3, 2, 0};  
const uint16_t num_rows = 22;

const float phi_clk[] = { 120.f, 180.f, -120.f, -60.f, 0.f, -60.f };
const float phi_cclk[] = { -60.f, 0.f, 60.f, 120.f, 180.f, -120.f};
const float theta_clk  = -90.f;
const float theta_cclk = 90.f; 

DropletCustomSix::DropletCustomSix(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{	return; }



DropletCustomSix::~DropletCustomSix() { 
		return; }



void DropletCustomSix::DropletInit()
{
	init_all_systems();
	set_init_states();
}



void DropletCustomSix::DropletMainLoop()
{
	
	switch(state)
	{
	case SEARCH:
		if(!search_color_set)
		{
			led_off();
			search_color_set = true;
		}

		handle_search();
		break;

	case CALL:
		if(!call_color_set)
		{
			set_rgb_led(CALL_COLOR_R, CALL_COLOR_G, CALL_COLOR_B);
			call_color_set = true;
		}

		handle_call();
		break;

	case ASSIGNED:
		if(!assigned_color_set)
		{
			set_rgb_led(ASSIGNED_COLOR_R, ASSIGNED_COLOR_G, ASSIGNED_COLOR_B);
			assigned_color_set = true;
		}

		handle_assigned();
		break;

	case SET:
		if(!set_color_set)
		{
			set_rgb_led(SET_COLOR_R, SET_COLOR_G, SET_COLOR_B);
			set_color_set = true;
		}

		handle_set();
		break;

	case ERR :
		if(check_timer(0))
		{
			set_timer(1500, 0);
			error_color_set = !error_color_set;
			if(!error_color_set)
				set_rgb_led(ERROR_COLOR_R, ERROR_COLOR_G, ERROR_COLOR_B);
			else
				led_off();
		}
	}
}



void DropletCustomSix::set_init_states(void)
{
	call_color_set = false;
	search_color_set = true;
	assigned_color_set = false;
	set_color_set = false;
	error_color_set = false;
	wait_resp_color_set = false;
	saw_mark = false;

	pos_rqrd = POS_NOT_SET;
	pos_rqsd = POS_NOT_SET;

	my_guitar_id = 0;
	max_guitar_id = 0;
	uint16_t i = 0;
	while(i < num_rows)
	{
		max_guitar_id += guitar_rows[i++];
	}

	state = SEARCH;
}



void DropletCustomSix::handle_search(void)
{
	// Look for the starting mark
	uint8_t r, g, b;
	get_rgb_sensor(&r, &g, &b);
	if(r > 200) // The guitar start mark is colored red
	{
		saw_mark = true;
		my_guitar_id = 1;
		state = CALL;
		search_color_set = false;
		cancel_move();
		cancel_rotate();
		return;
	}

	// Move around
	if(!is_moving())
	{
		move_steps((rand_byte() % 6) + 1, rand_byte() / 2);
	}
	if(!is_rotating())
	{
		int16_t deg = 0;
		int16_t sign = rand_byte() < 128 ? -1 : 1;
		while((deg = rand_byte()) <= 180);

		rotate_degrees(sign * deg);
	}
}



void DropletCustomSix::handle_call(void)
{
	if(pos_rqrd == 0) // Do you need find where to call someone?
	{
		if(my_guitar_id < 1) // i.e. Your guitar id is not yet assigned
		{
			state = ERR;
			call_color_set = false;
			return;
		}

		if(my_guitar_id == max_guitar_id)
		{
			state = SET;
			call_color_set = false;
			return;
		}

		// Find what row you're on
		int16_t tmp = my_guitar_id;
		row = 0;
		while((tmp -= guitar_rows[row]) > 0) { row++; }

		// Find the next required position to be called
		find_rqrd_pos(tmp == 0);
	}

	else // Start calling someone to fill the position next to you
	{
		if(check_timer(1))
		{
			char msg[2];
			msg[0] = REQUEST_DROPLET;
			msg[1] = (char)pos_rqrd;
			ir_broadcast(msg, sizeof(char) * 2);

			set_timer(1000, 1);
		}
		else
		{
			handle_msgs();
		}
	}
}



void DropletCustomSix::handle_assigned(void)
{

}

void DropletCustomSix::handle_set(void)
{

}

void DropletCustomSix::find_rqrd_pos(bool last_element)
{
	// Is the row even or odd?
	if(row % 2 == 0) // Even row
	{
		if(last_element) // Last element
		{
			if(guitar_rows[row + 1] > guitar_rows[row]) // Next row larger
			{
				pos_rqrd = 0;
			}
			else // Next row not larger
			{
				pos_rqrd = 5;
			}
		}
		else // Not last element
		{
			pos_rqrd = 1;
		}
	}
	else // Odd row
	{
		if(last_element) // Last element
		{
			if(guitar_rows[row + 1] > guitar_rows[row]) // Next row larger
			{
				pos_rqrd = 5;
			}
			else // Next row not larger
			{
				pos_rqrd = 0;
			}
		}
		else // Not last element
		{
			pos_rqrd = 4;
		}
	}
}

void DropletCustomSix::handle_msgs(void)
{
	if(check_for_new_messages())
	{
		char *in_msg = (char *)malloc(global_rx_buffer.data_len);
		memcpy(in_msg, global_rx_buffer.buf, global_rx_buffer.data_len);
		droplet_id_type sender_id = global_rx_buffer.sender_ID;
		global_rx_buffer.read = true;

		switch(in_msg[0])
		{
		case RESPOND_DROPLET :
			break;

		case REQUEST_DROPLET :
			if(state == SEARCH)
			{
				pos_rqrd = in_msg[1];
				cancel_move();
				cancel_rotate();
				search_color_set = false;
				state = WAIT_RESP;
			}
			break;

		case ACK :
			break;
		}

		free(in_msg);
	}
}