/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomEight : public DSimDroplet
{
private :

public :
	DropletCustomEight(ObjectPhysicsData *objPhysics);
	~DropletCustomEight(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};