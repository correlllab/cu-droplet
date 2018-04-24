#include "user_template.h"

volatile uint8_t stop_msg = 0;
volatile uint8_t start_msg = 0;
volatile uint8_t read_count = 0;
//volatile uint8_t broadcast_count = 0;
volatile uint32_t lastBroadcastTime;

volatile uint32_t stop_time = 0;
uint8_t stop_recieved = 0;
//int32_t circle_arr[6][2] ={{200,190},{200,50},{-280,50},{-280,190},{210,120},{-290,120}};
//int32_t circle_arr[4][2] ={{200,700},{700,200},{-300,200},{200,-300}};
int32_t circle_arr[3][2] ={{190,690},{690,210},{553, 553}};
uint8_t loop_count = 0;
uint8_t stopmsg_sent = 0;
uint8_t averaged = 0;

float last_heading;
int32_t last_x;
int32_t last_y;
int8_t receive_count = 0;
int32_t avg_radius = 0;
int32_t average_x = 0;
int32_t average_y = 0;

int32_t avg_range = 0;

int8_t move_count = 0;

void curve_fit_check(uint8_t count){
	//if((getDropletID() == ORIGIN_DROPLET_ID) && (loop_count == 0)){
		for(uint8_t index = 0; index <count; index++){
			measLog[index].x = circle_arr[index][0];
		measLog[index].y = circle_arr[index][1]; }
		read_count = count;
		//printf("\r\nRNB readings taken.\r\n");
		calculate_Center(measLog, read_count);
		calculate_Radius(measLog, read_count);
		setRGB(0,100,0);
		printf("\nCenter and Radius calculations done. \r\n");
		setRGB(100,0,100);
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
/*
void send_Radius_Center(radiusCenterStruct* rc_Struct){
	irSend(ALL_DIRS, (char*)rc_Struct, sizeof(radiusCenterStruct));
	
}
*/

void prepSpeedMsg(void){
	SpeedMsgNode* msgNode = (SpeedMsgNode*)myMalloc(sizeof(SpeedMsgNode));
	msgNode->numTries = 0;
	msgNode->msg = radius_center_calc_struct;
	sendSpeedMsg(msgNode);
}

uint32_t getExponentialBackoff(uint8_t c){
	volatile uint32_t k;
	volatile uint32_t N;
	
	N= (((uint32_t)1)<<c);
	
	k = randQuad()%N;
	return ((k*16)+5);///20000000;
	
}

void sendSpeedMsg(SpeedMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if(msgNode->numTries>6){
			myFree(msgNode);
			printf("freeing message..\r\n");
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendSpeedMsg, msgNode);
			printf("Backingoff..\r\n");
		}
		msgNode->numTries++;
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(radiusCenterStruct));
		myFree(msgNode);
		lastMessageTime = getTime();
		printf("Message Sent!\r\n");
	}
}
/**************************************************************************
*
* @name - handleMsg()
*
* @brief - After each pass through loop(), the robots for all messages
*		   it has received, and calls this function once for each message.
*		   The message being received here has radius and center measurements.
*
* @param - msgStruct (structure containing radius and rnb of the center.)
*
* @return - none
*
***************************************************************************/

void handleMsg(irMsg* msgStruct){
	
	if(getDropletID() == MOVING_DROPLET_ID ){
		if(msgStruct->length==sizeof(radiusCenterStruct)){
			radiusCenterStruct* msg = (radiusCenterStruct*)(msgStruct->msg);
			if(msg->rc_check_flag == SEND_RC_FLAG){
				radius_center_receive_struct[receive_count].radius = msg->radius;
				radius_center_receive_struct[receive_count].center_x = msg->center_x;
				radius_center_receive_struct[receive_count].center_y = msg->center_y;
				radius_center_receive_struct[receive_count].range = msg->range;
				
				setRGB(100,100,0);
				
				printf("\r\nRadius center received from %04X, Count: %u\r\n", msgStruct->senderID, receive_count);
				printf("\r\nRadius = %d,  range = %hu\r\n",radius_center_receive_struct[receive_count].radius, radius_center_receive_struct[receive_count].range);
				printf("\r\nCenter(%d,%d)\r\n",radius_center_receive_struct[receive_count].center_x,radius_center_receive_struct[receive_count].center_y);
				receive_count++;
			}
		}
	}
	else{
		if(msgStruct->length==sizeof(MoveStatusMsg)){
			MoveStatusMsg* msg = (MoveStatusMsg*)(msgStruct->msg); 
			if(msg->msg_check_flag == START_MESSAGE_FLAG){
				start_msg = START_MESSAGE_FLAG;
				setRGB(0,100,0);
				printf("\r\nStart message received\r\n");
				printf("Taking RNB reading\r\n");
				stop_msg = 0;
				stop_recieved = 0;
				receive_count = 0;
				read_count = 0;
			}
			if(msg->msg_check_flag == STOP_MESSAGE_FLAG &&stop_recieved == 0){
				stop_msg = STOP_MESSAGE_FLAG;
				if(read_count<3){
					setRGB(0,50,20);
					return;
				}
				printf("\r\nRNB readings taken.\r\n");
				avg_range/=read_count;
				calculate_Center(measLog, read_count);
				calculate_Radius(measLog, read_count);
				calculate_heading(measLog);
				change_axes(measLog);
				printf("\nCenter and Radius calculations done. \r\n");

				//send_Radius_Center(&radius_center_calc_struct);
				if(radius_center_calc_struct.radius >= 0){
					printf("Sending radius and center to moving droplet\r\n");
					prepSpeedMsg();
					setRGB(100,0,100);
				}else{
					setRGB(50,20,10);
				}
				
			
				read_count = 0;
				start_msg = 0;
				stop_recieved = 1;
			
			}
		}
	}
}


