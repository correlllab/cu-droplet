#include "droplet_init.h"

void init()
{
	
}

void loop()
{
	int8_t r, g, b;
	get_rgb_sensors(&r, &g, &b);
	printf("R=%hd, G=%hd, B=%hd\n", r, g, b);
	delay_ms(500);
}

void handle_msg(ir_msg* msg_struct)
{

}
