/* *** PROGRAM DESCRIPTION ***
 * Inverse Tag
 */
#pragma once

#ifndef _DROPLET_CUSTOM_ONE
#define _DROPLET_CUSTOM_ONE

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>
#include <cmath>

class DropletCustomOne : public DSimDroplet
{
private :

	enum State
	{
		TARGET,
		WALKER
	} state;

	char ack;
	droplet_id_type target;
	bool target_set_rgb, walker_set_rgb, toggle;
	
public :
	DropletCustomOne(ObjectPhysicsData *objPhysics);
	~DropletCustomOne(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif