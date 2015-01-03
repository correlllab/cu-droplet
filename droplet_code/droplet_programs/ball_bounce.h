#include "droplet_init.h"

#define HEARTBEAT_PERIOD 3000	//ms
#define GROUP_TIMEOUT 15000		//ms
#define BALL_PERIOD 300			//ms

#define LOW_PRIORITY_IR_POWER 90
#define HIGH_PRIORITY_IR_POWER 256

typedef struct neighbor_struct
{
	uint16_t ID;
	uint16_t ms_age;
} neighbor;
neighbor neighbors[6];

typedef enum
{
	BALL,
	NOT_BALL
} State;
State	state;

typedef struct throw_ball_msg_struct
{
	char msg_flag;
	uint16_t soft_tgt_id;
} throw_ball_msg;

throw_ball_msg*	outgoing_ball;
uint32_t		last_loop_time;
uint32_t		last_heartbeat_time;
uint32_t		last_ball_time;
uint8_t			num_neighbors;

uint8_t ball_in_dir;
char* heartbeat_msg;


void		init();
void		loop();
void		handle_msg		(ir_msg* msg_struct);
uint8_t		check_bounce	(uint8_t dir_in);
void		add_group_member(uint16_t senderID, uint8_t dir);
uint8_t		update_group	(uint32_t time_to_add);

void        wait_waiting		();

void		change_state		( State		new_state	);





