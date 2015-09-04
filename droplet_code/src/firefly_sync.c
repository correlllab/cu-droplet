#include "firefly_sync.h"


void firefly_sync_init()
{
	
	EVSYS.CH0MUX = EVSYS_CHMUX_PRESCALER_4096_gc;
	
	TCE0.CTRLA = TC_CLKSEL_EVCH0_gc;
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;

	TCE0.PER =  FFSYNC_FULL_PERIOD;
	TCE0.INTCTRLA = TC_OVFINTLVL_MED_gc;
	TCE0.CNT = 0;
}

ISR(TCE0_OVF_vect)
{
	int16_t change;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint16_t the_count = RTC.CNT;
		uint16_t remainder = the_count%FFSYNC_FULL_PERIOD_MS;
		//printf("Count: %u. Remainder: %u.\r\n", the_count, remainder);

		if(remainder>(FFSYNC_FULL_PERIOD_MS/2))
		{
			change = FFSYNC_FULL_PERIOD_MS-remainder;
			if((RTC.PER-change)<the_count) rtc_epoch++;			//0xFFFF: RTC.PER
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
	
	//change represents how the RTC clock's measure of 2048ms differs from the synchronization's measure.
	//If change is quite large, then probably we're still getting sync'd - so no implications about the RTC clock.
	//If it's smallish, though, the code below adjusts the factory-set calibration value to minimize this difference.
	//(From observations, changing the calibration by one seemed to effect the change by about 10ms, so if we're within
	//11ms, we won't get any better.)
	if(abs(change)<(FFSYNC_MAX_DEVIATION*10))
	{
		if(change>0) OSC.RC32KCAL++;
		else if(change<-FFSYNC_MAX_DEVIATION) OSC.RC32KCAL--;
	}
	hp_ir_cmd(ALL_DIRS, NULL, 0);
	//set_rgb(255,255,255);		
	//light_start = get_time();
	//printf("Delta Count: %d\r\n",change);

}
