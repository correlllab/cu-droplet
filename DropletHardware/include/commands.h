#define F_CPU 32000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include <stdio.h>

#include "IRcom.h"
//#include "boot.h"
#include "motor.h"
#include "RGB_LED.h"



#define COMMAND_QUIET_bm                0x01
#define COMMAND_STOP_bm                 0x02
#define COMMAND_DARK_bm                 0x04

uint8_t pc_command;
//extern USART_t* channel[];

void handle_command(uint8_t *buf);

// command_program() located in boot.h and boot.c
uint8_t command_settings(uint8_t *buf);
uint8_t command_settings_motors(uint8_t *buf);
uint8_t command_settings_ID(uint8_t *buf);
uint8_t command_settings_read(uint8_t *buf);

uint8_t command_stop(uint8_t *buf);
uint8_t command_unstop(uint8_t *buf);
uint8_t command_quiet(uint8_t *buf);
uint8_t command_unquiet(uint8_t *buf);
uint8_t command_dark(uint8_t *buf);
uint8_t command_undark(uint8_t *buf);

uint8_t command_motor_setup(uint8_t *buf);
uint8_t command_motor_run(uint8_t *buf);

uint8_t command_light(uint8_t *buf);