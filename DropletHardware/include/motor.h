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
#define MOTOR_SPINUP_TIME		25L
#define MOTOR_ON_TIME			100L
#define MOTOR_OFF_TIME			25L

#include "droplet_init.h"
#include "scheduler.h"

volatile uint8_t motor_status; // [ on, cancel, 0, 0, 0, direction(2-0) ] 
volatile uint16_t motor_num_steps; // total number of steps to take in current walk command
volatile uint16_t motor_curr_step; // current step number in current walk command

int8_t motor_duty_cycle[3][8]; // Table that holds the motor settings for moving in different directions

// Sets up the timers for the motors PWM, pins to motor controller, and 
// reads the motor settings from non-volatile memory (user signature row)
void motor_init();

// Stops all motors
void stop();

// Walk in specified direction for specified number of steps
// direction (0-7, see #defines above for which direction maps to what number)
uint8_t move_steps(uint8_t direction, uint16_t num_steps);
// duration in milliseconds
uint8_t move_duration(uint8_t direction, uint16_t duration);

// Rotate in specified direction (1=clockwise, -1=counterclockwise)
uint8_t rotate_steps(int8_t direction, uint16_t num_steps);	// direction: 1 = CW, -1 = CCW
uint8_t rotate_duration(int8_t direction, uint16_t duration);

uint16_t cancel_rotate(void); // returns the number of steps taken (1 step = 3 sub-steps)
uint16_t cancel_move(void); // returns the number of steps taken (1 step = 2 sub-steps)

int8_t is_rotating(void); // returns 0 if droplet is not rotating, 1 if rotating cw, -1 if rotating ccw
uint8_t is_moving(void); // returns 0 if droplet is not moving, otherwise returns the direction of motion (1-6)

// Getter and setter for individual motor settings when moving in direction
int8_t	get_motor_duty_cycle(uint8_t motor_num, uint8_t direction);
void	set_motor_duty_cycle(uint8_t motor_num, uint8_t direction, int8_t duty_cycle);
void	read_motor_settings();
void	write_motor_settings();

void print_motor_settings();

void motor_set_period(uint8_t dir, uint16_t per);
void motor_set_duty_cycle(uint8_t dir, float duty_cycle); // Note: 0 <= duty_cycle <= 1

void motor_off(uint8_t num);
void motor_forward(uint8_t num);
void motor_backward(uint8_t num);

void take_step(void* arg);



#endif
