/* *** PROGRAM DESCRIPTION ***
 * Inverse Tag
 */
#pragma once

#ifndef _DROPLET_CUSTOM_ONE
#define _DROPLET_CUSTOM_ONE

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>
#include <cmath>

class DropletCustomOne : public IDroplet
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