/**
 * \file	cu-droplet\DropletSimulator\DropletSimLibrary\include\DSimGlobals.h
 *
 * \brief	This file contains global variables.
 */

#pragma once

#ifndef _DROPLET_SIM_GLOBALS
#define _DROPLET_SIM_GLOBALS

#ifdef _WIN32
#include "inttypes.h"   // inttypes.h is included in UNIX systems
#else 
#include <inttypes.h>
#endif


#define SAFE_DELETE(x) { if(x) { delete x; x = NULL; } }

// HARDWARECODE begin

/**
 * \typedef	unsigned char DS_RESULT
 *
 * \brief	Simulator Error Codes
 *
 */

#define DS_SUCCESS	0
#define DS_WARNING	1
#define DS_ERROR	2
#define DS_FATAL	3
typedef unsigned char DS_RESULT;

/**
 * \typedef	uint8_t move_direction
 *
 * \brief	droplet move directions
 *
 */
#define NORTH		0
#define NORTH_EAST	1
#define SOUTH_EAST	2
#define SOUTH		3
#define SOUTH_WEST	4
#define NORTH_WEST	5
#define TURN_CLOCKWISE			6
#define TURN_COUNTERCLOCKWISE	7
#define TURN_ANTICLOCKWISE		7
typedef uint8_t move_direction;

// Droplet Communication Error Codes

/**
 * \def		IR_RX_STATUS_BUSY
 *
 * \brief	a macro that defines IR RX status busy
 *
 */

#define IR_RX_STATUS_BUSY	1

// Droplet Constants

/**
 * \typedef	uint16_t droplet_id_type
 *
 * \brief	droplet ID types.
 *
 */
#define DROPLET_ID_START		100 // Droplet ids will start incrementing from this number
typedef uint16_t droplet_id_type;

#define BANDS_PER_TILE 11.f

#define STAGGERED_START false

// Droplet Communication Constants
#define OLDEST_MSG_FIRST 0
#define NEWEST_MSG_FIRST 1
typedef uint8_t msg_order;

#define NUM_COMM_CHANNELS 100

#define DROPLET_REL_POS_UPDATE_TIME .1f // seconds
#define BROADCAST_THRESHOLD	50.f // cm

#define IR_MAX_DATA_SIZE	72
#define IR_BUFFER_SIZE		sizeof(droplet_id_type) + IR_MAX_DATA_SIZE

// Physics Constants
#define PHYSICS_GRAVITY			9.8f // Bullet seems to only accept gravity in units of m/s^2
#define IMPULSE_SCALING			2.f // scale this to match hardware speed
#define MOTOR_POS_SCALING		.95f  // Since the motors are not located right on the 
									  // outer edge of the droplet
#define MOTOR_ERROR				.05f  // Motors have a 5% variance per step.
#define FLOOR_FRICTION			.5f   // experimentally determined

// Damping values set to simulate material elasticity for collisions and reduce sliding
#define DROPLET_LINEAR_DAMPING		.9f
#define DROPLET_ANGULAR_DAMPING		.9f

#define OBJECT_LINEAR_DAMPING		.9f
#define OBJECT_ANGULAR_DAMPING		.9f

// Times constants
#define ROTATE_RATE_MS_PER_DEG 26
#define WALK_STEP_TIME 1000.f / 60.f

// HARDWARECODE end
#endif
