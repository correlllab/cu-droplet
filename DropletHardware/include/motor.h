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
#define MOTOR_ON_TIME			30
#define MOTOR_OFF_TIME			20

#include "droplet_init.h"
#include "scheduler.h"

volatile uint8_t motor_status; // [ on, cancel, 0, 0, 0, direction(2-0) ] 
volatile Task_t* current_motor_task;
uint8_t motor_flipped;

#define MOTOR_0_FLIPPED_bm	0x01	// 0000 0001
#define MOTOR_1_FLIPPED_bm	0x02
#define MOTOR_2_FLIPPED_bm	0x04

int16_t motor_on_time;
int16_t motor_off_time;


/*
* <0 if motor is going backwards, =0 if motor isn't spinning, >0 if motor is going forwards
* This should be set by whatever function causes the motors to run.
*/
int8_t current_motor_direction[3];
volatile uint8_t current_motor_num; //The motor currently spinning, if any.
volatile uint16_t current_step_num; //total number of steps taken.

/*
 * motor_adjusts[mot][backward] is how much we adjust motor mot by when going 0: forward, 1: backward.
 * changing motor_adjusts[mot][backward] by 1 will cause the motor to spin for an extra 32 microseconds. Wooo.
 */
int16_t motor_adjusts[3][2]; 
int8_t motor_signs[8][3];

uint16_t mm_per_kilostep[8];


// Sets up the timers for the motors PWM, pins to motor controller, and 
// reads the motor settings from non-volatile memory (user signature row)
void	motor_init();

uint8_t take_steps_two(uint8_t motor_num, uint16_t duty_cycle, int16_t num_steps);

// Walk in specified direction for specified number of steps
// direction (0-7, see #defines above for which direction maps to what number)
uint8_t	move_steps(uint8_t direction, uint16_t num_steps);
uint8_t move_steps_two(uint8_t direction, uint16_t num_steps);
uint8_t take_steps(uint8_t motor_num, int16_t num_steps);
//void	take_step(void* arg);

void walk(uint8_t direction, uint16_t mm);

// Stops all motors
void stop();

uint8_t is_moving(void); // returns 0 if droplet is not moving, otherwise returns the direction of motion (1-6)

// Getter and setter for individual motor settings when moving in direction
uint16_t	get_mm_per_kilostep(uint8_t direction);
void		set_mm_per_kilostep(uint8_t direction, uint16_t dist);
void		read_motor_settings();
void		write_motor_settings();
void		print_motor_adjusts();
void		broadcast_motor_adjusts();
void		print_dist_per_step();
void		broadcast_dist_per_step();

//void motor_forward(uint8_t num);
//void motor_backward(uint8_t num);

static inline void motor_forward(uint16_t num)
{
	uint16_t val = (0x0800+((0x1&num)<<6)+((0x2&num)<<8));
	((*(TC0_t *) val)).CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;
}

static inline void motor_backward(uint16_t num)
{
	uint16_t val = (0x0800+((0x1&num)<<6)+((0x2&num)<<8));
	(*(TC0_t *) val).CTRLB = TC_WGMODE_SS_gc | TC0_CCBEN_bm;
}

uint16_t get_mm_per_kilostep(uint8_t direction);
void set_mm_per_kilostep(uint8_t direction, uint16_t dist);

#endif
