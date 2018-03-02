#include "user_template.h"

volatile uint8_t stop_msg = 0;
volatile uint8_t start_msg = 0;
volatile uint8_t read_count = 0;
//volatile uint8_t broadcast_count = 0;
volatile uint32_t lastBroadcastTime;
uint8_t stop_recieved = 0;
//int32_t circle_arr[6][2] ={{200,190},{200,50},{-280,50},{-280,190},{210,120},{-290,120}};
//int32_t circle_arr[4][2] ={{200,700},{700,200},{-300,200},{200,-300}};
int32_t circle_arr[3][2] ={{190,690},{690,210},{553, 553}};
uint8_t loop_count = 0;

int16_t last_heading;
int32_t last_x;
int32_t last_y;
CircleMeas last_update[0];


void curve_fit_check(uint8_t count){
	//if((getDropletID() == ORIGIN_DROPLET_ID) && (loop_count == 0)){
		for(uint8_t index = 0; index <count; index++){
			measLog[index].x = circle_arr[index][0];
		measLog[index].y = circle_arr[index][1]; }
		read_count = count;
		//printf("\r\nRNB readings taken.\r\n");
		calculate_Center(measLog, read_count);
		calculate_Radius(measLog, read_count);
		setRGB(0,255,0);
		printf("\nCenter and Radius calculations done. \r\n");
		setRGB(255,0,255);
		loop_count = 1;
	
}

/**************************************************************************
*
* @name - send_Radius_Center()
*
* @brief - This function sends the radius and center measurements to
*		   the moving droplet.
*
* @param - rc_Struct (structure containing radius and rnb of the center.)
*
* @return - none
*
***************************************************************************/

void send_Radius_Center(radiusCenterStruct* rc_Struct){
	irSend(ALL_DIRS, (char*)rc_Struct, sizeof(radiusCenterStruct));
	
}


/**************************************************************************
*
* @name - handleMsg()
*
* @brief - After each pass through loop(), the robot checks for all messages
*		   it has received, and calls this function once for each message.
*		   The message being received here has radius and center measurements.
*
* @param - msgStruct (structure containing radius and rnb of the center.)
*
* @return - none
*
***************************************************************************/

void handleMsg(irMsg* msgStruct){
	if(msgStruct->length==sizeof(radiusCenterStruct)){
		radiusCenterStruct* msg = (radiusCenterStruct*)(msgStruct->msg);
		if(msg->rc_check_flag == SEND_RC_FLAG){
			radius_center_struct.radius = msg->radius;
			radius_center_struct.center_x = msg->center_x;
			radius_center_struct.center_y = msg->center_y;
			setRGB(255,255,0);
			printf("\r\nRadius center received!\r\n");
			printf("\r\nRadius = %ld\r\n",radius_center_struct.radius);
			printf("\r\nCenter(%ld,%ld)\r\n",radius_center_struct.center_x,radius_center_struct.center_y);
		}
	}
	if(msgStruct->length==sizeof(MoveStatusMsg)){
		MoveStatusMsg* msg = (MoveStatusMsg*)(msgStruct->msg);
		if(msg->msg_check_flag == START_MESSAGE_FLAG){
			start_msg = START_MESSAGE_FLAG;
			setRGB(0,255,0);
			printf("\r\nStart message received\r\n");
			printf("Taking RNB reading\r\n");
			stop_msg = 0;
			stop_recieved = 0;
		}
		if(msg->msg_check_flag == STOP_MESSAGE_FLAG &&stop_recieved == 0){
			stop_msg = STOP_MESSAGE_FLAG;
			setRGB(0,0,255);
			printf("\r\nRNB readings taken.\r\n");
			calculate_Center(measLog, read_count);
			calculate_Radius(measLog, read_count);
			setRGB(0,255,0);
			change_axes();
			printf("\nCenter and Radius calculations done. \r\n");

			printf("Sending radius and center to moving droplet\r\n");
			send_Radius_Center(&radius_center_calc_struct);
			setRGB(255,0,255);
			printf("Radius and Center sent!\r\n");
			read_count = 0;
			start_msg = 0;
			stop_recieved = 1;
		}
	}
}

/**************************************************************************
*
* @name - send_Radius_Center()
*
* @brief - This function sends the radius and center measurements to
*		   the moving droplet.
*
* @param - rc_Struct (structure containing radius and rnb of the center.)
*
* @return - none
*
***************************************************************************/

void take_Rnb_Reading(CircleMeas *meas_log){
	
	uint32_t time_data;
	meas_log->x = ((last_good_rnb.range)*(cos(degToRad(last_good_rnb.bearing)))); //xi = r*cos(theta)
	meas_log->y = ((last_good_rnb.range)*(sin(degToRad(last_good_rnb.bearing)))); //yi = r*sin(theta)
	(time_data) = getTime();
	last_heading = last_good_rnb.heading;
	//last_update->y = meas_log->y;
	//last_update->x = meas_log->x;
	last_x = meas_log->x;
	last_y = meas_log->y;
	printf("{{%u, %d, %d}, {%ld, %ld}, %lu},\r\n",last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading, meas_log->x, meas_log->y, time_data);
	//printf("\r\nReading %hu\tCoordinates (%d,%d)\tTime - %lu\r\n",read_count,meas_log->x,meas_log->y,time_data);
	//printf("%d, (%ld, %ld), (%ld, %ld)\r\n", last_heading, last_x, last_y, meas_log->x, meas_log->y);

	rnb_updated = 0;
}

void calculate_Center(CircleMeas* meas_log, uint8_t count){
	
	int64_t alpha = 0;
	int64_t beta = 0;
	int64_t gamma = 0;
	int64_t delta = 0;
	int64_t epsilon = 0;
	
	int64_t xi_sq = 0;
	int64_t yi_sq = 0;
	int64_t sum_xi = 0;
	int64_t sum_yi = 0;
	int64_t sum_xiyi = 0;
	int64_t sum_xi_yisq = 0;
	int64_t sum_yi_xisq = 0;
	int64_t xi_cu = 0;
	int64_t yi_cu = 0;
	
	int64_t center_x_coordinate;
	int64_t center_y_coordinate;
	
	int64_t meas_x,meas_y;
	

	/*****Changing axes wrt moving droplet***/
/*	int16_t h = last_heading;
	int32_t meas_x1;
	int32_t meas_y1;
	float a1, b1, a2, b2;
	float cos_val = cos(degToRad(h));
	float sin_val = sin(degToRad(h));
	printf("%d, (%ld, %ld); (%f, %f)\r\n", last_heading, last_x, last_y, cos_val, sin_val);
	for(uint8_t i=0; i<count; i++){
		meas_x1 = (meas_log->x);
		meas_y1 = (meas_log->y);
		//printf("(%ld, %ld)\r\n", meas_log->x, meas_log->y);
		//printf("(%ld, %ld)\r\n", meas_x1, meas_y1);	
		a1 = ((meas_x1 - last_x)*cos_val);
		b1 = ((meas_y1 - last_y)*sin_val);
		a2 = ((last_x - meas_x1)*sin_val);
		b2 = ((meas_y1 - last_y)*cos_val);
		printf("%f, %f, %f, %f\n\r", a1, b1, a2, b2);
		meas_log->x = (int32_t)(a1+b1);
		meas_log->y = (int32_t)(a2+b2);
		printf("(%ld, %ld)\r\n", meas_log->x, meas_log->y);
		meas_log++;
		
	}
	
	for(uint8_t i=0;i<count;i++)
		meas_log--;
	printf("After changing coordinates..\n\r");
*/
	for(uint8_t i=0;i<count;i++){
		
		meas_x = ((int64_t)(meas_log->x));
		meas_y = ((int64_t)(meas_log->y));
		
		xi_sq += ((meas_x)*(meas_x));
		sum_xi += (meas_x);
		xi_cu += ((meas_x)*(meas_x)*(meas_x));
		
		yi_sq += ((meas_y)*(meas_y));
		sum_yi += (meas_y);
		yi_cu += ((meas_y)*(meas_y)*(meas_y));
		
		sum_xiyi += ((meas_x)*(meas_y));
		sum_xi_yisq += ((meas_x)*(meas_y)*(meas_y));
		sum_yi_xisq += ((meas_y)*(meas_x)*(meas_x));
		
		//printf("(%ld, %ld)\r\n", meas_log->x, meas_log->y);
		meas_log++;
		
	}
		
	alpha = (((count)*(xi_sq)) - ((sum_xi)*(sum_xi)));
	beta = (((count)*(sum_xiyi)) - ((sum_xi)*(sum_yi)));
	gamma = (((count)*(yi_sq)) - ((sum_yi)*(sum_yi)));
	
	delta = ((((count)*(sum_xi_yisq)) - ((sum_xi)*(yi_sq))) + ((count)*(xi_cu)) - ((sum_xi)*(xi_sq)))>>1;
	epsilon = ((((count)*(sum_yi_xisq)) - ((sum_yi)*(xi_sq))) + ((count)*(yi_cu)) - ((sum_yi)*(yi_sq)))>>1;

	center_x_coordinate  = ((((delta)*(gamma)) - ((beta)*(epsilon))) / (((alpha)*(gamma)) - ((beta)*(beta))));
	center_y_coordinate  = ((((alpha)*(epsilon)) - ((beta)*(delta))) / (((alpha)*(gamma)) - ((beta)*(beta))));
	
	//center_x_coordinate >>= 1;
	//center_y_coordinate >>= 1;
	
	radius_center_calc_struct.center_x  = ((int32_t)(center_x_coordinate));
	radius_center_calc_struct.center_y  = ((int32_t)(center_y_coordinate));
	
	printf("\nCenter coordinates (%ld, %ld)\r\n", radius_center_calc_struct.center_x, radius_center_calc_struct.center_y);
	
}

