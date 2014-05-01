#ifndef _MAIN_
#define _MAIN_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "droplet_init.h"
//#include "pc_com.h"
//#include "motor.h"
//#include "IRcom.h"

#define GROUP_MEMBERSHIP_TIMEOUT 200
#define HEART_RATE 100

struct GROUP_LIST_ITEM
{
	uint16_t ID;
	uint16_t ms_age;
	struct GROUP_LIST_ITEM* next;
	struct GROUP_LIST_ITEM* prev;
};
typedef struct GROUP_LIST_ITEM group_item;

int16_t tau;
double theta;
group_item* group_root;
uint16_t current_group_size;
uint16_t yes_count;
uint8_t collaborating;

volatile double a;

void print_shit();

//uint8_t roll_that_sigmoid(int16_t group_size);
//void check_votes();
inline void collaborative_task(){;}
//void reset_experiment();
//void broadcast_heartbeat();
//uint16_t update_group_size();
//void add_group_member(uint16_t senderID);
inline void set_tau(int16_t t){;}
inline void set_theta(double th){;}
//void check_messages();
//void color_cycle();

#endif