void average_cicfit(void){
	int32_t sum_range = 0;
	
	if(receive_count == 0){
		printf("Did not recieve any radius\r\n");
	}
	else{
		for(int i=0; i<receive_count; i++)
		{
			avg_radius += ((200 - radius_center_receive_struct[i].range )*radius_center_receive_struct[i].radius) ;
			sum_range += (200 - radius_center_receive_struct[i].range );
		}
		avg_radius /= sum_range;
	
		for(int i=0; i<receive_count; i++)
			average_x += ((200 - radius_center_receive_struct[i].range )*radius_center_receive_struct[i].center_x );
		average_x /= sum_range;
	
		for(int i=0; i<receive_count; i++)
			average_y += ((200 - radius_center_receive_struct[i].range )*radius_center_receive_struct[i].center_y );
		average_y/= sum_range;
	
		printf("Average Radius: %ld, Average Center Coordinates: (%ld, %ld)\n\r", avg_radius, average_x, average_y);
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
	
	avg_range += last_good_rnb.range;
//	last_x = meas_log->x;
//	last_y = meas_log->y;
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
	radius_center_calc_struct.rc_check_flag = SEND_RC_FLAG;
	radius_center_calc_struct.center_x  = ((int16_t)(center_x_coordinate));
	radius_center_calc_struct.center_y  = ((int16_t)(center_y_coordinate));
	radius_center_calc_struct.range = (uint8_t)avg_range;
	
	printf("\nCenter coordinates (%d, %d)\r\n", radius_center_calc_struct.center_x, radius_center_calc_struct.center_y);
	
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
	
	radius_center_calc_struct.radius = (int16_t)radius_msq;
	
	printf("\n Radius - %ld\n",((int32_t)(radius_msq)));
	
}

void calculate_heading(CircleMeas *meas_log){
	
	float heading = 0 , head_check = 0, pi = 3.141;
	printf("Last heading: %f\n\r", degToRad(last_heading));
	
	printf("{%d, %d}, {%ld, %ld}, {%ld, %ld}\n\r", radius_center_calc_struct.center_x, radius_center_calc_struct.center_y, meas_log[read_count-1].x, meas_log[read_count -1].y, meas_log[read_count-2].x, meas_log[read_count-2].y );	
	
	float tan_slope1 = ((float)( radius_center_calc_struct.center_x - meas_log[read_count-1].x))/((float)(meas_log[read_count - 1].y - radius_center_calc_struct.center_y));
	float tan_slope2 = ((float)( meas_log[read_count-2].y - meas_log[read_count-1].y))/((float)( meas_log[read_count-2].x - meas_log[read_count-1].x));
	printf("tan_slope1: %f\n\r", tan_slope1);	
	printf("tan_slope2: %f\n\r", tan_slope2);	
	heading = atan(tan_slope1);
	head_check = atan(tan_slope2);

	
	if(heading < 0)
			heading = pi + heading;
	if(head_check < 0)
		head_check = pi + head_check;		
	
	printf("Heading: %f\n\r", (heading));
	printf("Head check: %f\n\r", (head_check));		
	
	if(head_check > heading){
		if((head_check - heading) < (float)pi/2 )
			last_heading = pi + heading;
		else 
			last_heading = heading; 
	}
	else{
		if((heading - head_check) < (float)pi/2)
			last_heading = pi + heading;
		else
			last_heading = heading;
	}		
	/*if ((((head_check - heading) < (float)(pi/2)) && ((head_check - heading) > 0)) || (((heading - head_check) < (float)(pi/2))  && ((heading - head_check) >0 )) )
		last_heading = heading;
	else
		last_heading = pi - heading;*/
	

	//printf("read_count: %d\n\r", read_count);	
	printf("Last heading: %f\n\r", (float)(last_heading));	
								
							
}

void change_axes(CircleMeas *meas_log){
	float a1, b1, a2, b2;
	int32_t prev_x = radius_center_calc_struct.center_x;
	int32_t prev_y = radius_center_calc_struct.center_y;
	
	float cos_val = cos(last_heading);
	float sin_val = sin(last_heading);
	a1 = ((prev_x - meas_log[read_count-1].x)*cos_val);
	b1 = ((prev_y - meas_log[read_count-1].y)*sin_val);
	a2 = ((meas_log[read_count-1].x - prev_x)*sin_val);
	b2 = ((prev_y - meas_log[read_count-1].x)*cos_val);
	radius_center_calc_struct.center_x = (int32_t)(a1+b1);
	radius_center_calc_struct.center_y = (int32_t)(a2+b2);
	printf("(%ld, %ld), (%ld, %ld), %f, (%f, %f), (%f, %f), (%f, %f)\n\r", meas_log[read_count-1].x, meas_log[read_count-1].y, prev_x, prev_y, last_heading, cos_val, sin_val, a1, b1, a2, b2);
	printf("New coordinates:(%d, %d)\n\r", radius_center_calc_struct.center_x, radius_center_calc_struct.center_y);
}

void send_message(char msgFlag){
	MoveStatusMsg msg;
	msg.msg_check_flag = msgFlag;
	
	irSend(ALL_DIRS, (char*)(&msg), sizeof(MoveStatusMsg));

}

void startMove(uint8_t dir, uint16_t numSteps){
	send_message(START_MESSAGE_FLAG);
	waitForTransmission(ALL_DIRS);
	//moveSteps(dir, numSteps);
	scheduleTask(90000, stopMove, NULL);
}



/********************** INIT **************************************/

void init(void){
/*	radius_center_calc_struct.center_x = -50;
	radius_center_calc_struct.center_y = -350;
	last_x = -110;
	last_y = 90;
	last_heading = 100 ;
	
	change_axes();
*/		
	if( getDropletID() == MOVING_DROPLET_ID ){
		lastMessageTime = 0;
		
		if(MOVING_DROPLET_ID == 0x8625){
			/**** Motor adjust values for 8625**/
			motorAdjusts[0][0] = 700;
			motorAdjusts[0][1] = 700;
			motorAdjusts[0][2] = 0;	
		}
		
		else if(MOVING_DROPLET_ID == 0xFCD0){
			/**** Motor adjust values for FCD0**/
			motorAdjusts[0][0] = 0;
			motorAdjusts[0][1] = -400;
			motorAdjusts[0][2] = -400;
		}
		
		else if(MOVING_DROPLET_ID == 0x4327){
			/**** Motor adjust values for 4327**/
			motorAdjusts[0][0] = 0;
			motorAdjusts[0][1] = 1000;
			motorAdjusts[0][2] = -1000;
		}

		begun = 0;
		setRGB(100,0,0);
	}else{
		read_count = 0;		
	}

}

/********************** LOOP **************************************/

void loop(void){

	/********************** MOVING_DROPLET LOOP CODE ******************/
	if(( getDropletID() == MOVING_DROPLET_ID )){
		if(!begun && (getTime() > 2000)){
			startMove(0,150);
			begun = 1;
		}
		

		if(isMoving() == -1 && move_count <20){
			
			if(getTime()-lastBroadcastTime > RNB_BROADCAST_PERIOD){
				setRGB(0,100,0);
				broadcastRnbData();
				lastBroadcastTime = getTime();
				move_count++;
				moveSteps(0,5);
			}
		}else if(!irIsBusy(ALL_DIRS) && move_count == 20){
			if(getTime()-lastMessageTime > MESSAGE_PERIOD && stopmsg_sent < 20){
				send_message(STOP_MESSAGE_FLAG);
				setRGB(0,0,100);
				stopmsg_sent++;
				stop_time = getTime();
			}
		
			if((getTime() - stop_time) > WAIT_TIME && averaged == 0){
				printf("Averaging..\r\n");
				average_cicfit();
				averaged = 1;
			}
		}

	}else{
	/******************** ORIGIN_DROPLET LOOP CODE ******************/		
		if((start_msg == START_MESSAGE_FLAG) && (stop_msg == 0)){
			setRGB(0,100,100);
			if(rnb_updated == 1){
				take_Rnb_Reading(&(measLog[read_count]));
				read_count++;
			}
		}
	}

	/*****************************************************************/
}
	

