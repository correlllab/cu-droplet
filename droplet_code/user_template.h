#pragma once

#include "droplet_init.h"


typedef struct motor_settings_msg_struct{
	int16_t settings[4][3];
	uint8_t flag;
}MotorSettingsMsg;

void		init();
void		loop();
void		handle_msg			(ir_msg* msg_struct);
//void		send_motor_settings();
void		send_motor_settings_two_A();
void		send_motor_settings_two_B();
void move_to_closest();
//char *custom_itoa(int value);
//void custom_atoi(char*);
void edge_following(int16_t, int16_t);
void follow_edge_withrnb(int dir);

void check_ir_coll_max(uint8_t* input_dir, int16_t* input_val);
void check_if_motor_calibrated();
//void handle_motor_settings_msg(ir_msg* msg_struct);
void handle_motor_settings_msg_two(int16_t settings[4][3], uint8_t whichSet);