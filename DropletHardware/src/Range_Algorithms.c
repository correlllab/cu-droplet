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

#include <avr/io.h>
#include <math.h>
#include <stdio.h> 
#include <avr/delay.h>

#include "Range_Algorithms.h"

float basis[6][2] = {	{0.866025 , -0.5}, 
						{0        , -1  }, 
						{-0.866025, -0.5}, 
						{-0.866025,  0.5}, 
						{0        , 1   }, 
						{0.866025 , 0.5 }	};
						
float basis_angle[6] = {-0.523599, -1.5708, -2.61799, 2.61799, 1.5708, 0.523599};
	
//Should maybe be elsewhere?
uint8_t bright_meas[6][6][NUMBER_OF_RB_MEASUREMENTS];

void range_algorithms_init()
{
	for(uint8_t i; i<NUMBER_OF_RB_MEASUREMENTS; i++)
	{
		for(uint8_t j; j<6;j++)
		{
			for(uint8_t k ; k<6 ; k++)
			{
				bright_meas[j][k][i] = 0;
			}
		}
	}
}

void collect_rnb_data(void* arg)
{	
	delay_ms(100);
	uint8_t power = (uint8_t)arg;
	char* cmd = "rnb_t ";
	cmd[6] = power;
	get_baseline_readings(bright_meas);
	uint8_t error = 1;
	uint8_t brightness_matrix[6][6];
	uint8_t number_of_tries = 0;
	while(error && (number_of_tries < 5))
	{
		if(number_of_tries > 0) delay_ms(1000);
		if(!OK_to_send())
		{
			printf("ERROR: RNB request failed. Not ok to send.");
			return;
		}

		ir_broadcast_cmd(cmd, 7);
		while(ir_tx[5].ir_status & IR_TX_STATUS_BUSY_bm);
		get_IR_range_readings();
		error = pack_measurements_into_matrix(brightness_matrix);
		number_of_tries++;
	}
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
	//brightness_meas_printout_mathematica();
	if(error){
		printf("ERROR: Gave up hearing from other droplet.\r\n");
		return;
	}
	use_rnb_data(power);
}

//TODO: handle variable power.

void broadcast_rnb_data()
{
	uint16_t power = 257;
	ir_broadcast_cmd("rnb_r",5);
	while(ir_tx[5].ir_status & IR_TX_STATUS_BUSY_bm);
	IR_range_blast(power);
}

void receive_rnb_data()
{
	get_IR_range_readings();
	get_baseline_readings();
	schedule_task(10,use_rnb_data,NULL);
}

void use_rnb_data(uint8_t power)
{
	uint8_t brightness_matrix[6][6];
	pack_measurements_into_matrix(brightness_matrix);;
	//print_brightness_matrix(brightness_matrix);
	uint8_t emitter_total[6];
	uint8_t sensor_total[6];
	
	fill_S_and_T(brightness_matrix, sensor_total, emitter_total);
	
	float bearing = get_bearing(sensor_total);
	float heading = get_heading(emitter_total, bearing);
	
	float initial_range = get_initial_range_guess(bearing, heading, sensor_total, emitter_total, brightness_matrix, power);
	float range = range_estimate(brightness_matrix, initial_range, bearing, heading, power);
	//TODO: Actually incorporate the ID #.
	//rnb temp = {.range = range, .bearing = bearing, .heading = heading, .id_number = 0};
	//printf("Bearing: %f | Heading: %f | Initial Range Guess: %f | Range: %f\r\n", rad_to_deg(bearing), rad_to_deg(heading), initial_range, range);
	last_good_rnb.range = range;
	last_good_rnb.bearing = bearing;
	last_good_rnb.heading = heading;
	last_good_rnb.brightness_matrix_ptr = brightness_matrix;
	last_good_rnb.id_number = last_command_source_id;
	rnb_updated=1;
}

float get_bearing(uint8_t sensor_total[6])
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

float get_heading(uint8_t emitter_total[6], float bearing)
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

float get_initial_range_guess(float bearing, float heading, uint8_t sensor_total[6], uint8_t emitter_total[6], uint8_t brightness_matrix[6][6], uint8_t power)
{
	uint8_t i, e, s, best_e, best_s;
	uint16_t biggest_e_val = 0;
	uint16_t biggest_s_val = 0;
	float alpha, beta;
	float EC, amplitude;
	float inital_guess;

	for(i = 0; i < 6; i++)
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
	
	/*printf("BEST SENSOR: %u\r\n", best_s);
	printf("BEST EMITTER: %u\r\n", best_e);*/

	// find alpha using infinite approximation

	alpha = bearing - atan2f(basis[best_s][1],basis[best_s][0]); // TODO: dont use atan2 for this simple task
	alpha = pretty_angle(alpha);

	// find beta using infinite approximation

	beta = bearing - heading - atan2f(basis[best_e][1],basis[best_e][0]) - M_PI; // TODO: dont use atan2 for this simple task
	beta = pretty_angle(beta);
	
	// expected contribution (using infinite distance approximation)

	if((alpha > M_PI/2)||(alpha < -M_PI/2))			// pi/2 = 1.5708
	{
		printf("ERROR: alpha out of range (alpha: %f, sensor %u)\r\n",alpha, best_s);
	}

	if((beta > M_PI/2)||(beta < -M_PI/2))
	{
		printf("ERROR: beta out of range (beta: %f, emitter %u)\r\n", beta, best_e);
	}

	EC = sensor_model(alpha)*emitter_model(beta);

	if(EC > 0)
	{
		amplitude = brightness_matrix[best_e][best_s]/EC;
	}		
	else
	{
		printf("ERROR: EC is negative (or zero)! oh, my (EC = %f)\r\n", EC);
	}

	return inverse_amplitude_model(amplitude, power) + 2*DROPLET_RADIUS;
}

float range_estimate(uint8_t brightness_matrix[6][6], float range_upper_limit, float bearing, float heading, uint8_t power)
{
	float alpha, beta;

	float SEcontribution;

	float sensorRXx, sensorRXy, sensorTXx, sensorTXy;
	
	float calcRIJmag, calcRmag;
	float calcRx, calcRy;
	
	float range_matrix[6][6];

	uint8_t maxBright = 0;
	uint8_t maxE;
	uint8_t maxS;
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
				sensorTXx = DROPLET_SENSOR_RADIUS*basis[e][0] + range_upper_limit*cosf(bearing);
				sensorTXy = DROPLET_SENSOR_RADIUS*basis[e][1] + range_upper_limit*sinf(bearing);

				alpha = atan2f(sensorTXy-sensorRXy,sensorTXx-sensorRXx) - basis_angle[s];
				beta = atan2f(sensorRXy-sensorTXy,sensorRXx-sensorTXx) - basis_angle[e] - heading;

				alpha = pretty_angle(alpha);
				beta = pretty_angle(beta);
				
				//if((alpha < M_PI/2)&&(alpha > -M_PI/2))
				//{
					//if((beta < M_PI/2)&&(beta > -M_PI/2))
					//{
						SEcontribution = sensor_model(alpha)*emitter_model(beta);

						calcRIJmag = inverse_amplitude_model(brightness_matrix[e][s]/SEcontribution, power);
						calcRx = calcRIJmag*cosf(alpha) + DROPLET_SENSOR_RADIUS*(basis[s][0] - basis[e][0]);
						calcRy = calcRIJmag*sinf(alpha) + DROPLET_SENSOR_RADIUS*(basis[s][1] - basis[e][1]);
						range_matrix[e][s] = sqrt(calcRx*calcRx + calcRy*calcRy);
						continue;
					//}
				//}
			}
			range_matrix[e][s]=0;
		}
	}
	
	//
	//for(uint8_t e=0; e<6 ; e++){
		//for(uint8_t s=0; s<6; s++){
			//printf("%02.3f  ",range_matrix[e][s]);
		//}
		//printf("\r\n");
	//}
	float range = range_matrix[maxE][maxS];
	//printf("range from 0,0: %f\r\n",range_matrix[maxE][maxS]);
	return range;
}

void fill_S_and_T(uint8_t brightness_matrix[6][6], uint8_t sensor_total[6], uint8_t emitter_total[6])
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

uint8_t pack_measurements_into_matrix(uint8_t brightness_matrix[6][6])
{
	uint8_t high, low, diff;
	
	uint8_t max_diff = 0;

	uint8_t probable_timing_fail = 0;

	uint8_t print_results = 0;

	// do some math with the measurements to find the correct values to put into the brightness matrix
	for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++)
	{
		// find measurement value (greatest difference) from the NUMBER_OF_RB_MEASUREMENTS measurements
		// usually, the 0th value (baseline) will provide the lowest value, but this is not guaranteed

		for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
		{
			high = 0;
			low = 255;
			
			for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
			{
				
				if(bright_meas[emitter_dir][sensor_num][meas_num] > high)
				{
					high = bright_meas[emitter_dir][sensor_num][meas_num];
				}
				if(bright_meas[emitter_dir][sensor_num][meas_num] < low)
				{
					low = bright_meas[emitter_dir][sensor_num][meas_num];
				}
			}

			diff = high - low;

			//if(print_results)
				//printf("[S%u E%u] lowest %u, highest %u, diff %u ", sensor_num, emitter_dir, low, high, diff);

			//if(low + BASELINE_NOISE_THRESHOLD < bright_meas[sensor_num][emitter_dir][0])
			////if(low_meas[sensor_num] < bright_meas[sensor_num][emitter_dir][0])
			//{
				//printf("base: %u, low: %u\r\n", bright_meas[sensor_num][emitter_dir][0], low);		// "lowest measurement is not the first"
				//printf("TIMING FAIL? sensor %u emitter %u\r\n", sensor_num, emitter_dir);
				//probable_timing_fail = 1;
			//}
			//else if(abs(bright_meas[sensor_num][emitter_dir][0] - bright_meas[sensor_num][emitter_dir][NUMBER_OF_RB_MEASUREMENTS-1]) > BASELINE_NOISE_THRESHOLD)
			//{
				//printf("TIMING FAIL? (FRAME ERROR) sensor %u emitter %u\r\n", sensor_num, emitter_dir);
				//probable_timing_fail = 2;
			//}
			//else 
			//{
				//if(print_results)	printf("\r\n");
			//}
			
			brightness_matrix[emitter_dir][sensor_num] = diff;
			if(diff > max_diff) max_diff = diff;
		}

	}// end loop through DOING MATH WITH the 6 emitters DATA
	if(max_diff < BASELINE_NOISE_THRESHOLD) probable_timing_fail = 1;
	return probable_timing_fail;
}

void get_baseline_readings()
{
	// Baseline measurements
	// take 6 readings, one for each 'emitter'
	for (uint8_t emitter_num = 0; emitter_num <6; emitter_num++)
	{			//[emitter#][sensor#][meas#]
		for (uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
		{
			bright_meas[emitter_num][sensor_num][0] = get_IR_sensor(sensor_num);
		}			
	}
}

void IR_range_blast(uint16_t power)
{
	delay_ms(POST_BROADCAST_DELAY);
	
	//uint32_t timer[20];
	//timer[0] = get_16bit_time(); //Top of the function.
	uint16_t pre_sync_op = get_16bit_time();
	for(uint8_t dir = 0; dir < 6; dir++)
	{
		set_ir_power(dir, power);
	}
	while((get_16bit_time() - pre_sync_op) < TIME_FOR_SET_IR_POWERS);


	//set_rgb(128,0,64);
	//_delay_ms(1);
	//led_off();
	//_delay_ms(10);



	for(uint8_t dir = 0; dir < 6; dir++)
	{
		//IR_emit(dir, DELAY_BETWEEN_RB_MEASUREMENTS*NUMBER_OF_RB_MEASUREMENTS);
		//timer[(3*dir + 0) + 1] = get_16bit_time();//Top of the for loop.
		delay_ms((DELAY_BETWEEN_RB_MEASUREMENTS + TIME_FOR_GET_IR_VALS)*(NUMBER_PRE_MEASUREMENTS));
		IR_emit(dir, (DELAY_BETWEEN_RB_MEASUREMENTS + TIME_FOR_GET_IR_VALS)*(NUMBER_OF_RB_MEASUREMENTS - (NUMBER_PRE_MEASUREMENTS + NUMBER_POST_MEASUREMENTS + 1))); //The -1 is 'cause we shouldn't count the baseline measurements.
		delay_ms((DELAY_BETWEEN_RB_MEASUREMENTS + TIME_FOR_GET_IR_VALS)*(NUMBER_POST_MEASUREMENTS));
		//timer[(3*dir + 1) + 1] = get_16bit_time();//post emit
		set_green_led(100);
		delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		led_off();
		//timer[(3*dir + 2) + 1] = get_16bit_time();//bottom of the for loop.
		
		//_delay_ms(10);
		//IR_emit(1, DELAY_BETWEEN_RB_MEASUREMENTS*NUMBER_OF_RB_MEASUREMENTS-20); // strictly for debugging the timing
		//_delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS+10);
	}
	//timer[19] = get_16bit_time();//End of function.
	//debug_print_timer(timer);
}

void get_IR_range_readings()
{
	delay_ms(POST_BROADCAST_DELAY);
	//uint32_t timer[20];
	//timer[0] = get_16bit_time(); //Top of the function.
	
	delay_ms(TIME_FOR_SET_IR_POWERS);
	
	for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++) // Transmitter goes through 6 directions, one at a time
	{
		//timer[(3*emitter_dir + 0) + 1] = get_16bit_time();//Top of the for loop.
		for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
		{
			uint16_t pre_sync_op = get_16bit_time();
			for (uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
			{
				bright_meas[emitter_dir][sensor_num][meas_num] = get_IR_sensor(sensor_num);
			}
			while((get_16bit_time() - pre_sync_op) < TIME_FOR_GET_IR_VALS);
			
			//if (meas_num < NUMBER_OF_RB_MEASUREMENTS - 1)
			//{
			delay_ms(DELAY_BETWEEN_RB_MEASUREMENTS);
			//}
		}
		//timer[(3*emitter_dir + 1) + 1] = get_16bit_time();//post emit

		set_green_led(100);
		delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
		led_off();
		//timer[(3*emitter_dir + 2) + 1] = get_16bit_time(); //bottom of the for loop.

	}// end loop through PHYSICALLY LOOKING AT the 6 emitters, recording DATA
	//timer[19] = get_16bit_time();//End of function.
	//debug_print_timer(timer);
}

void IR_emit(uint8_t direction, uint8_t duration) // this is now BLOCKING ***
{
	// as of now, this routine:
	// 1. turns on a pink color for 200 ms
	// 2. turns off pink, turns on green color while it blasts IR for 100 ms
	// 3. turns off green, and returns
	// note that the IR blast occurs during the latter 1/3 of the routine
	// somewhere in there it toggles some pins to temporarily disable carrier wave and UART
	
	// note further that the brightness is set elsewhere in the code
	// note: IR carrier pins { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN7_bm, PIN6_bm };

	uint8_t USART_CTRLB_save;

	uint8_t carrier_wave_bm;
	uint8_t TX_pin_bm;

	PORT_t * the_UART_port;
	USART_t * the_USART;

	switch(direction)
	{
		case 0:	// WORKS!
		carrier_wave_bm = PIN0_bm;			// TODO, name these "PIN0_bm" to something more specific, like "IR_CARRIER_PIN_0"
		TX_pin_bm = PIN3_bm;
		the_UART_port = &PORTC;
		the_USART = &USARTC0;
		break;
		case 1:	// WORKS!
		carrier_wave_bm = PIN1_bm;
		TX_pin_bm = PIN7_bm;
		the_UART_port = &PORTC;
		the_USART = &USARTC1;
		break;
		case 2:	// WORKS!
		carrier_wave_bm = PIN4_bm;
		TX_pin_bm = PIN3_bm;
		the_UART_port = &PORTD;
		the_USART = &USARTD0;
		break;
		case 3:	// WORKS!
		carrier_wave_bm = PIN5_bm;
		TX_pin_bm = PIN3_bm;
		the_UART_port = &PORTE;
		the_USART = &USARTE0;
		break;
		case 4:	// WORKS!
		carrier_wave_bm = PIN7_bm;
		TX_pin_bm = PIN7_bm;
		the_UART_port = &PORTE;
		the_USART = &USARTE1;
		break;
		case 5:	// WORKS!
		carrier_wave_bm = PIN6_bm;
		TX_pin_bm = PIN3_bm;
		the_UART_port = &PORTF;
		the_USART = &USARTF0;
		break;
		default:
		break;
	}

	USART_CTRLB_save = the_USART->CTRLB;		// record the current state of the USART

	TCF2.CTRLB &= ~carrier_wave_bm;			// disable carrier wave output
	PORTF.DIRSET = carrier_wave_bm;			// enable user output on this pin
	PORTF.OUT |= carrier_wave_bm;			// high signal on this pin

	the_USART->CTRLB = 0;				// disable USART
	the_UART_port->DIRSET = TX_pin_bm;			// enable user output on this pin
	the_UART_port->OUT &= ~TX_pin_bm;			// low signal on TX pin		(IR LED is ON when this pin is LOW, these pins were inverted in software during initialization)

	// IR LIGHT IS ON NOW

	//_delay_ms(NUMBER_OF_RB_MEASUREMENTS * DELAY_BETWEEN_RB_MEASUREMENTS);
	// -1 only counting gaps between N measurements
	// 0.1 delay while taking a meas
	//_delay_ms((NUMBER_OF_RB_MEASUREMENTS-1) * (DELAY_BETWEEN_RB_MEASUREMENTS+0.1)*0.75);

	delay_ms(duration);

	// IR LIGHT IS about to go OFF

	the_UART_port->OUT |= TX_pin_bm;			// high signal on TX pin (turns IR blast OFF)

	the_USART->CTRLB = USART_CTRLB_save;	// re-enable USART (restore settings as it was before)
	PORTF.OUT &= ~carrier_wave_bm;			// low signal on the carrier wave pin, don't really know why we do this? probably not necessary
	TCF2.CTRLB |= carrier_wave_bm;			// re-enable carrier wave output

	set_rgb(0,0,0);
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
	volatile float alphaFourth = alpha*alpha*alpha*alpha;
	if(abs(alpha) < 0.618614)
	{
		return 1 - alphaFourth;
	}
	else
	{
		return 1/(8*alphaFourth);
	}
}

float emitter_model(float beta)
{	
	volatile float betaSquared = beta*beta;
	if(abs(beta) < 0.720527)
	{
		return 0.9375 + 0.5*betaSquared - betaSquared*betaSquared;
	}
	else
	{
		return 1/(4*betaSquared*betaSquared);
	}
}

float amplitude_model(float r, uint8_t power)
{
	if(power==255){
		return (14000/((r-3)*(r-3))) - 1;
	}else if(power ==250){
		return (1100./((r-4.)*(r-4.)))+12.5;
	}else{
		printf("ERROR: Unexpected power: %hhu\r\n",power);
		return 0;
	}
}

float inverse_amplitude_model(float ADC_val, uint8_t power)
{
	if(power==255){
		return (118/sqrtf(ADC_val+1)) + 3;
	}else if(power ==250){
		return (33.166/sqrtf(ADC_val - 12.5)) + 4;
	}else{
		printf("ERROR: Unexpected power: %hhu\r\n",power);
		return 0;
	}
}


void debug_print_timer(uint32_t timer[20])
{
	printf("Duration: %u\r\n",timer[19] - timer[0]);
	printf("|  ");
	for(uint8_t i=1 ; i<19 ; i++)
	{
		printf("%3u  |  ",timer[i] - timer[i-1]);
	}
	printf("\r\n");
}

void print_brightness_matrix(uint8_t brightness_matrix[6][6])
{
	printf("{");
	for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++)
	{
		printf("{");
		for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++)
		{
			printf("%u",brightness_matrix[emitter_num][sensor_num]);
			if(sensor_num<5) printf(",");
		}
		printf("}");
		if(emitter_num<5) printf(",");
	}
	printf("};\r\n");
}

void brightness_meas_printout_mathematica()
{
	// MATHEMATICA PRINTOUT //

	printf("data = {");
		for(uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
		{
			printf("\r\n{");
				for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
				{
					printf("\r\n(*s%u,e%u*){", emitter_num, sensor_num);
						for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
						{	//[sensor#][emitter#][meas#]
							if(meas_num == 10)
							printf("\r\n");
							printf("%u,",bright_meas[emitter_num][sensor_num][meas_num]);
						}
					printf("\b},");
				}
			printf("\b},");
		}
	printf("\b};\r\n");
}