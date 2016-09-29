#include "firefly_sync.h"

void set_sync_blink_color(uint8_t r, uint8_t g, uint8_t b){
	ffsync_blink_r = r;
	ffsync_blink_g = g;
	ffsync_blink_b = b;
}

void set_sync_blink_duration(uint16_t dur){
	ffsync_blink_dur = dur;
	if((TCE0.INTCTRLB & TC_CCAINTLVL_HI_gc) == TC_CCAINTLVL_HI_gc){
		TCE0.CCA = TCE0.CCB - (uint16_t)(ffsync_blink_dur*FFSYNC_MS_CONVERSION_FACTOR);
	}
}

void enable_sync_blink(uint16_t phase_offset){
	phase_offset = phase_offset%(((uint32_t)FFSYNC_FULL_PERIOD)/2);
	uint16_t turn_off_cc = FFSYNC_FULL_PERIOD - phase_offset;
	uint16_t turn_on_cc = turn_off_cc - (uint16_t)(ffsync_blink_dur*FFSYNC_MS_CONVERSION_FACTOR);
	//uint16_t turn_off_cc = (turn_on_cc+((uint16_t)(ffsync_blink_dur*FFSYNC_MS_CONVERSION_FACTOR)))%FFSYNC_FULL_PERIOD;
	TCE0.CCA = turn_on_cc;
	TCE0.CCB = turn_off_cc;
	TCE0.INTCTRLB = TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc;
}

uint8_t sync_blink_enabled(){
	return (TCE0.INTCTRLB & (TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc)) == (TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc);
}

void disable_sync_blink(){
	TCE0.INTCTRLB = TC_CCAINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc;
	TCE0.CCA = 0;
	TCE0.CCB = 0;
}


ISR(TCE0_CCA_vect){
	ffsync_blink_prev_r = get_red_led();
	ffsync_blink_prev_g = get_green_led();
	ffsync_blink_prev_b = get_blue_led();
	set_rgb(ffsync_blink_r,ffsync_blink_g,ffsync_blink_b);
}

ISR(TCE0_CCB_vect){
	set_rgb(ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b);	
}

void firefly_sync_init()
{
	ffsync_blink_r = 255;
	ffsync_blink_g = 255;
	ffsync_blink_b = 255;
	ffsync_blink_dur = 200;
	
	ffsync_blink_phase_offset_ms = 0;

	EVSYS.CH0MUX = EVSYS_CHMUX_PRESCALER_4096_gc;
	
	TCE0.CTRLA = TC_CLKSEL_EVCH0_gc;
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;

	TCE0.PER =  FFSYNC_FULL_PERIOD;
	TCE0.INTCTRLA = TC_OVFINTLVL_HI_gc;
	TCE0.INTCTRLB = TC_CCAINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc;	
	TCE0.CNT = 0;
		TCE0.CCA = 0;
		TCE0.CCB = 0;
	
	obsStart = malloc(sizeof(obsQueue));
	obsStart->obs = 0;
	obsStart->next = obsStart;
	obsStart->prev = obsStart;
}

ISR(TCE0_OVF_vect){
	schedule_task(rand_short()%FFSYNC_D, sendPing, (void*)((uint16_t)(get_time()&0xFFFF)));
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
	
	if(theCount<TCE0.CCB&&(theCount+(uint16_t)newStart)>TCE0.CCB){
		set_rgb(ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b);	
	}
	
	if((theCount+(uint16_t)newStart)>=FFSYNC_FULL_PERIOD){
		//printf("\tOVERFLOW\r\n");
		TCE0.CNT = FFSYNC_FULL_PERIOD-1;
	}else{
		TCE0.CNT = theCount+(uint16_t)newStart;
	}
}

void updateRTC(){
	int16_t change;
	int16_t remainder;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint32_t currTime = get_time();
		uint16_t theCount = currTime&0xFFFF;		
		remainder = (int16_t)(currTime%FFSYNC_FULL_PERIOD_MS);
		//printf("%u.\r\n", remainder);
	
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
	//printf("!! %d !!\r\n", change);
	/*
	 * change represents how the RTC clock's measure of 2048ms differs from the synchronization's measure.
	 * If change is quite large, then probably we're still getting sync'd - so no implications about the RTC clock.
	 * If it's smallish, though, the code below adjusts the factory-set calibration value to minimize this difference.
	 * (From observations, changing the calibration by one seemed to effect the change by about 10ms, so if we're within
	 * 11ms, we won't get any better.)
	 */
	if(abs(change)<(FFSYNC_MAX_DEVIATION*5)){
		if(change>0) OSC.RC32KCAL++;
		else if(change<-FFSYNC_MAX_DEVIATION) OSC.RC32KCAL--;
	}
	//printf("\t\t%d\r\n",change);
}

void sendPing(void* val){
	//uint16_t diff = ((uint16_t)(get_time()&0xFFFF))-((uint16_t)val);
	uint8_t result = hp_ir_targeted_cmd(ALL_DIRS, NULL, 64, (uint16_t)val);
	if(!result){
		//printf_P(PSTR("sendPing blocked by other hp ir activity.\r\n"));
	}
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