/* *** PROGRAM DESCRIPTION ***
 * Swarm collaboration using sigmoid function.
 */
#pragma once

#ifndef _DROPLET_CUSTOM_TWO
#define _DROPLET_CUSTOM_TWO

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>
#include <stdlib.h>

class DropletCustomTwo : public IDroplet
{
private :
	

public :
	DropletCustomTwo(ObjectPhysicsData *objPhysics);
	~DropletCustomTwo(void);

	void searching();
	void waiting_at_object();
	void collaborating();
	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif