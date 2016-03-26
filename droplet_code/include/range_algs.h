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
#define NUMBER_OF_RB_MEASUREMENTS 5
#define DELAY_BETWEEN_RB_MEASUREMENTS 5
#define POST_BROADCAST_DELAY 20
#define TIME_FOR_SET_IR_POWERS 2
#define TIME_FOR_GET_IR_VALS 3
#define TIME_FOR_ALL_MEAS (NUMBER_OF_RB_MEASUREMENTS*(TIME_FOR_GET_IR_VALS+DELAY_BETWEEN_RB_MEASUREMENTS))
#define DELAY_BETWEEN_RB_TRANSMISSIONS 10

#define DC_NOISE_REMOVAL_AMOUNT		0
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

rnb last_good_rnb;
volatile uint8_t rnb_updated;

volatile uint16_t cmdID;
volatile uint8_t processingFlag;

void range_algs_init();

void collect_rnb_data(uint16_t target_id, uint8_t power);
void broadcast_rnb_data();
void receive_rnb_data();
void use_rnb_data();
void use_cmd_rnb_data();

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
float calculate_innovation(float r, float b, float h, int16_t realBM[6][6]);