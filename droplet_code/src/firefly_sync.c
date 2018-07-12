#include "firefly_sync.h"

static uint8_t ffsync_blink_r, ffsync_blink_g, ffsync_blink_b;
static uint8_t ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b;
static uint16_t ffsync_blink_dur;
static uint16_t ffsync_blink_phase_offset_ms;

static void updateRTC(void);

void setSyncBlinkColor(uint8_t r, uint8_t g, uint8_t b){
	ffsync_blink_r = r;
	ffsync_blink_g = g;
	ffsync_blink_b = b;
}

void setSyncBlinkDuration(uint16_t dur){
	ffsync_blink_dur = dur;
	if((TCE0.INTCTRLB & TC_CCAINTLVL_HI_gc) == TC_CCAINTLVL_HI_gc){
		TCE0.CCA = TCE0.CCB - (uint16_t)(ffsync_blink_dur*FFSYNC_MS_CONVERSION_FACTOR);
	}
}

void enableSyncBlink(uint16_t phase_offset){
	phase_offset = phase_offset%(((uint32_t)FFSYNC_FULL_PERIOD)/2);
	uint16_t turn_off_cc = FFSYNC_FULL_PERIOD - phase_offset;
	uint16_t turn_on_cc = turn_off_cc - (uint16_t)(ffsync_blink_dur*FFSYNC_MS_CONVERSION_FACTOR);
	TCE0.CCA = turn_on_cc;
	TCE0.CCB = turn_off_cc;
	TCE0.INTCTRLB = TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc;
}

uint8_t syncBlinkEnabled(){
	return (TCE0.INTCTRLB & (TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc)) == (TC_CCAINTLVL_HI_gc | TC_CCBINTLVL_HI_gc);
}

void disableSyncBlink(){
	TCE0.INTCTRLB = TC_CCAINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc;
	TCE0.CCA = 0;
	TCE0.CCB = 0;
}


ISR(TCE0_CCA_vect){
	ffsync_blink_prev_r = getRedLED();
	ffsync_blink_prev_g = getGreenLED();
	ffsync_blink_prev_b = getBlueLED();
	setRGB(ffsync_blink_r,ffsync_blink_g,ffsync_blink_b);
}

ISR(TCE0_CCB_vect){
	setRGB(ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b);	
}

void fireflySyncInit()
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
	
	obsStart = (ObsQueue*)myMalloc(sizeof(ObsQueue));
	obsStart->obs = 0;
	obsStart->next = obsStart;
	obsStart->prev = obsStart;
}

ISR(TCE0_OVF_vect){
	updateRTC();	
	uint32_t randomWait = (randShort()%32)*(FFSYNC_D/31); //between 0 and FFSYNC_D
	//sendPing( (void*)((uint16_t)(get_time()&0xFFFF)));
	scheduleTask(randomWait, (arg_func_t)sendPing, (void*)((uint16_t)(getTime()&0xFFFF)));
	//printf("ovf @ %lu\r\n",get_time());
}

void processObsQueue(){
	float newStart=0.0;
	ObsQueue* curr = obsStart->next;
	ObsQueue* tmp;
	while(curr != obsStart){
		newStart += calculatePhaseJump(curr->obs);
		tmp = curr;
		curr = curr->next;
		myFree(tmp);	
	}
	//printf("Processing @ %lu | newStart: %f\r\n",get_time(), newStart);	
	obsStart->next = obsStart;
	obsStart->prev = obsStart;
	uint16_t theCount = TCE0.CNT;
	
	if(theCount<TCE0.CCB&&(theCount+(uint16_t)newStart)>TCE0.CCB){
		setRGB(ffsync_blink_prev_r, ffsync_blink_prev_g, ffsync_blink_prev_b);	
	}
	
	if((theCount+(uint16_t)newStart)>=FFSYNC_FULL_PERIOD){
		//printf("\tOVERFLOW\r\n");
		TCE0.CNT = FFSYNC_FULL_PERIOD-1;
	}else{
		TCE0.CNT = theCount+(uint16_t)newStart;
	}
}

static void updateRTC(void){
	//This is being called when TCE0.CNT is 0 (because it gets called from the TCE0 overflow interrupt).
	int16_t change;
	uint16_t remainder;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint32_t currTime = getTime();
		uint16_t theCount = currTime&0xFFFF;		
		remainder = (int16_t)(currTime%FFSYNC_FULL_PERIOD_MS);
		//printf("%u.\r\n", remainder);
	
		if(remainder>(FFSYNC_FULL_PERIOD_MS/2)){
			change = FFSYNC_FULL_PERIOD_MS-((int16_t)remainder);
			if((RTC.PER-change)<theCount) rtc_epoch++;			//0xFFFF: RTC.PER
		}else{
			change = -(int16_t)remainder;
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
		else if(abs(change)<FFSYNC_MAX_DEVIATION) OSC.RC32KCAL--;
	}
	//printf("\t\t%d\r\n",change);
}

void sendPing(uint16_t val){
	/*uint8_t result = */hpIrTargetedCmd(ALL_DIRS, NULL, 64, val);
	//if(!result){
		//printf_P(PSTR("Unable to send ff_sync ping due to other hp ir activity.\r\n"));
	//}
	scheduleTask(FFSYNC_W, processObsQueue, NULL);
}
