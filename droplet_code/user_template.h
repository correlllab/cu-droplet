#ifndef USER_TEMPLATE
#define USER_TEMPLATE

#include "droplet_init.h"

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

void task_one();
void task_two();

#endif
