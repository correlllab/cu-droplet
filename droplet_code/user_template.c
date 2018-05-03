#include "user_template.h"

volatile uint8_t stop_msg = 0;
volatile uint8_t start_msg = 0;
volatile uint8_t read_count = 0;
//volatile uint8_t broadcast_count = 0;
volatile uint32_t lastBroadcastTime;
uint8_t stop_recieved = 0;


float pt_score_0 = 0;
float pt_score_1 = 0;

uint8_t loop_count = 0;

int16_t last_heading;
int32_t last_x;
int32_t last_y;
CircleMeas last_update[0];

/////////////////////// POINT SCORE FUNCTIONS /////////////////////////////////////
#if 0
inline int32_t distanceBetween(CircleMeas* a, CircleMeas* b){
	return sqrt((a->x-b->x)*(a->x-b->x) + (a->y-b->y)*(a->y-b->y));
}
#endif
#if 0
void take_readings(CircleMeas* meas_log,uint8_t count){
	uint8_t k;
	for( k = 0; k<count;k++){
		meas_log->x = meas_arr[k][0];
		meas_log->y = meas_arr[k][1];
		printf("(%ld,%ld)\r\n",meas_log->x,meas_log->y);
		meas_log++;
	}
}
#endif

void calculateRadiusCenter(pointCircle ptCirc){

	int32_t a,b,c;

	a = (((ptCirc.x1)*(ptCirc.y2 - ptCirc.y3)) - ((ptCirc.y1)*(ptCirc.x2 - ptCirc.x3)) + ((ptCirc.x2)*(ptCirc.y3)) - ((ptCirc.x3)*(ptCirc.y2)));
	b = ((((ptCirc.x1*ptCirc.x1) + (ptCirc.y1*ptCirc.y1))*(ptCirc.y3 - ptCirc.y2)) + (((ptCirc.x2*ptCirc.x2) + (ptCirc.y2*ptCirc.y2))*(ptCirc.y1 - ptCirc.y3)) + (((ptCirc.x3*ptCirc.x3) + (ptCirc.y3*ptCirc.y3))*(ptCirc.y2 - ptCirc.y1)));
	c = ((((ptCirc.x1*ptCirc.x1) + (ptCirc.y1*ptCirc.y1))*(ptCirc.x2 - ptCirc.x3)) + (((ptCirc.x2*ptCirc.x2) + (ptCirc.y2*ptCirc.y2))*(ptCirc.x3 - ptCirc.x1)) + (((ptCirc.x3*ptCirc.x3) + (ptCirc.y3*ptCirc.y3))*(ptCirc.x1 - ptCirc.x2)));
	//d = ((((ptCirc.x1*ptCirc.x1) + (ptCirc.y1*ptCirc.y1))*(ptCirc.x3*ptCirc.y2 - ptCirc.x2*ptCirc.y3)) + (((ptCirc.x2*ptCirc.x2) + (ptCirc.y2*ptCirc.y2))*(ptCirc.x1*ptCirc.y3 - ptCirc.x3*ptCirc.y1)) + (((ptCirc.x3*ptCirc.x3) + (ptCirc.y3*ptCirc.y3))*(ptCirc.x2*ptCirc.y1 - ptCirc.x1*ptCirc.y2)));

	x_center = (-(b/(2*a)));
	y_center = (-(c/(2*a)));
	c_radius = abs(sqrt((pow(((x_center) - (ptCirc.x1)),2)) + (pow(((y_center) - (ptCirc.y1)),2))));


}

void comparePointScore(pointCircle point_Score_Circle){

	if((pt_score_0 > pt_score_1) || (pt_score_1 == 0)){
		
		pt_score_1 = pt_score_0;
		
		three_Point_Circle.x1 = point_Score_Circle.x1;
		three_Point_Circle.x2 = point_Score_Circle.x2;
		three_Point_Circle.x3 = point_Score_Circle.x3;
		three_Point_Circle.y1 = point_Score_Circle.y1;
		three_Point_Circle.y2 = point_Score_Circle.y2;
		three_Point_Circle.y3 = point_Score_Circle.y3;
		
		printf("\r\nCompared Point Score : %f",pt_score_1);
		
		calculateRadiusCenter(three_Point_Circle);
		printf("\r\ncenter (%ld,%ld)\r\n",x_center,y_center);
		printf("\r\nradius - %ld\r\n",c_radius);
	}

}

