#include "motor.h"
#include <stdlib.h>

void motor_init()
{
	PORTC.DIRSET = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	PORTE.DIRSET = PIN0_bm | PIN1_bm;

	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	//TCC0.PER = 32; This needs to be set for each move_step, now.
    
	TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
	//TCC0.PER = 32; This needs to be set for each move_step, now.

	TCE0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCE0.CTRLB = TC_WGMODE_SS_gc;
	//TCC0.PER = 32; This needs to be set for each move_step, now.
	
	motor_status = 0;
	
	motor_signs[0][0]=0;	motor_signs[0][1]=1;	motor_signs[0][2]=-1;  	//Towards motor 0.
	motor_signs[1][0]=-1;	motor_signs[1][1]=1;	motor_signs[1][2]=0;  	//Away from motor 2.
	motor_signs[2][0]=-1;	motor_signs[2][1]=0;	motor_signs[2][2]=1;  	//Towards motor 1.
	motor_signs[3][0]=0;	motor_signs[3][1]=-1;	motor_signs[3][2]=1;  	//Away from motor 0.
	motor_signs[4][0]=1;	motor_signs[4][1]=-1;	motor_signs[4][2]=0;  	//Towards motor 2.
	motor_signs[5][0]=1;	motor_signs[5][1]=0;	motor_signs[5][2]=-1;  	//Away from motor 1.
	motor_signs[6][0]=-1;	motor_signs[6][1]=-1;	motor_signs[6][2]=-1;  	//Clockwise spin.
	motor_signs[7][0]=1;	motor_signs[7][1]=1;	motor_signs[7][2]=1;  	//Anti-Clockwise spin.
	
	motor_on_time = MOTOR_ON_TIME;
	motor_off_time = MOTOR_OFF_TIME;
	
	read_motor_settings();
}

uint8_t move_steps(uint8_t direction, uint16_t num_steps)
{
	if(is_moving()) return 0;
	motor_status = MOTOR_STATUS_ON | (direction & MOTOR_STATUS_DIRECTION);
	
	uint16_t mot_durs[3]; //This is how long we want each motor to be on for.
	uint16_t total_time = 0; //This is the total length of a step, and will be the period of the PWM generation.
	/*
	 * The factors of 32 sprinkled below are to convert from time units of ms to the semi-understood "units" of the waveform generator.
	 * Given a prescalar of 1024 and single slope waveform generation, a factor of 32 converts the units to/from ms.
	 */	
	
	uint8_t motor_backward_q[3]; //for each motor, this arrays is 0 if the motor is going to spin forard and 1 if the motor is going to spin backward
	int8_t sign_flip;
	for(uint8_t mot=0 ; mot<3 ; mot++) //populating the motor_backward_q array.
	{
		sign_flip = ((((int8_t)((motor_flipped>>mot)&0x1))*-2)+1); //converting the bit mask to a 1 or -1 as needed.
		motor_backward_q[mot] = sign_flip*motor_signs[direction][mot]<0;
	}
	//printf("\r\nMotor backward? (%hhu, %hhu, %hhu).\r\n",motor_backward_q[0], motor_backward_q[1], motor_backward_q[2]);
	
	for(uint8_t mot=0 ; mot<3 ; mot++)
	{		
		if(motor_signs[direction][mot]==0){ mot_durs[mot]=0; continue;}
		mot_durs[mot] = 32*motor_on_time + motor_adjusts[mot][motor_backward_q[mot]];
		total_time += mot_durs[mot] + 32*motor_off_time + motor_adjusts[mot][motor_backward_q[mot]];//If we left the motor on for longer to compensate, we should wait a little longer before starting again.
	}
	//printf("Moving in dir: %hhu for %hu steps. Mot_durs: {%hu, %hu, %hu}. Total_time: %hu.\r\n",direction, num_steps, mot_durs[0], mot_durs[1], mot_durs[2], total_time);

	TCC0.PER = TCC1.PER = TCE0.PER = total_time;
	TCC0.CCA = TCC0.CCB = mot_durs[0]; //motor 0
	TCC1.CCA = TCC1.CCB = mot_durs[1]; //motor 1
	TCE0.CCA = TCE0.CCB = mot_durs[2]; //motor 2
	
	uint16_t current_offset = 0;
	
	for(uint8_t mot=0 ; mot<3 ; mot++) //This loops sets up the offsets correctly, so that (for example) the 30 ms that motor 1 is on won't start until 40ms after motor 0 turns off.
	{
		if(mot_durs[mot]==0) continue;
		switch(mot)
		{
			case 0: TCC0.CNT = ((total_time - current_offset)%total_time); break;
			case 1: TCC1.CNT = ((total_time - current_offset)%total_time); break;
			case 2: TCE0.CNT = ((total_time - current_offset)%total_time); break;
		}
		current_offset += mot_durs[mot] + 32*motor_off_time + motor_adjusts[mot][motor_backward_q[mot]];//If we left the motor on for longer to compensate, we should wait a little longer before starting again.
	}
	//printf("Offsets are: (%hu, %hu, %hu)\r\n",TCC0.CNT, TCC1.CNT, TCE0.CNT);
	if(current_offset != total_time) printf("ERROR (I think): current_offset: %hu and total_time: %hu not equal!\r\n", current_offset, total_time);
	
	
	for(uint8_t mot=0 ; mot<3 ; mot++) 	//Now we just need to tell the motors to go!
	{
		if(mot_durs[mot]==0) continue;
		if(motor_backward_q[mot]) motor_backward(mot);
		else motor_forward(mot);
	}
	uint32_t total_movement_duration = ((uint32_t)total_time)*((uint32_t)num_steps)/32;
	//printf("Total duration: %u ms.\r\n\n",total_movement_duration);
	current_motor_task = schedule_task(total_movement_duration, stop, NULL);
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
	//printf("Stopping.\r\n");	
	motor_status = MOTOR_STATUS_CANCEL;
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
	TCE0.CTRLB = TC_WGMODE_SS_gc;
	
	PORTC.OUTCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	PORTE.OUTCLR = PIN0_bm | PIN1_bm;
	remove_task(current_motor_task);	
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
	for (uint8_t motor_num = 0; motor_num < 3; motor_num++)
	{
		for (uint8_t cw_q = 0; cw_q < 2 ; cw_q++)
		{
			motor_adjusts[motor_num][cw_q] = ((((int16_t)SP_ReadUserSignatureByte(0x10 + 4*motor_num + 2*cw_q + 0))<<8) | ((int16_t)SP_ReadUserSignatureByte(0x10 + 4*motor_num + 2*cw_q + 1)));
		}

	}
	for (uint8_t direction = 0; direction < 8 ; direction++)
	{
		mm_per_kilostep[direction] =(uint16_t)SP_ReadUserSignatureByte(0x1c + 2*direction + 0)<<8 |
		(uint16_t)SP_ReadUserSignatureByte(0x1c + 2*direction + 1);
	}		
	motor_flipped = SP_ReadUserSignatureByte(0x2c);
}

void write_motor_settings()
{
	uint8_t page_buffer[512];
	for (uint16_t i = 0; i < 512; i++)
		page_buffer[i] = SP_ReadUserSignatureByte(i);
		
	for (uint8_t motor_num = 0; motor_num < 3; motor_num++)
	{
		for (uint8_t cw_q = 0; cw_q < 2 ; cw_q++)
		{
			int16_t temp = motor_adjusts[motor_num][cw_q];
			page_buffer[(0x10 + 4*motor_num + 2*cw_q + 0)] = (uint8_t)((temp>>8)&0xFF);
			page_buffer[(0x10 + 4*motor_num + 2*cw_q + 1)] = (uint8_t)(temp&0xFF);
		}		
	}
	
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		uint16_t temp = mm_per_kilostep[direction];
		page_buffer[(0x1c + 2*direction + 0)] = (uint8_t)((temp>>8)&0xFF);
		page_buffer[(0x1c + 2*direction + 1)] = (uint8_t)(temp&0xFF);
	}					
		
	page_buffer[0x2c] = motor_flipped;		
		
	SP_LoadFlashPage(page_buffer);
	
	SP_EraseUserSignatureRow();
	SP_WriteUserSignatureRow();
}

void print_motor_adjusts()
{
	printf("Motor Adjustments:\r\n");
	printf("\tmotor 0:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[0][0],motor_adjusts[0][1], !!(motor_flipped&MOTOR_0_FLIPPED_bm));
	printf("\tmotor 1:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[1][0],motor_adjusts[1][1], !!(motor_flipped&MOTOR_1_FLIPPED_bm));
	printf("\tmotor 2:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[2][0],motor_adjusts[2][1], !!(motor_flipped&MOTOR_2_FLIPPED_bm));		
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