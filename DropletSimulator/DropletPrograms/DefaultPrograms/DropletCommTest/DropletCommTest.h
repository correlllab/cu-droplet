#pragma once

#ifndef _DROPLET_COMM_TEST
#define _DROPLET_COMM_TEST

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCommTest : public IDroplet
{
private :

public :
	DropletCommTest(ObjectPhysicsData *objPhysics);
	~DropletCommTest(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif