#include "DropletCustomSeven.h"

DropletCustomSeven::DropletCustomSeven(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomSeven::~DropletCustomSeven() { return; }

void DropletCustomSeven::DropletInit()
{
	init_all_systems();
	local_x = 0;

}

void DropletCustomSeven::DropletMainLoop()
{ 
}