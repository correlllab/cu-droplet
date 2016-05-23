#pragma once

#include <avr/io.h>
#include <math.h>
#include <avr/pgmspace.h>

#include "droplet_init.h"
#include "scheduler.h"
#include "ir_sensor.h"
#include "rgb_led.h"
#include "ir_comm.h"
#include "i2c.h"
#include "delay_x.h"

#define CMD_DROPLET_ID	0x8F6D

#define DROPLET_RADIUS		2.0828f  //cm

//Synchronization Timing Constants:
#define POST_BROADCAST_DELAY 30
#define TIME_FOR_SET_IR_POWERS 2
#define TIME_FOR_GET_IR_VALS 8
#define DELAY_BETWEEN_RB_TRANSMISSIONS 8

//Constants for rnb processing:
#define MIN_MATRIX_SUM_THRESH		115

//This is based on the time that elapses between when a RXing Droplet gets the end
// of a message sent from dir N, and when the TXing droplet finishes on its last channel.
static const uint8_t txDirOffset[6] = {7, 6, 3, 5, 4, 2};

#define SQRT3_OVER2 0.8660254f
static const float bearingBasis[6][2]=	{
	{SQRT3_OVER2 , -0.5},
	{0           , -1  },
	{-SQRT3_OVER2, -0.5},
	{-SQRT3_OVER2,  0.5},
	{0           ,  1  },
	{SQRT3_OVER2 ,  0.5}
};

static const float scaledBearingBasis[6][2]= {
	{1.8038f , -1.0414f},
	{0.0f  , -2.0828f},
	{-1.8038f, -1.0414f},
	{-1.8038f,  1.0414f},
	{0.0f  ,  2.0828f},
	{1.8038f ,  1.0414f}
};

static const float headingBasis[6][2]={
	{-1          ,  0  },
	{-0.5,  SQRT3_OVER2},
	{ 0.5,  SQRT3_OVER2},
	{ 1          ,  0  },
	{ 0.5, -SQRT3_OVER2},
	{-0.5, -SQRT3_OVER2}
};

typedef struct list_el {
	float Rx;
	float Ry;
	float rijMag;
	uint8_t e;
	uint8_t s;
	struct list_el *next;
} rVectorNode;

typedef struct rnb_data {
	float range;
	float bearing;
	float heading;
	float conf;
	uint16_t id_number;
} rnb;

extern float basis_angle[6];
uint32_t sensorHealthHistory;
int16_t brightMeas[6][6];

rnb last_good_rnb;
volatile uint8_t rnb_updated;
volatile uint16_t rnbCmdID;
volatile uint32_t rnbCmdSentTime;
volatile uint8_t rnbProcessingFlag;

void range_algs_init();

void broadcast_rnb_data();
//void receive_rnb_data();
void use_rnb_data();

void calculate_bearing_and_heading(float* bearing, float* heading);
float get_initial_range_guess(float bearing, float heading, uint8_t power);
float range_estimate(float init_range, float bearing, float heading, uint8_t power);

int16_t processBrightMeas();

void get_baseline_readings();
void ir_range_meas();
void ir_range_blast(uint8_t power);

float pretty_angle(float alpha);
float rad_to_deg(float rad);
float deg_to_rad(float deg);
float sensor_model(float alpha);
float emitter_model(float beta);
float amplitude_model(float r, uint8_t power);
float inverse_amplitude_model(float ADC_val, uint8_t power);

void debug_print_timer(uint32_t timer[19]);
void print_brightMeas();

float expected_bright_mat(float r, float b, float h, uint8_t i, uint8_t j);
float finiteDifferenceStep(float r0, float b0, float h0, float* r1, float* b1, float* h1);
float calculate_innovation(float r, float b, float h);
float calculate_innovationNF(float r, float b, float h);

void full_expected_bright_mat(float bM[6][6], float r, float b, float h);

typedef union fd_step_union{
	float f;
	int32_t d;
} fdStep;

#define FD_MAX_STEP	0.5236f
#define FD_INIT_STEP	0.05f //0.05f
#define FD_MIN_STEP	0.0017f
//#define FD_STEP_GROW	1.5
//#define FD_STEP_SHRINK 0.5
#define FD_DELTA_B 0.001f
#define FD_DELTA_H 0.001f

fdStep rStep, bStep, hStep;
uint8_t bMinFlipCount, hMinFlipCount;
float prevSgnEdR, prevSgnEdB, prevSgnEdH;

static inline int8_t sgn(float x){
	return (0<x)-(x<0);
}

static float inline amplitude_modelNF(float r){
	return 15.91f+(12985.5f/powf(r+0.89f,2.0));
}

static inline float rnb_constrain(float x){ //constrains the value to be within or equal to the bounds.
	return (x < FD_MIN_STEP ? FD_MIN_STEP : (x > FD_MAX_STEP ? FD_MAX_STEP : x));
}

static inline float getCosBearingBasis(uint8_t i, uint8_t j){
	return bearingBasis[j][0];
}

static inline float getSinBearingBasis(uint8_t i, uint8_t j){
	return bearingBasis[j][1];
}

static inline float getCosHeadingBasis(uint8_t i, uint8_t j){
	return headingBasis[(j+(6-i))%6][0];
}

static inline float getSinHeadingBasis(uint8_t i, uint8_t j){
	return headingBasis[(j+(6-i))%6][1];
}
