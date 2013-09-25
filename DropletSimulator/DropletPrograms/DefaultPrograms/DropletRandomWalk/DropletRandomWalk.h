#pragma once

#ifndef _DROPLET_RANDOM_WALK
#define _DROPLET_RANDOM_WALK

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletRandomWalk : public IDroplet
{
private :

	// returns a random number between 0 and 255 (inclusive)
	uint8_t colorSet;

public :

	DropletRandomWalk(ObjectPhysicsData *objPhysics);
	~DropletRandomWalk();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif