#include "DefaultPrograms/DropletStickPullersUpdated/DropletStickPullersUpdated.h"
#include <iostream>
#include <fstream>

DropletStickPullersUpdated::DropletStickPullersUpdated(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletStickPullersUpdated::~DropletStickPullersUpdated() { return; }

void DropletStickPullersUpdated::DropletInit()
{
	init_all_systems();
	phase = SEARCH;


	first_wait = 1;
	blink_count = 3;
	wait_timeout = 3000;
	blind_timeout = 2000;

	group_size = 1;
	group_thresh = 2;
	gpx2 = group_thresh * 2;
	recv_ids = (droplet_id_type *)malloc(sizeof(droplet_id_type) * gpx2);
	uint16_t i;
	for(i = 0; i < gpx2; i++)
		recv_ids[i] = 0;
}

void DropletStickPullersUpdated::back_to_search()
{
	// reset the droplet id list
	group_size = 1;
	uint16_t j;
	for(j = 0; j < gpx2; j++)
		recv_ids[j] = 0;

	// Go back to search
	phase = SEARCH;
	set_timer(blind_timeout, 1); // so it has time to move away from the object
}

void DropletStickPullersUpdated::DropletMainLoop()
{
	switch(phase)
	{
	case SEARCH :
		if(!is_moving())
		{
			// Random motion
			rotate_duration((rand() % 2), 1000);
			move_duration((rand() % 6) + 1, ((rand() % 3) + 1) * 1000);

		}
		else if(check_timer(1))
		{
			uint8_t r, g, b;
			get_rgb_sensor(&r, &g, &b);

			// Check to see if we are under a red object
			if(r > 0 || g > 0 || b > 0)
			{
				phase = WAIT;
				first_wait = 1;
				cancel_move();
			}
		}
		break;

	case WAIT :		
		
		if(check_timer(0))
		{
			// Just entered wait mode
			if(first_wait)
			{
				// clear message buffers
				while(check_for_new_messages());

				// set timeout
				set_timer(wait_timeout, 0);
				first_wait = 0;
			}

			// Timed out on wait mode
			else back_to_search();
		}

		else
		{
			// Send your droplet ID ever 100ms
			if(check_timer(2))
			{
				set_timer(100, 2);
				uint8_t r, g, b;
				get_rgb_sensor(&r, &g, &b);
				uint8_t send[3];
				send[0] = r;
				send[1] = g;
				send[2] = b;
				ir_broadcast((char *)send, sizeof(send));
			}
			// Check for incoming messages
			else
			{
				if(check_for_new_messages())
				{
					uint16_t i;
					uint8_t found = 0;
					uint8_t r, g, b;
					get_rgb_sensor(&r, &g, &b);

					uint8_t *data = (uint8_t *)malloc(global_rx_buffer.data_len);
					memset(data, 0, global_rx_buffer.data_len);
					memcpy(data, global_rx_buffer.buf, global_rx_buffer.data_len);
					global_rx_buffer.read = 1;
			
					// Check to see if you have met the sender droplet before
					for(i = 0; i < gpx2; i++)
					{
						if(global_rx_buffer.sender_ID == recv_ids[i])
						{
							found = 1;
							break;
						}
						if(recv_ids[i] == 0)
							break;
					}
					// New guy! Add it's id to the list
					if( (!found) && data[0] == r  && data[1] == g && data[2] == b)
					{
						uint16_t l = i < gpx2 ? i : 0;
						recv_ids[l] = global_rx_buffer.sender_ID;
						group_size++;


						// Reset the wait timer
						set_timer(wait_timeout, 0);
					}
				}

				// Check to see if we have reached threshold
				if(group_size = group_thresh)
					phase = DONE;
			}
		}
		break;
	
	case DONE :
		if(check_timer(3))
		{
			set_timer(1000, 3);
			if(blink_count % 2)
				set_rgb_led(255, 255, 0);
			else
				set_rgb_led(0, 255, 255);

			if(--blink_count <= 0)
			{
				blink_count = 3;
				reset_rgb_led();
				back_to_search();
			}
		}
		break;
	default :
		break;
	}
}