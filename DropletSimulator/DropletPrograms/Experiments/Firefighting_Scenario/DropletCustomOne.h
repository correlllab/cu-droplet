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

#define MOVE_DIST_SCALAR            50
#define RED_THRESHOLD               200

#define WALKAWAY_TIME               15000   // in ms
#define START_DELAY_TIME            100     // in ms
#define COLLABORATE_DURATION        3000    // in ms

#define GROUP_MEMBERSHIP_TIMEOUT    2000    // in ms
#define HEART_RATE                  1000    // in ms

class DropletCustomOne : public DSimDroplet
{
private :
    
    uint32_t    start_delay_time;
    uint32_t    heartbeat_time;
    uint32_t    voting_time;
    uint32_t    collab_time;
    uint32_t    last_update_time;
    uint8_t     last_move_dir;
    double      tau, theta;

    std::map<droplet_id_type, std::pair<uint32_t, bool>> unique_ids;

    enum State
    {
        COLLABORATING,
        LEAVING,
        SAFE,
        SEARCHING,
        START_DELAY,
        WAITING
    } state;

    bool check_safe         ( void );
    void send_heartbeat     ( void );
    void update_group_size  ( void );
    void check_votes        ( void );
    bool roll_sigmoid       ( void );
    void random_walk        ( void );
    void change_state       ( State new_state );

public :
	DropletCustomOne(ObjectPhysicsData *objPhysics);
	~DropletCustomOne(void);
	
	void DropletInit(void);
	void DropletMainLoop(void);
};