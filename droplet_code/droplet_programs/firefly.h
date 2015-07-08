#ifndef FIREFLY_SYNC
#define FIREFLY_SYNC

#include "droplet_init.h"

#define REFRACTORY_PERIOD	15830	//~(1/32)ms
#define FULL_PERIOD			63323	//~(1/32)ms

float b, eps;


void init();
void loop();
void handle_msg(ir_msg* msg_struct);
uint8_t user_handle_command(char* command_word, char* command_args);

#endif