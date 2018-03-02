

#pragma once
#include "droplet_init.h"

#define MOVING_DROPLET_ID			(0x4327)
//#define MOVING_DROPLET_ID			(0xFCD0)
//#define ORIGIN_DROPLET_ID			(0x392B)

#define RNB_BROADCAST_PERIOD		(500)
#define MESSAGE_PERIOD				(500)
#define MAX_NUM_MEAS				(120)
#define SEND_RC_FLAG				'R'
#define START_MESSAGE_FLAG			'S'
#define STOP_MESSAGE_FLAG			'P'

extern volatile uint8_t stop_msg;
extern volatile uint8_t start_msg;
extern volatile uint8_t read_count;
//extern volatile uint8_t broadcast_count;
extern volatile uint32_t lastBroadcastTime;

typedef struct msg_struct{
	char msg_check_flag;
}MoveStatusMsg;

typedef struct radius_center {
	char rc_check_flag;
	int32_t radius;
	int32_t center_x;
	int32_t center_y;
	//float center_heading;
}radiusCenterStruct;

typedef struct circle_meas_struct{
	int32_t x;
	int32_t y;
}CircleMeas;

uint32_t lastMessageTime;
CircleMeas measLog[MAX_NUM_MEAS]; //initialize to UNDF
radiusCenterStruct radius_center_struct; // struct for receiving radius and center.
radiusCenterStruct radius_center_calc_struct; //struct for sending radius and center.

void handleMsg(irMsg* msgStruct);
void take_Rnb_Reading(CircleMeas *meas_log);
void calculate_Center(CircleMeas* meas_log, uint8_t count);
void calculate_Radius(CircleMeas* meas_log, uint8_t count);
void send_message(char msgFlag);
void doRnbBroadcast(void);
void startMove(uint8_t dir, uint16_t numSteps);
void curve_fit_check(uint8_t count);
void change_axes(void);

void init(void);
void loop(void);

