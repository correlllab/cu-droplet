#include "power.h"

void cap_monitor_init()
{
	PORTB.DIRCLR = PIN0_bm | PIN1_bm;
	
	ACB.AC0CTRL = AC_ENABLE_bm;
	ACB.AC1CTRL = AC_ENABLE_bm;
	
	ACB.AC0MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_BANDGAP_gc;
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

int8_t leg_status(uint8_t leg)
{
	switch (leg)
	{
		case 1:
		return leg1_status();
		case 2:
		return leg2_status();
		case 3:
		return leg3_status();
	}
	return 0x80;
}

int8_t leg1_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN2_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN2_gc;
	
	//delay_ms(1);
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t leg2_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN3_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN3_gc;

	//delay_ms(1);
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t leg3_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN4_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN4_gc;

	//delay_ms(1);
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t legs_powered()
{
	if ((leg1_status() == 1 || leg2_status() == 1 || leg3_status() == 1) &&
		(leg1_status() == -1 || leg2_status() == -1 || leg3_status() == -1))
		return 1;
	return 0;
}

uint8_t light_if_unpowered(uint8_t r, uint8_t g, uint8_t b)
{
	if (!legs_powered())
	set_rgb(r,g,b);
	else set_rgb(0,0,0);
}