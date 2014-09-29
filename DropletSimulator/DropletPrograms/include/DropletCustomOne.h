/* *** PROGRAM DESCRIPTION ***
 * 
 */
#pragma once

#include <DSimDroplet.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>
#include <inttypes.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define ID_LIST_TIME    1000
#define WAIT_TIME       1000
#define SEND_TIME       100

class DropletCustomOne : public DSimDroplet
{
private :
    enum
    {
        MAKE_ID_LIST,
        RNB
    } state;

    uint32_t init_time;
    uint32_t send_time;
    uint32_t wait_time;
    std::vector<droplet_id_type> id_list;

    FILE *fp;

public :
	DropletCustomOne(ObjectPhysicsData *objPhysics);
	~DropletCustomOne(void);

	
	void DropletInit(void);
	void DropletMainLoop(void);
};