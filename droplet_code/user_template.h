

#pragma once
#include "droplet_init.h"

///////////////// CURVE FIT METHOD MACROS ///////////////////

#define POINT_SCORE_MIN_INLIER
#define POINT_SCORE_MERGE
//#define REGULAR_CURVE_FIT
//#define REGULAR_CURVE_FIT_MERGE


/////////////////////////////////////////////////////////////

#define MOVING_DROPLET_ID			(0xDD21)

#define RNB_BROADCAST_PERIOD		(750)
#define MESSAGE_PERIOD				(500)
#define MAX_NUM_MEAS				(80)
#define SEND_RC_FLAG				'R'
#define START_MESSAGE_FLAG			'S'
#define STOP_MESSAGE_FLAG			'P'

#define INLIER_BOUND				(5)
#define MIN_INLIER_RATIO			(0.40)


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
typedef struct point_cicle{
	int32_t x1;
	int32_t x2;
	int32_t x3;
	int32_t y1;
	int32_t y2;
	int32_t y3;
}pointCircle;

int32_t c_radius;
int32_t x_center;
int32_t y_center;

uint32_t lastMessageTime;

CircleMeas measLog0[MAX_NUM_MEAS]; //initialize to UNDF
CircleMeas measLog1[MAX_NUM_MEAS];
CircleMeas measLog3[MAX_NUM_MEAS];

radiusCenterStruct radius_center_struct; // struct for receiving radius and center.
radiusCenterStruct radius_center_calc_struct; //struct for sending radius and center.

pointCircle three_Point_Circle;

/////////////////////// POINT SCORE FUNCTIONS /////////////////////////////////////

//inline int32_t distanceBetween(CircleMeas* a, CircleMeas* b);
void take_readings(CircleMeas* meas_log,uint8_t count);
void calculateRadiusCenter(pointCircle ptCirc);
void comparePointScore(pointCircle point_Score_Circle);
float inlierRatio(CircleMeas* meas_log, uint8_t count);
void randCircleIterations(CircleMeas* meas_log, uint8_t count);

////////////////////////////////////////////////////////////////////////////

////////////////////////////// MERGE FUNCTIONS /////////////////////////////

int32_t min_Distance(CircleMeas* meas_log, uint8_t count);
void mergePoints(CircleMeas* meas_log, uint8_t count,int32_t min_distance);
uint8_t mergeStructSort(CircleMeas* measlog0, CircleMeas* measlog1,uint8_t count);

////////////////////////////////////////////////////////////////////////////

//////////////////////////// INLIER POINT FUNCTION ////////////////////////////

uint8_t inlierPointsSave(CircleMeas* measlog0, CircleMeas* measlog1,uint8_t count);

////////////////////////////////////////////////////////////////////////////////

void handleMsg(irMsg* msgStruct);
void take_Rnb_Reading(CircleMeas *meas_log);
void calculate_Center(CircleMeas* meas_log, uint8_t count);
void calculate_Radius(CircleMeas* meas_log, uint8_t count);
void send_message(char msgFlag);
void doRnbBroadcast(void);
void startMove(uint8_t dir, uint16_t numSteps);
//void curve_fit_check(uint8_t count);
//void change_axes(void);

void init(void);
void loop(void);

