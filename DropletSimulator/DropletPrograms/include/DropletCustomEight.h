/* *** PROGRAM DESCRIPTION ***
 * 
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

#define DROPLET_RADIUS (float)2.2 //assuming units are cm
#define FORMATION_GAP (float)0.25 //still assuming units are cm
#define PROXIMITY_THRESHOLD (float)0.5
#define STUCK_DIST_THRESHOLD (float)0.01
#define ORIENT_THRESHOLD_DEG (float)3.0
#define MOVE_STEPS_AMOUNT 15
#define ROTATE_STEPS_AMOUNT 5
#define BIG_NUMBER 10000
#define SEED_TYPE_VALUE 7

#define START_INDICATOR_BYTE (uint8_t)0x3c
#define IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0x55
#define CLAIM_MSG_TYPE (uint8_t)0xf0
#define NOT_IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0xaa
#define GO_INDICATOR_BYTE (uint8_t)0xc3
#define DELAY_BEFORE_DECIDING_MS 4000
#define DELAY_BEFORE_MOVING_MS 10000
#define START_DELAY_MS 2000
#define SIDESTEPPING_DELAY_MS 5000
#define RANDOM_WALK_DELAY_MS 30000
#define START_DELAY_TIMER 0
#define DECIDING_DELAY_TIMER 1
#define MOVING_DELAY_TIMER 2
#define SIDESTEP_TIMER 3
#define RANDOM_WALK_TIMER 5
#define BACK_UP_TIMER 6
#define BACK_UP_TIMER_DELAY_MS 30000
#define NEIGHBOR_CALL_TIMEOUT_TIMER 4
#define NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS 300000

#define TYPE__	0x00
#define TYPE_A	0x01
#define TYPE_B	0x02
#define TYPE_S	0x04

#define STATE_WAITING_FOR_MSGS			0x00 //r__ (red)
#define STATE_ADJ_SPOTS_TO_BE_FILLED	0x01 //rgb (white)
#define STATE_ALL_ADJ_SPOTS_FILLED		0x02 //___ (off)
#define STATE_AWAITING_CONFIRMATION		0x04 //_gb (cyan)
#define STATE_MOVING_TO_CENTER			0x08 //_g_ (green)
#define STATE_MOVING_TO_SPOT			0x10 //r_b (pink)
#define STATE_DECIDING_SHOULD_MOVE		0x20 //rg_ (yellow)
#define STATE_START						0x40 //___ (off)
#define STATE_ADJUSTING_PHI				0x80 //__b (blue)				

#define MOVING_NORMAL					0x00
#define MOVING_BACKING_UP				0x01
#define MOVING_SIDESTEPPING				0x02

#define STATE_IN_ASSEMBLY				0x0f

struct recruitingRobot{
	uint8_t desiredNeighbors;
	float toNeighborDist[8];
	float toNeighborTheta[8];
	float range;
	float bearing;
	float heading;
};

struct favTgtMsg{
	uint8_t type;
	uint8_t dir;
	float dist;
	droplet_id_type id;
};

struct claimMsg{
	uint8_t type;
	droplet_id_type parent_id;
	uint8_t dir;
};

struct botPos{
	float r;
	float theta;
	droplet_id_type id;
};


class DropletCustomEight : public DSimDroplet
{
private :
	std::map<droplet_id_type, recruitingRobot*> recruiting_robots;
	std::vector<botPos*> other_bots;
	uint8_t state;
	FILE *file_handle;
	float closestDist;
	float last_move_r;
	float last_move_theta;
	float last_goal_r;
	float last_move_dist;
	float side_step_angle;
	droplet_id_type closestID;
	uint8_t closestDir;
	droplet_id_type move_target;
	uint8_t move_target_dir;
	uint8_t my_type;
	uint8_t my_type_value;
	uint8_t smallest_master_value;
	uint8_t my_filled_spots;
	uint8_t moving_state;
	bool got_go_from_parent;
	float avoid_target;
	droplet_id_type min_id;

	//state functions
	void handle_start_broadcast();
	void handle_move_to_center();
	void check_ready_to_move();
	void decide_if_should_move();
	void awaiting_confirmation();
	void adj_spots_to_fill();
	void handle_move_to_spot();
	void handle_adjusting_phi();
	void waiting_for_message();

	//functions to edit to change the shape you get. (also edit SEED_TYPE_VALUE)
	bool check_if_slave(uint8_t slave_q, uint8_t master_q);
	void get_neighbor_type(uint8_t type, uint8_t value, uint8_t dir, uint8_t* neighbor_type, uint8_t* neighbor_value);
	uint8_t get_spots_from_type(uint8_t type);

	//helper/utility functions.
	void reset_before_waiting_for_msgs();
	void handle_rotate_to_straight(float theta);
	void call_for_neighbors();
	bool check_if_stuck(float delta, float last_delta);
	void maintain_position(droplet_id_type bot, uint8_t dir);
	void broadcast_favorite_target();
	void broadcast_claim_msg(droplet_id_type parent, uint8_t dir);
	float get_distance(float rA, float thetaA, float rB, float thetaB);
	move_direction get_best_move_dir(float theta);
	void calculate_distance_to_target_positions();
	float getAngleFromDirMask(uint8_t dir_mask);
	void get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta);
	void add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths);
	float sub_polar_vec_mag(float r1, float th1, float r2, float th2);
	void remove_dir_from_spots_map(uint8_t dir, droplet_id_type id);
	void do_back_up();

	//quick math functions.
	float inline deg2rad(float deg);
	float inline rad2deg(float rad);
	float inline quick_and_dirty_mod(float theta);

public :
	DropletCustomEight(ObjectPhysicsData *objPhysics);
	~DropletCustomEight(void);
	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif