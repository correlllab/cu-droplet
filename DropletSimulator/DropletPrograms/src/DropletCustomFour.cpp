#include "DropletCustomFour.h"

#define NUMBER_OF_ITS 5
const char got_you = 'x';

DropletCustomFour::DropletCustomFour(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomFour::~DropletCustomFour() { return; }

void DropletCustomFour::DropletInit()
{
	init_all_systems();
	state = NOT_IT;
	ack = 1;
	it_init_required = true;
	notit_init_required = true;
	toggle = true;
	wait_for_ack = false;
	target_id = NULL;
	set_timer(3000, 0);
	my_id = get_droplet_id();
	marco = 'm';
	polo  = 'p';
	other_droplets.clear();
	other_droplets.push_back(my_id);
	droplet_status.clear();
}

void DropletCustomFour::acquireNewTarget()
{
	float dist, theta, phi;
	double min_dist = INT_MAX;
	std::vector<float> dists;
	dists.clear();

	float max = 0.f;
	for(unsigned int i=0 ; i< other_droplets.size() ; i++)
	{
		range_and_bearing(other_droplets[i], &dist, &theta, &phi);
		
		if(dist > max)
			max = dist;
		
		dists.push_back(dist);
	}

	float nmax = 128.f / max;

	for(unsigned int i=0 ; i< other_droplets.size() ; i++)
	{	
		if(rand_byte() < static_cast<uint8_t>(dists[i] * nmax))
		{
			if(droplet_status[other_droplets[i]] || my_id==other_droplets[i]) continue;
			target_id = other_droplets[i];
			break;
		}
		if(dists[i] < min_dist)
		{
			if(droplet_status[other_droplets[i]] || my_id==other_droplets[i]) continue;
			min_dist = dists[i];
			target_id = other_droplets[i];
		}
	}
}

void DropletCustomFour::DropletMainLoop()
{
	// 1 second of transmission and list building
	if(!check_timer(0))
	{
		if(rand_byte() < 64)
		{
			ir_broadcast(&ack, sizeof(char));
		}
		while(check_for_new_messages())
		{
			bool seen = false;
			for(std::vector<droplet_id_type>::iterator iter = other_droplets.begin() ;  iter != other_droplets.end(); ++iter)
			{
				if(*iter== global_rx_buffer.sender_ID)
				{
					seen = true;
					break;
				}
			}
			if(!seen)
			{
				droplet_status[target_id] = false;
				other_droplets.push_back(global_rx_buffer.sender_ID);
			}
		}
	}
	else
	{
		if(check_timer(2))
		{
			set_green_led(0);
		}
		if(notit_init_required && it_init_required)
		{
			std::sort(other_droplets.begin(), other_droplets.end());
			std::vector<droplet_id_type>::iterator it;

			for(unsigned int i=0 ; i<NUMBER_OF_ITS ; i++)
			{
				droplet_status[other_droplets[i]] = true;
				if(other_droplets[i] == my_id)
				{
					state = IT;
				}
			}
		}
		switch(state)
		{
		case IT :
			if(it_init_required)
			{
				while(check_for_new_messages());
				set_rgb_led(255, 0, 0);
				acquireNewTarget();
				it_init_required = false;
				set_timer(250, 1);
			}
			if(check_timer(1))
			{
				while(check_for_new_messages())
				{
					if(wait_for_ack)
					{
						if(global_rx_buffer.buf[0] == 1 && memcmp(&global_rx_buffer.buf[1], &my_id, 2) == 0)
						{
							if(global_rx_buffer.sender_ID == target_id)
							{
								state = NOT_IT;
								notit_init_required = true;
								wait_for_ack = false;
								break;
							}
						}
					}
					if(global_rx_buffer.buf[0] == 'm')
					{
						droplet_status[global_rx_buffer.sender_ID] = true;
					}
					else if(global_rx_buffer.buf[0] == 'p')
					{
						droplet_status[global_rx_buffer.sender_ID] = false;
					}

					if(rand_byte()%8 == 0)
					{
						ir_send(0,&marco,1);
					}
					set_timer(250, 1);
				}

			}
			if(!is_moving())
			{
				if(!(rand_byte() & rand_byte()<<5)) //change the modulus value to change the chances of swapping targets.
				{
					acquireNewTarget();
					set_green_led(255);
					set_timer(250, 2);
				}
				float dist, theta, phi;
				if(!range_and_bearing(target_id, &dist, &theta, &phi))
				{
					set_rgb_led(255, 0, 0);
					move_steps((rand_byte() % 6) + 1, rand_byte());
				}
				if(dist <= 5.f)
				{
					char msg[3];
					memset(msg, 0, 3);
					msg[0] = got_you;
					memcpy(&msg[1], &target_id, sizeof(droplet_id_type));
					ir_broadcast(msg, 3);
					wait_for_ack = true;
				}
				else if(!is_rotating() && abs(theta) > 2.5f)
				{
					rotate_degrees(static_cast<int16_t>(theta));
				}
				else if(!is_rotating() && abs(theta) <= 2.5f)
				{
					move_steps(NORTH, 10);
				}

			}
			break;

		case NOT_IT :
			if(notit_init_required)
			{
				while(check_for_new_messages());
				set_rgb_led(0, 0, 255);
				notit_init_required = false;
				set_timer(250, 1);
			}
			if(check_timer(1))
			{
				while(check_for_new_messages())
				{
					if(global_rx_buffer.buf[0] == got_you && memcmp(&global_rx_buffer.buf[1], &my_id, 2) == 0)
					{
						state = IT;
						it_init_required = true;
						char ack[3];
						ack[0] = 1;
						memcpy(&ack[1],	&(global_rx_buffer.sender_ID), sizeof(droplet_id_type));
						ir_broadcast(ack, 3);
						break;
					}
					else if(global_rx_buffer.buf[0] == 'm')
					{
						droplet_status[global_rx_buffer.sender_ID] = true;
					}
					else if(global_rx_buffer.buf[0] == 'p')
					{
						droplet_status[global_rx_buffer.sender_ID] = false;
					}
				}
				if(rand_byte()%8 == 0)
				{
					ir_send(0,&polo,1);
				}
				set_timer(250, 1);
			}
			if(!is_moving())
			{
				move_steps((rand_byte() % 6) + 1, rand_byte() * 10);
				/*float dist, theta, phi;

				if(!range_and_bearing(it_id, &dist, &theta, &phi))
				{
					set_rgb_led(255, 0, 0);
					move_steps((rand_byte() % 6) + 1, rand_byte());
					break;
				}
				if(!is_rotating() && abs(theta) - 180 > 2.5f)
				{
					rotate_degrees(static_cast<int16_t>(theta) - 180);
				}
				else if(!is_rotating() && abs(theta)-180 <= 2.5f)
				{
					move_steps(NORTH, 10);
				}*/
			}
			break;
		default :
			break;
		}
	}
}