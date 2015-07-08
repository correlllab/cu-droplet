#include "firefly_sync.h"


void firefly_sync_init()
{
	
	TCE0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;
	TCE0.CNT = 0;
	TCE0.PER =  FIREFLY_SYNC_FULL_PERIOD;
	TCE0.INTCTRLA = TC_OVFINTLVL_MED_gc;
	TCE0.CCA = FIREFLY_SYNC_WAIT_TIME;

}

ISR(TCE0_OVF_vect)
{
	ir_cmd(ALL_DIRS, NULL, 0);
	int16_t change;	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint16_t the_count = RTC.CNT;
		uint16_t remainder = the_count%2048;
		//printf("Count: %u. Remainder: %u.\r\n", the_count, remainder);

		if(remainder>1024)
		{
			change = 2048-remainder;
			if((0xFFFF-change)<the_count) rtc_epoch++;
		}
		else
		{
			change = -remainder;
			if(the_count<remainder) rtc_epoch--;
		}				

	
		while(RTC.STATUS & RTC_SYNCBUSY_bm);
		RTC.CNT =  (the_count+change);
		RTC.COMP = (RTC.COMP+change);
	}
	

	if(abs(change)<100)
	{
		if(change>0) OSC.RC32KCAL++;
		else if(change<-11) OSC.RC32KCAL--;
	}

	printf("Delta Count: %d\r\n",change);
}

