#pragma once

#ifndef _DROPLET_CUSTOM_SEVEN
#define _DROPLET_CUSTOM_SEVEN

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomSeven : public IDroplet
{
private :

public :
	DropletCustomSeven(ObjectPhysicsData *objPhysics);
	~DropletCustomSeven(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif