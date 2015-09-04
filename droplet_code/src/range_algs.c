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

float basis[6][2] = {	{0.866025 , -0.5}, 
						{0        , -1  }, 
						{-0.866025, -0.5}, 
						{-0.866025,  0.5}, 
						{0        , 1   }, 
						{0.866025 , 0.5 }	};
						
float basis_angle[6] = {-0.523599, -1.5708, -2.61799, 2.61799, 1.5708, 0.523599};
	
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
	ir_range_meas();
	get_baseline_readings();
	//uint8_t power = 25; //TODO: get this from the message.
	//schedule_task(10,brightness_meas_printout_mathematica,NULL);
	//printf("Finished collecting data.\r\n");
	schedule_task(10, use_rnb_data, NULL);
}

void use_rnb_data()
{
	uint8_t power = 255;
	int16_t brightness_matrix[6][6];
	uint8_t error = pack_measurements_into_matrix(brightness_matrix);
	//brightness_meas_printout_mathematica();
	//print_brightness_matrix(brightness_matrix);
	if(!error)
	{
		/*
		For testing, comment out the above and use a hardcoded matrix from the mathematica notebook.
		uint8_t brightness_matrix[6][6] = {
			{0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0}
		};
		*/
		//print_brightness_matrix(brightness_matrix);
	
		uint8_t numNonZero=0;
		for(uint8_t e=0;e<6;e++){
			for(uint8_t s=0;s<6;s++){
				if(brightness_matrix[e][s]!=0) numNonZero++;
			}
		}
		if(numNonZero<=1) return;
	
		int16_t emitter_total[6];	
		int16_t sensor_total[6];
		fill_S_and_T(brightness_matrix, sensor_total, emitter_total);
	
		float bearing = get_bearing(sensor_total);
		float heading = get_heading(emitter_total, bearing);
	
		float initial_range = get_initial_range_guess(bearing, heading, power, sensor_total, emitter_total, brightness_matrix);
		if(initial_range)
		{
			float range = range_estimate(initial_range, bearing, heading, power, brightness_matrix);
			last_good_rnb.range = range;
			last_good_rnb.bearing = bearing;
			last_good_rnb.heading = heading;
			last_good_rnb.brightness_matrix_ptr = brightness_matrix;
			last_good_rnb.id_number = cmd_sender_id;
	
			rnb_updated=1;
		}
	}
}

float get_bearing(int16_t sensor_total[6])
{
	float x_sum = 0.0;
	float y_sum = 0.0;

	for(uint8_t i = 0; i < 6; i++)
	{
		x_sum = x_sum + basis[i][0]*sensor_total[i];
		y_sum = y_sum + basis[i][1]*sensor_total[i];
	}
	
	return atan2f(y_sum, x_sum);
}

float get_heading(int16_t emitter_total[6], float bearing)
{
	volatile float x_sum = 0.0;
	volatile float y_sum = 0.0;

	volatile float bearing_according_to_TX;

	for(uint8_t i = 0; i < 6; i++)
	{
		x_sum = x_sum + basis[i][0]*emitter_total[i];
		y_sum = y_sum + basis[i][1]*emitter_total[i];
	}
	
	bearing_according_to_TX = atan2f(y_sum, x_sum);
	
	float heading = bearing + M_PI - bearing_according_to_TX;
	
	return pretty_angle(heading);
}

float get_initial_range_guess(float bearing, float heading, uint8_t power, int16_t sensor_total[6], int16_t emitter_total[6], int16_t brightness_matrix[6][6])
{
	int16_t best_e=-32768, best_s=-32768;
	int16_t biggest_e_val = 0;
	int16_t biggest_s_val = 0;

	for(uint8_t i = 0; i < 6; i++)
	{
		if(emitter_total[i] > biggest_e_val)
		{
			best_e = i;
			biggest_e_val = emitter_total[best_e];
		}
		if(sensor_total[i] > biggest_s_val)
		{
			best_s = i;
			biggest_s_val = sensor_total[best_s];
		}
	}
	
	float alpha, beta;
	
	// find alpha using infinite approximation
	alpha = bearing - basis_angle[best_s];
	if((alpha > M_PI_2) || (alpha < -M_PI_2))
	{
		//printf("ERROR: alpha out of range (alpha: %f, sensor %u)\r\n", alpha, best_s); 
		return 0;
	}
	
	// find beta using infinite approximation
	beta = bearing - heading - basis_angle[best_e] - M_PI;
	beta = pretty_angle(beta);
	if((beta > M_PI_2)  || (beta < -M_PI_2))
	{
		//printf("ERROR: beta out of range (beta: %f, emitter %u)\r\n",  beta, best_e); 
		return 0;
	}
	
	// expected contribution (using infinite distance approximation)
	float amplitude;
	float exp_con = sensor_model(alpha)*emitter_model(beta);
	
	if(exp_con > 0)	amplitude = brightness_matrix[best_e][best_s]/exp_con;	
	else
	{
		//printf("ERROR: exp_con (%f) is negative (or zero)!\r\n", exp_con); 
		return 0;
	}
	//printf("amplitude_for_inv: %f\r\n",amplitude);
	return inverse_amplitude_model(amplitude, power) + 2*DROPLET_RADIUS;
}

float range_estimate(float init_range, float bearing, float heading, uint8_t power, int16_t brightness_matrix[6][6])
{
	float range_matrix[6][6];
	
	float sensorRXx, sensorRXy, sensorTXx, sensorTXy;
	float alpha, beta, sense_emit_contr;
	float calcRIJmag, calcRx, calcRy;

	int16_t maxBright = 0;
	int16_t maxE=-32768;
	int16_t maxS=-32768;
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
			
			if(brightness_matrix[e][s] > BASELINE_NOISE_THRESHOLD)
			{				
				sensorRXx = DROPLET_SENSOR_RADIUS*basis[s][0];
				sensorRXy = DROPLET_SENSOR_RADIUS*basis[s][1];
				sensorTXx = DROPLET_SENSOR_RADIUS*basis[e][0] + init_range*cosf(bearing);
				sensorTXy = DROPLET_SENSOR_RADIUS*basis[e][1] + init_range*sinf(bearing);

				alpha = atan2f(sensorTXy-sensorRXy,sensorTXx-sensorRXx) - basis_angle[s];
				beta = atan2f(sensorRXy-sensorTXy,sensorRXx-sensorTXx) - basis_angle[e] - heading;

				alpha = pretty_angle(alpha);
				beta = pretty_angle(beta);
				
				sense_emit_contr = sensor_model(alpha)*emitter_model(beta);

				calcRIJmag = inverse_amplitude_model(brightness_matrix[e][s]/sense_emit_contr, power);
				calcRx = calcRIJmag*cosf(alpha) + DROPLET_SENSOR_RADIUS*(basis[s][0] - basis[e][0]);
				calcRy = calcRIJmag*sinf(alpha) + DROPLET_SENSOR_RADIUS*(basis[s][1] - basis[e][1]);
				range_matrix[e][s] = sqrt(calcRx*calcRx + calcRy*calcRy);
				continue;
			}
			range_matrix[e][s]=0;
		}
	}
	
	float range = range_matrix[maxE][maxS];
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

uint8_t pack_measurements_into_matrix(int16_t brightness_matrix[6][6])
{
	int16_t val, meas, min_meas, max_meas;
	int16_t max_val=0;
	for(uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
	{
		for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
		{
			min_meas = 32767;
			max_meas = -32768;

			for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
			{
				meas = bright_meas[meas_num][emitter_num][sensor_num];
				if(meas<min_meas) min_meas=meas;
				if(meas>max_meas) max_meas=meas; 
			}
			val = max_meas-min_meas;
			if(val>max_val) max_val=val;
			brightness_matrix[emitter_num][sensor_num] = val;
		}
	}
	//print_brightness_matrix(brightness_matrix);
	return (max_val<=BASELINE_NOISE_THRESHOLD);
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
			get_ir_sensors(bright_meas[meas_num][emitter_dir] , 5);
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
	if((alpha>(-M_PI_2))&&(alpha<M_PI_2))	return cos(alpha);
	else									return 0;
	//volatile float alphaFourth = alpha*alpha*alpha*alpha;
	//if(abs(alpha) < 0.618614)
	//{
		//return 1 - alphaFourth;
	//}
	//else
	//{
		//return 1/(8*alphaFourth);
	//}
}

float emitter_model(float beta)
{	
	if((beta>(-M_PI_2))&&(beta<M_PI_2)) return cos(beta);
	else								return 0;
	//volatile float betaSquared = beta*beta;
	//if(abs(beta) < 0.720527)
	//{
		//return 0.9375 + 0.5*betaSquared - betaSquared*betaSquared;
	//}
	//else
	//{
		//return 1/(4*betaSquared*betaSquared);
	//}
}

float amplitude_model(float r, uint8_t power)
{
	if(power==255)			return 6.109+(724.879/((0.567+r)*(0.567+r)));
	//else if(power ==250)	return (1100./((r-4.)*(r-4.)))+12.5;
	else					printf_P(PSTR("ERROR: Unexpected power: %hhu\r\n"),power);
	return 0;
}

float inverse_amplitude_model(float ADC_val, uint8_t power)
{
	if(power == 255)		return (-1.450 + 39.919/(sqrtf(ADC_val+2.640)));
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

void print_brightness_matrix(int16_t brightness_matrix[6][6])
{
	printf("{");
	for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++)
	{
		printf("{");
		for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++)
		{
			printf("%d",brightness_matrix[emitter_num][sensor_num]);
			if(sensor_num<5) printf(",");
		}
		printf("}");
		if(emitter_num<5) printf(",");
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