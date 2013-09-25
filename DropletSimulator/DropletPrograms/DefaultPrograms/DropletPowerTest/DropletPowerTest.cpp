#include "DefaultPrograms/DropletPowerTest/DropletPowerTest.h"

DropletPowerTest::DropletPowerTest(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletPowerTest::~DropletPowerTest() { return; }

void DropletPowerTest::DropletInit()
{
	init_all_systems();
	set_rgb_led(0, 0, 255);

	move = 0;
}

void DropletPowerTest::DropletMainLoop()
{
	if(!is_rotating())
	{
		rotate_duration(TURN_CLOCKWISE, 3000);
		move_duration(SOUTH_EAST, 3000);
	}
	// one leg on power
	if(leg1_status() == 1 && leg2_status() == -1 && leg3_status() == -1)
		set_rgb_led(255, 0, 0);
	if(leg2_status() == 1  && leg1_status() == -1 && leg3_status() == -1)
		set_rgb_led(0, 255, 0);
	if(leg3_status() == 1  && leg1_status() == -1 && leg2_status() == -1)
		set_rgb_led(0, 0, 255);
	// two legs on power
	if(leg1_status() == 1 && leg2_status() == 1 && leg3_status() == -1)
		set_rgb_led(125, 125, 0);
	if(leg1_status() == 1 && leg3_status() == 1 && leg2_status() == -1)
		set_rgb_led(125, 0, 125);
	if(leg2_status() == 1 && leg3_status() == 1 && leg1_status() == -1)
		set_rgb_led(0, 125, 125);
	// the following cases should not occur
	if(leg1_status() == 1 && leg2_status() == 1 && leg3_status() == 1)
		set_rgb_led(255, 255, 255);
	if(leg1_status() == -1 && leg2_status() == -1 && leg3_status() == -1)
		set_rgb_led(0, 0, 0);
}