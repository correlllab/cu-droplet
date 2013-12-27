/* *** CURRENT PROGRAM ***
 * 
 * Guitar Construction.
 */

#pragma once

#ifndef _DROPLET_CUSTOM_TWO
#define _DROPLET_CUSTOM_TWO

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>

#include <stdlib.h>

class DropletCustomTwo : public IDroplet
{
private :
	void find_neighbors(uint8_t guitar_id, uint8_t row_num, uint8_t num_in_row);
	uint8_t get_row_num(uint8_t guitar_id);
	uint8_t get_num_in_row(uint8_t guitar_id);

	enum State
	{
		WANDER,
		DOCK,
		ALIGN,
		CALL,
		SET
	} state;

	char ack;
	droplet_id_type target;
	uint8_t row_num, num_in_row, guitar_id;
	uint8_t slot_num, slots_needed, slots_set;
	uint8_t* neighbors;
	bool is_set_rgb, wander_rgb, dock_rgb, align_rgb, call_rgb;
	bool mover, first_set, first_search;

public :
	DropletCustomTwo(ObjectPhysicsData *objPhysics);
	~DropletCustomTwo(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif