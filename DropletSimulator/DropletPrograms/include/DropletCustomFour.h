/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomFour : public DSimDroplet
{
private :

public :
	DropletCustomFour(ObjectPhysicsData *objPhysics);
	~DropletCustomFour(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};