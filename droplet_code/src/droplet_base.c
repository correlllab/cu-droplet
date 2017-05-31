#include "droplet_base.h"

void delay_ms(uint16_t ms){
	uint32_t cur_time, end_time;
	cli(); cur_time = get_time(); sei();
	end_time = cur_time + ms;
	while (1){
		cli();
		if (get_time() >= end_time){
			sei();
			return;
		}
		sei();
		delay_us(10);
	}
}
