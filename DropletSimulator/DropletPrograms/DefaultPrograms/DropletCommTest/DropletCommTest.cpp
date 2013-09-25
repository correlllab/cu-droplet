#include "DefaultPrograms/DropletCommTest/DropletCommTest.h"

DropletCommTest::DropletCommTest(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletCommTest::~DropletCommTest() { return; }

void DropletCommTest::DropletInit()
{
	init_all_systems(); // Needs to be called. communication in sim does not work o/w
	set_red_led(255);
	set_timer(2000, 0);
}

void DropletCommTest::DropletMainLoop()
{
	// The following code simulates sending a msg and then waiting
	if(check_timer(0))
	{
		uint8_t msg = rand_byte();
		ir_broadcast((char *)&msg, sizeof(uint8_t));

		set_timer(2000, 0);
	}
	else
	{
		if(check_for_new_messages() == 1)
		{
			uint8_t color = 0;
			memcpy(&color, global_rx_buffer.buf, sizeof(uint8_t));
			set_rgb_led(0, color, color);
		}
	}
}