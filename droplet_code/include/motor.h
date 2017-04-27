#pragma once
#include "droplet_base.h"
#include "scheduler.h"
#include "flash_api.h"

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


int16_t motor_adjusts[8][3];
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