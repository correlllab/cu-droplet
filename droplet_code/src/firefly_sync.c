#include "firefly_sync.h"


void set_sync_blink_default(uint8_t r, uint8_t g, uint8_t b){
	sync_def_r = r;
	sync_def_g = g;
	sync_def_b = b;
}

void set_sync_blink_color(uint8_t r, uint8_t g, uint8_t b){
	sync_blink_r = r;
	sync_blink_g = g;
	sync_blink_b = b;
}

void set_sync_blink_duration(uint16_t dur){
	sync_blink_duration = dur;
	TCE0.CCB = (TCE0.CCA+((uint16_t)(sync_blink_duration*FFSYNC_MS_CONVERSION_FACTOR)))%FFSYNC_FULL_PERIOD;
}
void enable_sync_blink(uint16_t phase_offset_ms){
	uint16_t turn_on_cc = (((uint16_t)(phase_offset_ms*FFSYNC_MS_CONVERSION_FACTOR))%FFSYNC_FULL_PERIOD);
	uint16_t turn_off_cc = (turn_on_cc+((uint16_t)(sync_blink_duration*FFSYNC_MS_CONVERSION_FACTOR)))%FFSYNC_FULL_PERIOD;
	TCE0.CCA = turn_on_cc;
	TCE0.CCB = turn_off_cc;
	TCE0.INTCTRLB = TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc;
}

void disable_sync_blink(){
	TCE0.INTCTRLB = TC_CCAINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc;
	TCE0.CCA = 0;
	TCE0.CCB = 0;
}

ISR(TCE0_CCA_vect){
	set_rgb(sync_blink_r,sync_blink_g,sync_blink_b);
}

ISR(TCE0_CCB_vect){
	set_rgb(sync_def_r, sync_def_g, sync_def_b);	
}

void firefly_sync_init()
{
	sync_blink_r = 255;
	sync_blink_g = 255;
	sync_blink_b = 255;
	sync_def_r = 0;
	sync_def_g = 0;
	sync_def_b = 0;
	
	sync_blink_duration = 200;
	
	disable_sync_blink();
	EVSYS.CH0MUX = EVSYS_CHMUX_PRESCALER_4096_gc;
	
	TCE0.CTRLA = TC_CLKSEL_EVCH0_gc;
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;

	TCE0.PER =  FFSYNC_FULL_PERIOD;
	TCE0.INTCTRLA = TC_OVFINTLVL_MED_gc;
	TCE0.CNT = 0;
	
	obsStart = malloc(sizeof(obsQueue));
	obsStart->obs = 0;
	obsStart->next = obsStart;
	obsStart->prev = obsStart;
}

ISR(TCE0_OVF_vect){
	schedule_task(rand_short()%100, sendPing, (void*)((uint16_t)(get_time()&0xFFFF)));
	//sendPing( (void*)((uint16_t)(get_time()&0xFFFF)));
	updateRTC();
	//printf("ovf @ %lu\r\n",get_time());
}

void processObsQueue(){
	float newStart=0.0;
	obsQueue* curr = obsStart->next;
	obsQueue* tmp;
	while(curr != obsStart){
		newStart += (((float)(curr->obs))-newStart)/FFSYNC_EPSILON;
		tmp = curr;
		curr = curr->next;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			free(tmp);	
		}
	}
	//printf("Processing @ %lu | newStart: %f\r\n",get_time(), newStart);	
	obsStart->next = obsStart;
	obsStart->prev = obsStart;
	uint16_t theCount = TCE0.CNT;
	
	if((theCount+(uint16_t)newStart)>=FFSYNC_FULL_PERIOD){
		//printf("\tOVERFLOW\r\n");
		TCE0.CNT = FFSYNC_FULL_PERIOD-1;
	}else{
		TCE0.CNT = theCount+(uint16_t)newStart;
	}
}

void updateRTC(){
	int16_t change;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint32_t currTime = get_time();
		uint16_t theCount = currTime&0xFFFF;		
		int16_t remainder = (int16_t)(currTime%FFSYNC_FULL_PERIOD_MS);
		//printf("Count: %u. Remainder: %u.\r\n", the_count, remainder);
	
		if(remainder>(FFSYNC_FULL_PERIOD_MS/2)){
			change = FFSYNC_FULL_PERIOD_MS-remainder;
			if((RTC.PER-change)<theCount) rtc_epoch++;			//0xFFFF: RTC.PER
		}else{
			change = -remainder;
			if(theCount<remainder) rtc_epoch--;
		}
		while(RTC.STATUS & RTC_SYNCBUSY_bm);
		RTC.CNT =  (theCount+change);
		RTC.COMP = (RTC.COMP+change);
	}	
	
	///*
	 //* change represents how the RTC clock's measure of 2048ms differs from the synchronization's measure.
	 //* If change is quite large, then probably we're still getting sync'd - so no implications about the RTC clock.
	 //* If it's smallish, though, the code below adjusts the factory-set calibration value to minimize this difference.
	 //* (From observations, changing the calibration by one seemed to effect the change by about 10ms, so if we're within
	 //* 11ms, we won't get any better.)
	 //*/
	//if(abs(change)<(FFSYNC_MAX_DEVIATION*5)){
		//if(change>0) OSC.RC32KCAL++;
		//else if(change<-FFSYNC_MAX_DEVIATION) OSC.RC32KCAL--;
	//}
	////printf("\t\t%d\r\n",change);
}

void sendPing(void* val){
	uint16_t diff = ((uint16_t)(get_time()&0xFFFF))-((uint16_t)val);
	hp_ir_targeted_cmd(ALL_DIRS, NULL, 0, (uint16_t)val);
	schedule_task(FFSYNC_W, processObsQueue, NULL);
	//if(diff>(FFSYNC_W-5)){
		//processObsQueue();
	//}else{
		//schedule_task(200-diff, processObsQueue, NULL);
	//}
}

//ISR(TCE0_OVF_vect)
//{
	//int16_t change;
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	//{
		//uint16_t the_count = RTC.CNT;
		//int16_t remainder = (int16_t)(the_count%FFSYNC_FULL_PERIOD_MS);
		////printf("Count: %u. Remainder: %u.\r\n", the_count, remainder);
//
		//if(remainder>(FFSYNC_FULL_PERIOD_MS/2))
		//{
			//change = FFSYNC_FULL_PERIOD_MS-remainder;
			//if((RTC.PER-change)<the_count) rtc_epoch++;			//0xFFFF: RTC.PER
		//}
		//else
		//{
			//change = -remainder;
			//if(the_count<remainder) rtc_epoch--;
		//}
//
		//while(RTC.STATUS & RTC_SYNCBUSY_bm);
		//RTC.CNT =  (the_count+change);
		//RTC.COMP = (RTC.COMP+change);
	//}
//
	//
	//////change represents how the RTC clock's measure of 2048ms differs from the synchronization's measure.
	//////If change is quite large, then probably we're still getting sync'd - so no implications about the RTC clock.
	//////If it's smallish, though, the code below adjusts the factory-set calibration value to minimize this difference.
	//////(From observations, changing the calibration by one seemed to effect the change by about 10ms, so if we're within
	//////11ms, we won't get any better.)
	////if(abs(change)<(FFSYNC_MAX_DEVIATION*10))
	////{
		////if(change>0) OSC.RC32KCAL++;
		////else if(change<-FFSYNC_MAX_DEVIATION) OSC.RC32KCAL--;
	////}
	//////printf("\t\t%d\r\n",change);
//}