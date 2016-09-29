#pragma once

#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "eeprom_driver.h"

#define MOTOR_STATUS_DIRECTION		0x07
#define MOTOR_STATUS_ON				0x80

#define NORTH				0
#define NORTHEAST			1
#define SOUTHEAST			2
#define SOUTH				3 
#define SOUTHWEST			4
#define NORTHWEST			5
#define CLOCKWISE			6
#define COUNTERCLOCKWISE	7

// Timing for taking a step:
#define MOTOR_ON_TIME			20
#define MOTOR_OFF_TIME			40

#include "droplet_init.h"
#include "scheduler.h"
#include "flash_api.h"

volatile uint8_t motor_status;
volatile Task_t* current_motor_task;

int16_t motor_on_time;
int16_t motor_off_time;

int16_t motor_adjusts[8][3];
//int8_t motor_signs[8][3];

uint16_t mm_per_kilostep[8]; //For the spin directions, this is degrees per kilostep.

// Sets up the timers for the motors PWM, pins to motor controller, and 
// reads the motor settings from non-volatile memory (user signature row)
void	motor_init();

// Walk in specified direction for specified number of steps
// direction (0-7, see #defines above for which direction maps to what number)
uint8_t	move_steps(uint8_t direction, uint16_t num_steps);

void walk(uint8_t direction, uint16_t mm);

// Stops all motors
void stop_move();

int8_t is_moving(); // returns -1 if droplet is not moving, movement dir otherwise.

void		read_motor_settings();
void		write_motor_settings();
void		print_motor_values();
void		broadcast_motor_adjusts();
void		print_dist_per_step();
void		broadcast_dist_per_step();
uint16_t	get_mm_per_kilostep(uint8_t direction);
void		set_mm_per_kilostep(uint8_t direction, uint16_t dist);

static inline void motor_forward(uint8_t num)
{
	switch(num)
	{
		#ifdef AUDIO_DROPLET
			case 0: printf_P(PSTR("ERROR! motor_fw called with num=0\r\n")); break;
		#else
			case 0: TCC0.CTRLB |= TC0_CCBEN_bm; break;
		#endif		
		case 1: TCC1.CTRLB |= TC1_CCBEN_bm; break;
		case 2: TCD0.CTRLB |= TC0_CCBEN_bm; break;
	}
}

static inline void motor_backward(uint8_t num)
{
	switch(num)
	{
		#ifdef AUDIO_DROPLET
			case 0: printf_P(PSTR("ERROR! motor_bw called with num=0\r\n")); break;
		#else
			case 0: TCC0.CTRLB |= TC0_CCAEN_bm; break;
		#endif
		case 1: TCC1.CTRLB |= TC1_CCAEN_bm; break;
		case 2: TCD0.CTRLB |= TC0_CCAEN_bm; break;
	}
}