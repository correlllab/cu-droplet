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



#define DROPLET_RADIUS 2.22  //cm
#define DROPLET_SENSOR_RADIUS 2.5 //cm

//Synchronization Timing Constants:
#define POST_BROADCAST_DELAY			30
#define TIME_FOR_SET_IR_POWERS			2
#define TIME_FOR_GET_IR_VALS			8
#define DELAY_BETWEEN_RB_TRANSMISSIONS	8

//Constants for rnb processing:
#define MIN_MATRIX_SUM_THRESH	115
#define SQRT3_OVER2				0.8660254f
#define FD_MAX_STEP				0.5236f
#define FD_INIT_STEP			0.05f
#define FD_MIN_STEP				0.0017f
#define FD_DELTA_B				0.004f
#define FD_DELTA_H				0.004f

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
volatile id_t rnbCmdID;
volatile uint32_t rnbCmdSentTime;
volatile uint8_t rnbProcessingFlag;

void range_algs_init();

void broadcast_rnb_data(); //takes about 142ms.
//void receive_rnb_data();
void use_rnb_data();


void ir_range_meas();
void ir_range_blast(uint8_t power);

inline int8_t sgn(float x){
	return (0<x)-(x<0);
}

inline int16_t pretty_angle_deg(int16_t angle){
	return (angle>=0) ? (( (angle + 180) % 360 ) - 180) : (( (angle - 180) % 360 ) + 180);
}

inline float pretty_angle(float angle){
	return (angle>=0.0) ? ( fmodf(angle + M_PI, 2.0*M_PI) - M_PI ) : ( fmodf(angle - M_PI, 2.0*M_PI) + M_PI );
}

inline float rad_to_deg(float rad){
	return (pretty_angle(rad) / M_PI) * 180;
}

inline float deg_to_rad(float deg){
	return pretty_angle( (deg / 180) * M_PI );
}

