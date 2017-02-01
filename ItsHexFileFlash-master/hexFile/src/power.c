#include "power.h"

void	user_leg_status_interrupt() __attribute__((weak));

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
	
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
	ACA.WINCTRL = AC_WEN_bm;					// Enable window mode
	
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN2_gc;	// PA0 is VREF_HI
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN2_gc;	// PA1 is VREF_LO
	
	ACA.AC0CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	ACA.AC1CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	
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
	ACA.WINCTRL |= AC_WINTMODE_INSIDE_gc | AES_INTLVL_LO_gc;
}

void disable_leg_status_interrupt()
{
	ACA.WINCTRL &= ~(AC_WINTMODE_INSIDE_gc | AC_WINTMODE_OUTSIDE_gc | AES_INTLVL_LO_gc);
}

void leg_float()
{
	//disable_leg_status_interrupt();
	if(leg_status(0) != 0 ||leg_status(1) != 0 || leg_status(2) != 0) return;
	printf("!\r\n");
	if(user_leg_status_interrupt) user_leg_status_interrupt();
	//enable_leg_status_interrupt();
}


ISR(ACA_ACW_vect)
{
	uint32_t now = get_time();
	if((now-last_int_time)>200)
	{	
		if(ACA.WINCTRL & 0x8)
		{
			printf("I'm down!");
			delay_ms(10);
			printf("\tLeg Status': %hd\t%hd\t%hd\r\n", leg_status(0),leg_status(1),leg_status(2));
			last_int_time=now;
			ACA.WINCTRL = AC_WEN_bm | AES_INTLVL_LO_gc | AC_WINTMODE_INSIDE_gc;
			remove_task(leg_task);				
			leg_task = schedule_task(500, leg_float, NULL);	
		}
		else
		{
			printf("I'm up!\t");
			delay_ms(10);			
			printf("\tLeg Status': %hd\t%hd\t%hd\r\n", leg_status(0),leg_status(1),leg_status(2));
			last_int_time=now;
			ACA.WINCTRL = AC_WEN_bm | AES_INTLVL_LO_gc | AC_WINTMODE_OUTSIDE_gc;
			remove_task(leg_task);
			leg_task = schedule_task(500, leg_float, NULL);
		}
	}
}

int8_t leg_status(uint8_t leg)
{
	volatile uint8_t status;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		switch (leg)
		{
			case 1:
				ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN3_gc;
				ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN3_gc;
				break;
			case 2:
				ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN4_gc;
				ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN4_gc;
				break;
		}
		if(leg!=0) delay_us(100);		//The comparators seem to need a bit of time.
		status = ACA.STATUS;
		if(leg!=0)//Set MUX back to leg 0.
		{
			ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN2_gc;
			ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN2_gc;
		}
	}
	switch (status & AC_WSTATE_gm)
	{
		case AC_WSTATE_ABOVE_gc:	return 1;
		case AC_WSTATE_INSIDE_gc:	return 0;
		case AC_WSTATE_BELOW_gc:	return -1;
		default:					return 127;
	}
}

uint8_t legs_powered()
{
	if ((leg_status(0) ==  1 || leg_status(1) ==  1 || leg_status(2) ==  1) &&
		(leg_status(0) == -1 || leg_status(1) == -1 || leg_status(2) == -1))
		return 1;
	return 0;
}