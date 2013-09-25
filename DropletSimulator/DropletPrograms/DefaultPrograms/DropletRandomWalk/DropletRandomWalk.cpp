#include "DefaultPrograms/DropletRandomWalk/DropletRandomWalk.h"

DropletRandomWalk::DropletRandomWalk(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletRandomWalk::~DropletRandomWalk() { return; }

void DropletRandomWalk::DropletInit()
{
	init_all_systems();

	set_rgb_led(0, 0, 255);
}

void DropletRandomWalk::DropletMainLoop()
{
	if(!is_moving())
	{
		if(!is_rotating())
		{
			int16_t sign = rand_byte() < 128 ? -1 : 1;
			int16_t angle;
			while((angle = rand_byte()) > 180);
			rotate_degrees(sign * angle);
		}

		move_steps(NORTH, rand_byte() * 10);
	}
}