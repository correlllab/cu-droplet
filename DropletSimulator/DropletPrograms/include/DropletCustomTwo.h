/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomTwo : public DSimDroplet
{
private :

public :
	DropletCustomTwo(ObjectPhysicsData *objPhysics);
	~DropletCustomTwo(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};