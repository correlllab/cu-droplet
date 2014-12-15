#ifndef USER_TEMPLATE
#define USER_TEMPLATE

#include "droplet_init.h"

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

int16_t	tau;
double	theta;

#endif
