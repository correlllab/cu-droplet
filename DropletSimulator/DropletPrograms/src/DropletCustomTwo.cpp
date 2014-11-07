#include "DropletCustomTwo.h"

const double DropletCustomTwo::theta [] = { 0.1, 1.0, 10.0 };
const double DropletCustomTwo::tau   [] = { 2, 3, 4, 5, 6  };

DropletCustomTwo::DropletCustomTwo(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics)
{ return; }

DropletCustomTwo::~DropletCustomTwo() { if ( fp ) { fclose (fp); } return; }

void DropletCustomTwo::DropletInit()
{
    init_all_systems    ();
    unique_ids.clear    ();
    heartbeat_time      = 0;
    voting_time         = 0;
    exp_time            = get_32bit_time ();
    theta_id            = 0;
    tau_id              = 0;
    change_state        ( WAITING );

    num_collabs         = 0;
    char    filename[32];
    memset  ( filename, 0, 32 );
    sprintf ( filename, "out_%u.txt", get_droplet_id() );
    fp      = fopen ( filename, "w"  );
    fprintf ( fp, "time, theta, tau, collab\n" );
}

void DropletCustomTwo::DropletMainLoop()
{
    if ( (state != DONE) && (get_32bit_time() - exp_time >= EXP_LENGTH) )
    {
        exp_time = get_32bit_time ();
        if ( fp )
        {
            fprintf ( fp, "%u, %f, %f, %u\n",
                exp_time,
                theta[theta_id],
                tau[tau_id],
                num_collabs);
            num_collabs = 0;
        }

        if ( ((theta_id + 1) * (tau_id + 1)) == (NUM_THETA * NUM_TAU) )
        {
            change_state ( DONE );
            if ( fp )
            {
                fclose ( fp );
                fp = NULL;
            }
        }
        else 
        {
            if ( !(++tau_id % NUM_TAU) )
            {
                tau_id = 0;
                ++theta_id;
            }

            change_state ( WAITING );
        }
    }

    switch ( state )
    {
    case COLLABORATING:
        if ( get_32bit_time() - collab_time > COLLABORATE_DURATION )
        {
            change_state ( WAITING );
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

    case DONE:
    default:
        break;
    }
}

void DropletCustomTwo::send_heartbeat ( void )
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

void DropletCustomTwo::update_group_size ( void )
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

void DropletCustomTwo::check_votes ( void )
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

    if ( (unique_ids.size() >= 2) && (yes_votes * 2 > unique_ids.size()) )
    {
        ir_broadcast ( "GO", 2 );
        change_state ( COLLABORATING );
    }
}

bool DropletCustomTwo::roll_sigmoid ( void )
{
    double rand_val = rand_byte() / 256.0;
    double sig_val  = 1.0 / ( 1.0 + std::pow(M_E, theta[theta_id] * (tau[tau_id] - unique_ids.size())) );

    return (rand_val <= sig_val);
}

void DropletCustomTwo::change_state ( State new_state )
{
    state = new_state;
    switch ( state )
    {
    case COLLABORATING:
        set_rgb_led         ( 0, 0, 250 );
        collab_time         = get_32bit_time ();
        num_collabs++;
        break;

    case WAITING:
        unique_ids.clear    ();
        set_rgb_led         ( 0, 250, 0 );

        // Clear out all messages in buffer first.
        while               ( check_for_new_messages() );
        heartbeat_time      = get_32bit_time ();
        voting_time         = get_32bit_time ();
        send_heartbeat      ();
        break;

    case DONE:
    default:
        reset_rgb_led       ();
    }
}