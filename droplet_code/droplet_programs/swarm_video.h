#include "droplet_init.h"

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#define FRAME_DELAY 100
#define MAX_FRAME_COUNT 1200
#define NUM_CALIB_MEAS 16

typedef enum
{
	UNPROGRAMMED,
	RECORD,
	PLAY
} State;
State	state;

uint8_t vid_frames[MAX_FRAME_COUNT][3];

uint32_t loop_end;
uint32_t magic_number;
uint16_t frame_count;
uint16_t total_frames;
uint8_t interp;