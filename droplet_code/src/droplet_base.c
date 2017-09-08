#include "droplet_base.h"

void delayMS(uint16_t ms){
	uint32_t cur_time, end_time;
	cli(); cur_time = getTime(); sei();
	end_time = cur_time + ms;
	while (1){
		cli();
		if (getTime() >= end_time){
			sei();
			return;
		}
		sei();
		delay_us(10);
	}
}
