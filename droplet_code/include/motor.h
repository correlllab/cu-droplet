#pragma once
#include "droplet_base.h"
#include "scheduler.h"
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


int16_t motorAdjusts[8][3];
uint16_t mmPerKilostep[8]; //For the spin directions, this is degrees per kilostep.

// Sets up the timers for the motors PWM, pins to motor controller, and 
// reads the motor settings from non-volatile memory (user signature row)
void	motorInit(void);

// Walk in specified direction for specified number of steps
// direction (0-7, see #defines above for which direction maps to what number)
uint8_t	moveSteps(uint8_t direction, uint16_t num_steps);

void walk(uint8_t direction, uint16_t mm);

// Stops all motors
void stopMove(void);

int8_t isMoving(void); // returns -1 if droplet is not moving, movement dir otherwise.

void		readMotorSettings(void);
void		writeMotorSettings(void);
void		printMotorValues(void);
void		broadcastMotorAdjusts(void);
void		printDistPerStep(void);
void		broadcastDistPerStep(void);
uint16_t	getMMperKilostep(uint8_t direction);
void		setMMperKilostep(uint8_t direction, uint16_t dist);