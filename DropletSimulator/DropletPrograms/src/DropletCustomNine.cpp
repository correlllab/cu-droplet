#include "DropletCustomNine.h"

DropletCustomNine::DropletCustomNine(ObjectPhysicsData *objPhysics) 
	:   DSimDroplet(objPhysics),
        fh ( NULL )
{ return; }

DropletCustomNine::~DropletCustomNine() { if ( fh ) { fclose(fh); fh = NULL; } return; }

void DropletCustomNine::DropletInit()
{
	init_all_systems ();
	//char filename [50];
	//memset  ( filename, 0, 50 );
	//sprintf ( filename, "range_and_bearing_test_%u.txt", get_droplet_id());
	//fh = fopen ( filename, "w" );

 //   id_list.clear();
 //   state = ID;
 //   set_timer( 1000, 0 );
}
void DropletCustomNine::DropletMainLoop()
{
    //switch ( state )
    //{
    //case ID:
    //    if ( !check_timer(0) )
    //    {
    //        char empty_msg[1]; empty_msg[0] = '\0';
    //        ir_broadcast ( empty_msg, 1 );
    //        
    //        if ( check_for_new_messages() )
    //        {

    //            if ( std::find (
    //                id_list.begin(),
    //                id_list.end(),
    //                global_rx_buffer.sender_ID ) == id_list.end() )
    //            {
    //                id_list.push_back ( global_rx_buffer.sender_ID );
    //            }
    //        }
    //    }

    //    else
    //    {
    //        std::vector<droplet_id_type>::iterator min_it; 
    //        min_it = std::min_element(id_list.begin(), id_list.end());
    //        idle_droplet_id = *min_it;
    //        if ( idle_droplet_id > get_droplet_id() )
    //        {
    //            idle_droplet_id = get_droplet_id();
    //            state = IDLE;
    //        }
    //        else
    //        {
    //            state = MOVING;
    //        }
    //    }

    //    break;

    //case MOVING:
    //    if ( !is_moving() )
	   // {
    //        do_rnb();
    //        move_duration ( (rand_byte() % 6) + 1, 3000 );
	   // }
    //    break;

    //case IDLE:
    //    if ( check_timer (1) )
    //    {
    //        do_rnb();
    //        set_timer ( 3000, 1 );
    //    }

    //    break;
    //    
    //default:
    //    break;
    //}
}

//void DropletCustomNine::do_rnb()
//{
//    float dist, theta, phi;
//    uint8_t idx = rand_byte()%id_list.size();
//    range_and_bearing ( id_list[idx], &dist, &theta, &phi );
//    fprintf ( fh, "[%u --> %u] dist = %f, theta = %f, phi = %f\n",
//        get_droplet_id(),
//        id_list[idx], 
//        dist,
//        theta,
//        phi );
//}