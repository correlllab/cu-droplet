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

//uint8_t take_steps(uint8_t motor_num, int16_t num_steps)
//{
	//if(is_moving()) return 0;
	//motor_status = MOTOR_STATUS_ON | (1 & MOTOR_STATUS_DIRECTION); //I AM LYING ABOUT THE DIRECTION.
	//
	//uint16_t actual_num_steps = (uint16_t)num_steps;
	//
	//current_motor_direction[0] = 0; current_motor_direction[1]=0; current_motor_direction[2]=0;
	//
	//int8_t sign_flip = ((((int8_t)((motor_flipped>>motor_num)&0x1))*-2)+1);
	//current_motor_direction[motor_num] = (sign_flip*((num_steps>>15)*2+1));
	//
	//uint16_t motor_duration = 32*motor_on_time + motor_adjusts[motor_num][current_motor_direction[motor_num]<0];
	//uint16_t total_time = motor_duration + 32*motor_off_time;
	//
	//switch(motor_num)
	//{
		//case 0: TCC0.PER = total_time; TCC0.CCA = TCC0.CCB = motor_duration; TCC0.CNT=0; break;
		//case 1: TCC1.PER = total_time; TCC1.CCA = TCC1.CCB = motor_duration; TCC1.CNT=0; break;
		//case 2: TCE0.PER = total_time; TCE0.CCA = TCE0.CCB = motor_duration; TCE0.CNT=0; break;
	//}
	//
	//if(current_motor_direction[motor_num] <0) motor_backward(motor_num);
	//else if(current_motor_direction[motor_num] > 0) motor_forward(motor_num);
	//else printf("Shouldn't get here - we set current_motor_direction to be non-zero!\r\n");
	//
	//uint32_t total_movement_duration = (((uint32_t)total_time)*((uint32_t)abs(num_steps)))/32;
	////printf("Total duration: %u ms.\r\n\n",total_movement_duration);
	//current_motor_task = schedule_task(total_movement_duration, stop, NULL);	
//}

uint8_t move_steps(uint8_t direction, uint16_t num_steps)
{
	if(is_moving()) return 0;
	motor_status = MOTOR_STATUS_ON | (direction & MOTOR_STATUS_DIRECTION);
	
	uint16_t mot_durs[3]; //This is how long we want each motor to be on for.
	int8_t mot_dirs[3]; //This is what direction we want each motor to spin in. 1: CCW, -1: CW, 0: No spin.
	uint16_t total_time = 0; //This is the total length of a step, and will be the period of the PWM generation.
	
	int8_t sign_flip;
	for(uint8_t mot=0 ; mot<3 ; mot++)
	{		
		mot_durs[mot] = 32*motor_on_time + abs(motor_adjusts[direction][mot]);
		
		mot_dirs[mot] = ((((motor_adjusts[direction][mot]>>15)&0x1)*-2)+1)*motor_signs[direction][mot];
		
		if(mot_durs[mot]==0) continue;
		
		total_time += mot_durs[mot] + 32*motor_off_time;
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
		current_offset += mot_durs[mot] + 32*motor_off_time;//If we left the motor on for longer to compensate, we should wait a little longer before starting again.
	}
	//printf("Offsets are: (%hu, %hu, %hu)\r\n",TCC0.CNT, TCC1.CNT, TCE0.CNT);
	if(current_offset != total_time) printf("ERROR (I think): current_offset: %hu and total_time: %hu not equal!\r\n", current_offset, total_time);
	for(uint8_t mot=0 ; mot<3 ; mot++) 	//Now we just need to tell the motors to go!
	{
		if(mot_dirs[mot]<0) motor_backward_two(mot); 
		else if(mot_dirs[mot]>0)	motor_forward_two(mot);
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
	
	PORTC.PIN0CTRL = 0;
	PORTC.PIN1CTRL = 0;
	PORTC.PIN4CTRL = 0;
	PORTC.PIN5CTRL = 0;
	PORTE.PIN0CTRL = 0;
	PORTE.PIN1CTRL = 0;
	
	
	TCD0.CTRLB = TC_WGMODE_SS_gc;		
	TCD0.INTCTRLB = 0x0;
	
	remove_task(current_motor_task);	
}

void brake(uint8_t num)
{

	switch(num)
	{
		case 0: TCC0.CTRLB = TC_WGMODE_SS_gc; PORTC.OUTSET |= PIN0_bm | PIN1_bm; break;
		case 1: TCC1.CTRLB = TC_WGMODE_SS_gc; PORTC.OUTSET |= PIN4_bm | PIN5_bm; break;
		case 2: TCE0.CTRLB = TC_WGMODE_SS_gc; PORTE.OUTSET |= PIN0_bm | PIN1_bm; break;
	}
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
		for (uint8_t motor_num = 0; motor_num < 3 ; motor_num++)
		{
			motor_adjusts[direction][motor_num] = ((((int16_t)SP_ReadUserSignatureByte(0x10 + 6*direction + 2*motor_num + 0))<<8) | ((int16_t)SP_ReadUserSignatureByte(0x10 + 6*direction + 2*motor_num + 1)));
		}

	}
	for (uint8_t direction = 0; direction < 8 ; direction++)
	{
		mm_per_kilostep[direction] =(uint16_t)SP_ReadUserSignatureByte(0x40 + 2*direction + 0)<<8 |
		(uint16_t)SP_ReadUserSignatureByte(0x40 + 2*direction + 1);
	}
}

void write_motor_settings()
{
	uint8_t page_buffer[512];
	for (uint16_t i = 0; i < 512; i++)
		page_buffer[i] = SP_ReadUserSignatureByte(i);
		
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		for (uint8_t motor_num = 0; motor_num < 3 ; motor_num++)
		{
			int16_t temp = motor_adjusts[direction][motor_num];
			page_buffer[(0x10 + 6*direction + 2*motor_num + 0)] = (uint8_t)((temp>>8)&0xFF);
			page_buffer[(0x10 + 6*direction + 2*motor_num + 1)] = (uint8_t)(temp&0xFF);
		}		
	}
	
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		uint16_t temp = mm_per_kilostep[direction];
		page_buffer[(0x40 + 2*direction + 0)] = (uint8_t)((temp>>8)&0xFF);
		page_buffer[(0x40 + 2*direction + 1)] = (uint8_t)(temp&0xFF);
	}					
	
	SP_LoadFlashPage(page_buffer);
	
	SP_EraseUserSignatureRow();
	SP_WriteUserSignatureRow();
}

//void print_motor_adjusts()
//{
	//printf("Motor Adjustments:\r\n");
	//printf("\tmotor 0:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[0][0],motor_adjusts[0][1], !!(motor_flipped&MOTOR_0_FLIPPED_bm));
	//printf("\tmotor 1:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[1][0],motor_adjusts[1][1], !!(motor_flipped&MOTOR_1_FLIPPED_bm));
	//printf("\tmotor 2:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[2][0],motor_adjusts[2][1], !!(motor_flipped&MOTOR_2_FLIPPED_bm));		
//}
//

void print_motor_values()
{
	printf("Motor Values\r\n");
	for(uint8_t direction=0;direction<8;direction++)
	{
		printf("\tdir: %d\t",direction);
		for(uint8_t motor=0;motor<3;motor++)
		{
			printf("%d\t", motor_adjusts[direction][motor]);
		}
		printf("\r\n");
	}
	printf("\r\n");
}
void broadcast_motor_adjusts()
{
	//char buffer[128];
	//uint16_t data_len = sprintf(buffer, "ccw0: %hd, cw0: %hd, f0: %hhu, ccw1: %hd, cw1: %hd, f1: %hhu, ccw2: %hd, cw2: %hd, f2: %hhu", 
					//motor_adjusts[0][0], motor_adjusts[0][1], !!(motor_flipped&MOTOR_0_FLIPPED_bm),
					//motor_adjusts[1][0], motor_adjusts[1][1], !!(motor_flipped&MOTOR_1_FLIPPED_bm),
					//motor_adjusts[2][0], motor_adjusts[2][1], !!(motor_flipped&MOTOR_2_FLIPPED_bm));
					//
	//ir_broadcast(buffer, data_len);
	//printf("\tmotor 0:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[0][0],motor_adjusts[0][1], !!(motor_flipped&MOTOR_0_FLIPPED_bm));
	//printf("\tmotor 1:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[1][0],motor_adjusts[1][1], !!(motor_flipped&MOTOR_1_FLIPPED_bm));
	//printf("\tmotor 2:\tccw: %hd, cw: %hd, flipped: %hhu\r\n",motor_adjusts[2][0],motor_adjusts[2][1], !!(motor_flipped&MOTOR_2_FLIPPED_bm));
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
//void motor_forward(uint8_t num)
//{
	//switch (num)
	//{
		//case 0: TCC0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
		//case 1:	TCC1.CTRLB = TC1_CCAEN_bm | TC_WGMODE_SS_gc; break;
		//case 2: TCE0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_SS_gc; break;
	//}
//}

// Low-level control of individual motors
//void motor_backward(uint8_t num)
//{
	//switch (num)
	//{
		//case 0: TCC0.CTRLB = TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
		//case 1: TCC1.CTRLB = TC1_CCBEN_bm | TC_WGMODE_SS_gc; break;
		//case 2: TCE0.CTRLB = TC0_CCBEN_bm | TC_WGMODE_SS_gc; break;
	//}
//}