#pragma once

#ifndef _DROPLET_RGB_SENSE
#define _DROPLET_RGB_SENSE

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletRGBSense : public IDroplet
{
private :

	uint8_t moveCancelled;
	uint8_t pause;

public :

	DropletRGBSense(ObjectPhysicsData *objPhysics);
	~DropletRGBSense();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif