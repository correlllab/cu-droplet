#include "motor.h"
#include <stdlib.h>

void motor_init()
{
	PORTC.DIRSET = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	PORTE.DIRSET = PIN0_bm | PIN1_bm;

	TCC0.CTRLA |= TC_CLKSEL_DIV1024_gc;
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	TCC0.PER = 32;
    
	TCC1.CTRLA |= TC_CLKSEL_DIV1024_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
	TCC1.PER = 32;

	TCE0.CTRLA |= TC_CLKSEL_DIV1024_gc;
	TCE0.CTRLB = TC_WGMODE_SS_gc;
	TCE0.PER = 32;
	
	TCC0.CNTL = 0;
	TCC1.CNTL = 0;
	TCE0.CNTL = 0;
	
	motor_status = 0;
	
	read_motor_settings();
}

uint8_t move_steps(uint8_t direction, uint16_t num_steps)
{
	if(is_moving())
	return 0;
	
	printf("Other Moving... ");
	motor_num_steps = 0;
	motor_status = MOTOR_STATUS_ON | (direction & MOTOR_STATUS_DIRECTION);
	
	motor_num_steps = num_steps;
	motor_curr_step = 0;
	
	uint16_t step_param = (direction << 8) | 1;
	take_step(step_param);
	return 1;
}

void take_step(void* arg)
{
	uint8_t num = (uint8_t)((uint16_t)arg & 0xFF);
	uint8_t dir = (uint16_t)arg >> 8;
	int8_t duty_cycle = motor_duty_cycle[num][dir];
	
	if(duty_cycle<0)
	{
		motor_backward(num);
	}
	else
	{
		motor_forward(num);
	}
	set_motor_duty_cycle(num, (float)(abs(duty_cycle)/127.0f));
	delay_ms(MOTOR_ON_TIME);
	motor_off(num);
	
	num=(num+1)%3;

	if(num==0)
	{
		motor_curr_step+=1;
	}
	
	if ((motor_curr_step == motor_num_steps) || (motor_status & MOTOR_STATUS_CANCEL))
	{
		stop();
		motor_curr_step = 0;
		motor_num_steps = 0;
		motor_status = 0;
		return;
	}
	
	uint16_t step_param = (dir << 8) | num;
	schedule_task(MOTOR_OFF_TIME, take_step, step_param);
}


void walk(uint8_t direction, uint16_t mm)
{
	uint16_t mm_per_kilostep = get_mm_per_kilostep(direction);
	float mm_per_step = (1.0*mm_per_kilostep)/1000.0;
	float steps = (1.0*mm)/mm_per_step;
	printf("In order to go in direction %u for %u mm, taking %u steps.\r\n",direction, mm, (uint16_t)steps);
	move_steps(direction, (uint16_t)steps);
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

uint16_t cancel_move(void) // returns the number of steps taken
{
	motor_status |= MOTOR_STATUS_CANCEL;
	return motor_num_steps;
}

uint8_t is_moving(void) // returns 0 if droplet is not moving, (1-6) if moving
{
	if ((motor_status & MOTOR_STATUS_ON) || (motor_status & MOTOR_STATUS_DIRECTION < 8)){
		return (motor_status & MOTOR_STATUS_DIRECTION) + 1;
	}	
	
	return 0;
}

uint16_t get_mm_per_kilostep(uint8_t direction)
{
	return mm_per_kilostep[direction];
}

void set_mm_per_kilostep(uint8_t direction, uint16_t dist)
{
	mm_per_kilostep[direction] = dist;	
}

void read_motor_settings()
{
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		for (uint8_t motor_num = 0; motor_num < 3; motor_num++)
		{
			motor_duty_cycle[motor_num][direction] = ((int8_t)SP_ReadUserSignatureByte(0x10 + (motor_num) + 3*direction));
		}
		mm_per_kilostep[direction] =(uint16_t)SP_ReadUserSignatureByte(0x28 + 0 + 2*direction)<<8 |
									(uint16_t)SP_ReadUserSignatureByte(0x28 + 1 + 2*direction);
	}		
}

void write_motor_settings()
{
	uint8_t page_buffer[512];
	for (uint16_t i = 0; i < 512; i++)
		page_buffer[i] = SP_ReadUserSignatureByte(i);
	
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		for (uint8_t motor_num = 0; motor_num < 3; motor_num++)
		{
			page_buffer[(0x10 + (motor_num) + 3*direction)] = motor_duty_cycle[motor_num][direction];
		}
		uint16_t temp = mm_per_kilostep[direction];
		page_buffer[(0x28 + 0 + 2*direction)] = (uint8_t)((temp>>8)&0xFF);
		page_buffer[(0x28 + 1 + 2*direction)] = (uint8_t)(temp&0xFF);
	}					
		
	SP_LoadFlashPage(page_buffer);
	
	SP_EraseUserSignatureRow();
	SP_WriteUserSignatureRow();
}

void print_motor_duty_cycles()
{
	printf("Motor Duty Cycles\r\n");
	printf("\tN\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][0],motor_duty_cycle[1][0],motor_duty_cycle[2][0]);
	printf("\tNE\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][1],motor_duty_cycle[1][1],motor_duty_cycle[2][1]);
	printf("\tSE\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][2],motor_duty_cycle[1][2],motor_duty_cycle[2][2]);
	printf("\tS\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][3],motor_duty_cycle[1][3],motor_duty_cycle[2][3]);
	printf("\tSW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][4],motor_duty_cycle[1][4],motor_duty_cycle[2][4]);
	printf("\tNW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][5],motor_duty_cycle[1][5],motor_duty_cycle[2][5]);
	printf("\tCW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][6],motor_duty_cycle[1][6],motor_duty_cycle[2][6]);
	printf("\tCCW\tm1 %i\tm2 %i\tm3 %i\r\n", motor_duty_cycle[0][7],motor_duty_cycle[1][7],motor_duty_cycle[2][7]);
}

void print_dist_per_step()
{
	printf("Dist (mm) per kilostep\r\n");
	for(uint8_t direction = 0 ; direction<8; direction++)
	{
		printf("\t%i\t%hu\r\n", direction, mm_per_kilostep[direction]);	
	}
}

// Low-level control of individual motors
void motor_forward(uint8_t num)
{
	switch (num)
	{
		case 0: TCC0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
		case 1:	TCC1.CTRLB = TC1_CCAEN_bm | TC_WGMODE_SS_gc; break;
		case 2: TCE0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
	}
}

// Low-level control of individual motors
void motor_backward(uint8_t num)
{
	switch (num)
	{
		case 0: TCC0.CTRLB = TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
		case 1: TCC1.CTRLB = TC1_CCBEN_bm | TC_WGMODE_SS_gc; break;
		case 2: TCE0.CTRLB = TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
	}
}

// Low-level control of individual motors
void motor_off(uint8_t num)
{
	switch (num)
	{
		case 0:		TCC0.CTRLB = TC_WGMODE_SS_gc; break;
		case 1:		TCC1.CTRLB = TC_WGMODE_SS_gc; break;
		case 2:		TCE0.CTRLB = TC_WGMODE_SS_gc; break;
	}
}

// Sets the hardware period of the PWM
void motor_set_period(uint8_t num, uint16_t per)
{
	float duty_cycle_0 = (float)(TCC0.CCA)/(float)(TCC0.PER);
	float duty_cycle_1 = (float)(TCC1.CCA)/(float)(TCC1.PER);
	float duty_cycle_2 = (float)(TCE0.CCA)/(float)(TCE0.PER);
	
	switch (num)
	{
		case 0: TCC0.PER = per; break;
		case 1: TCC1.PER = per; break;
		case 2: TCE0.PER = per; break;
		default: TCC0.PER = TCC1.PER = TCE0.PER = per;
	}
	
	TCC0.CCA = TCC0.CCB = (uint16_t)(duty_cycle_0 * TCC0.PER);
	TCC1.CCA = TCC1.CCB = (uint16_t)(duty_cycle_1 * TCC1.PER);
	TCE0.CCA =TCE0.CCB = (uint16_t)(duty_cycle_2 * TCE0.PER);
}

// Sets the hardware duty cycle of the PWM
void set_motor_duty_cycle(uint8_t num, float duty_cycle)
{
	if (duty_cycle > 1 || duty_cycle < 0) return;

	switch (num)
	{
		case 0: TCC0.CCA = TCC0.CCB = (uint16_t)(duty_cycle * TCC0.PER); break;
		case 1: TCC1.CCA = TCC1.CCB = (uint16_t)(duty_cycle * TCC1.PER); break;
		case 2: TCE0.CCA = TCE0.CCB = (uint16_t)(duty_cycle * TCE0.PER); break;
		default: 
			 TCC0.CCA = TCC0.CCB = duty_cycle * TCC0.PER;
			 TCC1.CCA = TCC1.CCB = duty_cycle * TCC1.PER;
			 TCE0.CCA = TCE0.CCB = duty_cycle * TCE0.PER; 
	}
}
