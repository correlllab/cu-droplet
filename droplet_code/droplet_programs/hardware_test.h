#ifndef HARDWARE_TEST
#define HARDWARE_TEST

#include "droplet_init.h"

typedef enum
{
	MIC_TEST,
	IR_IO_TEST,
	RGB_IO_TEST,
	DONE
} State;

State state;

uint32_t last_meas_time;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#endif
