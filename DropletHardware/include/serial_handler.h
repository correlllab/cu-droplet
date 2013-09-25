#ifndef _SERIAL_HANDER
#define _SERIAL_HANDER

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "pc_com.h"
#include "RGB_LED.h"
#include "motor.h"
#include "Range_Algorithms.h"

void handle_set_led(char* command_args);
void handle_walk(char* command_args);
void handle_rnb_broadcast();
void hand_rnb_collect();
void handle_rnb_transmit();
void handle_rnb_receive();
void handle_spin(char* command_args);
void handle_set_motor(char* command_args);
void handle_ir_test(char* command_args);
void handle_cmd(char* command_args);
void handle_serial_command(char* command, uint16_t command_length);
void handle_data(char *command_args);
void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args);

void hsv_to_rgb(float h, float s, float v, uint8_t *rgb);

uint32_t last_serial_command_time;

#endif