#include "DropletCustomNine.h"

DropletCustomNine::DropletCustomNine(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomNine::~DropletCustomNine() { return; }

void DropletCustomNine::DropletInit()
{
	init_all_systems ();
	fh = fopen ( "testing.txt", "w" );
}

void DropletCustomNine::DropletMainLoop()
{
	if ( check_timer(0) )
	{
		set_timer ( 1000, 0 );

		char *test = "Hi";
		ir_broadcast ( test, 2 );
	}

	if ( check_for_new_messages() )
	{
		if ( fh != NULL)
		{
			fprintf ( 
				fh, 
				"[%u] global_rx_buffer.data_len = %u\n",
				get_droplet_id(),
				global_rx_buffer.data_len);
			fflush(fh);
		}

		global_rx_buffer.read = 1;
	}
}