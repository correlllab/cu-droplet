#include "DefaultPrograms/DropletTurnTest/DropletTurnTest.h"

DropletTurnTest::DropletTurnTest(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletTurnTest::~DropletTurnTest() { return; }

void DropletTurnTest::DropletInit()
{
	init_all_systems();
	set_rgb_led(0, 0, 255);

	move = 0;
}

void DropletTurnTest::DropletMainLoop()
{
	if(!is_rotating())
	{
		rotate_duration(TURN_CLOCKWISE, 30000);
	}
}