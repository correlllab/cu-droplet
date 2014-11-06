/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomThree : public DSimDroplet
{
private :

public :
	DropletCustomThree(ObjectPhysicsData *objPhysics);
	~DropletCustomThree(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};