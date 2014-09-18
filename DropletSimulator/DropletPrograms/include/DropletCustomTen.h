/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomTen : public DSimDroplet
{
private :

public :
	DropletCustomTen(ObjectPhysicsData *objPhysics);
	~DropletCustomTen(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};