#pragma once

#ifndef _DROPLET_POWER_TEST
#define _DROPLET_POWER_TEST

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>


class DropletPowerTest : public IDroplet
{
private :

	uint8_t move;

public :

	DropletPowerTest(ObjectPhysicsData *objPhysics);
	~DropletPowerTest();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif