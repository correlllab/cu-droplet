#include "power.h"

void power_init()
{
	cap_monitor_init();
	leg_monitor_init();
}

void cap_monitor_init()
{
	PORTB.DIRCLR = PIN0_bm | PIN1_bm;
	
	ACB.AC0CTRL = AC_ENABLE_bm;
	ACB.AC1CTRL = AC_ENABLE_bm;
	
	ACB.AC0MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_PIN0_gc; //Changed the muxneg from bandgap
	ACB.AC1MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_PIN0_gc;
	
	ACB.WINCTRL = AC_WEN_bm;
}

void leg_monitor_init()
{
	PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
	
	ACA.AC0CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	ACA.AC1CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc;			// PA0 is VREF_HI
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc;			// PA1 is VREF_LO
	
	ACA.AC0MUXCTRL |= AC_MUXPOS_PIN2_gc;		// Initially, look at leg1
	ACA.AC1MUXCTRL |= AC_MUXPOS_PIN2_gc;
	
	ACA.WINCTRL = AC_WEN_bm;					// Enable window mode
}


uint8_t cap_status()
{
	switch (ACB.STATUS & AC_WSTATE_gm)
	{
		case AC_WSTATE_ABOVE_gc:  return 1;
		case AC_WSTATE_INSIDE_gc: return 0;
		case AC_WSTATE_BELOW_gc:  return -1;
	}
	return -2;
}

void enable_leg_status_interrupt()
{
	last_int_time=get_time();
	ACA.WINCTRL |= AC_WINTMODE_INSIDE_gc | 0x01;
}

void disable_leg_status_interrupt()
{
	ACA.WINCTRL &= ~(AC_WINTMODE_INSIDE_gc | AC_WINTMODE_OUTSIDE_gc);
}

ISR(ACA_ACW_vect)
{
	uint32_t now = get_time();
	if(ACA.WINCTRL & 0x8)
	{
		if((now-last_int_time)>200)
		{
			printf("I'm down!\r\n");
			last_int_time=now;
		}
		ACA.WINCTRL = AC_WEN_bm | 0x01 | AC_WINTMODE_INSIDE_gc;
	}
	else
	{
		if((now-last_int_time)>200)
		{
			printf("I'm up!\r\n");
			last_int_time=now;
		}
		ACA.WINCTRL = AC_WEN_bm | 0x01 | AC_WINTMODE_OUTSIDE_gc;
	}
}

int8_t leg_status(uint8_t leg)
{
	switch (leg)
	{
		case 0: 
			ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN2_gc;
			ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN2_gc;
			break;
		case 1:
			ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN3_gc;
			ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN3_gc;
			break;
		case 2:
			ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN4_gc;
			ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN4_gc;
			break;
	}
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
		return 127;
	
	//Set MUX back to leg 1.
	ACA.AC0MUXCTRL |= AC_MUXPOS_PIN2_gc;	
	ACA.AC1MUXCTRL |= AC_MUXPOS_PIN2_gc;		
}

uint8_t legs_powered()
{
	if ((leg_status(0) ==  1 || leg_status(1) ==  1 || leg_status(2) ==  1) &&
		(leg_status(0) == -1 || leg_status(1) == -1 || leg_status(2) == -1))
		return 1;
	return 0;
}