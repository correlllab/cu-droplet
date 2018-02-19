#include "pc_comm.h"

static char serial_in_buffer[SRL_BUFFER_SIZE];
static uint8_t escaped;
static uint8_t escaped_arrow;
static uint16_t serial_in_index; //Points to where the next character we get will be added to the buffer.

static void handle_serial_input(void);
static int pc_comm_putchar(char c, FILE *stream);
static void handle_up_arrow(void);
static FILE mystdout = FDEV_SETUP_STREAM (pc_comm_putchar,NULL,_FDEV_SETUP_WRITE);

void pcCommInit(){
	PC_PORT.DIRSET = PIN7_bm; 	// TX as output
	PC_PORT.DIRCLR = PIN6_bm;	// RX as input

	PC_USART.CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;

	PC_USART.BAUDCTRLA = 0b00010111;		// Baud rate 115200
	PC_USART.BAUDCTRLB = 0b10100100;		// BSCALE = -6, BSEL = 1047 @ 32MHz clock

	PC_USART.CTRLB |= USART_RXEN_bm;
	PC_USART.CTRLB |= USART_TXEN_bm;

	stdout = &mystdout;
	serial_in_index = 0;
	
	PC_USART.CTRLA = (uint8_t) USART_RXCINTLVL_MED_gc;
	printf("\n");
}

ISR( USARTD1_RXC_vect ) { handle_serial_input(); }

static void handle_serial_input(void){
	char data = PC_USART.DATA;
	if(escaped_arrow==1){
		if(data == 65){
			handle_up_arrow();
		}
		escaped_arrow=0;
	}else if(escaped){
		if(data == 91){
			escaped_arrow=1;	
		}
		escaped=0;
	}else if(data == '\r' || data == '\n'){ //We got the newline character.
		printf("\r\n");
		serial_in_buffer[serial_in_index]='\0';
		handleSerialCommand(serial_in_buffer,serial_in_index);
		serial_in_index = 0;
	}else if(data == '\b'){ //We got the backspace character.
		if(serial_in_index > 0) serial_in_index--;
		printf("\b \b");
	}else if(data == 27){ //escape pressed.
		escaped = 1;
	}else if(serial_in_index==SRL_BUFFER_SIZE){ //Buffer overflow
		printf_P(PSTR("\r\nERROR: Serial input buffer capacity of %u characters exceeded.\r\n"),SRL_BUFFER_SIZE);
		serial_in_index = 0;
	}else{ //Everything is fine. Add the character to the buffer.
		serial_in_buffer[serial_in_index] = data;
		//printf("Got char '%c'\r\n",data);
		serial_in_index++;
		putchar(data);
	}
}

static void handle_up_arrow(void){
	uint8_t temp_index = serial_in_index;
			
	while(serial_in_buffer[temp_index] != '\0'){
		putchar(serial_in_buffer[temp_index]);
		temp_index++;
	}
				
	serial_in_index = temp_index;
}

static int pc_comm_putchar(char c, FILE *stream __attribute__ ((unused))){
	while( (PC_USART.STATUS & USART_DREIF_bm) == 0 ) {}
	PC_USART.DATA = c;
	return 1;
}
 
