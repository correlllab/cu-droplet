#pragma once

#ifndef _DROPLET_ANTS
#define _DROPLET_ANTS

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletAnts : public IDroplet
{
private :
	// phases of the behaviour:
	// 0-1 - searching for trail or object
	// 2 - orienting north
	// 3 - waiting for droplets
	// 4 - moving object
	// 5 - done
	int phase;
	// actions during each phase
	// 0 - move
	// 1 - rotate
	int action;
	// 1 - clockwise
	// -1 - counter clockwise
	turn_direction turnDir;
	uint8_t r, g, b;
	uint8_t new_r, new_g, new_b;

	// turns to orient north
	uint8_t turns_left;

	// comm vars
	bool cleared_buff;
	uint8_t group_size, group_thresh;
	uint16_t gpx2;
	droplet_id_type *recv_ids;

public :
	DropletAnts(ObjectPhysicsData *objPhysics);
	~DropletAnts(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif