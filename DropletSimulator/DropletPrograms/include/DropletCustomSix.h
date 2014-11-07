/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomSix : public DSimDroplet
{
private :

public :
	DropletCustomSix(ObjectPhysicsData *objPhysics);
	~DropletCustomSix(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};