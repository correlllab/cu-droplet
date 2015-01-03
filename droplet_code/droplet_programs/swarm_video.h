#include "droplet_init.h"

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
void get_rgb_wrapper(uint8_t* r_dest, uint8_t* g_dest, uint8_t* b_dest);
void get_calibrated_frame_vals(uint8_t* r_out, uint8_t* g_out, uint8_t* b_out, uint8_t r, uint8_t g, uint8_t b);
void invert_calib_matrix();

#define MIN_R_THRESH 80
#define MIN_G_THRESH 80
#define MIN_B_THRESH 20
#define MAX_FRAME_DELAY 1500
#define MAX_FRAME_COUNT 1024

typedef enum
{
	UNPROGRAMMED,
	CALIB_R,
	CALIB_G,
	CALIB_B,
	RECORD,
	PLAY
} State;
State	state;

typedef enum
{
	R, G, B
} Colors;

uint8_t vid_frames[3][MAX_FRAME_COUNT];

int16_t r_baseline, g_baseline, b_baseline;
uint8_t calib_white[3]; //calib_white[G] is the val of the green sensor when light was white.
int32_t calib_matrix[3][3]; //calib_matrix[R][B] is the val of blue sensor when light was full red.
float inv_calib_matrix[3][3];
uint32_t time_vals[3];
uint32_t frame_delay;
uint16_t frame_count;
int16_t	tau;
double	theta;

//r->g:		1/13
//r->b:		1/80
//g->r:		1/12
//g->b:		1/10
//b->r:		0
//b->g:		1/5