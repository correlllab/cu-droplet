/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_FIVE
#define _DROPLET_CUSTOM_FIVE

#define _USE_MATH_DEFINES

#include <IDroplet.h>
#include <DropletSimGlobals.h>
#include <DropletDataStructs.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

class DropletCustomFive : public IDroplet
{
private :
	enum
	{
		WANDER,
		DOCK,
		ALIGN,
		PRESET,
		CALL,
		SET
	}state;

	droplet_id_type target;
	uint8_t *neighbors;
	uint8_t slot_num, slots_needed, slots_set;
	uint8_t guitar_id, row_num, num_in_row;
	bool wander_rgb, move_rgb, call_rgb, dock_rgb, align_rgb, preset_rgb, is_set_rgb;
	bool first_set, first_search, clock_wise;

	void find_empty_neighbors(uint8_t guitar_id, uint8_t row_num, uint8_t num_in_row);
	uint8_t get_row_num(uint8_t guitar_id);
	uint8_t get_num_in_row(uint8_t guitar_id);

public :
	DropletCustomFive(ObjectPhysicsData *objPhysics);
	~DropletCustomFive(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif