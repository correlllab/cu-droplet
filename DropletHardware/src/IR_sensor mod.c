
#include "IRcom.h"







void IR_all_brightness_matrices(uint8_t range_request_dir, uint16_t range_request_brightness, uint8_t*** bright_meas)
{
	uint8_t range_buf[3] = { 'r', range_request_brightness};

	// Baseline measurements
	// take 6 readings
	for (range_request_dir = 0; range_request_dir <6; range_request_dir++)
	{			//[sensor#][transmitter#][meas#]
		bright_meas[0][range_request_dir][0] = get_IR_sensor(0);
		bright_meas[1][range_request_dir][0] = get_IR_sensor(1);
		bright_meas[2][range_request_dir][0] = get_IR_sensor(2);
		bright_meas[3][range_request_dir][0] = get_IR_sensor(3);
		bright_meas[4][range_request_dir][0] = get_IR_sensor(4);
		bright_meas[5][range_request_dir][0] = get_IR_sensor(5);
	}

	uint8_t range_req_res = 0;
	if (!OK_to_send())	printf("cant send ERROR 88888888 TX IS ON\r\n");
	range_req_res = ir_send(range_request_dir, range_buf, 2);		// 2 is the length
	while (ir_tx[range_request_dir].ir_status & IR_TX_STATUS_BUSY_bm);

	// OKAY DO YOUR COORDINATED EVENT HERE
	// Start synchronization timer
	uint32_t timer = get_time();
	set_green_led(100);
	_delay_ms(4); // Important delay to let the communication IR light fade
	led_off();

	if(range_req_res != 0)
	{
		printf("ir_send failed, code %i\r\n", range_req_res);
		if(range_req_res == 1)
		ir_reset_rx(range_request_dir);
		ir_reset_tx(range_request_dir);
	}

	for(uint8_t j = 0; j < 6; j++) // Transmitter goes through 6 directions
	{
		set_green_led(100);
		_delay_ms(1);
		led_off();
	
		range_request_dir = j;
	
	
		// take 19 more measurements
		for(uint8_t meas_num = 1; meas_num < NUMBER_OF_RB_MEASUREMENTS; meas_num++)
		{			//[sensor#][transmitter#][meas#]
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

	
		if (range_request_dir < 5) _delay_ms(DELAY_BETWEEN_RB_TRANSMISSIONS);
	}// end loop through 6 dirs
	
}

void IR_brightness_matrix(uint8_t range_request_dir, uint16_t range_request_brightness, uint8_t** brightness_matrix)
{
	uint8_t low_meas[6] = {255, 255, 255, 255, 255, 255};
	uint8_t high_meas[6] = {0};
	uint8_t diff_meas[6];

	uint8_t bright_meas[6][6][NUMBER_OF_RB_MEASUREMENTS]; 

	IR_all_brightness_matrices(range_request_dir, range_request_brightness, bright_meas);
	
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
	{
		diff_meas[k] = high_meas[k] - low_meas[k];

		brightness_matrix[range_request_dir][0] = diff_meas[0];
		brightness_matrix[range_request_dir][1] = diff_meas[1];
		brightness_matrix[range_request_dir][2] = diff_meas[2];
		brightness_matrix[range_request_dir][3] = diff_meas[3];
		brightness_matrix[range_request_dir][4] = diff_meas[4];
		brightness_matrix[range_request_dir][5] = diff_meas[5];
	}
}

