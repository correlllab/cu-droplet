#include "CustomPrograms/DropletCustomOne/DropletCustomOne.h"

const char *r_talker = "rt";
const char *d_talker = "dt";

DropletCustomOne::DropletCustomOne(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletCustomOne::~DropletCustomOne() { return; }

void DropletCustomOne::DropletInit()
{
	init_all_systems();

	target = get_droplet_id();
	state = TARGET;
	ack = 1;
	target_set_rgb = false;
	walker_set_rgb = false;
	toggle = true;
	set_timer(3000, 0);
}

void DropletCustomOne::DropletMainLoop()
{
	// 3 seconds of transmission and list building
	if(!check_timer(0))
	{
		if(rand_byte() < 64)
		{
			ir_broadcast(&ack, sizeof(char));
		}
		while(check_for_new_messages())
		{
			if(global_rx_buffer.sender_ID < target)
			{
				target = global_rx_buffer.sender_ID;
				state = WALKER;
			}
			global_rx_buffer.read = true;
		}
	}
	else
	{
		switch(state)
		{
		case TARGET :
			if(!target_set_rgb)
			{
				set_rgb_led(0, 0, 255);
				target_set_rgb = true;
			}
			if(!is_moving())
			{
				move_steps((rand_byte() % 6) + 1, rand_byte() * 10);
			}
			break;

		case WALKER :
			if(!walker_set_rgb)
			{
				set_rgb_led(255, 0, 0);
				walker_set_rgb = true;
			}
			if(!is_moving())
			{
				float dist, theta, phi;

				if(!range_and_bearing(target, &dist, &theta, &phi))
				{
					set_rgb_led(255, 0, 0);
					move_steps((rand_byte() % 6) + 1, rand_byte());
					break;
				}
				if(!is_rotating() && abs(theta) > 2.5f)
				{
					rotate_degrees(theta);
				}
				else if(!is_rotating() && abs(theta) <= 2.5f)
				{
					move_steps(NORTH, 10);
				}
			}
			break;
		default :
			break;
		}
	}
}