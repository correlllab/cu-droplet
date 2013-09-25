#include "DefaultPrograms/DropletRGBSense/DropletRGBSense.h"

DropletRGBSense::DropletRGBSense(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{
	moveCancelled = 0;
	pause = 0;
}

DropletRGBSense::~DropletRGBSense() { return; }

void DropletRGBSense::DropletInit()
{
	init_all_systems();

	set_rgb_led(0, 0, 255);
}

void DropletRGBSense::DropletMainLoop()
{
	if(is_moving() == 0 && moveCancelled == 0)
	{
		if(pause)
		{
			move_duration(MOVE_OFF, 2000);
			pause = !pause;
		}
		else
		{
			move_duration((rand_byte() % 6) + 1, 2000);
			pause = !pause;
		}
	}
	else if(moveCancelled == 0)
	{
		uint8_t r, g, b;
		get_rgb_sensor(&r, &g, &b);
		if(r == 255 && g < 10 && b < 10)
		{
			set_rgb_led(255, 0, 0);
			cancel_move();
			moveCancelled = 1;
		}
	}
}