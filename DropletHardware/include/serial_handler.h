#ifndef _SERIAL_HANDER
#define _SERIAL_HANDER

#include <avr/io.h>
#include <string.h>
#include "droplet_init.h"
#include "pc_comm.h"
#include "rgb_led.h"
#include "range_algs.h"
#include "scheduler.h"

#include "main.h"

void handle_serial_command(char* command, uint16_t command_length);
void handle_check_collisions();
void handle_move_steps(char* command_args);
void handle_walk(char* command_args);
void handle_set_tau(char* command_args);
void handle_set_theta(char* command_args);
void handle_stop_walk();
void handle_set_motors(char* command_args);
void handle_set_mm_per_kilostep(char* command_args);
void handle_rnb_broadcast();
void handle_rnb_collect(char* command_args);
void collect_rnb_data_wrapper(void* arg);
void handle_rnb_transmit(char* command_args);
void handle_rnb_receive();
void handle_set_led(char* command_args);
void handle_broadcast_id();
void handle_get_id();
void send_id();
void handle_cmd(char* command_args, uint8_t should_broadcast);
void handle_targeted_cmd(char* command_args);
void handle_shout(char* command_args);
void handle_target(char* command_args);
void handle_reset();
void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args);


#endif