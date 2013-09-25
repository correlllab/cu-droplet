#pragma once

#ifndef _DROPLET_MARCH
#define _DROPLET_MARCH

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

#define NUM_STEPS 1000 

class DropletMarch : public IDroplet
{
private :

	// returns a random number between 0 and 255 (inclusive)
	uint8_t colorSet;
	move_direction currMoveDir;

public :

	DropletMarch(ObjectPhysicsData *objPhysics);
	~DropletMarch();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif