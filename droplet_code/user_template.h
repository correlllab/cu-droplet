#ifndef USER_TEMPLATE
#define USER_TEMPLATE

#include "droplet_init.h"

uint32_t last_meas_time;
uint16_t last_r,last_g,last_b,last_c;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#endif
