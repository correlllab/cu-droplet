#pragma once

#ifndef _DROPLET_STICK_PULLERS
#define _DROPLET_STICK_PULLERS

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletStickPullers : public IDroplet
{
private :
	enum phases
	{
		SEARCH,
		WAIT,
		DONE
	} phase;

	uint8_t first_wait;
	uint8_t moveCounter;
	uint8_t r_thresh;
	uint16_t wait_timeout;
	uint16_t blind_timeout;

	uint8_t group_size, group_thresh, blink_count;
	uint16_t gpx2;
	droplet_id_type *recv_ids;

	void back_to_search(void);

public :

	DropletStickPullers(ObjectPhysicsData *objPhysics);
	~DropletStickPullers();

	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif