#include "DefaultPrograms/DropletGranola/DropletGranola.h"

DropletGranola::DropletGranola(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletGranola::~DropletGranola() { return; }

void DropletGranola::DropletInit()
{
	init_all_systems();
	get_rgb_sensor(&r, &g, &b);
	new_g = g;
	new_b = b;
	direction_flag = 0;
	move_flag = 0;
	wiggle_flag = 0;
	wait = 0;
}

void DropletGranola::DropletMainLoop()
{
	if(wait > 0)
	{
		wait--;
	}
	else
	{
		if(is_moving() == 0 && is_rotating() == 0 && direction_flag == 0)
		{
			get_rgb_sensor(&r, &new_g, &new_b);
			if(move_flag == 0)
			{
				set_rgb_led(255,255,255);
				if(new_g > g)
				{
					rotate_steps(TURN_COUNTERCLOCKWISE, 100);
				}
				else if(new_g < g)
				{
					rotate_steps(TURN_CLOCKWISE, 100);
				}
				else if(new_b > b)
				{
					rotate_steps(TURN_COUNTERCLOCKWISE, 100);
				}
				else if(new_b == b && new_g == g)
				{
					if(wiggle_flag == 0)
					{
						rotate_steps(TURN_COUNTERCLOCKWISE, 100);
						wiggle_flag = 1;
					}
					else
					{
						rotate_steps(TURN_CLOCKWISE, 100);
						wiggle_flag = 0;
					}
				}
				else
				{
					//set_rgb_led(0,255,255);
					wait = 100;
				}

				move_flag = 1;
			}
			else
			{
				set_rgb_led(0,255,0);
				move_steps(NORTH, 1000);
				//direction_flag = 1;
				move_flag = 0;
			}
			b = new_b;
			g = new_g;

		}
		/*else if(is_moving() == 0 && is_rotating() == 0 && direction_flag == 1)
		{
			set_rgb_led(255,0,0);
			move_steps(SOUTH, 500);
			direction_flag = 0;
		}*/
	}


}