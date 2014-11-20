#ifndef USER_TEMPLATE
#define USER_TEMPLATE

#include "droplet_init.h"

#define RED_THRESHOLD               4

#define MOVE_DIST_SCALAR            50
#define WALKAWAY_STEPS              250

#define COLLABORATE_DURATION        5000    // in ms

#define GROUP_MEMBERSHIP_TIMEOUT    2000    // in ms
#define HEART_RATE                  1000    // in ms
#define LIGHT_CHECK_RATE			1000

typedef struct GROUP_LIST_ITEM
{
	uint16_t ID;
	uint32_t ms_age;
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
	WAITING,
	NOTHING
} State;
State	state;

int16_t	tau;
double	theta;

uint16_t	current_group_size;
uint16_t	yes_count;
uint32_t	light_check_time;
uint32_t	heartbeat_time;
uint32_t	last_update_time;
uint32_t	collab_time;
uint8_t		last_move_dir;

// Droplet Movement Helper Functions
uint8_t		check_safe	();
void		random_walk	();

// Task Allocation Helper Functions
void		send_heartbeat		();
uint16_t	update_group_size	( uint32_t	time_to_add	);
void		check_votes			();
uint8_t		roll_sigmoid		( int16_t	group_size	);
void		add_group_member	( uint16_t	senderID	);

// Droplet Communication Helper Functions
void		check_messages		();
void		clear_state			();
void		change_state		( State		new_state	);

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#endif