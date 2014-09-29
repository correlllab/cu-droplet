#include "DropletCustomOne.h"

DropletCustomOne::DropletCustomOne(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomOne::~DropletCustomOne() { if ( fp ) { fclose(fp); fp = NULL; } return; }

void DropletCustomOne::DropletInit()
{
    init_all_systems    ();

    id_list.clear       ();
//    id_list.push_back   ( get_droplet_id() );

    state       = MAKE_ID_LIST;
    init_time   = get_32bit_time ();
    send_time   = get_32bit_time ();
    wait_time   = get_32bit_time ();

    char filename [64];
    memset ( filename, 0, 64 );
    sprintf ( filename, "%u.txt", get_droplet_id() );
    fp = fopen ( filename, "w" );
}

void DropletCustomOne::DropletMainLoop()
{
	switch ( state )
    {
    case MAKE_ID_LIST:
        if ( get_32bit_time() - init_time < ID_LIST_TIME )
        {
            if ( get_32bit_time() - send_time >= SEND_TIME )
            {
                send_time = get_32bit_time ();
                ir_broadcast ( "hi", 2 );
            }
            if ( check_for_new_messages () )
            {
                if ( std::find(id_list.begin(), id_list.end(), global_rx_buffer.sender_ID) == id_list.end() )
                {
                    id_list.push_back ( global_rx_buffer.sender_ID );
                }
           
                global_rx_buffer.read = 1;
            }
        }
        else
        {
            while ( check_for_new_messages() );
            state = RNB;
        }
        break;

    case RNB:
        if ( get_32bit_time() - wait_time >= WAIT_TIME )
        {
            wait_time = get_32bit_time();
            if ( id_list.size() > 0 )
            {
                float dist, theta, tau;
                droplet_id_type partner_id = id_list[rand_byte() % id_list.size()];
                range_and_bearing ( partner_id, &dist, &theta, &tau );
                fprintf ( fp, "[%u --> %u] dist=%f tau=%f theta=%f\n", get_droplet_id(), partner_id, dist, theta, tau );
                fflush  ( fp );
            }
        }
        break;
    }
}