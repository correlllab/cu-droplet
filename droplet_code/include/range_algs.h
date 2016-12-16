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

#define DROPLET_RADIUS 22  //mm

//Synchronization Timing Constants:
#define POST_BROADCAST_DELAY			30
#define TIME_FOR_SET_IR_POWERS			2
#define TIME_FOR_GET_IR_VALS			8
#define TIME_FOR_IR_MEAS				30

#define SQRT3_OVER2				0.8660254f
#define M_PI_6					0.5235988f

typedef struct rnb_data {
	int16_t range; //distance, in mm.
	int16_t bearing; //bearing, in degrees.
	id_t id;
} rnb;

rnb last_good_rnb;
volatile uint8_t rnb_updated;
volatile id_t rnbCmdID;
volatile uint32_t rnbCmdSentTime;
volatile uint8_t rnbProcessingFlag;
volatile Task_t* rnbProcessingTask;
volatile uint32_t rnbPFsetTime;
#define RNB_PROCESSING_TIMEOUT 1000

void range_algs_init();

void broadcast_rnb_data(); //should take no more than 76 milliseconds.
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

