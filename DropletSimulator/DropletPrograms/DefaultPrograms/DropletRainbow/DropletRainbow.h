#pragma once

#ifndef _DROPLET_RAINBOW
#define _DROPLET_RAINBOW
#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

class DropletRainbow : public IDroplet
{
private :

	// returns a random number between 0 and 255 (inclusive)
	float rs, gs, bs;
	float rt, gt, bt;
	float pi2;
	uint8_t r, g, b;

public :

	DropletRainbow(ObjectPhysicsData *objPhysics);
	~DropletRainbow();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif