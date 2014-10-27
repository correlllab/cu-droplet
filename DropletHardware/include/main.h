#ifndef _MAIN_
#define _MAIN_
#include "droplet_init.h"

#define MOVE_DIST_SCALAR            50
#define RED_THRESHOLD               200

#define WALKAWAY_TIME               15000   // in ms
#define START_DELAY_TIME            100     // in ms
#define COLLABORATE_DURATION        3000    // in ms

#define GROUP_MEMBERSHIP_TIMEOUT    2000    // in ms
#define HEART_RATE                  1000    // in ms

typedef struct GROUP_LIST_ITEM
{
	uint16_t ID;
	uint16_t ms_age;
	struct GROUP_LIST_ITEM* next;
	struct GROUP_LIST_ITEM* prev;
} group_item;
group_item* group_root;

typedef enum
{
	COLLABORATING,
	LEAVING,
	SAFE,
	SEARCHING,
	START_DELAY,
	WAITING
} State;
State	state;
	
int16_t	tau;
double	theta;

uint16_t	current_group_size;
uint16_t	yes_count;
uint32_t	start_delay_time;
uint32_t	heartbeat_time;
uint32_t	voting_time;
uint32_t	collab_time;
uint8_t		last_move_dir;

// Droplet Movement Helper Functions
uint8_t		check_safe	();
void		random_walk	();

// Task Allocation Helper Functions
void		send_heartbeat		();
uint16_t	update_group_size	( uint16_t	time_to_add	);
void		check_votes			();
uint8_t		roll_sigmoid		( int16_t	group_size	);
void		add_group_member	( uint16_t	senderID	);

void		change_state		( State		new_state	);

// Droplet Communication Helper Functions
void		check_messages		();
void		clear_msg_buffers	();
void		clear_state			();
#endif