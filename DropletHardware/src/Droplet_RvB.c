// RED VS. BLUE, this code is for either a red droplet or blue droplet (change ~line 100) to select which one
// this code is the 'pick a color and tell your neighbor about it' program, only I have only permitted red or
// blue colors (or for a twist there could be green)

#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#include <avr/interrupt.h>

#include "stddef.h"				// required to use "offsetof(_,_)" for production row access

#define AUTO_RESET_ALLOWED 0
#define TALK_ABOUT_COLOR_ALLOWED 0

#include "debug.h"
#include "pc_com.h"
#include "motor.h"
#include "power.h"
#include "RGB_LED.h"
#include "RGB_sensor.h"
#include "i2c.h"
#include "IRcom.h"
#include "IR_sensor.h"
#include "ecc.h"
#include "random.h"
#include "sp_driver.h"
#include "boot.h"
#include "scheduler.h"
#include "droplet_init.h"
#include "Range_Algorithms.h"


#define RED_COLOR 1
#define BLUE_COLOR 2

static uint8_t DEBUG_MODE = 1;

uint8_t RVB_DEBUG = 1;

uint16_t range_brightness = 256;
uint8_t brightness_changed = 0;

uint16_t theta = 0;
uint16_t phi = 0;
uint16_t range = 0;

uint16_t count = 0;		// Sample count for range and bearing measurements

int main(void)
{
/**********************************/


/***********************************/
	init_all_systems();
	
	uint8_t my_color;
	uint8_t swarm_id;
	
	swarm_id = get_id_number();
	if(swarm_id%2 == 0)	
		my_color = RED_COLOR;	// even IDs become red
	else
		my_color = BLUE_COLOR;  // odd IDs become blue


	printf("Droplet swarm_id: %X\r\n\n",swarm_id);	

	uint8_t out_channel = 0;

	uint8_t request_out_channel = 0;

	// Initialize variables for pretty lights
	uint8_t r=0, g=0, b=0;

	uint8_t dir;
	//uint8_t i = 0; 
	uint8_t t = 0;

	uint8_t range_request_dir;
	uint16_t range_request_brightness;

	uint8_t transmit_cnt = 0;
	uint8_t choose_a_color = 0;
	
	uint8_t transmit_green = 0;
	uint8_t transmit_my_color = 0;

	char input_ch;

	// Set IR power to max in all directions
	for (dir = 0; dir < 6; dir++)
		set_ir_power(dir, 256);

	while (1)
	{
		//_delay_ms(10);	// dont delay

		if (!legs_powered())
			set_red_led(100);
		else
			set_red_led(0);
		
		if(global_rx_buffer.read == 0)	// if the message has not yet been read
		{
			printf("check your messages\r\n");
		}		
		

		if(check_for_new_messages())
		{
			printf("Checking Data > ");
			if(global_rx_buffer.read)
				printf("already read it\r\n");
			else
				printf("haven't yet read\r\n");
					
			printf("its from %02X\r\n",global_rx_buffer.sender_ID);
			printf("the length is %u\r\n",global_rx_buffer.data_len);
			
			uint8_t data_length = global_rx_buffer.data_len;

			// WELL, WHATS THE MESSAGE?
			printf("GLOBAL RECEIVE dir(s) %i: ", global_rx_buffer.receivers_used);
			for(uint16_t char_num = 0; char_num < data_length; char_num++)
				printf("%02X ", global_rx_buffer.buf[char_num]);	// space after this char

			printf("\r\n{current T: %u}", get_current_time());
			printf("\r\n{message arrival T: %u}", global_rx_buffer.initial_receive_time);
			printf("\r\n{message last-ac T: %u}", global_rx_buffer.last_access_time);

			printf("\r\n");
			
			global_rx_buffer.read = 1;		// mark new message as read
			printf("<marked as read>\r\n\r\n");
			
			// ------------------ PROCESS THE MESSAGE ------------------------------------------------------------------------
			
			if (global_rx_buffer.buf[0] == 'c' && data_length == 5) // Our message format is ['c'] [red value] [green value] [blue value] [time in centiseconds]
			{
				printf("TOLD A COLOR\r\n");
					
				set_rgb(global_rx_buffer.buf[1], global_rx_buffer.buf[2], global_rx_buffer.buf[3]);

				t = global_rx_buffer.buf[4];
					
				printf("color told RGB=(%i,%i,%i),t=%02X ",
					global_rx_buffer.buf[1],
					global_rx_buffer.buf[2],
					global_rx_buffer.buf[3], t);
				printf("\t{T: %u}\r\n", get_current_time());
				
				for (uint8_t i = 0; i <= t; i++)
					_delay_ms(10);
					
				set_rgb(0, 0, 0);
			}	

			else if (global_rx_buffer.buf[0] == 'r' && data_length == 2) // Our message format is ['r'] [brightness-1]
			{
				
				
				//printf("IR RANGE EMIT\r\n");
				
				range_request_brightness = global_rx_buffer.buf[1]+1;
				
				IR_range_blast(range_request_brightness);
				
				//range_request_dir = global_rx_buffer.buf[2];
				
				//set_ir_power(range_request_dir, range_request_brightness);
				
				//printf("range request: %u, dir %u", range_request_brightness ,range_request_dir);

				//printf("\t{T: %u}\r\n", get_current_time());
			
			}


			// ------------------ END: PROCESS THE MESSAGE -------------------------------------------------------------------
			
			
			
			

		}
		
		

		// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// ------------------------------------- //
		// CHOOSE A COLOR AND TRANSMIT THE COLOR //
		// ------------------------------------- //


		if ((get_current_time()/10 > 1000*(transmit_cnt+1)))
		//if (rand_byte() < 2 && rand_byte() < 28)
		{
			transmit_cnt++;
			if(TALK_ABOUT_COLOR_ALLOWED)
				choose_a_color = 1; // 1 = TRUE
		}
		
		else if(transmit_my_color || transmit_green)
		{
			transmit_cnt++;
			choose_a_color = 1; // 1 = TRUE
			transmit_my_color = 0;
		}

		if(choose_a_color)
		{
			if(RVB_DEBUG)
				printf("\r\nCHOOSE A COLOR %i\r\n", transmit_cnt);
			
			choose_a_color = 0;

			t = rand_byte();
			r = 0;
			g = 0;
			b = 0;
			
			if(transmit_green == 1)
			{
				g = rand_byte()/2;
				transmit_green = 0;
			}			
			else if(my_color == RED_COLOR)
				r = rand_byte()/2;
			else if(my_color == BLUE_COLOR)
				b = rand_byte()/2;
			else
				g = rand_byte()/2;	// there really isn't a way to get here

			if(RVB_DEBUG)	printf("color pick RGB=(%i,%i,%i),t=%02X ",r,g,b,t);
			if(RVB_DEBUG)	printf("{T: %u}\r\n", get_current_time());
				
			//set_rgb(r,g,b);

			uint8_t buf[5] = { 'c', r, g, b, t };
			//uint8_t head_buf[HEADER_LEN] = {5, swarm_id};	// packet length


			//dir = 0;	// only TX out dir 0
			dir = out_channel;
			
			if(!OK_to_send())
			{
				printf("cant send ERROR 88888888 TX IS ON\r\n");
			}
			else
				printf("abt to TX\r\n");
			
			uint8_t res = 0;
			res = ir_send(dir ,buf, 5);
			
			if(res != 0)
			{
				printf("ir_send failed, code %i\r\n", res);
				if(res == 1)
					ir_reset_rx(dir);
					ir_reset_tx(dir);
			}			
			
			while((DEBUG_MODE >= 1)&&(res != 0))
			{
				printf("ir_send failed, code %i\r\n", res);
				if(res == 1)
					ir_reset_tx(dir);
				else if(res == 2)
					ir_reset_rx(dir);
			
				res = ir_send(dir, buf, 5);	// resend
			}

			_delay_ms(110);	// message takes 110 ms to propagate through the air and to be read by the receiving droplet

			set_rgb(r,g,b);

			for (uint8_t i = 0; i <= t; i++)			
				_delay_ms(10);
						
			//set_blue_led(0);
			set_rgb(0,0,0);
			
		}				
		

		// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// ------------------------------------- //
		// CHECK THE KEYBOARD FOR INPUT			 //
		// ------------------------------------- //

		if(AUTO_RESET_ALLOWED)
			if (get_current_time() > 65000)
				droplet_reboot();
	
		input_ch = get_char_nonblocking();
		
		if(input_ch)
		{
			//printf("input: %c T:%u\r\n", input_ch, get_current_time());
			
			if(input_ch == 'b')	// 'b' for brightness
			{
				brightness_changed = 1;
				if(range_brightness < 256)
					range_brightness++;

				printf("bright0: %i\r\n", range_brightness);
			}

			else if(input_ch == 'B')	// 'B' for brightness
			{
				brightness_changed = 1;
				if(range_brightness > 0)
					range_brightness--;

				printf("bright0: %i\r\n", range_brightness);
			}

			
			else if(input_ch == 'c')	// 'c' for color
			{
				transmit_my_color = 1;
			}

			else if(input_ch == 'd')	// 'd' for dirset/dirclr
			{
				printf("PORTB: %u, %u\r\n", PORTB.DIR, PORTB.OUT);
				PORTB.DIRSET = 0b00111111;
				PORTB.OUTTGL = 0b00111111;
				_delay_ms(10);
				printf("PORTB: %u, %u\r\n", PORTB.DIR, PORTB.OUT);
				
				/*PORTB.OUT |= 0b00000001;
				_delay_ms(1000);
				printf("PORTB: %u, %u\r\n", PORTB.DIR, PORTB.OUT);

				PORTB.DIRCLR = 0b00000001;
				_delay_ms(1000);
				printf("PORTB: %u, %u\r\n", PORTB.DIR, PORTB.OUT);
				*/
				printf("\r\n");
			}

			else if(input_ch == 'e')	// 'e' for erase buffers
			{
				erase_RX_buffer(0);
				erase_RX_buffer(1);
				erase_RX_buffer(2);
				erase_RX_buffer(3);
				erase_RX_buffer(4);
				erase_RX_buffer(5);

				printf("\r\n BUFFERS ERASED \r\n");
			}

			else if(input_ch == 'E')	// 'E' for check buffers for errors
			{
				print_RX_buffer_status(0);
				print_RX_buffer_status(1);
				print_RX_buffer_status(2);
				print_RX_buffer_status(3);
				print_RX_buffer_status(4);
				print_RX_buffer_status(5);

				printf("\r\n BUFFER STATUS CHECKED \r\n");
			
				check_buffers_for_packets(1);
			}


			else if(input_ch == 'f')	// 'f' for flash some LED
			{
				IR_emit(0,100);
				IR_emit(1,100);
				IR_emit(2,100);
				IR_emit(3,100);
				IR_emit(4,100);
				IR_emit(5,100);
				
			}
			
			else if(input_ch == 'g')	// 'g' for choose green
			{
				transmit_green = 1;
			}
			
			else if(input_ch == 'i')	// 'i' for IR sense
			{
				//IR_sensor_init();
				
				uint8_t sensor_val[6];
				

				// TEST CHANNEL 0
				
				for(uint8_t i = 0; i < 6; i++)
					sensor_val[i] = get_IR_sensor(i);
				
				for(uint8_t i = 0; i < 6; i++)
					printf("IR%i: %u\r\n", i, sensor_val[i]);

	
			}

			else if(input_ch == 'I')	// 'I' for ID
			{
				printf("my ID: %X\r\n", get_id_number());
			}

			else if(input_ch == 'k')	// 'k' for CTRL
			{
				printf("ADCB.CRTLA: %u\r\n", ADCB.CTRLA);
				printf("ADCB.CRTLB: %u\r\n", ADCB.CTRLB);
				printf("ADCB.REFCRTL: %u\r\n", ADCB.REFCTRL);
				printf("ADCB.CALL: %u\r\n", ADCB.CALL);
				printf("ADCB.CALH: %u\r\n", ADCB.CALH);
				printf("\r\n");
				printf("ADCB.CH0.CTRL: %u\r\n", ADCB.CH0.CTRL);
				printf("ADCB.CH1.CTRL: %u\r\n", ADCB.CH1.CTRL);
				printf("ADCB.CH2.CTRL: %u\r\n", ADCB.CH2.CTRL);
				printf("ADCB.CH3.CTRL: %u\r\n", ADCB.CH3.CTRL);	
			}
			
			else if(input_ch == 'm')	// 'm' for check for messages
			{
				//printf("my ID: %X\r\n", calculate_id_number());
				check_buffers_for_packets(1);


				if(check_for_new_messages())
				{
					printf("FORCE FOUND A MESSAGE\r\n");
				}

				else
				{
					printf("FORCE COULD NOT FIND A MESSAGE\r\n");
				}

			}

			// //////////////////////////////////////////////////////////////////////////////////// 'n' BELOW //////////////////////////////////////////////////////////////////////////////

			else if(input_ch == 'n' || input_ch == 'N')	// 'n' for range and bearing!
			{
				set_red_led(255);
				// Get channel to transmit request
				out_channel = get_char_nonblocking();
				while (out_channel < '0' || out_channel > '5') out_channel = get_char_nonblocking();
				dir = out_channel - '0';		// convert char to int with same 'value'
										
				// Get brightness to request	
				uint8_t first_num = 'x';
				uint8_t second_num = 'x';
				uint8_t third_num = 'x';

				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9'); 

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= 48;	// convert char to int with same 'value'
				second_num -= 48;
				third_num -= 48;

				range_request_brightness = 100*first_num + 10*second_num + 1*third_num - 1;	// HEY, this minus 1 is because brightnesses goes from 0 - 256, but we downshift prior to communicating this
				
				printf("\r\nmaking request out dir = %u\r\n", dir);			
				printf("brightness requested = %u\r\n", range_request_brightness+1);

				led_off();	// RED LED OFF indicates user successfully input request
							
				//uint32_t start_time = get_time();
							
				uint8_t range_buf[2] = {'r', range_request_brightness};

				//uint8_t sense_dir = 0;

				uint8_t bright_meas[6][6][NUMBER_OF_RB_MEASUREMENTS];
				//uint8_t low_meas[6] = {255, 255, 255, 255, 255, 255};
				//uint8_t high_meas[6] = {0, 0, 0, 0, 0, 0};
				//uint8_t diff_meas[6] = {0, 0, 0, 0, 0, 0};



				// Baseline measurements
				// take 6 readings, one for each 'emitter'
				for (uint8_t sensor_num = 0; sensor_num <6; sensor_num++)
				{			//[sensor#][emitter#][meas#]
					bright_meas[sensor_num][0][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][1][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][2][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][3][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][4][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][5][0] = get_IR_sensor(sensor_num);
				}

				uint8_t range_req_res = 0;
				if (!OK_to_send())	printf("cant send ERROR 88888888 TX IS ON\r\n");
				range_req_res = ir_send(dir, range_buf, 2);		// 2 is the length
				
				// wait for the message to complete
				while (ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm);
				
				// OKAY DO YOUR COORDINATED EVENT HERE
				// Start synchronization timer
				//uint32_t timer = get_time();
				set_red_led(255);
				_delay_ms(4); // Important delay to let the communication IR light fade (this is _complete_ BS)
				led_off();
			
				if(range_req_res != 0)
				{
					printf("ir_send failed, code %i\r\n", range_req_res);
					if(range_req_res == 1)
					{
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						
						while(!get_char_nonblocking()){};
						
						ir_reset_rx(dir);
						ir_reset_tx(dir);
					}
					
				}
				
				_delay_ms(10);	// a small wait here, this helps to 'center' the emissions on the 20 measurements (so we see both the rising edge and falling edge) 
				
				
				for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++) // Transmitter goes through 6 directions, one at a time
				{
					// each pass through this for loop is expected to take ~ 100 ms
					
					set_red_led(255);
					_delay_ms(1);
					led_off();		
				
					for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						//uint32_t t0 = get_time();		
						//[sensor#][emitter#][meas#]
						
						
						bright_meas[0][emitter_dir][meas_num] = get_IR_sensor(0);
						bright_meas[1][emitter_dir][meas_num] = get_IR_sensor(1);
						bright_meas[2][emitter_dir][meas_num] = get_IR_sensor(2);
						bright_meas[3][emitter_dir][meas_num] = get_IR_sensor(3);
						bright_meas[4][emitter_dir][meas_num] = get_IR_sensor(4);
						bright_meas[5][emitter_dir][meas_num] = get_IR_sensor(5);
						
						/* DEBUG-O-RAMA
						bright_meas[0][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						bright_meas[1][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						bright_meas[2][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						bright_meas[3][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						bright_meas[4][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						bright_meas[5][emitter_dir][meas_num] = meas_num*(emitter_dir+1);
						*/



						//if (meas_num < NUMBER_OF_RB_MEASUREMENTS - 1)
						//{
							_delay_ms(DELAY_BETWEEN_RB_MEASUREMENTS);	// right now, this value is 5 ms, 5ms*20 = 100ms
						//}
					}

					if (emitter_dir < 5)
					{	
						set_red_led(255);
						_delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS-1-25);	// minus 1 is for the time wasted on the red LED
						led_off();
					}

				}// end loop through PHYSICALLY LOOKING AT the 6 emitters, recording DATA

				set_red_led(255);

				_delay_ms(10);

				led_off();


				uint8_t high, low, diff;

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
							//if(bright_meas[sensor_num][emitter_dir][meas_num] < low_meas[sensor_num])
							if(bright_meas[sensor_num][emitter_dir][meas_num] < low)
							{
								//low_meas[sensor_num] = bright_meas[sensor_num][emitter_dir][meas_num];
								low = bright_meas[sensor_num][emitter_dir][meas_num];
							}
						
							//if(bright_meas[sensor_num][emitter_dir][meas_num] > high_meas[sensor_num])
							if(bright_meas[sensor_num][emitter_dir][meas_num] > high)
							{
								//high_meas[sensor_num] = bright_meas[sensor_num][emitter_dir][meas_num];
								high = bright_meas[sensor_num][emitter_dir][meas_num];
							}
						}	
							
						//diff_meas[sensor_num] = high_meas[sensor_num] - low_meas[sensor_num];
						diff = high - low;

						if(input_ch == 'n')
						{
							printf("[S%u E%u] lowest %u, highest %u, diff %u ", sensor_num, emitter_dir, low, high, diff);
							//printf("[S%u E%u] lowest %u, highest %u, diff %u ", sensor_num, emitter_dir, low_meas[sensor_num], high_meas[sensor_num], diff_meas[sensor_num]);

							if(low < bright_meas[sensor_num][emitter_dir][0])
							//if(low_meas[sensor_num] < bright_meas[sensor_num][emitter_dir][0])
							{
								printf("L M N T F: %u\r\n", bright_meas[sensor_num][emitter_dir][0]);
								//printf("lowest measurement is not the first: sensor %u emitter %u\r\n", sensor_num, emitter_dir);
							}
							else
							{
								printf("\r\n");
							}
						}
						
						
						brightness_matrix[sensor_num][emitter_dir] = diff;
					}

					
				}// end loop through DOING MATH WITH the 6 emitters DATA


				fflush(stdout);

				printf("MATRIX:\r\n");
				for(uint8_t sensor = 0; sensor < 6; sensor++)
				{	// MATRIX ALWAYS PRINTED WITH
					// sensor numbers across the top
					// emitter numbers down the side

					printf("%i,%i,%i,%i,%i,%i\r\n",		
					brightness_matrix[0][sensor],
					brightness_matrix[1][sensor],
					brightness_matrix[2][sensor],
					brightness_matrix[3][sensor],
					brightness_matrix[4][sensor],
					brightness_matrix[5][sensor]);
				}

/*				
				if(input_ch == 'N')
				{
					float bearing, heading, range;

					fill_S_and_T();

					bearing = get_bearing();
					heading = get_heading(bearing);

					printf("bearing: ");
					RNB_print_float(bearing * 180/M_PI);
					printf(" deg\r\nheading: ");
					RNB_print_float(heading * 180/M_PI);M255400400130T
					printf(" deg\r\n");

					range = get_inital_range_guess(bearing, heading) + 2*DROPLET_RADIUS;

					printf("\r\n");
					printf("range upper bound: ");
					RNB_print_float(range);
					printf("\r\n");

					double best_range = range_estimate(range, bearing, heading);

					printf("\r\n");
					printf("actual range: ");
					RNB_print_float(best_range);
					printf("\r\n");
				}
*/				
				led_off();


				// MATHEMATICA PRINTOUT //

				printf("data = {");
				for(uint8_t emitter_num = 0; emitter_num < 6; emitter_num++)
				{
					printf("\r\n{");
					for(uint8_t sensor_num = 0; sensor_num < 6; sensor_num++)
					{
						printf("\r\n(* s%u,e%u *){", sensor_num, emitter_num);
						for(uint8_t meas_num = 0; meas_num < 20; meas_num++)
						{	//[sensor#][emitter#][meas#]
							printf("%u,",bright_meas[sensor_num][emitter_num][meas_num]);
						}
						printf("\b},");
					}
					printf("\b},");
				}
				printf("\b};\r\n");

			}

			// //////////////////////////////////////////////////////////////////////////////////// 'n' ABOVE //////////////////////////////////////////////////////////////////////////////

			else if(input_ch == 'o')	// 'o' for orange
			{
				uint8_t dir = 0;
				printf("psych!, orange: (%i)\r\n",dir);
				uint8_t buf[5] = { 'c', 100, 50, 0, 200 };
	
				if(ir_send(dir ,buf, 5) != 0)
				{
					printf("ir_send failed\r\n");
					
					ir_reset_rx(dir);
					ir_reset_tx(dir);
				}
					
			}
			
			else if(input_ch == 'q')	// 'q' for quit/reboot
			{
				droplet_reboot();
			}			

			// 'R' for range (with full data) or 'r' (just the matrix)
			// 'T' for range (with full data, text format) or 't' (just the matrix, text format)
			// 'M' for range (with full data, mathematica format)
			// Must be followed with 0-5 for direction to output the range request,
			// then XXX where XXX is brightness to request in 3-digit decimal
			// then XXX where XXX is theta in 3-digit decimal (0-399 steps of stepper motor)
			// then XXX where XXX is phi in 3-digit decimal
			// then XXX where XXX is r in 3-digit decimal (0-999mm)
			else if(input_ch == 'R' || input_ch == 'r' || input_ch == 'T' || input_ch == 't' || input_ch == 'M')
			{
				set_red_led(100);
				// Get channel to transmit request
				out_channel = get_char_nonblocking();
				while (out_channel < '0' || out_channel > '5') out_channel = get_char_nonblocking();
				dir = out_channel - '0';		// convert char to int with same 'value'
							
				// Get brightness to request
				
				/*
				uint8_t brightness_high_nibble = get_char_nonblocking();
				while (brightness_high_nibble < '0' || brightness_high_nibble > 'F') brightness_high_nibble = get_char_nonblocking();
				uint8_t brightness_low_nibble = get_char_nonblocking();
				while (brightness_low_nibble < '0' || brightness_high_nibble > 'F') brightness_low_nibble = get_char_nonblocking();			
				brightness_high_nibble -= '0';
				if (brightness_high_nibble > 9) brightness_high_nibble += 10 - ('A'-'0');
				brightness_low_nibble -= '0';
				if (brightness_low_nibble > 9) brightness_low_nibble += 10 - ('A'-'0');
				*/	
					
				uint8_t first_num = 'x';
				uint8_t second_num = 'x';
				uint8_t third_num = 'x';

				// Get range request brightness
				
				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9'); 

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= '0';	// convert char to int with same 'value'
				second_num -= '0';
				third_num -= '0';

				range_request_brightness = 100*first_num + 10*second_num + 1*third_num - 1;	// HEY, this minus 1 is because brightnesses goes from 0 - 256, but we downshift prior to communicating this
				
		
				// Get Theta
				
				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9');

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= '0';	// convert char to int with same 'value'
				second_num -= '0';
				third_num -= '0';

				theta = 100*first_num + 10*second_num + 1*third_num;

				// Get Phi
				
				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9');

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= '0';	// convert char to int with same 'value'
				second_num -= '0';
				third_num -= '0';

				phi = 100*first_num + 10*second_num + 1*third_num;

				// Get R
				
				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9');

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= '0';	// convert char to int with same 'value'
				second_num -= '0';
				third_num -= '0';

				range = 100*first_num + 10*second_num + 1*third_num;
				
							
				//printf("brightness requested = %u\r\n", range_request_brightness);

				led_off();	// GREEN LED OFF indicates user successfully input request
							
				//uint32_t start_time = get_time();
				count++;		// count the number of samples taken
							
				uint8_t range_buf[2] = { 'r', range_request_brightness};

				//uint8_t sense_dir = 0;

				uint8_t bright_meas[6][6][NUMBER_OF_RB_MEASUREMENTS];
				//uint8_t low_meas[6] = {255, 255, 255, 255, 255, 255};
				//uint8_t high_meas[6] = {0};
				//uint8_t diff_meas[6];

				// Baseline measurements
				// take 6 readings
				for (uint8_t sensor_num = 0; sensor_num <6; sensor_num++)
				{			//[sensor#][emitter#][meas#]
					bright_meas[sensor_num][0][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][1][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][2][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][3][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][4][0] = get_IR_sensor(sensor_num);
					bright_meas[sensor_num][5][0] = get_IR_sensor(sensor_num);
				}

				uint8_t range_req_res = 0;
				if (!OK_to_send())	printf("cant send ERROR 88888888 TX IS ON\r\n");
				range_req_res = ir_send(dir, range_buf, 2);		// 2 is the length
				
				// wait for the message to complete
				while (ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm);
				
				// OKAY DO YOUR COORDINATED EVENT HERE
				// Start synchronization timer
				//uint32_t timer = get_time();
				set_red_led(255);
				_delay_ms(4); // Important delay to let the communication IR light fade (this is _completely_ BS)
				led_off();
				
				if(range_req_res != 0)
				{
					printf("ir_send failed, code %i\r\n", range_req_res);
					if(range_req_res == 1)
					{
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						printf("\r\nWARNING!!! user code is resetting RX and TX !!!\r\n");
						
						while(!get_char_nonblocking()){};
					
						ir_reset_rx(dir);
						ir_reset_tx(dir);
					}
				
				}
			
				_delay_ms(10);	// a small wait here, this helps to 'center' the emissions on the 20 measurements (so we see both the rising edge and falling edge)
			

				for(uint8_t emitter_dir = 0; emitter_dir < 6; emitter_dir++) // Transmitter goes through 6 directions
				{
					set_green_led(100);
					_delay_ms(1);
					led_off();
				
				
					for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						//uint32_t t0 = get_time();
						bright_meas[0][emitter_dir][meas_num] = get_IR_sensor(0);
						bright_meas[1][emitter_dir][meas_num] = get_IR_sensor(1);
						bright_meas[2][emitter_dir][meas_num] = get_IR_sensor(2);
						bright_meas[3][emitter_dir][meas_num] = get_IR_sensor(3);
						bright_meas[4][emitter_dir][meas_num] = get_IR_sensor(4);
						bright_meas[5][emitter_dir][meas_num] = get_IR_sensor(5);

						_delay_ms(DELAY_BETWEEN_RB_MEASUREMENTS);	// right now, this value is 5 ms, 5ms*20 = 100ms
					}

					if (emitter_dir < 5)
					{
						set_green_led(100);
						_delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS-1-25);	// minus 1 is for the time wasted on the green LED
						//	minus 25 is for unknown, but it works, details will be posted later 
						led_off();
					}

					// DO NO PROCESSING OF DATA IN THIS LOOP!

				}// end loop through PHYSICALLY LOOKING AT the 6 emitters, recording DATA
					
				// IF DEALING WITH A CAPITAL LETTER
				if (input_ch == 'R' || input_ch == 'T' || input_ch == 'M')
				{
					if (input_ch == 'R') printf("%c%c",(count>>8)&0xFF,count&0xFF);
					else if (input_ch == 'R') printf("%i,%i,",(count>>8)&0xFF,count&0xFF);
					else printf("{\n  {R->%i,\\[Theta]->%i,\\[Phi]->%i,brightness->%i},\n  {\n",range,theta,phi,range_request_brightness);
					for (uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						if (input_ch == 'R') printf("%c",meas_num);
						else if (input_ch == 'T') printf("%i,",meas_num);
						else printf("    {");
						for (range_request_dir = 0; range_request_dir < 6; range_request_dir++)
						{
							if (input_ch == 'T') printf("%i,%i,%i,%i,%i,%i,%i,",range_request_dir,
							bright_meas[0][range_request_dir][meas_num],
							bright_meas[1][range_request_dir][meas_num],
							bright_meas[2][range_request_dir][meas_num],
							bright_meas[3][range_request_dir][meas_num],
							bright_meas[4][range_request_dir][meas_num],
							bright_meas[5][range_request_dir][meas_num]);
							else if (input_ch == 'R') printf("%c%c%c%c%c%c%c",range_request_dir,
							bright_meas[0][range_request_dir][meas_num],
							bright_meas[1][range_request_dir][meas_num],
							bright_meas[2][range_request_dir][meas_num],
							bright_meas[3][range_request_dir][meas_num],
							bright_meas[4][range_request_dir][meas_num],
							bright_meas[5][range_request_dir][meas_num]);
							else { printf("{%i,%i,%i,%i,%i,%i}",
							bright_meas[0][range_request_dir][meas_num],
							bright_meas[1][range_request_dir][meas_num],
							bright_meas[2][range_request_dir][meas_num],
							bright_meas[3][range_request_dir][meas_num],
							bright_meas[4][range_request_dir][meas_num],
							bright_meas[5][range_request_dir][meas_num]);
								if (range_request_dir < 5) printf(",");
							}								
						}
						if (input_ch == 'M') { printf("}");
							if (meas_num < NUMBER_OF_RB_MEASUREMENTS - 1) printf(",\n");
							else printf("\n");
						}							
					}
					if (input_ch == 'T') printf("\n");
					else if (input_ch == 'M') printf("  }\n},\n");
				}
				else for(uint8_t i = 0; i < 6; i++)
				{
					if (input_ch == 'r') printf("%c%c",(count>>8)&0xFF,count&0xFF);
					else printf("%i,%i,",(count>>8)&0xFF,count&0xFF);
					if (input_ch == 'r') printf("%c%c%c%c%c%c",
					brightness_matrix[i][0],
					brightness_matrix[i][1],
					brightness_matrix[i][2],
					brightness_matrix[i][3],
					brightness_matrix[i][4],
					brightness_matrix[i][5]);
					else printf("%i,%i,%i,%i,%i,%i,",
					brightness_matrix[i][0],
					brightness_matrix[i][1],
					brightness_matrix[i][2],
					brightness_matrix[i][3],
					brightness_matrix[i][4],
					brightness_matrix[i][5]);
				}
			}


			
			
			else if(input_ch == 'y')	// 'y' for production signatures
			{
				printf("PRODSIGNATURES_USBRCOSC: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, USBRCOSC )));
				printf("PRODSIGNATURES_USBRCOSCA: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, USBRCOSCA )));
				printf("PRODSIGNATURES_ADCACAL0: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCACAL0 )));
				printf("PRODSIGNATURES_ADCACAL1: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCACAL1 )));
				printf("PRODSIGNATURES_ADCBCAL0: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCBCAL0 )));
				printf("PRODSIGNATURES_ADCBCAL1: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCBCAL1 )));
				printf("PRODSIGNATURES_TEMPSENSE0: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, TEMPSENSE0 )));
				printf("PRODSIGNATURES_TEMPSENSE1: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, TEMPSENSE1 )));
			}

			else if(input_ch == 'Y')	// 'Y' for more production signatures
			{
				printf("WAFNUM: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, WAFNUM )));
				printf("COORDX0: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, COORDX0 )));
				printf("COORDX1: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, COORDX1 )));
				printf("COORDY0: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, COORDY0 )));
				printf("COORDY1: %u\r\n",SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, COORDY1 )));
			}
			
			else if(input_ch == '0')	// '0' for IR range 0
			{
				IR_emit(0,100);
			}

			else if(input_ch == '1')	// '1' for IR sense 1
			{
				IR_emit(1,100);
			}

			else if(input_ch == '2')	// '2' for IR sense 2
			{
				IR_emit(2,100);
			}

			else if(input_ch == '3')	// '3' for IR sense 3
			{
				IR_emit(3,100);
			}

			else if(input_ch == '4')	// '4' for IR sense 4
			{
				IR_emit(4,100);
			}

			else if(input_ch == '5')	// '5' for IR sense 5
			{
				IR_emit(5,100);
			}

			else if(input_ch == ')')	// ')' for IR 0
			{
				out_channel = 0;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '!')	// '!' for IR 1
			{
				out_channel = 1;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '@')	// '@' for IR 2
			{
				out_channel = 2;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '#')	// '#' for IR 3
			{
				out_channel = 3;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '$')	// '$' for IR 4
			{
				out_channel = 4;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '%')	// '%' for IR 5
			{
				out_channel = 5;
				printf("out channel: %u\r\n", out_channel);
			}

			else if(input_ch == '<')	// '<' for request channel down
			{
				if(request_out_channel > 0)
					request_out_channel--;
				
				printf("request out channel: %u\r\n", request_out_channel);
			}

			else if(input_ch == '>')	// '>' for request channel up
			{
				if(request_out_channel < 5)
					request_out_channel++;
				
				printf("request out channel: %u\r\n", request_out_channel);
			}

			input_ch = 0;
		}

		
		
		//_delay_ms(10);
		
	}
}







// old 'R' function:


// ------------------------------------------------------------------------------------------------------------ 'R'
				
				/*
				for(uint8_t j = 0; j < 6; j++)
				{
					range_request_brightness = range_brightness-1;
					range_request_dir = j;
				
					//printf("requesting a range, use %u brightness out direction %u\r\n", range_request_brightness+1, range_request_dir);

					uint8_t range_buf[3] = { 'r', range_request_brightness, range_request_dir};

					uint8_t sense_dir = 0;

					dir = out_channel;
			
					if(!OK_to_send())	printf("cant send ERROR 88888888 TX IS ON\r\n");
			
					//else	printf("abt to TX\r\n");
			
					uint8_t range_req_res = 0;
					range_req_res = ir_send(dir ,range_buf, 3);		// 3 is the length
			
					if(range_req_res != 0)
					{
						printf("ir_send failed, code %i\r\n", range_req_res);
						if(range_req_res == 1)
						ir_reset_rx(dir);
						ir_reset_tx(dir);
					}
			
					while((DEBUG_MODE >= 1)&&(range_req_res != 0))
					{
						printf("ir_send failed, code %i\r\n", range_req_res);
						if(range_req_res == 1)
						ir_reset_tx(dir);
						else if(range_req_res == 2)
						ir_reset_rx(dir);
						range_req_res = ir_send(dir, range_buf, 3);	// resend
					}

					uint8_t bright_meas[6][20] = {0};
					uint8_t low_meas[6] = {255, 255, 255, 255, 255, 255};
					uint8_t high_meas[6] = {0};
					uint8_t diff_meas[6];

					//bright_meas[0][0] = get_IR_sensor(sense_dir);
					bright_meas[0][0] = get_IR_sensor(0);
					bright_meas[1][0] = get_IR_sensor(1);
					bright_meas[2][0] = get_IR_sensor(2);
					bright_meas[3][0] = get_IR_sensor(3);
					bright_meas[4][0] = get_IR_sensor(4);
					bright_meas[5][0] = get_IR_sensor(5);

					// MATH
					_delay_ms(97);	// message takes 110 ms (for 5 byte msg) to propagate through the air and to be read by the receiving droplet
					// message takes 97 ms (for 3 byte msg) to propagate through the air and to be read by the receiving droplet

					// OKAY DO YOUR COORDINATED EVENT HERE

					_delay_ms(100);

					for(uint8_t meas_num = 1; meas_num < 20; meas_num++)
					{
						//bright_meas[0][meas_num] = get_IR_sensor(sense_dir);
						bright_meas[0][meas_num] = get_IR_sensor(0);
						bright_meas[1][meas_num] = get_IR_sensor(1);
						bright_meas[2][meas_num] = get_IR_sensor(2);
						bright_meas[3][meas_num] = get_IR_sensor(3);
						bright_meas[4][meas_num] = get_IR_sensor(4);
						bright_meas[5][meas_num] = get_IR_sensor(5);
	
						_delay_ms(10);
					}

					//printf("range meas dir %u: \r\n", sense_dir);
					for(uint8_t meas_num = 0; meas_num < 20; meas_num++)
					{
						//printf("meas # %u: %u\r\n",meas_num,bright_meas[0][meas_num]);


						if(meas_num > 0)
						{
							for(uint8_t k = 0; k < 6; k++)
							{
								if(bright_meas[k][meas_num] < low_meas[k])
								{
									low_meas[k] = bright_meas[k][meas_num];
									//printf("new low %u %u\r\n", k, low_meas[k]);
								}
			
								if(bright_meas[k][meas_num] > high_meas[k])
								{
									high_meas[k] = bright_meas[k][meas_num];
									//printf("new high %u %u\r\n", k, high_meas[k]);
								}
							}
						}
					}

					for(uint8_t k = 0; k < 6; k++)
						diff_meas[k] = high_meas[k] - low_meas[k];

					brightness_matrix[range_request_dir][0] = diff_meas[0];
					brightness_matrix[range_request_dir][1] = diff_meas[1];
					brightness_matrix[range_request_dir][2] = diff_meas[2];
					brightness_matrix[range_request_dir][3] = diff_meas[3];
					brightness_matrix[range_request_dir][4] = diff_meas[4];
					brightness_matrix[range_request_dir][5] = diff_meas[5];


					_delay_ms(100);

				}// end loop through 6 dirs
				// ------------------------------------------------------------------------------------------------------------ 'R'

				for(uint8_t i = 0; i < 6; i++)
				{
					printf("MATRIX %u: %03u, %03u, %03u, %03u, %03u, %03u\r\n", i,
						brightness_matrix[i][0],
						brightness_matrix[i][1],
						brightness_matrix[i][2],
						brightness_matrix[i][3],
						brightness_matrix[i][4],
						brightness_matrix[i][5]);
				}

		
*/


/*



			// 'R' for range (with full data) or 'r' (just the matrix)
			// 'T' for range (with full data, text format) or 't' (just the matrix, text format)
			// Must be followed with 0-5 for direction to output the range request,
			// then XX where XX is brightness to request in hex
			else if(input_ch == 'R' || input_ch == 'r' || input_ch == 'T' || input_ch == 't')
			{
				set_green_led(100);
				// Get channel to transmit request
				out_channel = get_char_nonblocking();
				while (out_channel < '0' || out_channel > '5') out_channel = get_char_nonblocking();
				dir = out_channel - '0';		// convert char to int with same 'value'
							
							
				// Get brightness to request
			
					
				uint8_t first_num = 'x';
				uint8_t second_num = 'x';
				uint8_t third_num = 'x';

				do{first_num = get_char_nonblocking();}
				while (first_num < '0' || first_num > '9'); 

				do{second_num = get_char_nonblocking();}
				while (second_num < '0' || second_num > '9');

				do{third_num = get_char_nonblocking();}
				while (third_num < '0' || third_num > '9');

				first_num -= 48;	// convert char to int with same 'value'
				second_num -= 48;
				third_num -= 48;

				range_request_brightness = 100*first_num + 10*second_num + 1*third_num - 1;	// HEY, this minus 1 is because brightnesses goes from 0 - 256, but we downshift prior to communicating this
							
				//printf("brightness requested = %u\r\n", range_request_brightness);

				led_off();	// GREEN LED OFF indicates user successfully input request
							
				//uint32_t start_time = get_time();
				count++;		// what are we counting??
							
				uint8_t range_buf[2] = { 'r', range_request_brightness};

				//uint8_t sense_dir = 0;

				uint8_t bright_meas[6][6][NUMBER_OF_RB_MEASUREMENTS];
				uint8_t low_meas[6] = {255, 255, 255, 255, 255, 255};
				uint8_t high_meas[6] = {0};
				uint8_t diff_meas[6];

				// Baseline measurements
				// take 6 readings
				for (range_request_dir = 0; range_request_dir <6; range_request_dir++)
				{
					bright_meas[0][range_request_dir][0] = get_IR_sensor(0);
					bright_meas[1][range_request_dir][0] = get_IR_sensor(1);
					bright_meas[2][range_request_dir][0] = get_IR_sensor(2);
					bright_meas[3][range_request_dir][0] = get_IR_sensor(3);
					bright_meas[4][range_request_dir][0] = get_IR_sensor(4);
					bright_meas[5][range_request_dir][0] = get_IR_sensor(5);
				}

				uint8_t range_req_res = 0;
				if (!OK_to_send())	printf("cant send ERROR 88888888 TX IS ON\r\n");
				range_req_res = ir_send(dir, range_buf, 2);		// 2 is the length
				while (ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm);
				// OKAY DO YOUR COORDINATED EVENT HERE
				// Start synchronization timer
				//uint32_t timer = get_time();
				set_green_led(100);
				_delay_ms(4); // Important delay to let the communication IR light fade
				led_off();
			
				if(range_req_res != 0)
				{
					printf("ir_send failed, code %i\r\n", range_req_res);
					if(range_req_res == 1)
					ir_reset_rx(dir);
					ir_reset_tx(dir);
				}

				for(uint8_t j = 0; j < 6; j++) // Transmitter goes through 6 directions
				{
					set_green_led(100);
					_delay_ms(1);
					led_off();
				
					range_request_dir = j;
				
					for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						//uint32_t t0 = get_time();
						bright_meas[0][range_request_dir][meas_num] = get_IR_sensor(0);
						bright_meas[1][range_request_dir][meas_num] = get_IR_sensor(1);
						bright_meas[2][range_request_dir][meas_num] = get_IR_sensor(2);
						bright_meas[3][range_request_dir][meas_num] = get_IR_sensor(3);
						bright_meas[4][range_request_dir][meas_num] = get_IR_sensor(4);
						bright_meas[5][range_request_dir][meas_num] = get_IR_sensor(5);
						if (meas_num < NUMBER_OF_RB_MEASUREMENTS - 1)
						{
							_delay_ms(DELAY_BETWEEN_RB_MEASUREMENTS);
						}
					}

					for(uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						for(uint8_t k = 0; k < 6; k++)
						{
							if(bright_meas[k][range_request_dir][meas_num] < low_meas[k])
							{
								low_meas[k] = bright_meas[k][range_request_dir][meas_num];
							}
						
							if(bright_meas[k][range_request_dir][meas_num] > high_meas[k])
							{
								high_meas[k] = bright_meas[k][range_request_dir][meas_num];
							}
						}
					}

					for(uint8_t k = 0; k < 6; k++)
					diff_meas[k] = high_meas[k] - low_meas[k];

					brightness_matrix[range_request_dir][0] = diff_meas[0];
					brightness_matrix[range_request_dir][1] = diff_meas[1];
					brightness_matrix[range_request_dir][2] = diff_meas[2];
					brightness_matrix[range_request_dir][3] = diff_meas[3];
					brightness_matrix[range_request_dir][4] = diff_meas[4];
					brightness_matrix[range_request_dir][5] = diff_meas[5];
				
					if (range_request_dir < 5) _delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
				}// end loop through 6 dirs


				// IF DEALING WITH A CAPITAL LETTER
				if (input_ch == 'R' || input_ch == 'T')
				{
					if (input_ch == 'R') printf("%c%c",(count>>8)&0xFF,count&0xFF);
					else printf("%i,%i,",(count>>8)&0xFF,count&0xFF);
					for (uint8_t meas_num = 0; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
					{
						if (input_ch == 'R') printf("%c",meas_num);
						else printf("%i,",meas_num);
						for (range_request_dir = 0; range_request_dir < 6; range_request_dir++)
						{
							if (input_ch == 'T') printf("%i,%i,%i,%i,%i,%i,%i,",range_request_dir,
							bright_meas[0][range_request_dir][meas_num],
							bright_meas[1][range_request_dir][meas_num],
							bright_meas[2][range_request_dir][meas_num],
							bright_meas[3][range_request_dir][meas_num],
							bright_meas[4][range_request_dir][meas_num],
							bright_meas[5][range_request_dir][meas_num]);
							else printf("%c%c%c%c%c%c%c",range_request_dir,
							bright_meas[0][range_request_dir][meas_num],
							bright_meas[1][range_request_dir][meas_num],
							bright_meas[2][range_request_dir][meas_num],
							bright_meas[3][range_request_dir][meas_num],
							bright_meas[4][range_request_dir][meas_num],
							bright_meas[5][range_request_dir][meas_num]);
						}
					}
					if (input_ch == 'T') printf("\n");
				}
				else for(uint8_t i = 0; i < 6; i++)
				{
					if (input_ch == 'r') printf("%c%c",(count>>8)&0xFF,count&0xFF);
					else printf("%i,%i,",(count>>8)&0xFF,count&0xFF);
					if (input_ch == 'r') printf("%c%c%c%c%c%c",
					brightness_matrix[i][0],
					brightness_matrix[i][1],
					brightness_matrix[i][2],
					brightness_matrix[i][3],
					brightness_matrix[i][4],
					brightness_matrix[i][5]);
					else printf("%i,%i,%i,%i,%i,%i,",
					brightness_matrix[i][0],
					brightness_matrix[i][1],
					brightness_matrix[i][2],
					brightness_matrix[i][3],
					brightness_matrix[i][4],
					brightness_matrix[i][5]);
				}

			}

*/