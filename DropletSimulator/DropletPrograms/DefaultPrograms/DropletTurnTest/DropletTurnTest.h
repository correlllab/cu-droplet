#pragma once

#ifndef _DROPLET_TURN_TEST
#define _DROPLET_TURN_TEST

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletTurnTest : public IDroplet
{
private :

	uint8_t move;

public :

	DropletTurnTest(ObjectPhysicsData *objPhysics);
	~DropletTurnTest();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif