#pragma once

#ifndef _DROPLET_GRANOLA
#define _DROPLET_GRANOLA

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletGranola : public IDroplet
{
private :
	uint8_t r, g, b, new_g, new_b, wait;
	bool direction_flag, move_flag, wiggle_flag;

public :
	DropletGranola(ObjectPhysicsData *objPhysics);
	~DropletGranola(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif