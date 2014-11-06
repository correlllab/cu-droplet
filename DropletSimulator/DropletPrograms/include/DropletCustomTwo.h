/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>

#include <inttypes.h>
#include <map>
#include <utility>
#include <cstdlib>
#include <cstring>
#define _USE_MATH_DEFINES
#include <math.h>

#define COLLABORATE_DURATION        3000    // in ms

#define GROUP_MEMBERSHIP_TIMEOUT    2000    // in ms
#define HEART_RATE                  1000    // in ms
#define EXP_LENGTH                  30 * 60 * 1000 // 30 min in ms
#define NUM_THETA                   3
#define NUM_TAU                     5


class DropletCustomTwo : public DSimDroplet
{
private :
    
    uint32_t    heartbeat_time;
    uint32_t    voting_time;
    uint32_t    collab_time;
    uint32_t    last_update_time;
    uint32_t    exp_time;
    uint8_t     theta_id, tau_id;
    uint32_t    num_collabs;

    static const double theta [ NUM_THETA ];
    static const double tau   [ NUM_TAU   ];

    FILE *fp;

    std::map<droplet_id_type, std::pair<uint32_t, bool>> unique_ids;

    enum State
    {
        COLLABORATING,
        WAITING,
        DONE
    } state;

    void send_heartbeat     ( void );
    void update_group_size  ( void );
    void check_votes        ( void );
    bool roll_sigmoid       ( void );
    void change_state       ( State new_state );

public :
	DropletCustomTwo(ObjectPhysicsData *objPhysics);
	~DropletCustomTwo(void);
	
	void DropletInit(void);
	void DropletMainLoop(void);
};