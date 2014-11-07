#include "DropletCustomOne.h"

DropletCustomOne::DropletCustomOne(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomOne::~DropletCustomOne() { return; }

void DropletCustomOne::DropletInit()
{
    init_all_systems();
}

void DropletCustomOne::DropletMainLoop()
{
    if ( !is_moving(NULL) )
    {
        switch ( rand_byte() % 3 )
        {
        case 0:
            {
                int8_t sign = rand_byte() % 2 ? 1 : -1;
                rotate_degrees ( 90 * sign );
                set_rgb_led ( 50, 0, 0 );
            }
            break;

        case 1:
            move_duration ( rand_byte() % 6, 3000 );
            set_rgb_led ( 0, 50, 0 );
            break;

        case 2:
            move_steps ( rand_byte() % 6, 100 );
            set_rgb_led ( 0, 0, 50 );
            break;

        default:
            break;
        }
        
    }
}