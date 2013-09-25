#include "DefaultPrograms/DropletRainbow/DropletRainbow.h"

DropletRainbow::DropletRainbow(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletRainbow::~DropletRainbow() { return; }

void DropletRainbow::DropletInit()
{
	rs = 0.01f;
	gs = 0.01f;
	bs = 0.01f;
	
	rt = 0.f;
	gt = 2.1f;
	bt = 4.2f;
	pi2 = (float)M_PI * 2.0f;

	init_all_systems();
}

void DropletRainbow::DropletMainLoop()
{
	r = (uint8_t)(127.0f * sinf(rt) + 127.0f);
	g = (uint8_t)(127.0f * sinf(gt) + 127.0f);
	b = (uint8_t)(127.0f * sinf(bt) + 127.0f);
	
	if(rt >= pi2)
		rt = 0.001f;
	else
		rt += rs;

	if(gt >= pi2)
		gt = 0.001f;
	else
		gt += gs; 
	
	if(bt >= pi2)
		bt = 0.001f;
	else
		bt += bs;

	set_rgb_led(r, g, b);
}