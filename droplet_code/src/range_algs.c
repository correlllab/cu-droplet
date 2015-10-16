/*
*	README:
*	For consistencies sake, any time you loop through the brightness matrix, it should look like this:
*		for(emitter)
*		{
*			for(sensor)
*			{
*				brightness_matrix[emitter][sensor]
*			}
*		}
*	There were previous inconsistencies in this code.
*/
#include "range_algs.h"
						
float basis_angle[6] = {-(M_PI/6.0), -M_PI_2, -((5.0*M_PI)/6.0), ((5.0*M_PI)/6.0), M_PI_2, (M_PI/6.0)};
	
//Should maybe be elsewhere?
int16_t bright_meas[NUMBER_OF_RB_MEASUREMENTS][6][6];

void range_algs_init()
{
	for(uint8_t i=0 ; i<NUMBER_OF_RB_MEASUREMENTS ; i++)
	{
		for(uint8_t j=0 ; j<6 ;j++)
		{
			for(uint8_t k=0 ; k<6 ; k++)
			{
				bright_meas[i][j][k] = 0;
			}
		}
}
	cmdID=0;
	processingFlag=0;
}

void collect_rnb_data(uint16_t target_id, uint8_t power)
{	
	char cmd[7] = "rnb_t ";
	cmd[6] = power;
	get_baseline_readings(bright_meas);
	
	hp_ir_targeted_cmd(ALL_DIRS, cmd, 7, target_id);
	delay_ms(POST_MESSAGE_DELAY);
	ir_range_meas();
	//brightness_meas_printout_mathematica();
	use_rnb_data();
}

//TODO: handle variable power.
void broadcast_rnb_data()
{
	uint8_t power = 255;
	hp_ir_cmd(ALL_DIRS, "rnb_r", 5);
	delay_ms(POST_MESSAGE_DELAY);	
	ir_range_blast(power);
}

void receive_rnb_data()
{
	if(!processingFlag){
		cmdID=cmd_sender_id;
		processingFlag=1;
		ir_range_meas();
		get_baseline_readings();
		//uint8_t power = 25; //TODO: get this from the message.
		//schedule_task(10,brightness_meas_printout_mathematica,NULL);
		//brightness_meas_printout_mathematica();
		schedule_task(10, use_rnb_data, NULL);
	}
}

void use_rnb_data()
{
	uint8_t power = 255;
	int16_t brightness_matrix[6][6];
	int16_t matrixSum = pack_measurements_into_matrix(brightness_matrix);
	//print_brightness_matrix(brightness_matrix, matrixSum);		
	//brightness_meas_printout_mathematica();
	if(matrixSum<MIN_MATRIX_SUM_THRESH){
		//printf_P(PSTR("RNB Data not retrieved, as sum<MIN_MATRIX_SUM_THRESH.\r\n"));
		return;
	}
	
	//For testing, comment out the above and use a hardcoded matrix from the mathematica notebook.
	
	float bearing, heading;
	calculate_bearing_and_heading(brightness_matrix, &bearing, &heading);
		
		
	float initial_range = get_initial_range_guess(bearing, heading, power, brightness_matrix);
	printf("%04X | B: %f\tH: %f\tiR: %f\t", last_good_rnb.id_number, rad_to_deg(bearing), rad_to_deg(heading), initial_range);		
	if(initial_range)
	{
		float range = range_estimate(initial_range, bearing, heading, power, brightness_matrix);
		last_good_rnb.range = (range+initial_range)/2.0;
		last_good_rnb.bearing = bearing;
		last_good_rnb.heading = heading;
		last_good_rnb.id_number = cmdID;	
		rnb_updated=1;
	}else{
		printf("R: %f (ERROR)\r\n\n", initial_range);
	}
	processingFlag=0;
}

void calculate_bearing_and_heading(int16_t brightness_matrix[6][6], float* bearing, float* heading)
{
	float bearingX = 0;
	float bearingY = 0;
	float headingX = 0;
	float headingY = 0;
	for(uint8_t e=0;e<6;e++){
		for(uint8_t s=0;s<6;s++){
			bearingX+=brightness_matrix[e][s]*getCosBearingBasis(e,s);
			bearingY+=brightness_matrix[e][s]*getSinBearingBasis(e,s);
			headingX+=brightness_matrix[e][s]*getCosHeadingBasis(e,s);
			headingY+=brightness_matrix[e][s]*getSinHeadingBasis(e,s);
		}
	}
	*bearing = atan2f(bearingY, bearingX);	
	*heading = atan2f(headingY, headingX);
}

float get_initial_range_guess(float bearing, float heading, uint8_t power, int16_t brightness_matrix[6][6])
{
	int8_t bestS = (6-((int8_t)ceilf((3.0*bearing)/M_PI)))%6;
	float alpha = pretty_angle(bearing - basis_angle[bestS]);				  //alpha using infinite approximation
	int8_t bestE = (6-((int8_t)ceilf((3.0*(bearing-heading-M_PI))/M_PI)))%6;					
	float  beta = pretty_angle(bearing - heading - basis_angle[bestE] - M_PI); //beta using infinite approximation	
	
	//printf("(alpha: %f, sensor %u)\r\n", rad_to_deg(alpha), bestS); 	
	if((alpha > M_PI_2) || (alpha < -M_PI_2)){
		printf("ERROR: alpha out of range (alpha: %f, sensor %u)\r\n", rad_to_deg(alpha), bestS); 
		return 0;
	}
	if((beta > M_PI_2)  || (beta < -M_PI_2)){
		printf("ERROR: beta out of range (beta: %f, emitter %u)\r\n",  beta, bestE); 
		return 0;
	}
	//printf("(beta: %f, emitter %u)\r\n",  rad_to_deg(beta), bestE); 	
	// expected contribution (using infinite distance approximation)
	float amplitude;
	float exp_con = sensor_model(alpha)*emitter_model(beta);
	
	if(exp_con > 0)	amplitude = brightness_matrix[bestE][bestS]/exp_con;	
	else
	{
		printf("ERROR: exp_con (%f) is negative (or zero)!\r\n", exp_con); 
		return 0;
	}
	//printf("amp_for_inv: %f\t",amplitude);
	float rMagEst = inverse_amplitude_model(amplitude, power);
	
	float RX = rMagEst*cos(bearing)+DROPLET_SENSOR_RADIUS*(bearingBasis[bestS][0]-headingBasis[bestE][0]);
	float RY = rMagEst*sin(bearing)+DROPLET_SENSOR_RADIUS*(bearingBasis[bestS][1]-headingBasis[bestE][1]);
	
	float rangeEst = hypotf(RX,RY);
	
	return rangeEst;
}

float range_estimate(float init_range, float bearing, float heading, uint8_t power, int16_t brightness_matrix[6][6])
{
	float range_matrix[6][6];
	
	float sensorRXx, sensorRXy, sensorTXx, sensorTXy;
	float alpha, beta, sense_emit_contr;
	float calcRIJmag, calcRx, calcRy;

	int16_t maxBright = -32768;
	uint8_t maxE=0;
	uint8_t maxS=0;
	for(uint8_t e = 0; e < 6; e++)
	{
		for(uint8_t s = 0; s < 6; s++)
		{
			if(brightness_matrix[e][s]>maxBright)
			{
				maxBright = brightness_matrix[e][s];
				maxE = e;
				maxS = s;
			}
			
			if(brightness_matrix[e][s] > 0)
			{												
				sensorRXx = DROPLET_SENSOR_RADIUS*getCosBearingBasis(0,s);
				sensorRXy = DROPLET_SENSOR_RADIUS*getSinBearingBasis(0,s);
				sensorTXx = DROPLET_SENSOR_RADIUS*cosf(basis_angle[e]+heading) + init_range*cosf(bearing);
				sensorTXy = DROPLET_SENSOR_RADIUS*sinf(basis_angle[e]+heading) + init_range*sinf(bearing);

				alpha = atan2f(sensorTXy-sensorRXy,sensorTXx-sensorRXx) - basis_angle[s];
				beta = atan2f(sensorRXy-sensorTXy,sensorRXx-sensorTXx) - basis_angle[e] - heading;

				alpha = pretty_angle(alpha);
				beta = pretty_angle(beta);
				
				sense_emit_contr = sensor_model(alpha)*emitter_model(beta);
				//printf("sense_emit_contr: %f\r\n",sense_emit_contr);
				if(sense_emit_contr>0){
					calcRIJmag = inverse_amplitude_model(brightness_matrix[e][s]/sense_emit_contr, power);
				}else{
					calcRIJmag = 0;
				}
				calcRx = calcRIJmag*cosf(alpha) + sensorRXx - DROPLET_SENSOR_RADIUS*cosf(basis_angle[e]+heading);
				calcRy = calcRIJmag*sinf(alpha) + sensorRXy - DROPLET_SENSOR_RADIUS*sinf(basis_angle[e]+heading);
				range_matrix[e][s] = hypotf(calcRx, calcRy);
				continue;
			}
			range_matrix[e][s]=0;
		}
	}
	
	float rangeMatSubset[3][3];
	float brightMatSubset[3][3];
	float froebNormSquared=0;
	for(uint8_t e = 0; e < 3; e++){
		for(uint8_t s = 0; s < 3; s++){
			uint8_t otherE = ((maxE+(e+5))%6);
			uint8_t otherS = ((maxS+(s+5))%6);
			rangeMatSubset[e][s] = range_matrix[otherE][otherS];
			brightMatSubset[e][s] = (float)brightness_matrix[otherE][otherS];
			froebNormSquared+=powf(brightMatSubset[e][s],2);
		}
	}
	float froebNorm = sqrtf(froebNormSquared);
	float range = 0;
	for(uint8_t e = 0; e < 3; e++){
		for(uint8_t s = 0; s < 3; s++){
			range+= rangeMatSubset[e][s]*powf(brightMatSubset[e][s]/froebNorm,2);
		}
	}
	printf("R: %f\r\n", range);	
	print_range_matrix(range_matrix);
	printf("\n");
	return range;
}

void fill_S_and_T(int16_t brightness_matrix[6][6], int16_t sensor_total[6], int16_t emitter_total[6])
{
	
	for(uint8_t i = 0; i < 6; i++)
	{
		emitter_total[i] = 0;
		sensor_total[i] = 0;
	}
	
	uint8_t s, e;
	
	for(e = 0; e < 6; e++)
	{
		for(s = 0; s < 6; s++)
		{
			sensor_total[s] += brightness_matrix[e][s];
			emitter_total[e] += brightness_matrix[e][s];
		}
	}
}

int16_t pack_measurements_into_matrix(int16_t brightness_matrix[6][6])
{
	int16_t val, meas, max_meas;
	int16_t valSum=0;
	int16_t rowSum[6];
	for(uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
	{
		rowSum[emitter_num]=0;
		for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
		{
			max_meas = -32768;
			for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
			{
				meas = bright_meas[meas_num][emitter_num][sensor_num];
				if(meas>max_meas) max_meas=meas; 
			}
			val = max_meas-bright_meas[0][emitter_num][sensor_num]-DC_NOISE_REMOVAL_AMOUNT;
			if(val<0) val=0;
			valSum+=val;
			rowSum[emitter_num]+=val;
			brightness_matrix[emitter_num][sensor_num] = val;
		}
	}
	for(uint8_t emitter_num=0;emitter_num<6;emitter_num++){
		if(((1.0*rowSum[emitter_num])/(1.0*valSum))>OVER_DOMINANT_ROW_THRESH){
			//printf_P(PSTR("Row %hu 's values represent more than 0.75 of the full brightness matrix."));
			//printf_P(PSTR("In practice, this seemed to indicate a certain infrequent error condition."));
			//printf_P(PSTR("Thus, this row is being thrown out.\r\n"));
			for(uint8_t sensor_num=0;sensor_num<6;sensor_num++){
				brightness_matrix[emitter_num][sensor_num] = 0;	
			}
			valSum-=rowSum[emitter_num];
			break;
		}
	}
	return valSum;
}


void get_baseline_readings()
{
	uint8_t meas_num = 0;
	for (uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
	{
		get_ir_sensors(bright_meas[meas_num][emitter_num], 5);
	}
}

void ir_range_meas()
{
	busy_delay_ms(POST_BROADCAST_DELAY);
	busy_delay_ms(TIME_FOR_SET_IR_POWERS);
	
	for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++)
	{
		uint32_t outer_pre_sync_op = get_time();
		for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
		{
			uint32_t inner_pre_sync_op = get_time();
			get_ir_sensors(bright_meas[meas_num][emitter_dir] , 7);
			while((get_time() - inner_pre_sync_op) < TIME_FOR_GET_IR_VALS){};
			
			delay_ms(DELAY_BETWEEN_RB_MEASUREMENTS);
		}
		//printf("Dur: %lu.\r\n",get_time()-outer_pre_sync_op);
		while((get_time() - outer_pre_sync_op) < TIME_FOR_ALL_MEAS){};

		//set_green_led(100);		
		delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		//set_green_led(0);
	}
	//printf("Argh!\r\n");
}

void ir_range_blast(uint8_t power)
{
	//set_blue_led(255);
	delay_ms(POST_BROADCAST_DELAY);
	//set_blue_led(0);
	uint32_t pre_sync_op = get_time();
	//set_all_ir_powers(((uint16_t)power)+1);
	set_all_ir_powers(256);
	while((get_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS){};

	for(uint8_t dir = 0; dir < 6; dir++)
	{
		pre_sync_op = get_time();
		ir_led_on(dir);
		while((get_time() - pre_sync_op) < TIME_FOR_ALL_MEAS){};
		ir_led_off(dir);
		//set_green_led(100);
		delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		//set_green_led(0);
	}
}

float pretty_angle(float alpha)
{
	float val;
	if (alpha >= 0) val = fmodf(alpha + M_PI, 2*M_PI) - M_PI;
	else val = fmodf(alpha - M_PI, 2*M_PI) + M_PI;
	return val;
}

float rad_to_deg(float rad){
	return (pretty_angle(rad) / M_PI) * 180;
}

float deg_to_rad(float deg){
	return pretty_angle( (deg / 180) * M_PI );
}

float sensor_model(float alpha)
{
	if(fabsf(alpha)>=1.5){
		return 0.0;
	}else if(fabsf(alpha)<=0.62){
		return (1-powf(alpha,4));
	}else{
		return 0.125/powf(alpha,4);
	}
}

float emitter_model(float beta)
{	
	if(fabsf(beta)>=1.5){
		return 0.0;
	}else if(fabsf(beta)<=0.72){
		return (0.94+powf(beta,2)*0.5-powf(beta,4));
	}else{
		return 0.25/powf(beta,4);
	}
}

float amplitude_model(float r, uint8_t power)
{
	if(power==255)			return 15.91+(12985.5/powf(r+0.89,2.0));
	//else if(power ==250)	return (1100./((r-4.)*(r-4.)))+12.5;
	else					printf_P(PSTR("ERROR: Unexpected power: %hhu\r\n"),power);
	return 0;
}

float inverse_amplitude_model(float ADC_val, uint8_t power)
{
	if(power == 255)		return (-1.5+(131.5/sqrtf(ADC_val-3.85)));
	//else if(power == 250) return (33.166/sqrtf(ADC_val - 12.5)) + 4;
	else					printf_P(PSTR("ERROR: Unexpected power: %hhu\r\n"),power);
	return 0;
}


void debug_print_timer(uint32_t timer[14])
{
	printf_P(PSTR("Duration: %lu\r\n"),(timer[13] - timer[0]));
	printf("|  ");
	for(uint8_t i=1 ; i<13 ; i++)
	{
		printf_P(PSTR("%3lu  |  "),timer[i] - timer[i-1]);
	}
	printf("\r\n");
}

void print_brightness_matrix(int16_t brightness_matrix[6][6], int16_t sum)
{
	printf("{\r\n");
	for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++)
	{
		printf("\t{");
		for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++)
		{
			printf("%4d",brightness_matrix[emitter_num][sensor_num]);
			if(sensor_num<5) printf(",");
		}
		printf("}");		
		if(emitter_num<5) printf(",");
		printf("\r\n");
	}
	printf("}; sum=%d\r\n",sum);
}

void print_range_matrix(float range_matrix[6][6])
{
	printf("{\r\n");
		for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++)
		{
			printf("\t{");
				for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++)
				{
					printf("%3.2f",range_matrix[emitter_num][sensor_num]);
					if(sensor_num<5) printf(",");
				}
			printf("}");
			if(emitter_num<5) printf(",");
			printf("\r\n");
		}
	printf("};\r\n");
}

void brightness_meas_printout_mathematica()
{
	printf_P(PSTR("data = {"));
		for(uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
		{
			printf("\r\n{");
				for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
				{
					printf_P(PSTR("\r\n(*e%hu,s%hu*){"), emitter_num, sensor_num);
						for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
						{
							//if(meas_num == 10)
							//printf("\r\n");
							printf("%d,",bright_meas[meas_num][emitter_num][sensor_num]);
						}
					printf("\b},");
				}
			printf("\b},");
		}
	printf_P(PSTR("\b};\r\n"));
}