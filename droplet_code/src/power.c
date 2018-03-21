#include "power.h"

void powerInit()
{
	capMonitorInit();
	legMonitorInit();
}

void capMonitorInit(){
	PORTB.DIRCLR = PIN0_bm | PIN1_bm;

	PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;

	ACB.CTRLB = 14; //This sets the scaling of Vcc used in the comparison, to roughly (ACB.CTRLB+1)*(0.26744 Volts)
	ACB.AC0MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_SCALER_gc; //this sets the (-) part of the comparison to be a scaled value of Vcc (3.3V)

	ACB.AC0CTRL = AC_ENABLE_bm;
	#ifdef FIX_UNPOWERED_STATE
		unpoweredFixActive = 0;
		scheduleTask(1000, enableLowPowerInterrupt, NULL);
	#endif
	delay_us(1);
}

void enableLowPowerInterrupt(){
	ACB.AC0CTRL |= AC_INTMODE_BOTHEDGES_gc | AC_INTLVL_MED_gc | AC_HYSMODE_LARGE_gc;
}

void disableLowPowerInterrupt(){
	ACB.AC0CTRL &= ~(AC_INTMODE_BOTHEDGES_gc | AC_INTLVL_MED_gc | AC_HYSMODE_LARGE_gc);
}

void legMonitorInit(){
	PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
	
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
	ACA.WINCTRL = AC_WEN_bm;					// Enable window mode
	
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc; //VREF_HI
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc; //VREF_LO
	
	ACA.AC0CTRL = AC_HSMODE_bm | AC_ENABLE_bm; //The comparators take up to 500ns in low-power mode and up to 100ns in high-power mode. Since 500ns is plenty fast for me, I'm leaving them in low-power mode.
	ACA.AC1CTRL = AC_HSMODE_bm | AC_ENABLE_bm;	//The comparators take up to 500ns in low-power mode and up to 100ns in high-power mode. Since 500ns is plenty fast for me, I'm leaving them in low-power mode.
	delay_us(1);
	
}

//Returns '1' if cap is powered, and '0' otherwise.
uint8_t capStatus(){
	
	return ((ACB.STATUS & AC_AC0STATE_bm)>>4);
/*
	switch (ACB.STATUS & AC_WSTATE_gm)
	{
		case AC_WSTATE_ABOVE_gc:  return 1;
		case AC_WSTATE_INSIDE_gc: return 0;
		case AC_WSTATE_BELOW_gc:  return -1;
	}
	return -2;
	*/
}

int8_t legStatus(uint8_t leg){
	uint8_t status;
	uint8_t muxPinMask;
	switch(leg){
		case 0: muxPinMask = AC_MUXPOS_PIN2_gc; break;
		case 1: muxPinMask = AC_MUXPOS_PIN3_gc; break;
		case 2: muxPinMask = AC_MUXPOS_PIN4_gc; break;
		default: muxPinMask = AC_MUXPOS_PIN0_gc; //should never happen
	}
	ACA.AC0MUXCTRL |= muxPinMask;
	ACA.AC1MUXCTRL |= muxPinMask;
	busy_delay_ns(150); //In high-power mode, the comparators take up to 110ns to update after changing the inputs.
	//busy_delay_ns(500); //In low-power mode, the comparators take up to 500ns to update after changing the inputs.
	status = ACA.STATUS;
	ACA.AC0MUXCTRL &= AC_MUXPOS_PIN0_gc;
	ACA.AC1MUXCTRL &= AC_MUXPOS_PIN0_gc;
	switch (status & AC_WSTATE_gm){
		case AC_WSTATE_ABOVE_gc:	return 1;
		case AC_WSTATE_INSIDE_gc:	return 0;
		case AC_WSTATE_BELOW_gc:	return -1;
		default:					return 127;
	}
}

uint8_t legsPowered(){
	int8_t leg0status = legStatus(0);
	int8_t leg1status = legStatus(1);
	int8_t leg2status = legStatus(2);
	uint8_t someLegsHigh = (leg0status==1) || (leg1status==1) || (leg2status==1);
	uint8_t someLegsLow = (leg0status==-1) || (leg1status==-1) || (leg2status==-1);
	return (someLegsHigh && someLegsLow);
}

uint8_t legsFloating(){
	int8_t leg0status = legStatus(0);
	int8_t leg1status = legStatus(1);
	int8_t leg2status = legStatus(2);
	return !(leg0status || leg1status || leg2status);
}

#ifdef FIX_UNPOWERED_STATE

void checkLegsTask(){
	if(!unpoweredFixActive){
		return;
	}
	if(legsPowered()){
		unpoweredFixActive = 0; //Nothing left to do.
	}else if(legsFloating()){
		unpoweredFixActive = 0; //Problem isn't resolved, but it seems like we simply aren't on the floor at all anymore.
	}else{
		//Lets move the Droplet to try and fix things.
		if(failedLegChecks < 4){
			//At first, we're going to use the calibrated motor values:
			moveSteps(6,50);
		}else if(failedLegChecks < 8){ 
			//Assuming at this point that this Droplet hasn't been calibrated, so we're using progressively 
			//larger and flipped guesses at calibrated motor values.
			int16_t adj[3];
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				adj[0] = motorAdjusts[6][0];
				adj[1] = motorAdjusts[6][1];
				adj[2] = motorAdjusts[6][2];
				//The equation below should cause the temporary motor adjustment values to alternate between positive and negative, and increase by 500 if that doesn't work.
				motorAdjusts[6][0] = ((int16_t)pow(-1, failedLegChecks-5))*(1000 + (((failedLegChecks - 5)/2)*500));
				motorAdjusts[6][1] = ((int16_t)pow(-1, failedLegChecks-5))*(1000 + (((failedLegChecks - 5)/2)*500));
				motorAdjusts[6][2] = ((int16_t)pow(-1, failedLegChecks-5))*(1000 + (((failedLegChecks - 5)/2)*500));
			}
			moveSteps(6,50);
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				motorAdjusts[6][0] = adj[0];
				motorAdjusts[6][1] = adj[1];
				motorAdjusts[6][2] = adj[2];
			}
		}else{
			unpoweredFixActive = 0;
		}
		scheduleTask(150, stopLowPowerMoveTask, NULL);
		failedLegChecks++;
	}
}

void stopLowPowerMoveTask(){
	stopMove();
	if(!unpoweredFixActive){
		return;
	}	
	scheduleTask(50, checkLegsTask, NULL);
}

inline static void capFallingEdgeISR(void){
	if(!unpoweredFixActive){
		if(!(legsPowered() || legsFloating())){
			unpoweredFixActive = 1;
			moveSteps(6,50);
			failedLegChecks = 0;
			scheduleTask(150, stopLowPowerMoveTask, NULL);
		}
	}
}

inline static void capRisingEdgeISR(void){
	if(unpoweredFixActive){
		stopMove();
		failedLegChecks = 0;
		unpoweredFixActive = 0;
	}
}

ISR( ACB_AC0_vect ){
	//Triggers on both rising and falling edge, so check the current status to know what we have
	if(capStatus()){
		capRisingEdgeISR();
	}else{
		capFallingEdgeISR();
	}
}
#endif