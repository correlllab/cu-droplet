#include "droplet_init.h"

#define HEARTBEAT_PERIOD 1000 //ms

typedef struct NEIGHBOR
{
	uint16_t ID;
	uint16_t ms_age;
} neighbor;
neighbor* neighbors[6];

typedef enum
{
	BALL,
	NOT_BALL
} State;
State	state;


uint32_t	last_loop_time;
uint32_t	last_heartbeat_time;
uint8_t num_neighbors;

uint8_t in_dir;
char* heartbeat_msg;
char* catch;
char* ack;

void		init();
void		loop();
void		handle_msg		(ir_msg* msg_struct);
uint8_t		check_bounce	(uint8_t dir_in);
void		add_group_member(uint16_t senderID, uint8_t dir);
void		update_group	(uint32_t time_to_add);

void        wait_waiting		();

void		change_state		( State		new_state	);





