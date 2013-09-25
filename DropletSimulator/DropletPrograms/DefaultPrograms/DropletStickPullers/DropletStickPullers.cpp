#include "DefaultPrograms/DropletStickPullers/DropletStickPullers.h"

DropletStickPullers::DropletStickPullers(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletStickPullers::~DropletStickPullers() { return; }

void DropletStickPullers::DropletInit()
{
	init_all_systems();
	phase = SEARCH;


	first_wait = 1;
	blink_count = 3;
	r_thresh = 200;
	wait_timeout = 3000;
	blind_timeout = 2000;

	group_size = 1;
	group_thresh = 5;
	gpx2 = group_thresh * 2;
	recv_ids = (droplet_id_type *)malloc(sizeof(droplet_id_type) * gpx2);
	uint16_t i;
	for(i = 0; i < gpx2; i++)
		recv_ids[i] = 0;
}

void DropletStickPullers::back_to_search()
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

void DropletStickPullers::DropletMainLoop()
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
			if(r > r_thresh)
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
				char send = group_size;
				ir_broadcast(&send, sizeof(char));
			}
			// Check for incoming messages
			else
			{
				if(check_for_new_messages())
				{
					uint16_t i;
					uint8_t found = 0;

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
					if(!found)
					{
						uint16_t l = i < gpx2 ? i : 0;
						recv_ids[l] = global_rx_buffer.sender_ID;
						group_size++;

						// Reset the wait timer
						set_timer(wait_timeout, 0);
					}
				}

				// Check to see if we have reached threshold
				if(group_size >= group_thresh)
					phase = DONE;
			}
		}
		break;
	
	case DONE :
		if(check_timer(3))
		{
			set_timer(500, 3);
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