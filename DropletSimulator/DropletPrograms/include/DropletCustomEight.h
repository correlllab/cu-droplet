/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_EIGHT
#define _DROPLET_CUSTOM_EIGHT

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomEight : public IDroplet
{
private :

public :
	DropletCustomEight(ObjectPhysicsData *objPhysics);
	~DropletCustomEight(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif