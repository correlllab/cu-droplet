#ifndef FIREFLY_SYNC
#define FIREFLY_SYNC
#include "droplet_init.h"

#define REF_DELAY	        512   // ms
#define BASE_LISTEN_TIME    2048. // ms
#define TRANSMIT_DELAY      32    // ms

typedef enum
{
    SHINE,
    LISTEN,
    TRANSMIT
} State;
State state;

uint32_t state_start_time, last_update_time;
double listen_time;
double b, eps;

void change_state(State new_state);

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
uint8_t user_handle_command(char* command_word, char* command_args);

#endif