float inlierRatio(CircleMeas* meas_log, uint8_t count){
	
	//CircleMeas meas_temp;
	
	int32_t dist;
	float point_score;
	uint8_t inlier_count = 0;
	uint8_t outlier_count = 0;
	uint8_t total_count = 0;
	float inlier;
	float outlier;
	float total;
	
	int32_t x_meas = x_center;
	int32_t y_meas = y_center;
	int32_t rad_meas = c_radius;
	
	for(uint8_t q=0;q<count;q++){
		
		dist = abs(sqrt((pow((x_meas - meas_log[q].x),2)) + (pow((y_meas - meas_log[q].y),2))));
		//printf("\r\nmeas_log(%ld,%ld) and center(%ld,%ld)",meas_log[q].x,meas_log[q].y,x_meas,y_meas);
		//printf("\r\ndistance 1 - %ld",dist);
		dist = abs(dist - rad_meas);
		//printf("\r\ndistance 2 - %ld",dist);
		if(dist <= INLIER_BOUND){
			inlier_count++;
		}
		else{
			outlier_count++;
		}
		
	}
	inlier = (float)(inlier_count);
	outlier = (float)(outlier_count);
	total = (float)(total_count);
	total = inlier + outlier;
	point_score = ((inlier)/(total));
	//printf("\r\nPoint Score : %f\tInlier Count : %f\tOutlier Count : %f\tTotal Count : %f\r\n",point_score,inlier,outlier,total);
	
	return point_score;
	
}

void randCircleIterations(CircleMeas* meas_log, uint8_t count){
	
	pt_score_1 = 0;
	
	pointCircle point_circ;
	
	
	for(uint8_t l=0;l<200;l++){
		
		uint8_t a,b,c;
		pt_score_0 = 0;
		uint8_t u = 0;
		
		//radiusCenterStruct radc;
		
		do{
			
			a = (randByte()%count);
			b = (randByte()%count);
			c = (randByte()%count);
			if((a!=b)&&(b!=c)&&(a!=c)){
				u = 0x23;
			}
		}
		while(u != 0x23);
		
		//printf("\r\na=%hu\tb=%hu\tc=%hu\r\n",a,b,c);
		
		point_circ.x1 =  meas_log[a].x;
		point_circ.x2 =  meas_log[b].x;
		point_circ.x3 =  meas_log[c].x;
		point_circ.y1 =  meas_log[a].y;
		point_circ.y2 =  meas_log[b].y;
		point_circ.y3 =  meas_log[c].y;
		
		
		//CircleMeas* meas_1;
		//CircleMeas* meas_2;
		
		calculateRadiusCenter(point_circ);
		//printf("\r\nout_center(%ld,%ld)",x_center,y_center);
		pt_score_0 = inlierRatio(meas_log, count);
		
		if(pt_score_0 >= MIN_INLIER_RATIO){
			comparePointScore(point_circ);
			//a = 24;
		}
	}
	printf("\r\n\r\nbest fit circle parameters\r\n\r\n");
	calculateRadiusCenter(three_Point_Circle);
	printf("\r\ncenter (%ld,%ld)\r\n",x_center,y_center);
	printf("\r\nradius - %ld\r\n",c_radius);
}

////////////////////////////////////////////////////////////////////////////

//////////////////////////// MERGE FUNCTIONS ///////////////////////////////

