/* *** PROGRAM DESCRIPTION ***
 * Shape Prediction
 */
#pragma once

#ifndef _DROPLET_CUSTOM_THREE
#define _DROPLET_CUSTOM_THREE

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

class DropletCustomThree : public DSimDroplet
{
private :

	enum
	{
		WAIT,
		SET,
		CALL,
		MOVE
	}state;

	char ack;
	droplet_id_type target;
	uint8_t layer_num, counter, needed, id;
	bool is_set_rgb, move_rgb, wait_rgb, call_rgb;
	bool mover, corner, first_set, layer_set;

	uint8_t get_next_id();
	
public :
	DropletCustomThree(ObjectPhysicsData *objPhysics);
	~DropletCustomThree(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif