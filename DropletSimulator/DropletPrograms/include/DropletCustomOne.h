/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>

#include <inttypes.h>
#include <map>
#include <utility>
#include <cstdlib>
#include <cstring>
#define _USE_MATH_DEFINES
#include <math.h>

class DropletCustomOne : public DSimDroplet
{
private :
 
public :
	DropletCustomOne(ObjectPhysicsData *objPhysics);
	~DropletCustomOne(void);
	
	void DropletInit(void);
	void DropletMainLoop(void);
};