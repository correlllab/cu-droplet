#pragma once

#ifndef _DROPLET_CUSTOM_NINE
#define _DROPLET_CUSTOM_NINE

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomNine : public IDroplet
{
private :

public :
	DropletCustomNine(ObjectPhysicsData *objPhysics);
	~DropletCustomNine(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif