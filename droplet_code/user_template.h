#ifndef USER_TEMPLATE
#define USER_TEMPLATE

#include "droplet_init.h"

uint32_t last_meas_time;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#endif
