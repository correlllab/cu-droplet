#include "motor.h"
#include <stdlib.h>

void motor_init()
{
	PORTC.DIRSET = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	PORTE.DIRSET = PIN0_bm | PIN1_bm;

	TCC0.CTRLA |= TC_CLKSEL_DIV1_gc;
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	TCC0.PER = 0xff;
    
	TCC1.CTRLA |= TC_CLKSEL_DIV1_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
	TCC1.PER = 0xff;

	TCE0.CTRLA |= TC_CLKSEL_DIV1_gc;
	TCE0.CTRLB = TC_WGMODE_SS_gc;
	TCE0.PER = 0xff;
	
	TCC0.CNTL = 0;			// Stagger the motors start time to lower peak current draw
	TCC1.CNTL = 85;
	TCE0.CNTL = 170;
	
	motor_status = 0;
	
	read_motor_settings();
}

// Turn all motors off, set status to cancel to prevent any currently scheduled tasks
// from turning the motors back on
void stop()
{
	motor_status = MOTOR_STATUS_CANCEL;
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
	TCE0.CTRLB = TC_WGMODE_SS_gc;
	
	PORTC.OUTCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	PORTE.OUTCLR = PIN0_bm | PIN1_bm;
}

// Walk in specified direction for specified number of steps
// direction 0--7 (see #defines in motor.h)
// Can also be used to rotate, if direction is set as CLOCKWISE or COUNTERCLOCKWISE
uint8_t move_steps(uint8_t direction, uint16_t num_steps)
{
	//stop(); Pretty sure john added this.
	//cancel_move(); Maybe put this in.
	if(is_moving()||is_rotating())
		return 0;
	
	printf("Moving... ");
	motor_num_steps = 0;
	motor_status = MOTOR_STATUS_ON | (direction & MOTOR_STATUS_DIRECTION);
	
	motor_num_steps = num_steps;
	motor_curr_step = 0;
	
	take_step(NULL);
	
	return 1;
}

// Walk in specified direction for duration (in milliseconds)
// direction 0--7 (see #defines in motor.h)
// Can also be used to rotate, if direction is set as CLOCKWISE or COUNTERCLOCKWISE
uint8_t move_duration(uint8_t direction, uint16_t duration)
{
	if(is_moving()||is_rotating())
		return 0;
	
	move_steps(direction, duration/300);
	
	return 1;
}

// Rotate in specified direction (1=clockwise, -1=counterclockwise)
uint8_t rotate_steps(int8_t direction, uint16_t num_steps)
{
	if(is_moving()||is_rotating())
	return 0;
	
	if (direction == 1 || direction == CLOCKWISE)
	move_steps(CLOCKWISE, num_steps);
	else if (direction == -1 || direction == COUNTERCLOCKWISE)
	move_steps(COUNTERCLOCKWISE, num_steps);
	
	return 1;
}

uint8_t rotate_duration(int8_t direction, uint16_t duration)
{
	if(is_moving()||is_rotating())
		return 0;
	
	if (direction == 1 || direction == CLOCKWISE)
		move_steps(CLOCKWISE, duration/450);
	else if (direction == -1 || direction == COUNTERCLOCKWISE)
		move_steps(COUNTERCLOCKWISE, duration/450);	
		
	return 1;
}

uint16_t cancel_rotate(void) // returns the number of steps taken
{
	motor_status |= MOTOR_STATUS_CANCEL;
	return motor_num_steps;
}

uint16_t cancel_move(void) // returns the number of steps taken
{
	motor_status |= MOTOR_STATUS_CANCEL;
	return motor_num_steps;
}

int8_t is_rotating(void) // returns 0 if droplet is not rotating, 1 if rotating cw, -1 if rotating ccw
{
	if (motor_status & MOTOR_STATUS_DIRECTION == CLOCKWISE)
		return 1;
	if (motor_status & MOTOR_STATUS_DIRECTION == COUNTERCLOCKWISE)
		return -1;
	return 0;
}

uint8_t is_moving(void) // returns 0 if droplet is not moving, (1-6) if moving
{
	if (motor_status & MOTOR_STATUS_ON)
	if (motor_status & MOTOR_STATUS_DIRECTION < 6)
	return (motor_status & MOTOR_STATUS_DIRECTION) + 1;
	
	return 0;
}

int8_t get_motor_duty_cycle(uint8_t motor_num, uint8_t direction)
{
	return motor_duty_cycle[motor_num-1][direction];
}

void set_motor_duty_cycle(uint8_t motor_num, uint8_t direction, int8_t duty_cycle)
{
	motor_duty_cycle[motor_num-1][direction] = duty_cycle;
}

void read_motor_settings()
{
	for (uint8_t motor_num = 1; motor_num <= 3; motor_num++) 
		for (uint8_t direction = 0; direction < 8; direction++)
			motor_duty_cycle[motor_num-1][direction] = ((int8_t)SP_ReadUserSignatureByte(0x10 + (motor_num-1) + 3*direction));
}

void write_motor_settings()
{
	uint8_t page_buffer[512];
	for (uint16_t i = 0; i < 512; i++)
		page_buffer[i] = SP_ReadUserSignatureByte(i);
	
	for (uint8_t motor_num = 1; motor_num <= 3; motor_num++)
		for (uint8_t direction = 0; direction < 8; direction++)
			page_buffer[(0x10 + (motor_num-1) + 3*direction)] = motor_duty_cycle[motor_num-1][direction];
		
	SP_LoadFlashPage(page_buffer);
	
	SP_EraseUserSignatureRow();
	SP_WriteUserSignatureRow();
}

void print_motor_settings()
{
	printf("Motor Settings\r\n");
	printf("\tN\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][0],motor_duty_cycle[1][0],motor_duty_cycle[2][0]);
	printf("\tNE\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][1],motor_duty_cycle[1][1],motor_duty_cycle[2][1]);
	printf("\tSE\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][2],motor_duty_cycle[1][2],motor_duty_cycle[2][2]);
	printf("\tS\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][3],motor_duty_cycle[1][3],motor_duty_cycle[2][3]);
	printf("\tSW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][4],motor_duty_cycle[1][4],motor_duty_cycle[2][4]);
	printf("\tNW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][5],motor_duty_cycle[1][5],motor_duty_cycle[2][5]);
	printf("\tCW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][6],motor_duty_cycle[1][6],motor_duty_cycle[2][6]);
	printf("\tCCW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][7],motor_duty_cycle[1][7],motor_duty_cycle[2][7]);
}

// SCHEDULED TASK IN TAKE_STEP
// DO NOT CALL
void mot_spin_up(void* arg)
{
	uint8_t num = (uint8_t)((uint16_t)arg & 0xFF);
	uint8_t dir = (uint16_t)arg >> 8;
	motor_set_duty_cycle(num, 1.0f);
	if (get_motor_duty_cycle(num,dir) > 0) motor_forward(num);
	else motor_backward(num);
}

// SCHEDULED TASK IN TAKE_STEP
// DO NOT CALL
void mot_on(void* arg)
{
	uint8_t num = (uint8_t)((uint16_t)arg & 0xFF);
	uint8_t dir = (uint16_t)arg >> 8;
	motor_set_duty_cycle(num, abs(get_motor_duty_cycle(num,dir))/100.0f);
}

// SCHEDULED TASK IN TAKE_STEP
// DO NOT CALL
void mot_off(void* arg)
{
	uint8_t num = (uint8_t)((uint16_t)arg & 0xFF);
	motor_off(num);
}

// Recursive function to walk
// CALLED BY move_steps().  DO NOT CALL
void take_step(void* arg)
{
	uint8_t direction = motor_status & MOTOR_STATUS_DIRECTION;
	
	if ((motor_curr_step == motor_num_steps) || (motor_status & MOTOR_STATUS_CANCEL))
	{
		stop();
		motor_curr_step = 0;
		motor_num_steps = 0;
		motor_status = 0;
		return;
	}
		
	motor_curr_step++;
	
	uint32_t step_time = 5L;

	if (get_motor_duty_cycle(1, direction) != 0)
	{
		uint16_t step_param = (direction << 8) | 1;
		schedule_task(step_time, mot_spin_up, step_param); step_time += MOTOR_SPINUP_TIME;
		schedule_task(step_time, mot_on, step_param); step_time += MOTOR_ON_TIME;
		schedule_task(step_time, mot_off, step_param); step_time += MOTOR_OFF_TIME;
	}
	if (get_motor_duty_cycle(2, direction) != 0)
	{
		uint16_t step_param = (direction << 8) | 2;
		schedule_task(step_time, mot_spin_up, step_param); step_time += MOTOR_SPINUP_TIME;
		schedule_task(step_time, mot_on, step_param); step_time += MOTOR_ON_TIME;
		schedule_task(step_time, mot_off, step_param); step_time += MOTOR_OFF_TIME;
	}
	if (get_motor_duty_cycle(3, direction) != 0)
	{
		uint16_t step_param = (direction << 8) | 3;
		schedule_task(step_time, mot_spin_up, step_param); step_time += MOTOR_SPINUP_TIME;
		schedule_task(step_time, mot_on, step_param); step_time += MOTOR_ON_TIME;
		schedule_task(step_time, mot_off, step_param); step_time += MOTOR_OFF_TIME;
	}
	schedule_task(step_time, take_step, NULL);
}

// Low-level control of individual motors
void motor_forward(uint8_t num)
{
	switch (num)
	{
		case 1: TCC0.CTRLB |= TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
		case 2:	TCC1.CTRLB |= TC1_CCAEN_bm | TC_WGMODE_SS_gc; break;
		case 3: TCE0.CTRLB |= TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
	}
}

// Low-level control of individual motors
void motor_backward(uint8_t num)
{
	switch (num)
	{
		case 1: TCC0.CTRLB |= TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
		case 2: TCC1.CTRLB |= TC1_CCBEN_bm | TC_WGMODE_SS_gc; break;
		case 3: TCE0.CTRLB |= TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
	}
}

// Low-level control of individual motors
void motor_on(int8_t num)
{
	if (num > 0)
		motor_forward(num);
	else
		motor_backward(-num);
		
}

// Low-level control of individual motors
void motor_off(uint8_t num)
{
	switch (num)
	{
		case 1:		TCC0.CTRLB = TC_WGMODE_SS_gc; break;
		case 2:		TCC1.CTRLB = TC_WGMODE_SS_gc; break;
		case 3:		TCE0.CTRLB = TC_WGMODE_SS_gc; break;
	}
}

// Sets the hardware period of the PWM
void motor_set_period(uint8_t num, uint16_t per)
{
	float duty_cycle_1 = (float)(TCC0.CCA)/(float)(TCC0.PER);
	float duty_cycle_2 = (float)(TCC1.CCA)/(float)(TCC1.PER);
	float duty_cycle_3 = (float)(TCE0.CCA)/(float)(TCE0.PER);
	
	switch (num)
	{
		case 1: TCC0.PER = per; break;
		case 2: TCC1.PER = per; break;
		case 3: TCE0.PER = per; break;
		default: TCC0.PER = TCC1.PER = TCE0.PER = per;
	}
	
	TCC0.CCA = TCC0.CCB = duty_cycle_1 * TCC0.PER;
	TCC1.CCA = TCC1.CCB = duty_cycle_2 * TCC1.PER;
	TCE0.CCA =TCE0.CCB = duty_cycle_3 * TCE0.PER;
}

// Sets the hardware duty cycle of the PWM
void motor_set_duty_cycle(uint8_t num, float duty_cycle)
{
	if (duty_cycle > 1 || duty_cycle < 0) return;

	switch (num)
	{
		case 1: TCC0.CCA = TCC0.CCB = duty_cycle * TCC0.PER; break;
		case 2: TCC1.CCA = TCC1.CCB = duty_cycle * TCC1.PER; break;
		case 3: TCE0.CCA = TCE0.CCB = duty_cycle * TCE0.PER; break;
		default: 
			 TCC0.CCA = TCC0.CCB = duty_cycle * TCC0.PER;
			 TCC1.CCA = TCC1.CCB = duty_cycle * TCC1.PER;
			 TCE0.CCA = TCE0.CCB = duty_cycle * TCE0.PER; 
	}
}
