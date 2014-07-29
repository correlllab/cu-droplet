/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_NINE
#define _DROPLET_CUSTOM_NINE

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomNine : public DSimDroplet
{
private :

public :
	DropletCustomNine(ObjectPhysicsData *objPhysics);
	~DropletCustomNine(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif