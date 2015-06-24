#include "ir_led.h"

USART_t* channel[] = {
	&USARTC0,  //   -- Channel 0
	&USARTC1,  //   -- Channel 1
	&USARTD0,  //   -- Channel 2
	&USARTE0,  //   -- Channel 3
	&USARTE1,  //   -- Channel 4
	&USARTF0   //   -- Channel 5
};

void ir_led_init()
{
	/* Initialize carrier waves */
	PORTF.DIRSET = ALL_EMITTERS_CARWAV_bm;
	
	TCF2.CTRLE = TC_BYTEM_SPLITMODE_gc;		// "split mode" puts this timer counter into "Type 2 mode"
	TCF2.CTRLA |= TC_CLKSEL_DIV4_gc;		// see CTRLA description in TC2 mode
	
	TCF2.HPER = 211; TCF2.LPER = 211; // 32MHz / (4 * 211) = 38kHz
	TCF2.HCMPA = 105; TCF2.HCMPB = 105; TCF2.HCMPC = 105; // 50% Duty Cycle
	TCF2.HCMPD = 105; TCF2.LCMPA = 105; TCF2.LCMPB = 105; // 50% Duty Cycle
	TCF2.CTRLB = ALL_EMITTERS_CARWAV_bm;	// Set TC outputs on carrier wave pins (see CTRLA description in TC2 mode)

	// TX pins as outputs:
	PORTC.DIRSET = PIN3_bm | PIN7_bm;		// DIR 0,1
	PORTD.DIRSET = PIN3_bm;					// DIR 2
	PORTE.DIRSET = PIN3_bm | PIN7_bm;		// DIR 3,4
	PORTF.DIRSET = PIN3_bm;					// DIR 5
	
	PORTC.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTD.OUTCLR = PIN3_bm;
	PORTE.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTF.OUTCLR = PIN3_bm;
		
	// Invert the output pins:
	PORTC.PIN3CTRL = PORT_INVEN_bm;
	PORTC.PIN7CTRL = PORT_INVEN_bm;
	PORTD.PIN3CTRL = PORT_INVEN_bm;
	PORTE.PIN3CTRL = PORT_INVEN_bm;
	PORTE.PIN7CTRL = PORT_INVEN_bm;
	PORTF.PIN3CTRL = PORT_INVEN_bm;
	
	set_all_ir_powers(256);
}

void set_all_ir_powers(uint16_t power)
{
	if(power>256) return;
	if(curr_ir_power==power) return;
	curr_ir_power = power;
	uint8_t power_high = (power>>8);
	uint8_t power_low = (power&0xFF);
	volatile uint8_t write_buffer[6] = {0x00|power_high,power_low,0x10|power_high,power_low,0x60|power_high, power_low};
		
	uint8_t result;
	result = TWI_MasterWrite(IR_POWER_ADDR_A, write_buffer, 6);
	if(!result) printf("First IR_POWER setting failed.\r\n");
	delay_ms(5);
	result = TWI_MasterWrite(IR_POWER_ADDR_B, write_buffer, 6);
	if(!result) printf("Second IR_POWER setting failed.\r\n");
}