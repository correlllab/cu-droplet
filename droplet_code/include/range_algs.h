#pragma once

#include "droplet_base.h"
#include "scheduler.h"
#include "ir_sensor.h"
#include "rgb_led.h"

#define DROPLET_RADIUS 22U  //mm
#define DROPLET_RADIUS_SQ 484U //mm
#define DROPLET_DIAMETER 44 //mm

//Synchronization Timing Constants:
#define POST_BROADCAST_DELAY			30U
#define TIME_FOR_SET_IR_POWERS			2U
#define TIME_FOR_IR_MEAS				12U
#define TIME_FOR_GET_IR_VALS			16U
#define DELAY_BETWEEN_RB_TRANSMISSIONS	10U

//Constants for rnb processing:
#define MIN_MATRIX_SUM_THRESH	115
#define ELEVEN_SQRT3		   19.0525588833f
#define SQRT3_OVER2				0.8660254038f
#define RNB_DUR					220

typedef struct list_el {
	float Rx;
	float Ry;
	float rijMag;
	uint8_t e;
	uint8_t s;
	struct list_el *next;
} rVectorNode;

typedef struct rnb_data {
	uint16_t range;
	int16_t bearing;
	int16_t heading;
	id_t id;
} rnb;


rnb last_good_rnb;
volatile uint8_t rnb_updated;
volatile id_t rnbCmdID;
volatile uint8_t processing_rnb_flag;

void rangeAlgsInit(void);

void broadcastRnbData(void); //takes about 142ms.
//void receive_rnb_data(void);
void useRnbData(void);

void irRangeMeas(uint32_t rnbCmdSentTime);
void irRangeBlast(uint32_t rnbCmdSentTime, uint8_t power);

inline int8_t sgn(float x){
	return (0<x)-(x<0);
}

inline int16_t prettyAngleDeg(int16_t angle){
	return (angle>=0) ? (( (angle + 180) % 360 ) - 180) : (( (angle - 180) % 360 ) + 180);
}

inline float prettyAngle(float angle){
	return (angle>=0.0) ? ( fmodf(angle + M_PI, 2.0*M_PI) - M_PI ) : ( fmodf(angle - M_PI, 2.0*M_PI) + M_PI );
}

inline float radToDeg(float rad){
	return (prettyAngle(rad) / M_PI) * 180;
}

inline float degToRad(float deg){
	return prettyAngle( (deg / 180) * M_PI );
}
