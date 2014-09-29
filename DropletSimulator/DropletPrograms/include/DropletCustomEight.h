/* *** PROGRAM DESCRIPTION ***
* This is the tile assembly model for making concentric sqaures
*/
#pragma once

#ifndef _DROPLET_CUSTOM_EIGHT
#define _DROPLET_CUSTOM_EIGHT

#define _USE_MATH_DEFINES

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>
//#include "stdafx.h"
#include <sstream>
#include <stdio.h>
//#include <math.h>
#include <string>
#include <iostream>
#include <time.h>
#include <limits.h>
#include <algorithm>    // std::sort
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>

//#define env				10
//#define robots			10
//#define center			5
#define DIR_MASK_N		0x1
#define DIR_MASK_NE		0x2
#define DIR_MASK_E		0x4
#define DIR_MASK_SE		0x8
#define DIR_MASK_S		0x10
#define DIR_MASK_SW		0x20
#define DIR_MASK_W		0x40
#define DIR_MASK_NW		0x80
#define ANGLED_DIR (DIR_MASK_NE|DIR_MASK_SE|DIR_MASK_NW|DIR_MASK_SW)

#define DROPLET_RADIUS (float)2.2 //assuming units are cm
#define FORMATION_GAP (float)0.25 //still assuming units are cm
#define PROXIMITY_THRESHOLD (float)(DROPLET_RADIUS/2.)
#define STUCK_DIST_THRESHOLD (float)0.01
#define ORIENT_THRESHOLD_DEG (float)1.0
#define MOVE_STEPS_AMOUNT 15
#define ROTATE_STEPS_AMOUNT 5
#define BIG_NUMBER 10000

#define IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0x55
#define CLAIM_MSG_TYPE (uint8_t)0xf0
#define NOT_IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0xaa
#define DELAY_BEFORE_DECIDING_MS 5000
#define DELAY_BEFORE_MOVING_MS 5000
#define SIDESTEPPING_DELAY_MS 10000
#define RANDOM_WALK_DELAY_MS 30000
#define BACK_UP_TIMER_DELAY_MS 30000
#define NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS 150000
#define WAIT_FOR_LAYER_DELAY 30000

#define TYPE__			0x0000
#define TYPE_SEED		0x0100

//#define TYPE_E			0x0001
//#define TYPE_W			0x0002
//#define TYPE_N			0x0004
//#define TYPE_S			0x0008
//#define TYPE_NE			0x0010
//#define TYPE_SE			0x0020
//#define TYPE_SW			0x0040
//#define	TYPE_NW			0x0080

//types for counter
#define TYPE_W00		0x0001
#define TYPE_W01		0x0002
#define TYPE_W10		0x0004
#define TYPE_W11		0x0008			
#define TYPE_N10		0x0010
#define TYPE_N11		0x0020
#define TYPE_W0			0x0040
#define ALL_W_TYPES			0x004F
#define ALL_N_TYPES			0x0030
#define DISPLAY_0_TYPES		(TYPE_W0|TYPE_SEED|TYPE_W00|TYPE_W11|TYPE_N11)
#define DISPLAY_1_TYPES		(TYPE_W01|TYPE_W10|TYPE_N10)
#define SEED_TYPE_VALUE 5 //width of number-1

#define STATE_WAITING_FOR_MSGS			0x00 //r__ (red)
#define STATE_ADJ_SPOTS_TO_BE_FILLED	0x01 //rgb (white)
#define STATE_ALL_ADJ_SPOTS_FILLED		0x02 //___ (off)
#define STATE_AWAITING_CONFIRMATION		0x04 //_gb (cyan)
#define STATE_PRE_ASSEMBLY				0x08 //_g_ (green)
#define STATE_MOVING_TO_SPOT			0x10 //r_b (pink)
#define STATE_DECIDING_SHOULD_MOVE		0x20 //rg_ (yellow)
#define STATE_ADJUSTING_PHI				0x80 //__b (blue)				

#define MOVING_NORMAL					0x00
#define MOVING_BACKING_UP				0x01
#define MOVING_SIDESTEPPING				0x02

#define STATE_IN_ASSEMBLY				0x07

