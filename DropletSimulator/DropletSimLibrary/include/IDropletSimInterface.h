#pragma once

#ifndef _I_DROPLET_SIM_INTERFACE
#define _I_DROPLET_SIM_INTERFACE

#include "DropletSim.h"
#include "DropletSimGlobals.h"

enum BasicObjectShapes
{
	SPHERE,
	CUBOID,
	CYLINDER,
	CONE,
	PLANE
};

class IDropletSimInterface
{
	private :

	protected :

		DropletSim *sim;

	public :

		IDropletSimInterface(void);
		virtual ~IDropletSimInterface(void);

		DS_RESULT InitializeSim(char *initFilePath);
		DS_RESULT SetDropletCollisionShape(BasicObjectShapes shape);

		DS_RESULT CreateDroplet();

};

#endif