int32_t min_Distance(CircleMeas* meas_log, uint8_t count){

	int32_t xcor_1,xcor_2;
	int32_t ycor_1,ycor_2;
	int32_t dist_sq[3] = {0,0,0};
	int32_t distance;
	uint8_t a,b;
	uint8_t i = 0;
	uint8_t l;

	for(a=0;a < count-1;a++){

		xcor_1 = meas_log[a].x;
		ycor_1 = meas_log[a].y;

		for(b=a+1;b<count;b++){

			xcor_2 = meas_log[b].x;
			ycor_2 = meas_log[b].y;

			distance = abs(sqrt((pow(xcor_1 - xcor_2,2)) + (pow(ycor_1 - ycor_2,2))));
			if(i == 0){
				dist_sq[0] = distance;
				l = 0;
				i = 1;
				//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
			}
			else{

				if((distance < dist_sq[0]) && (dist_sq[1] == 0) && (dist_sq[2] == 0)){
					l=1;
					dist_sq[1]= distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
				else if((distance < dist_sq[0]) && (distance <= dist_sq[1]) && (dist_sq[2] == 0)){
					l=2;
					dist_sq[2] = distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
				else if((distance <= dist_sq[0]) && (distance >= dist_sq[1]) && (dist_sq[2] == 0)){
					l=3;
					dist_sq[2] = dist_sq[1];
					dist_sq[1] = distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
				else if((distance <= dist_sq[0]) && (distance >= dist_sq[1]) && (distance > dist_sq[2])){
					l=4;
					dist_sq[0] = distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
				else if((distance < dist_sq[0]) && (distance <= dist_sq[1]) && (distance >= dist_sq[2])){
					l=5;
					dist_sq[0] = dist_sq[1];
					dist_sq[1] = distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
				else if((distance < dist_sq[0]) && (distance < dist_sq[1]) && (distance <= dist_sq[2])){
					l=6;
					dist_sq[0] = dist_sq[1];
					dist_sq[1] = dist_sq[2];
					dist_sq[2] = distance;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}

				else{
					l=7;
					//printf("(%ld,%ld) - (%ld,%ld)\t%ld\t(  %ld,  %ld,  %ld), %d\r\n",xcor_1,ycor_1,xcor_2,ycor_2,distance,dist_sq[0],dist_sq[1],dist_sq[2],l);
				}
			}
		}
	}
	printf("\r\n1st minimum distance = %ld\r\n",dist_sq[2]);
	printf("\r\n2nd minimum distance = %ld\r\n",dist_sq[1]);
	printf("\r\n3rd minimum distance = %ld\r\n",dist_sq[0]);

	if((dist_sq[0] != 0)&&(dist_sq[0]<=8)){
		return dist_sq[0];
	}
	else if((dist_sq[0] == 0) && (dist_sq[1] != 0)&&(dist_sq[1]<=8)){
		return dist_sq[1];
	}
	else if((dist_sq[0] == 0) && (dist_sq[1] == 0) && (dist_sq[2] != 0)&&(dist_sq[2]<=8)){
		return dist_sq[2];
	}
	else{
		return 0xAAA;
	}

}

void mergePoints(CircleMeas* meas_log, uint8_t count,int32_t min_distance){

	int32_t xcor_1,xcor_2;
	int32_t ycor_1,ycor_2;
	int32_t distance;
	int32_t merge_count = 1;
	uint8_t a,b;
	

	for(a=0;a<count-1;a++){

		merge_count = 1;

		if(((meas_log[a].x) != 0xF0F0) && ((meas_log[a].y) != 0xF0F0)){

			xcor_1 = meas_log[a].x;
			ycor_1 = meas_log[a].y;

			for(b=a+1;b<count;b++){

				if(((meas_log[b].x) != 0xF0F0) && ((meas_log[b].y) != 0xF0F0)){

					xcor_2 = meas_log[b].x;
					ycor_2 = meas_log[b].y;
					
					distance = sqrt((pow(xcor_1 - xcor_2,2)) + (pow(ycor_1 - ycor_2,2)));

					if(distance < min_distance){

						xcor_1 += meas_log[b].x;
						ycor_1 += meas_log[b].y;
						merge_count++;
						//printf("\r\nmerged (%ld,%ld) with (%ld,%ld)\r\n",meas_log[b].x,meas_log[b].y,meas_log[a].x,meas_log[a].y);
						meas_log[b].x = 0xF0F0;
						meas_log[b].y = 0xF0F0;
						//printf(" merged point(%ld,%ld)\r\n",meas_log[b].x,meas_log[b].y);

					}
				}
			}

			meas_log[a].x = ((xcor_1)/(merge_count));
			meas_log[a].y = ((ycor_1)/(merge_count));

		}
	}
}

uint8_t mergeStructSort(CircleMeas* measlog0, CircleMeas* measlog1,uint8_t count){

	uint8_t a;
	uint8_t b=0;

	for(a=0;a<count;a++){
		if(((measlog0[a].x) != 0xF0F0)&&((measlog0[a].y) != 0xF0F0)){
			measlog1[b].x = measlog0[a].x;
			measlog1[b].y = measlog0[a].y;
			b++;
		}
	}
	
	for(a=0;a<MAX_NUM_MEAS;a++){
		measlog0[a].x = 0;
		measlog0[a].y = 0;
	}

	return b;

}

//////////////////////////// INLIER POINT FUNCTION ////////////////////////////

uint8_t inlierPointsSave(CircleMeas* measlog0, CircleMeas* measlog1,uint8_t count){
	
	uint8_t inlier_count = 0;
	uint8_t a;
	uint32_t distance;
	
	printf("\r\nINLIER POINTS - \r\n\r\n{");
		
		for(a=0;a<count;a++){
			
			distance = (abs(c_radius - sqrt(pow((x_center - (measlog1[a].x)),2) + pow((y_center - (measlog1[a].y)),2))));
			
			
			if(distance <= INLIER_BOUND){
				
				measlog0[inlier_count].x = measlog1[a].x;
				measlog0[inlier_count].y = measlog1[a].y;
				
				printf("{94, 126, -114}, {%ld, %ld}, 237060},\r\n",measlog0[inlier_count].x,measlog0[inlier_count].y);
				
				inlier_count++;
				
			}
		}
		
	printf("};\r\n");
	
	printf("\r\nINLIER POINT COUNT : %hu",inlier_count);
	
	return inlier_count;
}

////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////



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
			//int32_t min_dist;
			printf("\r\nRNB readings taken.\r\n");
			
			
			////////////// POINT SCORE METHOD (MIN INLIER) //////////////////

			#ifdef POINT_SCORE_MIN_INLIER

			printf("\r\n\n\nPOINT SCORE METHOD (MIN INLIER)\n\n\n");
			randCircleIterations(measLog0,read_count);
			uint8_t r;
			r = inlierPointsSave(measLog3,measLog0,read_count);
			calculate_Center(measLog3, r);
			calculate_Radius(measLog3, r);
			
			#endif

			/////////////////////////////////////////////////////////////////

			//////////////////// REGULAR CURVE FIT METHOD ////////////////////////////

			#ifdef REGULAR_CURVE_FIT
			
			printf("\r\n\n\nREGULAR CURVE FIT METHOD\n\n\n");
			
			calculate_Center(measLog0, read_count);
			calculate_Radius(measLog0, read_count);
			
			#endif

			///////////////////////////////////////////////////////////////////

			////////////// POINT SCORE METHOD WITH MERGE (MIN INLIER) //////////////////

			#ifdef POINT_SCORE_MERGE

			printf("\r\n\n\nPOINT SCORE METHOD WITH MERGE\n\n\n");

			int32_t min;
			//int min_1;
			uint8_t p,q;

			min = min_Distance(measLog0,read_count);
			//min_1 = (int)(min);
			if(min != 0xAAA){
				mergePoints(measLog0,read_count,min);
				//printf("\r\nMerge done");
			}
			p = mergeStructSort(measLog0,measLog1,read_count);
			randCircleIterations(measLog1,p);
			
			q = inlierPointsSave(measLog0,measLog1,p);
			calculate_Center(measLog0, q);
			calculate_Radius(measLog0, q);
			
			
			#endif

			////////////////////////////////////////////////////////////////////

			/////////////////// REGULAR CURVE FIT WITH MERGE ///////////////////

			#ifdef REGULAR_CURVE_FIT_MERGE

			printf("\r\n\n\nREGULAR CURVE FIT WITH MERGE\n\n\n");
			
			calculate_Center(measLog1, p);
			calculate_Radius(measLog1, p);
			
			#endif
			
			//calculate_Center(measLog, read_count);
			//calculate_Radius(measLog, read_count);
			setRGB(0,255,0);
			//change_axes();
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
	
	for(uint8_t i=0;i<count;i++){
		
		//if((meas_log->x != 0xF0F0) && (meas_log->x != 0xF0F0)){
		
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
		//}
		
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
		
		//if((meas_log->x != 0xF0F0) && (meas_log->x != 0xF0F0)){
		
		meas_x = ((int64_t)(meas_log->x));
		meas_y = ((int64_t)(meas_log->y));
		
		radius_x_sq = (((meas_x) - (center_x_coordinate))*((meas_x) - (center_x_coordinate)));
		radius_y_sq = (((meas_y) - (center_y_coordinate))*((meas_y) - (center_y_coordinate)));
		
	radius_msq += sqrt(((radius_x_sq) + (radius_y_sq)));  //}
	meas_log++;
}

radius_msq /= count;

radius_center_calc_struct.radius = radius_msq;

printf("\n Radius - %ld\n",((int32_t)(radius_msq)));

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
	
	//read_count = 15;
	//take_readings(measLog0,read_count);
	
	if( getDropletID() == MOVING_DROPLET_ID ){
		lastMessageTime = 0;
		motorAdjusts[0][0] = 0;
		motorAdjusts[0][1] = -650;
		motorAdjusts[0][2] = -650;
		
		startMove(0,250);
		setRGB(255,0,0);
		}else{
		read_count = 0;
		setRGB(250,0,0);
	}


	//setRGB(0,200,200);
}

/********************** LOOP **************************************/

void loop(void){
	
	//#if 0
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
				take_Rnb_Reading(&(measLog0[read_count]));
				read_count++;
			}
		}
	}

	/*****************************************************************/
	//#endif

}