void calculate_Radius(CircleMeas* meas_log, uint8_t count){
	//uint8_t count = 20;
	
	int64_t radius_x_sq = 0;
	int64_t radius_y_sq = 0;
	int64_t radius_msq = 0;
	
	int64_t center_x_coordinate = ((int64_t)(radius_center_calc_struct.center_x));
	int64_t center_y_coordinate = ((int64_t)(radius_center_calc_struct.center_y));
	
	int64_t meas_x,meas_y;
	
	for(uint8_t i=0;i<count;i++){
		
		meas_x = ((int64_t)(meas_log->x));
		meas_y = ((int64_t)(meas_log->y));
		
		radius_x_sq = (((meas_x) - (center_x_coordinate))*((meas_x) - (center_x_coordinate)));
		radius_y_sq = (((meas_y) - (center_y_coordinate))*((meas_y) - (center_y_coordinate)));
		
		radius_msq += sqrt(((radius_x_sq) + (radius_y_sq)));  
		meas_log++;
	}
	
	radius_msq /= count;
	
	radius_center_calc_struct.radius = radius_msq;
	
	printf("\n Radius - %ld\n",((int32_t)(radius_msq)));
	
}

void change_axes(void){
	float a1, b1, a2, b2;
	int32_t prev_x = radius_center_calc_struct.center_x;
	int32_t prev_y = radius_center_calc_struct.center_y;
	float cos_val = cos(degToRad(last_heading));
	float sin_val = sin(degToRad(last_heading));
	a1 = ((prev_x - last_x)*cos_val);
	b1 = ((prev_y - last_y)*sin_val);
	a2 = ((last_x - prev_x)*sin_val);
	b2 = ((prev_y - last_y)*cos_val);
	radius_center_calc_struct.center_x = (int32_t)(a1+b1);
	radius_center_calc_struct.center_y = (int32_t)(a2+b2);
	printf("New coordinates:(%ld, %ld)\n\r", radius_center_calc_struct.center_x, radius_center_calc_struct.center_y);
}

void send_message(char msgFlag){
	MoveStatusMsg msg;
	msg.msg_check_flag = msgFlag;
	
	irSend(ALL_DIRS, (char*)(&msg), sizeof(MoveStatusMsg));

}

void doRnbBroadcast(void){
	broadcastRnbData();
	if(isMoving()>=0){
		scheduleTask(RNB_BROADCAST_PERIOD, doRnbBroadcast, NULL);
	}
}

void startMove(uint8_t dir, uint16_t numSteps){
	send_message(START_MESSAGE_FLAG);
	waitForTransmission(ALL_DIRS);
	doRnbBroadcast();
	moveSteps(dir, numSteps);
	scheduleTask(90000, stopMove, NULL);
}



/********************** INIT **************************************/

void init(void){
	if( getDropletID() == MOVING_DROPLET_ID ){
		lastMessageTime = 0;
		motorAdjusts[0][0] = 0;
		motorAdjusts[0][1] = 1000;
		motorAdjusts[0][2] = -1000;
		
		startMove(0,150);
		setRGB(255,0,0);
	}else{
		read_count = 0;		
	}

}

/********************** LOOP **************************************/

void loop(void){

	/********************** MOVING_DROPLET LOOP CODE ******************/

	if(( getDropletID() == MOVING_DROPLET_ID )){
		if(isMoving() != -1){
			if(getTime()-lastBroadcastTime > RNB_BROADCAST_PERIOD){
				setRGB(0,255,0);
				broadcastRnbData();
				lastBroadcastTime = getTime();
			}
		}else if(!irIsBusy(ALL_DIRS)){
			if(getTime()-lastMessageTime > MESSAGE_PERIOD){
				send_message(STOP_MESSAGE_FLAG);
				setRGB(0,0,255);
				lastMessageTime = getTime();
			}
		}
	}else{
	/******************** ORIGIN_DROPLET LOOP CODE ******************/		
		if((start_msg == START_MESSAGE_FLAG) && (stop_msg == 0)){
			setRGB(0,255,255);
			if(rnb_updated == 1){
				take_Rnb_Reading(&(measLog[read_count]));
				read_count++;
			}
		}
	}

	/*****************************************************************/
}
	

