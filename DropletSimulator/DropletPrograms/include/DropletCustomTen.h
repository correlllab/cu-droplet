/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_TEN
#define _DROPLET_CUSTOM_TEN

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomTen : public IDroplet
{
private :

public :
	DropletCustomTen(ObjectPhysicsData *objPhysics);
	~DropletCustomTen(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif