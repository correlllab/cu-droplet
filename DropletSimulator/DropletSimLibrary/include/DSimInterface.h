#pragma once

#ifndef _I_DROPLET_SIM_INTERFACE
#define _I_DROPLET_SIM_INTERFACE

#include "DSim.h"
#include "DSimGlobals.h"

enum BasicObjectShapes
{
	SPHERE,
	CUBOID,
	CYLINDER,
	CONE,
	PLANE
};

class DSimInterface
{
	private :

	protected :

		DSim *sim;

	public :

		DSimInterface(void);
		virtual ~DSimInterface(void);

		DS_RESULT InitializeSim(char *initFilePath);
		DS_RESULT SetDropletCollisionShape(BasicObjectShapes shape);

		DS_RESULT CreateDroplet();

};

#endif