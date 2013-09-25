#include "DefaultPrograms/DropletMarch/DropletMarch.h"

DropletMarch::DropletMarch(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletMarch::~DropletMarch() { return; }

void DropletMarch::DropletInit()
{
	init_all_systems();

	set_rgb_led(0, 0, 255);

	colorSet = 0;
	currMoveDir = NORTH;
}

void DropletMarch::DropletMainLoop()
{
	
	if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == NORTH)
	{

		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(255, 0, 0);
		currMoveDir = SOUTH;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == SOUTH)
	{
		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(0, 255, 0);
		currMoveDir = NORTH_WEST;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == NORTH_EAST)
	{
		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(255, 0, 0);
		currMoveDir = SOUTH_WEST;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == SOUTH_WEST)
	{
		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(0, 255, 0);
		currMoveDir = NORTH;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == NORTH_WEST)
	{
		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(255, 0, 0);
		currMoveDir = SOUTH_EAST;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == SOUTH_EAST)
	{
		move_steps(currMoveDir, NUM_STEPS);
		set_rgb_led(0, 255, 0);
		currMoveDir = NORTH_EAST;
	}
	else if(is_moving() == 0 && is_rotating() == 0 && currMoveDir == MOVE_OFF)
	{
		rotate_steps(TURN_CLOCKWISE, NUM_STEPS);
		set_rgb_led(0, 255, 255);
		currMoveDir = NORTH;
		
	}
}