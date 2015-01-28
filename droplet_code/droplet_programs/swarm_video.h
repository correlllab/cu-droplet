#include "droplet_init.h"

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
void get_rgb_wrapper(uint8_t* r_dest, uint8_t* g_dest, uint8_t* b_dest);
void get_calibrated_frame_vals(uint8_t* r_out, uint8_t* g_out, uint8_t* b_out, uint8_t r, uint8_t g, uint8_t b);
void calc_calib_matrix();

#define MIN_R_THRESH 0
#define MIN_G_THRESH 0
#define MIN_B_THRESH 0
#define FRAME_DELAY 100
#define MAX_FRAME_COUNT 300
#define NUM_CALIB_MEAS 16

typedef enum
{
	UNPROGRAMMED,
	RECORD,
	PLAY
} State;
State	state;

typedef enum
{
	R, G, B
} Colors;

uint8_t vid_frames[3][MAX_FRAME_COUNT];

char col_names[] = {'R', 'G', 'B', 'C', 'M', 'Y', 'W'};

const uint8_t known_calib_vals[3][NUM_CALIB_MEAS] = {{255,0,0,0,0,0,255,127,255,255,127,255,255,127,255,255},
													 {0,255,0,255,127,255,0,0,0,255,255,127,255,255,127,255},
													 {0,0,255,255,255,127,255,255,127,0,0,0,255,255,255,127}};

int16_t r_baseline, g_baseline, b_baseline;
uint8_t off_count;
uint8_t cm[NUM_CALIB_MEAS][3];
float calib_result[3][3];
uint32_t time_vals[3];
uint32_t frame_delay;
uint16_t frame_count;
uint16_t total_frames;
int16_t	tau;
double	theta;

//r->g:		1/13
//r->b:		1/80
//g->r:		1/12
//g->b:		1/10
//b->r:		0
//b->g:		1/5