/* *** PROGRAM DESCRIPTION ***
 * Testing global_rx_buffer.data_len bug
 */
#pragma once

#ifndef _DROPLET_CUSTOM_NINE
#define _DROPLET_CUSTOM_NINE

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>
#include <vector>
#include <algorithm>
#include <stdlib.h>

class DropletCustomNine : public DSimDroplet
{
private :
	FILE *fh;
    std::vector<droplet_id_type> id_list;
    droplet_id_type idle_droplet_id;

    enum droplet_state
    {
        ID,
        MOVING,
        IDLE
    } state;
    
    //void do_rnb ();

public :
	DropletCustomNine(ObjectPhysicsData *objPhysics);
	~DropletCustomNine(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};

#endif