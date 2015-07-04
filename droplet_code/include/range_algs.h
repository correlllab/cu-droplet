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

#define DROPLET_RADIUS 2.0828
#define DROPLET_SENSOR_RADIUS 2.0828
#define BRIGHTNESS_THRESHOLD 1

#define POST_MESSAGE_DELAY ((3*((1000/IR_UPKEEP_FREQUENCY)))/2)

#define NUMBER_OF_RB_MEASUREMENTS 4
#define DELAY_BETWEEN_RB_MEASUREMENTS 5
#define POST_BROADCAST_DELAY 15
#define TIME_FOR_SET_IR_POWERS 2
#define TIME_FOR_GET_IR_VALS 3
#define TIME_FOR_ALL_MEAS 23
#define DELAY_BETWEEN_RB_TRANSMISSIONS 15

#define BASELINE_NOISE_THRESHOLD 1

struct list_el {
	float Rx;
	float Ry;
	float rijMag;
	uint8_t e;
	uint8_t s;
	struct list_el *next;
};

typedef struct list_el rVectorNode;

struct rnb_data {
	float range;
	float bearing;
	float heading;
	uint8_t (*brightness_matrix_ptr)[6]; //almost definitely take this out, later
	uint16_t id_number;
};

typedef struct rnb_data rnb;

rnb last_good_rnb;
volatile uint8_t rnb_updated;

void range_algs_init();

void collect_rnb_data(uint16_t target_id, uint8_t power);
void broadcast_rnb_data();
void receive_rnb_data();
void use_rnb_data();

float get_bearing(uint8_t sensor_total[6]);
float get_heading(uint8_t emitter_total[6], float bearing);
float get_initial_range_guess(float bearing, float heading, uint8_t power, uint8_t sensor_total[6], uint8_t emitter_total[6], uint8_t brightness_matrix[6][6]);
float range_estimate(float init_range, float bearing, float heading, uint8_t power, uint8_t brightness_matrix[6][6]);

void fill_S_and_T(uint8_t brightness_matrix[6][6], uint8_t sensor_total[6], uint8_t emitter_total[6]);
uint8_t pack_measurements_into_matrix( uint8_t brightness_matrix[6][6]);

void get_baseline_readings();
void ir_range_meas();
void ir_range_blast(uint8_t power);
void ir_emit(uint8_t direction, uint8_t duration);

float pretty_angle(float alpha);
float rad_to_deg(float rad);
float deg_to_rad(float deg);
float sensor_model(float alpha);
float emitter_model(float beta);
float amplitude_model(float r, uint8_t power);
float inverse_amplitude_model(float ADC_val, uint8_t power);

void debug_print_timer(uint32_t timer[19]);
void print_brightness_matrix(uint8_t brightness_matrix[6][6]);
void brightness_meas_printout_mathematica();