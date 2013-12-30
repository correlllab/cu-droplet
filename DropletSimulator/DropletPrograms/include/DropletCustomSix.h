/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#ifndef _DROPLET_CUSTOM_SIX
#define _DROPLET_CUSTOM_SIX

#include "IDroplet.h"
#include "DropletSimGlobals.h"
#include "DropletDataStructs.h"

#include <inttypes.h>
#include <stdlib.h>
#include <math.h>

#define CALL_COLOR_R 0
#define CALL_COLOR_G 50
#define CALL_COLOR_B 0

#define ASSIGNED_COLOR_R 0
#define ASSIGNED_COLOR_G 0
#define ASSIGNED_COLOR_B 50

#define SET_COLOR_R 50
#define SET_COLOR_G 25
#define SET_COLOR_B 0

#define ERROR_COLOR_R 50
#define ERROR_COLOR_G 0
#define ERROR_COLOR_B 0

#define POS_NOT_SET 7
#define MAX_ANG_ERR 2 // degrees

#define ACK				'a'
#define REQUEST_DROPLET	'r'
#define RESPOND_DROPLET 'p'

class DropletCustomSix : public IDroplet
{
private :

	typedef enum
	{
		SEARCH,
		CALL,
		ASSIGNED,
		SET,
		WAIT_RESP,
		ERR
	} State;

	State state;
	bool call_color_set, search_color_set, assigned_color_set;
	bool set_color_set, error_color_set, wait_resp_color_set, saw_mark;
	uint16_t my_guitar_id, max_guitar_id, row;
	uint8_t pos_rqrd, pos_rqsd;

	void set_init_states(void);
	void handle_search(void);
	void handle_call(void);
	void handle_assigned(void);
	void handle_set(void);
	void handle_msgs(void);
	void find_rqrd_pos(bool last_element);
	void transition_from_seach(State new_state);
public :
	DropletCustomSix(ObjectPhysicsData *objPhysics);
	~DropletCustomSix(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif