/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_SEVEN
#define _DROPLET_CUSTOM_SEVEN

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomSeven : public DSimDroplet
{
private :

public :
	DropletCustomSeven(ObjectPhysicsData *objPhysics);
	~DropletCustomSeven(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif