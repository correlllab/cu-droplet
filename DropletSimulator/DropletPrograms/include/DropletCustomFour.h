#pragma once

#ifndef _DROPLET_CUSTOM_FOUR
#define _DROPLET_CUSTOM_FOUR

#include "IDroplet.h"
#include "DropletSimGlobals.h"
#include "DropletDataStructs.h"

#include <algorithm>
#include <map>
#include <vector>
#include <inttypes.h>
#include <stdlib.h>

class DropletCustomFour : public IDroplet
{
private :

	enum State
	{
		IT,
		NOT_IT
	} state;

	std::vector<droplet_id_type> other_droplets;
	char ack;
	char marco;
	char polo;
	droplet_id_type my_id, target_id;
	bool it_init_required, notit_init_required, wait_for_ack, toggle;
public :
	DropletCustomFour(ObjectPhysicsData *objPhysics);
	~DropletCustomFour(void);
	std::map<droplet_id_type,bool> droplet_status;
	void acquireNewTarget();
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif