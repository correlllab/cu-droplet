#include "main.h"

volatile double a=0.0;

int main(void)
{
	init_all_systems();
	intmax_t int_result;
	float float_result;
	intmax_t int_a;
	printf("Initialized.\r\n");
	print_shit();
	a=0.0;
    while(1)
    {	
    }
}

void print_shit(){
	float float_result = exp(a);
	printf("a: %f, e^a: %f, red_sense: %hhu\r\n", a, float_result, get_red_sensor());
	a+=0.1;
	if(a>5){
		a=0.0;
	}	
	schedule_task(100, print_shit,NULL);
}

