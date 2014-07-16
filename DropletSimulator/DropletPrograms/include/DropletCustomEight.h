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
#include <math.h>
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
#include <math.h>
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
#define FORMATION_GAP (float)0.2 //still assuming unist are cm
#define PROXIMITY_THRESHOLD (float)0.1

#define START_INDICATOR_BYTE (uint8_t)0x3c
#define IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0x55
#define NOT_IN_ASSEMBLY_INDICATOR_BYTE (uint8_t)0xaa
#define DELAY_BEFORE_DECIDING_MS 3000
#define DELAY_BEFORE_MOVING_MS 3000
#define START_DELAY_MS 3000
#define START_DELAY_TIMER 0
#define DECIDING_DELAY_TIMER 1
#define MOVING_DELAY_TIMER 2

#define STATE_ADJ_SPOTS_TO_BE_FILLED	0x01
#define STATE_ALL_ADJ_SPOTS_FILLED		0x02
#define STATE_AWAITING_CONFIRMATION		0x04
#define STATE_MOVING_TO_SPOT			0x08
#define STATE_DECIDING_SHOULD_MOVE		0x10
#define STATE_START						0x20

#define STATE_IN_ASSEMBLY				0x07

struct recruitingRobot{
	uint8_t desiredNeighbors;
	float toNeighborDist[8];
	float toNeighborTheta[8];
	float range;
	float bearing;
	float heading;
};

class DropletCustomEight : public DSimDroplet
{
private :
	std::map<droplet_id_type, recruitingRobot> recruiting_robots;
	uint8_t state;
	//FILE *file_handle;
	float closestDist;
	droplet_id_type closestID;
	uint8_t closestDir;
	droplet_id_type move_target;
	uint8_t move_target_dir;
	uint8_t my_spots_to_fill;

	void handle_start_broadcast();
	void handle_done_deciding();
	void check_ready_to_move();
	void awaiting_confirmation();
	void adj_spots_to_fill();
	void move();
	void decide_if_should_move();
	void waiting_for_message();

	void broadcast_favorite_target();
	float get_distance(float rA, float thetaA, float rB, float thetaB);
	uint8_t get_spots_from_pos(uint8_t dir_pos);
	move_direction get_best_move_dir(float theta);
	void calculate_distance_to_target_positions();
	float getAngleFromDirMask(uint8_t dir_mask);
	void get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta);
	void add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths);

public :
	DropletCustomEight(ObjectPhysicsData *objPhysics);
	~DropletCustomEight(void);
	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif