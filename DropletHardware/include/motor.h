#include <avr/io.h>

#ifndef motor_h
#define motor_h

#define MOTOR_STATUS_DIRECTION		0x07
#define MOTOR_STATUS_CANCEL			0x40
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
#define MOTOR_ON_TIME			30L
#define MOTOR_OFF_TIME			40L

#include "droplet_init.h"
#include "scheduler.h"


volatile uint8_t motor_status; // [ on, cancel, 0, 0, 0, direction(2-0) ] 
volatile uint16_t motor_num_steps; // total number of steps to take in current walk command
volatile uint16_t motor_curr_step; // current step number in current walk command
volatile int32_t buckets[3];

int8_t motor_strengths[3];
int8_t motor_signs[8][3];

int8_t motor_duty_cycle[3][8]; // Table that holds the motor settings for moving in different directions
uint16_t mm_per_kilostep[8];


// Sets up the timers for the motors PWM, pins to motor controller, and 
// reads the motor settings from non-volatile memory (user signature row)
void	motor_init();

// Walk in specified direction for specified number of steps
// direction (0-7, see #defines above for which direction maps to what number)
uint8_t	move_steps(uint8_t direction, uint16_t num_steps);

void	take_step(void* arg);

void walk(uint8_t direction, uint16_t mm);

// Stops all motors
void stop();

uint16_t cancel_move(void); // returns the number of steps taken (1 step = 2 sub-steps)

uint8_t is_moving(void); // returns 0 if droplet is not moving, otherwise returns the direction of motion (1-6)

// Getter and setter for individual motor settings when moving in direction
uint16_t	get_mm_per_kilostep(uint8_t direction);
void		set_mm_per_kilostep(uint8_t direction, uint16_t dist);
void		read_motor_settings();
void		write_motor_settings();
void		print_motor_duty_cycles();
void		print_dist_per_step();

void motor_forward(uint8_t num);
void motor_backward(uint8_t num);
void motor_off(uint8_t num);

uint16_t get_mm_per_kilostep(uint8_t direction);
void set_mm_per_kilostep(uint8_t direction, uint16_t dist);

void motor_set_period(uint8_t dir, uint16_t per);
void set_motor_duty_cycle(uint8_t dir, float duty_cycle); // Note: 0 <= duty_cycle <= 1

#endif
