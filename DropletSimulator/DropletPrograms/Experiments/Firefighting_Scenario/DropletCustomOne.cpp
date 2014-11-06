#include "DropletCustomOne.h"

DropletCustomOne::DropletCustomOne(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics)
{ return; }

DropletCustomOne::~DropletCustomOne() { return; }

void DropletCustomOne::DropletInit()
{
    init_all_systems ();
    unique_ids.clear ();
    start_delay_time    = 0;
    heartbeat_time      = 0;
    voting_time         = 0;

    tau                 = 10.0;
    theta               = 1.0;

    change_state    ( START_DELAY );
}

void DropletCustomOne::DropletMainLoop()
{
    switch ( state )
    {
    case COLLABORATING:
        if ( get_32bit_time() - collab_time > COLLABORATE_DURATION )
        {
            change_state ( LEAVING );
        }
        break;

    case LEAVING:
        if ( !is_moving(NULL) )
        {
            change_state ( SAFE );
        }
        break;

    case SAFE:
        if ( check_safe () )
        {
            change_state ( SEARCHING );
        }

        // If you start in the red region then try to get out before you die
        else
        {
            random_walk ();
        }
        break;

    case SEARCHING:
        random_walk ();
        if ( !check_safe() )
        {
            change_state ( WAITING );
        }
        break;

    case START_DELAY:
        {
            uint32_t curr_time = get_32bit_time ();
            if ( curr_time - start_delay_time > START_DELAY_TIME )
                change_state ( SAFE );
        }
        break;

    case WAITING:
        if ( get_32bit_time() - heartbeat_time > HEART_RATE )
        {
            heartbeat_time = get_32bit_time ();
            send_heartbeat ();
        }

        // Checks incoming messages and updates group size.
        // There is a chance the state can be changed to COLLABORATING in
        // this function if the droplet sees a GO message.
        update_group_size ();

        if (    get_32bit_time() - voting_time > HEART_RATE &&
                state == WAITING )
        {
            voting_time = get_32bit_time ();
            check_votes ();
        }

        break;

    default:
        break;
    }
}

bool DropletCustomOne::check_safe ( void )
{
    if ( get_red_sensor() > RED_THRESHOLD )
    {
        return false;
    }

    return true;
}

void DropletCustomOne::send_heartbeat ( void )
{
    bool sig_val = roll_sigmoid();
    if ( sig_val )
    {
        ir_broadcast ( "<3Y", 3 );
    }
    else
    {
        ir_broadcast ( "<3N", 3 );
    }

    std::pair<uint32_t, bool> my_vote = std::make_pair ( 0, sig_val );
    unique_ids[get_droplet_id()] = my_vote;
}

void DropletCustomOne::update_group_size ( void )
{
    // Clear out-of-date data
    std::map<droplet_id_type, std::pair<uint32_t, bool>>::iterator it = unique_ids.begin();
    while ( it != unique_ids.end() )
    {
        std::pair<uint32_t, bool> old_data = it->second;
        std::pair<uint32_t, bool> new_data = std::make_pair
            (   old_data.first + (get_32bit_time() - last_update_time),
                old_data.second );
        if ( new_data.first > GROUP_MEMBERSHIP_TIMEOUT )
        {
            unique_ids.erase(it++);
        }
        else
        {
            unique_ids[it->first] = new_data;
            it++;
        }
    }
    

    // Update group size based on incoming messages
    while ( check_for_new_messages() )
    {
        char    in_msg[4];
        memset  ( in_msg, 0, 4 );
        memcpy  ( in_msg, global_rx_buffer.buf, global_rx_buffer.data_len );

        if ( strcmp(in_msg, "GO") == 0 )
        {
            change_state ( COLLABORATING );
            return;
        }
        else if ( strstr(in_msg, "<3") != NULL )
        {
            std::pair<uint32_t, bool> new_droplet_data = std::make_pair
                (   0,
                    in_msg[2] == 'Y' );

            unique_ids[global_rx_buffer.sender_ID] = new_droplet_data;
        }

        // Mark the message in the global receive buffer as read
        global_rx_buffer.read = 1;
    }

    last_update_time = get_32bit_time ();
}

void DropletCustomOne::check_votes ( void )
{
    uint32_t yes_votes = 0;

    std::map<droplet_id_type, std::pair<uint32_t, bool>>::iterator it = unique_ids.begin();
    while ( it != unique_ids.end() )
    {
        std::pair<uint32_t, bool> vote_data = it->second;
        if ( vote_data.second )
        {
            yes_votes++;
        }
        it++;
    }

    if ( yes_votes * 2 > unique_ids.size() )
    {
        ir_broadcast ( "GO", 2 );
        change_state ( COLLABORATING );
    }
}

bool DropletCustomOne::roll_sigmoid ( void )
{
    double rand_val = rand_byte() / 256.0;
    double sig_val  = 1.0 / ( 1.0 + std::pow(M_E, theta * (tau - unique_ids.size())) );

    return (rand_val <= sig_val);
}

void DropletCustomOne::random_walk ( void )
{
    if ( !is_moving(NULL) )
    {
        // Either walk or rotate
        if ( rand_byte() % 3 == 0 )
        {
            int8_t  sign = rand_byte() % 2 ? 1 : -1;
            uint8_t degrees;
            while ( (degrees = rand_byte()) <= 90 );
            rotate_degrees ( sign * degrees );
        }
        else
        {
            last_move_dir = rand_byte() % 6;
            move_duration ( last_move_dir, rand_byte() * MOVE_DIST_SCALAR );
        }
    }
}

void DropletCustomOne::change_state ( State new_state )
{
    state = new_state;
    switch ( state )
    {
    case COLLABORATING:
        set_rgb_led         ( 0, 0, 250 );
        collab_time         = get_32bit_time ();
        break;

    case LEAVING:
        set_rgb_led         ( 0, 250, 250 );
        move_duration       ( (last_move_dir + 3) % 6, WALKAWAY_TIME );

    case SAFE:
        set_rgb_led         ( 255, 255, 0 );
        break;

    case START_DELAY:
        start_delay_time    = get_32bit_time ();
        break;

    case WAITING:
        cancel_move         ();
        unique_ids.clear    ();
        set_rgb_led         ( 0, 255, 0 );

        // Clear out all messages in buffer first.
        while               ( check_for_new_messages() );
        heartbeat_time      = get_32bit_time ();
        voting_time         = get_32bit_time ();
        send_heartbeat      ();
        break;

    case SEARCHING:
    default:
        reset_rgb_led       ();
    }
}