/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomFive : public DSimDroplet
{
private :

public :
	DropletCustomFive(ObjectPhysicsData *objPhysics);
	~DropletCustomFive(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};