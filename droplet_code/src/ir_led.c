#include "ir_led.h"

USART_t* channel[6] = {
	&USARTC0,  //   -- Channel 0
	&USARTC1,  //   -- Channel 1
	&USARTD0,  //   -- Channel 2
	&USARTE0,  //   -- Channel 3
	&USARTE1,  //   -- Channel 4
	&USARTF0   //   -- Channel 5
};

static uint8_t carrier_wave_pins[6] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN7_bm, PIN6_bm};
static uint8_t tx_pins[6] = {PIN3_bm, PIN7_bm, PIN3_bm, PIN3_bm, PIN7_bm, PIN3_bm};
static PORT_t* uart_ch[6] = {&PORTC, &PORTC, &PORTD, &PORTE, &PORTE, &PORTF};
static uint8_t saved_usart_ctrlb_vals[6] = {0,0,0,0,0,0};

void irLedInit()
{
	/* Initialize carrier waves */
	PORTF.DIRSET = ALL_EMITTERS_CARWAV_bm;
	
	TCF2.CTRLE = TC2_BYTEM_SPLITMODE_gc;		// "split mode" puts this timer counter into "Type 2 mode"
	TCF2.CTRLA |= TC2_CLKSEL_DIV4_gc;		// see CTRLA description in TC2 mode
	
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
	
}

void irLedOn(uint8_t direction)
{
	//Turning on the light.
	saved_usart_ctrlb_vals[direction]	  = channel[direction]->CTRLB;	// record the current state of the USART
	TCF2.CTRLB			 &= ~carrier_wave_pins[direction];	// disable carrier wave output
	PORTF.DIRSET		  =  carrier_wave_pins[direction];	// enable user output on this pin
	PORTF.OUT			 |=  carrier_wave_pins[direction];	// high signal on this pin
	channel[direction]->CTRLB  	  =  0;					// disable USART
	uart_ch[direction]->DIRSET =  tx_pins[direction];			// enable user output on this pin
	uart_ch[direction]->OUT	 &= ~tx_pins[direction];			// low signal on TX pin (remember: these pins were inverted during init)

}

void irLedOff(uint8_t direction)
{
	//Turning off the light.
	uart_ch[direction]->OUT  |=  tx_pins[direction];			// high signal on TX pin (turns IR blast OFF)
	channel[direction]->CTRLB	 =  saved_usart_ctrlb_vals[direction];	// re-enable USART (restore settings as it was before)
	PORTF.OUT			&= ~carrier_wave_pins[direction];	// low signal on the carrier wave pin, just in casies.
	TCF2.CTRLB			|=  carrier_wave_pins[direction];	// re-enable carrier wave output
}

void setAllirPowers(volatile uint16_t power){
	if(power>256) return;
	if(curr_ir_power==power) return;
	uint8_t power_high = (power>>8);
	uint8_t power_low = (power&0xFF);
	uint8_t write_buffer[6] = {power_high,power_low,0x10|power_high,power_low,0x60|power_high, power_low};
	
	uint8_t aResult = 0;
	uint8_t bResult = 0;
	char callerDescr[7] = "Set IR\0";
	aResult = twiWriteWrapper(IR_POWER_ADDR_A, write_buffer, 6, callerDescr);
	if(!aResult){
		return;
	}
	bResult = twiWriteWrapper(IR_POWER_ADDR_B, write_buffer, 6, callerDescr);
	if(!bResult){
		return;
	}
	
	if((aResult+bResult)>2){
		printf_P(PSTR("\tDone waiting for TWI. IR powers set successfully.\r\n"));
	}
	curr_ir_power = power;
}
