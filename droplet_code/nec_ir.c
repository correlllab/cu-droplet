#include "nec_ir.h"

static uint8_t tx_pins[6] = {PIN3_bm, PIN7_bm, PIN3_bm, PIN3_bm, PIN7_bm, PIN3_bm};
static PORT_t* uart_ch[6] = {&PORTC, &PORTC, &PORTD, &PORTE, &PORTE, &PORTF};
static uint8_t saved_usart_ctrlb_vals[6] = {0,0,0,0,0,0};

static uint8_t takeOverIRHardware(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(irIsBusy(ALL_DIRS)>=4){
			return 0;
		}
		for(uint8_t dir=0;dir<6;dir++){
			//These two lines are part of what I do in all_hp_ir_cmds:
			channel[dir]->CTRLB &= ~USART_RXEN_bm;
			ir_rxtx[dir].status = IR_STATUS_BUSY_bm | IR_STATUS_COMMAND_bm;
			//These three lines are part of ir_led.c's irLedOn
			saved_usart_ctrlb_vals[dir]	  = channel[dir]->CTRLB;	// record the current state of the USART
			channel[dir]->CTRLB  	  =  0;					// disable USART
			TCF2.CTRLB |= carrier_wave_pins[dir];		// Turn on carrier wave on port dir
			uart_ch[dir]->OUTSET = tx_pins[dir];		//Set pins to 'high' so the LED is off by default.
			uart_ch[dir]->DIRSET =  tx_pins[dir];			// enable user output on this pin
			
		}
	}
	return 1;
}

static void releaseIRHardware(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for(uint8_t dir=0;dir<6;dir++){
			//These two lines are what irTransmit does at end of buffer:
			ir_rxtx[dir].data_crc		= 0;
			ir_rxtx[dir].targetID		= 0;
			ir_rxtx[dir].curr_pos		= 0;
			ir_rxtx[dir].calc_crc		= 0;
			ir_rxtx[dir].data_length	= 0;
			ir_rxtx[dir].status			= 0;
			ir_rxtx[dir].senderID		= 0;	
	
			uart_ch[dir]->DIRCLR = tx_pins[dir];
			uart_ch[dir]->OUTCLR = tx_pins[dir];
			channel[dir]->CTRLA &= ~USART_DREINTLVL_gm;
			channel[dir]->CTRLB	 =  saved_usart_ctrlb_vals[dir];
			TCF2.CTRLB &= ~carrier_wave_pins[dir]; //Turn off the carrier wave.
			channel[dir]->STATUS |= USART_TXCIF_bm;		// writing a 1 to this bit manually clears the TXCIF flag
			channel[dir]->CTRLB |= USART_RXEN_bm;	// this enables receive on the USART
		}
		hpIrBlock_bm = 0;
	}
}

static inline void irOn(void){
	// low signal on TX pin (remember: these pins were inverted during init)
	PORTC.OUTCLR = PIN3_bm/*|PIN7_bm;
	PORTD.OUTCLR = PIN3_bm;
	PORTE.OUTCLR = PIN3_bm|PIN7_bm;
	PORTF.OUTCLR = PIN3_bm*/;
}

static inline void irOff(void){
	// high signal on TX pin (turns IR blast OFF)
	PORTC.OUTSET = PIN3_bm/*|PIN7_bm;
	PORTD.OUTSET = PIN3_bm;
	PORTE.OUTSET = PIN3_bm|PIN7_bm;
	PORTF.OUTSET = PIN3_bm*/;
}

static inline void sendBit(uint8_t bit){
	irOn();
	busy_delay_us(600);
	irOff();
	if(bit){	//Logical '1'
		busy_delay_us(1600);
	}else{		//Logical '0'			
		busy_delay_us(500);
	}
}

static inline void sendByte(uint8_t byte){
	sendBit(byte&0x80);
	sendBit(byte&0x40);
	sendBit(byte&0x20);
	sendBit(byte&0x10);
	sendBit(byte&0x01);
	sendBit(byte&0x02);
	sendBit(byte&0x04);
	sendBit(byte&0x08);
}


//NEC IR Transmission Protocol:
//9ms leading pulse burst (16 times pulse burst length used for a logical data bit)
//a 4.5ms space
//the 8-bit address for the receiving device
//the 8-bit logical inverse of the address
//the 8-bit command
//the 8-bit logical inverse of the command
//a final 562.5us pulse burst to signify the end of message transmission.
//(the four bytes of data bits should be sent LSB first)
void sendIRBurst(uint8_t addr, uint8_t cmd){
	if(!takeOverIRHardware()){
		printf("IR Transmission failed; some other high-priority task is using IR hardware.\r\n");
		return;
	}
	setAllirPowers(256);//just in case.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		irOff();
		busy_delay_ms(500);
	}	
	uint32_t startTime = getTime();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		irOn();
		busy_delay_us(9000);
		irOff();
		busy_delay_us(4400);
		sendByte(addr);
		sendByte(~addr);
		sendByte(cmd);
		sendByte(~cmd);
		irOn();
		busy_delay_us(600);
		irOff();	
	}
	uint32_t elapsedTime = getTime()-startTime;
	printf("IR Burst Complete! Time elapsed: %lu ms\r\n", elapsedTime); //should be 67.5ms
	releaseIRHardware();
}

