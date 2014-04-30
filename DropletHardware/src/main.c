#include "main.h"
volatile double a=0.0;

int main(void)
{
	init_all_systems();
	
	intmax_t int_result;
	float float_result;
	intmax_t int_a;
	printf("\n\n\n\r");
	while(1)
	{
		float_result = exp(a);
		int_result = (intmax_t)(float_result*10.0);
		int_a = (intmax_t)(a*10.0);
		printf("int_result: %ld, ",int_result);
		printf("int_a: %ld\r\n",int_a);
		printf("result: %f\r\n", float_result);
		_delay_us(100000);
		a+=0.1;
		if(a>5){
			a=0.0;
		}
	}
}