#include "motor.h"

static volatile uint8_t motor_status;
static volatile Task_t* current_motor_task;

static int16_t motor_on_time;
static int16_t motor_off_time;

static inline void motor_forward(uint8_t num)
{
	switch(num)
	{
		#ifdef AUDIO_DROPLET
		case 0: printf_P(PSTR("ERROR! motor_fw called with num=0\r\n")); break;
		#else
		case 0: TCC0.CTRLB |= TC0_CCBEN_bm; TCC0.CTRLC = 0; TCC0.CTRLA = TC_CLKSEL_DIV1024_gc; break;
		#endif
		case 1: TCC1.CTRLB |= TC1_CCBEN_bm; TCC1.CTRLC = 0; TCC1.CTRLA = TC_CLKSEL_DIV1024_gc; break;
		case 2: TCD0.CTRLB |= TC0_CCBEN_bm; TCD0.CTRLC = 0; TCD0.CTRLA = TC_CLKSEL_DIV1024_gc; break;
	}
}

static inline void motor_backward(uint8_t num)
{
	switch(num)
	{
		#ifdef AUDIO_DROPLET
		case 0: printf_P(PSTR("ERROR! motor_bw called with num=0\r\n")); break;
		#else
		case 0: TCC0.CTRLB |= TC0_CCAEN_bm; TCC0.CTRLC = 0; TCC0.CTRLA = TC_CLKSEL_DIV1024_gc; break;
		#endif
		case 1: TCC1.CTRLB |= TC1_CCAEN_bm; TCC1.CTRLC = 0; TCC1.CTRLA = TC_CLKSEL_DIV1024_gc; break;
		case 2: TCD0.CTRLB |= TC0_CCAEN_bm; TCD0.CTRLC = 0; TCD0.CTRLA = TC_CLKSEL_DIV1024_gc; break;
	}
}


void motorInit()
{
	#ifdef AUDIO_DROPLET
		PORTC.DIRSET = PIN4_bm | PIN5_bm;
	#else
		PORTC.DIRSET = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	#endif
	PORTD.DIRSET = PIN0_bm | PIN1_bm; 

	#ifndef AUDIO_DROPLET
		TCC0.CTRLA = TC_CLKSEL_OFF_gc;
		TCC0.CTRLB = TC_WGMODE_SS_gc;
	#endif
	
    TCC1.CTRLA = TC_CLKSEL_OFF_gc;
    TCC1.CTRLB = TC_WGMODE_SS_gc;

    TCD0.CTRLA = TC_CLKSEL_OFF_gc;
    TCD0.CTRLB = TC_WGMODE_SS_gc;  
	
	#ifndef AUDIO_DROPLET
	PORTC.PIN0CTRL = PORT_INVEN_bm;
	PORTC.PIN1CTRL = PORT_INVEN_bm;
	#endif
	PORTC.PIN4CTRL = PORT_INVEN_bm;
	PORTC.PIN5CTRL = PORT_INVEN_bm;
	PORTD.PIN0CTRL = PORT_INVEN_bm;
	PORTD.PIN1CTRL = PORT_INVEN_bm;
	
	#ifndef AUDIO_DROPLET
	PORTC.OUTCLR = PIN0_bm | PIN1_bm;
	#endif
	PORTC.OUTCLR = PIN4_bm | PIN5_bm;
	PORTD.OUTCLR = PIN0_bm | PIN1_bm;

	motor_status = 0;

	motor_on_time = MOTOR_ON_TIME;
	motor_off_time = MOTOR_OFF_TIME;
	
	readMotorSettings();
}

uint8_t moveSteps(uint8_t direction, uint16_t num_steps)
{
	#ifdef AUDIO_DROPLET
		motorAdjusts[direction][0]=0;
	#endif
	if(isMoving()>=0) return 0;
	motor_status = MOTOR_STATUS_ON | (direction & MOTOR_STATUS_DIRECTION);
	
	uint16_t mot_durs[3]; //This is how long we want each motor to be on for.
	int8_t mot_dirs[3]; //This is what direction we want each motor to spin in. 1: CCW, -1: CW, 0: No spin.
	uint16_t total_time = 0; //This is the total length of a step, and will be the period of the PWM generation.
	
	for(uint8_t mot=0 ; mot<3 ; mot++)
	{	
		if(motorAdjusts[direction][mot]==0)
		{
			mot_durs[mot] = 0;
			mot_dirs[mot] = 0;
			continue;
		}
		else
		{
			mot_durs[mot] = 32*motor_on_time + abs(motorAdjusts[direction][mot]);			
			mot_dirs[mot] = ((((motorAdjusts[direction][mot]>>15)&0x1)*-2)+1)/**motor_signs[direction][mot]*/;
			total_time += mot_durs[mot] + 32*motor_off_time;
		}
	}
	//printf("Moving in dir: %hu for %u steps. Mot_durs: {%u, %u, %u}. Total_time: %u.\r\n",direction, num_steps, mot_durs[0], mot_durs[1], mot_durs[2], total_time);
	//printf("Mot_dirs: {%hd, %hd, %hd}.\r\n", mot_dirs[0], mot_dirs[1], mot_dirs[2]);

	#ifdef AUDIO_DROPLET
		TCC1.PER = TCD0.PER = total_time; 
		TCC1.CCA = TCC1.CCB = mot_durs[1]; //motor 1
		TCD0.CCA = TCD0.CCB = mot_durs[2]; //motor 2
	#else
		TCC0.PER = TCC1.PER = TCD0.PER = total_time;
		TCC0.CCA = TCC0.CCB = mot_durs[0]; //motor 0
		TCC1.CCA = TCC1.CCB = mot_durs[1]; //motor 1
		TCD0.CCA = TCD0.CCB = mot_durs[2]; //motor 2
	#endif
	
	uint16_t current_offset = 0;
	
	for(uint8_t mot=0 ; mot<3 ; mot++) //This loops sets up the offsets correctly, so that (for example) the 30 ms that motor 1 is on won't start until 40ms after motor 0 turns off.
	{
		if(mot_durs[mot]==0) continue;
		switch(mot)
		{
			#ifdef AUDIO_DROPLET
				case 0: printf_P(PSTR("ERROR! In move_steps, mot_durs[0]!=0\r\n")); break;
			#else
				case 0: TCC0.CNT = ((total_time - current_offset)%total_time); break;
			#endif
			case 1: TCC1.CNT = ((total_time - current_offset)%total_time); break;
			case 2: TCD0.CNT = ((total_time - current_offset)%total_time); break;
		}
		current_offset += mot_durs[mot] + 32*motor_off_time;//If we left the motor on for longer to compensate, we should wait a little longer before starting again.
	}
	
	if(current_offset != total_time) printf_P(PSTR("ERROR: current_offset: %u and total_time: %u not equal!\r\n"), current_offset, total_time);
	//printf("Just about to turn on motors: %lu\r\n",get_time());
	//TCC0.CTRLFSET = TC_TC0_CMD_RESET_gc;
	//TCC1.CTRLFSET = TC_TC0_CMD_RESET_gc;
	//TCD0.CTRLFSET = TC_TC0_CMD_RESET_gc;


	for(uint8_t mot=0 ; mot<3 ; mot++) 	//Now we just need to tell the motors to go!
	{
		if(mot_dirs[mot]<0) motor_backward(mot); 
		else if(mot_dirs[mot]>0)	motor_forward(mot);
	}
	uint32_t total_movement_duration = (((uint32_t)total_time)*((uint32_t)num_steps))/32;
	//printf("Total duration: %lu ms.\r\n\n",total_movement_duration);
	current_motor_task = scheduleTask(total_movement_duration, stopMove, NULL);
	if(current_motor_task==NULL) printf_P(PSTR("Error! Failed to schedule stop_move task."));
	return 1;
}

void walk(uint8_t direction, uint16_t mm)
{
	uint16_t mm_per_kilostep = getMMperKilostep(direction);
	if(abs((0xFFFF-((uint32_t)mm_per_kilostep)))<1000){
		printf_P(PSTR("Error: Don't have calibrated values for this direction.\r\n"));
		if(direction>5){
			mm_per_kilostep = 2000;
		}else{
			mm_per_kilostep = 600;
		}
	}
	float mm_per_step = (1.0*mm_per_kilostep)/1000.0;
	float steps = (1.0*mm)/mm_per_step;
	delayMS(10);
	printf_P(PSTR("In order to go in direction %hu for %u mm, taking %u steps.\r\n"),direction, mm, (uint16_t)steps);
	delayMS(10);
	moveSteps(direction, (uint16_t)steps);
}

void stopMove()
{
	//printf("Stopping.\r\n");
	
	#ifndef AUDIO_DROPLET
		TCC0.CTRLB &= ~(TC0_CCAEN_bm | TC0_CCBEN_bm);
		TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	#endif
	TCC1.CTRLB  &= ~(TC1_CCAEN_bm | TC1_CCBEN_bm);
	TCC1.CTRLA = TC_CLKSEL_OFF_gc;
	TCD0.CTRLB  &= ~(TC0_CCAEN_bm | TC0_CCBEN_bm);
	TCD0.CTRLA = TC_CLKSEL_OFF_gc;
	
	#ifdef AUDIO_DROPLET
		PORTC.OUTCLR = PIN4_bm | PIN5_bm;
	#else
		PORTC.OUTCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;	
	#endif	
	PORTD.OUTCLR = PIN0_bm | PIN1_bm; 
	
	motor_status = 0;
	removeTask((Task_t*)current_motor_task);
	current_motor_task = NULL;
}

int8_t isMoving() // returns -1 if droplet is not moving, movement dir otherwise.
{
	if (motor_status & MOTOR_STATUS_ON){
		return (motor_status & MOTOR_STATUS_DIRECTION);
	}	
	return -1;
}

uint16_t getMMperKilostep(uint8_t direction)
{
	return mmPerKilostep[direction];
}

void setMMperKilostep(uint8_t direction, uint16_t dist)
{
	mmPerKilostep[direction] = dist;	
}

void readMotorSettings()
{
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		for (uint8_t motor_num = 0; motor_num < 3 ; motor_num++)
		{
			motorAdjusts[direction][motor_num] = ((((int16_t)EEPROM_read_byte(0x10 + 6*direction + 2*motor_num + 0))<<8) | ((int16_t)EEPROM_read_byte(0x10 + 6*direction + 2*motor_num + 1)));
		}

	}
	for (uint8_t direction = 0; direction < 8 ; direction++)
	{
		mmPerKilostep[direction] =(uint16_t)EEPROM_read_byte(0x40 + 2*direction + 0)<<8 | (uint16_t)EEPROM_read_byte(0x40 + 2*direction + 1);
	}
}

void writeMotorSettings()
{
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		for (uint8_t motor_num = 0; motor_num < 3 ; motor_num++)
		{
			int16_t temp = motorAdjusts[direction][motor_num];
			EEPROM_write_byte(0x10 + 6*direction + 2*motor_num + 0, (uint8_t)((temp>>8)&0xFF));
			EEPROM_write_byte(0x10 + 6*direction + 2*motor_num + 1, (uint8_t)(temp&0xFF));
		}
	}
	
	for (uint8_t direction = 0; direction < 8; direction++)
	{
		uint16_t temp = mmPerKilostep[direction];
		EEPROM_write_byte(0x40 + 2*direction + 0, (uint8_t)((temp>>8)&0xFF));
		EEPROM_write_byte(0x40 + 2*direction + 1, (uint8_t)(temp&0xFF));
	}
}

void printMotorValues()
{
	printf_P(PSTR("Motor Values\r\n"));
	for(uint8_t direction=0;direction<8;direction++)
	{
		printf_P(PSTR("\tdir: %hu\t"),direction);
		for(uint8_t motor=0;motor<3;motor++)
		{
			printf("%d\t", motorAdjusts[direction][motor]);
		}
		printf("\r\n");
	}
	printf("\r\n");
}
void broadcastMotorAdjusts()
{
	// TODO: Deprecated?
}

void printDistPerStep()
{
	printf_P(PSTR("Dist (mm) per kilostep\r\n"));
	for(uint8_t direction = 0 ; direction<8; direction++)
	{
		printf_P(PSTR("\t%hu\t%u\r\n"), direction, mmPerKilostep[direction]);	
	}
}
