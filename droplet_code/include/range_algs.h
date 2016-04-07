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

#define DROPLET_RADIUS 2.0828  //cm
#define DROPLET_SENSOR_RADIUS 2.0828 //cm
#define BRIGHTNESS_THRESHOLD 1
#define CMD_DROPLET_ID	0x8F6D

#define POST_MESSAGE_DELAY ((3*((1000/IR_UPKEEP_FREQUENCY)))/2)
#define R_VAR_SCALE_FACTOR 162.05
#define NUMBER_OF_RB_MEASUREMENTS 1
#define DELAY_BETWEEN_RB_MEASUREMENTS 5
#define POST_BROADCAST_DELAY 20
#define TIME_FOR_SET_IR_POWERS 2
#define TIME_FOR_GET_IR_VALS 10
#define TIME_FOR_ALL_MEAS (NUMBER_OF_RB_MEASUREMENTS*TIME_FOR_GET_IR_VALS+(NUMBER_OF_RB_MEASUREMENTS-1)*DELAY_BETWEEN_RB_MEASUREMENTS)
#define DELAY_BETWEEN_RB_TRANSMISSIONS 10

#define TOTAL_RNB_TIME (TIME_FOR_SET_IR_POWERS+POST_BROADCAST_DELAY+6*(TIME_FOR_ALL_MEAS+DELAY_BETWEEN_RB_TRANSMISSIONS))

#define MIN_MATRIX_SUM_THRESH		115
#define SKEPTIC_MATRIX_SUM_THRESH	160
#define OVER_DOMINANT_ROW_THRESH	0.75

extern float basis_angle[6];

#define SQRT3_OVER2 0.8660254

static const float bearingBasis[6][2]=	{
	{SQRT3_OVER2 , -0.5},
	{0           , -1  },
	{-SQRT3_OVER2, -0.5},
	{-SQRT3_OVER2,  0.5},
	{0           ,  1  },
	{SQRT3_OVER2 ,  0.5}
};

static const float headingBasis[6][2]={
	{-1          ,  0  },
	{-0.5,  SQRT3_OVER2},		
	{ 0.5,  SQRT3_OVER2},	
	{ 1          ,  0  },
	{ 0.5, -SQRT3_OVER2},
	{-0.5, -SQRT3_OVER2}
};

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

uint8_t sensorHealthHistory[6];

rnb last_good_rnb;
volatile uint8_t rnb_updated;

volatile uint16_t cmdID;
volatile uint8_t processingFlag;

void range_algs_init();

void collect_rnb_data(uint16_t target_id, uint8_t power);
void broadcast_rnb_data();
void receive_rnb_data();
void use_rnb_data();

void calculate_bearing_and_heading(int16_t brightness_matrix[6][6], float* bearing, float* heading);
float get_initial_range_guess(float bearing, float heading, uint8_t power, int16_t brightness_matrix[6][6]);
float range_estimate(float init_range, float bearing, float heading, uint8_t power, int16_t brightness_matrix[6][6]);

void fill_S_and_T(int16_t brightness_matrix[6][6], int16_t sensor_total[6], int16_t emitter_total[6]);
int16_t pack_measurements_into_matrix( int16_t brightness_matrix[6][6]);

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
void print_brightness_matrix(int16_t brightness_matrix[6][6], int16_t sum);
void brightness_meas_printout_mathematica();

float expected_bright_mat(float r, float b, float h, uint8_t i, uint8_t j);
float finiteDifferenceStep(float r0, float b0, float h0, float* r1, float* b1, float* h1, int16_t realBM[6][6]);
float calculate_innovation(float r, float b, float h, int16_t realBM[6][6]);

static inline int8_t sgn(float x){
	return (0<x)-(x<0);
	//if(x>0) return 1;
	//else if(x<0) return -1;
	//else return 0;
}

#define MAX_STEP	0.5
#define INIT_STEP	0.05
#define MIN_STEP	0.0001
#define STEP_GROW	1.5
#define STEP_SHRINK 0.5


float rStep, bStep, hStep;
float prevDeltaEdR, prevDeltaEdB, prevDeltaEdH;

static inline float rnb_constrain(float x){ //constrains the value to be within or equal to the bounds.
	return (x < MIN_STEP ? MIN_STEP : (x > MAX_STEP ? MAX_STEP : x));
}

//typedef struct attempt_data_struct{
	//float b;
	//float h;
	//float err;
//} attemptData;
//
//attemptData records[144];
//
//static int attemptCmpFunc(const void* a, const void* b){
	//const attemptData* aD = ((const attemptData*)a);
	//const attemptData* bD = ((const attemptData*)b);
	//float aE = aD->err;
	//float bE = bD->err;
	//if(aE > bE){
		//return 1;
	//}else if(aE < bE){
		//return -1;
	//}else{
		//return 0;
	//}
//}