#define CONF_REQ_MSG_TYPE (uint8_t) 0x66
#define FAV_TGT_MSG_TYPE (uint8_t) 0x11
#define NEIGHBOR_CALL_MSG_TYPE (uint8_t)0x0f
#define CONF_MSG_TYPE (uint8_t)0x33
#define CLAIM_MSG_TYPE (uint8_t)0xf0

struct confReqMsg{
	uint8_t type;
	droplet_id_type target;
	uint8_t dir;
};


struct favTgtMsg{
	uint8_t type;
	uint8_t dir;
	float dist;
	droplet_id_type id;
};


struct neighborCallMsg{
	uint8_t type;
	uint8_t dir_mask;
};


struct confMsg{
	uint8_t type;
	droplet_id_type target;
	uint16_t bot_type;
	int8_t value;
};

struct claimMsg{
	uint8_t type;
	droplet_id_type parent_id;
	uint8_t dir;
	uint16_t bot_type;
	int8_t bot_type_value;
};

struct recruitingRobot{
	uint8_t desiredNeighbors;
	float toNeighborDist[8];
	float toNeighborTheta[8];
	float range;
	float bearing;
	float heading;
};

struct botPos{
	float r;
	float theta;
	droplet_id_type id;
};

class DropletCustomEight : public DSimDroplet
{
private:
	std::map<droplet_id_type, recruitingRobot*> recruiting_robots;
	uint8_t state;
	FILE *file_handle;
	float closestDist;
	float last_move_r, last_move_theta, last_goal_r, last_move_dist;
	float side_step_angle;
	droplet_id_type closestID;
	droplet_id_type move_target;
	uint8_t closestDir;

	uint32_t moving_delay_start; //DELAY_BEFORE_MOVING_MS
	uint32_t neighbor_call_timeout_start; //NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS
	uint32_t backing_up_start; //BACK_UP_TIMER_DELAY_MS
	uint32_t backup_duration;
	uint32_t sidestep_start; //backup_duration;
	uint32_t deciding_delay_start; //DELAY_BEFORE_DECIDING_MS

	uint8_t move_target_dir;
	uint16_t my_type;
	int8_t my_type_value;
	uint8_t my_filled_spots;
	uint8_t moving_state;
	float avoid_target;
	uint32_t last_greater_val_time;

	//state functions
	void check_ready_to_move();
	void decide_if_should_move();
	void awaiting_confirmation();
	void adj_spots_to_fill();
	void handle_move_to_spot();
	void handle_adjusting_phi();
	void handle_pre_assembly();
	void waiting_for_message();

	//functions to edit to change the shape you get. (also edit SEED_TYPE_VALUE)
	void get_neighbor_type(uint16_t type, int8_t value, uint8_t dir, uint16_t* neighbor_type, int8_t* neighbor_value);
	uint8_t get_soft_spots_from_type(uint16_t type);
	uint8_t get_spots_from_type(uint16_t type);

	//helper/utility functions
	void handle_soft_confirm(droplet_id_type sender);
	void handle_hard_confirm(droplet_id_type sender, uint8_t dir);
	void get_dir_string_from_dir(uint8_t dir, char* dirStr);
	void add_recruiting_robot(droplet_id_type id, uint8_t dirs);
	void reset_before_waiting_for_msgs();
	void handle_rotate_to_straight(float theta);
	void call_for_neighbors();
	bool check_if_stuck(float delta, float last_delta);
	void maintain_position(droplet_id_type bot, uint8_t dir);
	void broadcast_favorite_target();
	void broadcast_claim_msg(droplet_id_type parent, uint8_t dir);
	uint8_t getDirMaskFromAngle(float theta);
	move_direction get_best_move_dir(float theta);
	void calculate_distance_to_target_positions();
	float getAngleFromDirMask(uint8_t dir_mask);
	void get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta);
	void add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths);
	void sub_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths);
	void remove_dir_from_spots_map(uint8_t dir, droplet_id_type id);
	void do_back_up();

	//quick math functions.
	float inline deg2rad(float deg);
	float inline rad2deg(float rad);
	float inline quick_and_dirty_mod(float theta);
	bool inline one_bit_set(uint16_t n);

public:
	DropletCustomEight(ObjectPhysicsData *objPhysics);
	~DropletCustomEight(void);

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif