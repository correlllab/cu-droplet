//#include <stdio.h>

#include "pc_com.h"
#include "util/delay.h"

//static FILE mystdout = FDEV_SETUP_STREAM (pc_com_putchar, pc_com_getchar, _FDEV_SETUP_RW);
static FILE mystdout = FDEV_SETUP_STREAM (pc_com_putchar,NULL,_FDEV_SETUP_WRITE);
//static FILE mystdin = FDEV_SETUP_STREAM (NULL,pc_com_getchar,_FDEV_SETUP_READ);
//extern USART_t* channel[];

void pc_com_init()
{
	// TX as output
	PC_PORT.DIRSET = PIN7_bm;
	// RX as input
	PC_PORT.DIRCLR = PIN6_bm;

	PC_USART.CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;

	// try changing this to 2400 (same as IR) to resolve PC com/IR com conflict

	PC_USART.BAUDCTRLA = 0b00010111;		// Baud rate 115200
	PC_USART.BAUDCTRLB = 0b10100100;		// BSCALE = -6, BSEL = 1047 @ 32MHz clock
	

	PC_USART.CTRLB |= USART_RXEN_bm;
	PC_USART.CTRLB |= USART_TXEN_bm;
	
	pc_com_to_ir_buffer = 0;

	stdout = &mystdout;
	//stdin = &mystdin;
	serial_in_index = 0;
	
	PC_USART.CTRLA = (uint8_t) USART_RXCINTLVL_MED_gc;
}

ISR( USARTD1_RXC_vect ) { handle_serial_input(); }

void handle_serial_input()
{
	char data = PC_USART.DATA;
	if(escaped_arrow==1)
	{
		if(data == 65)
		{
			handle_up_arrow();
		}
		escaped_arrow=0;
	}
	else if(escaped)
	{
		if(data == 91)
		{
			escaped_arrow=1;	
		}
		escaped=0;
	}
	else if(data == '\r') //We got the newline character.
	{
		printf("\r\n");
		serial_in_buffer[serial_in_index]='\0';
		handle_serial_command(serial_in_buffer,serial_in_index);
		serial_in_index = 0;
	}
	else if(data == '\b') //We got the backspace character.
	{
		if(serial_in_index > 0) serial_in_index--;
		printf("\b \b");
	}
	else if(data == 27) //escape pressed.
	{
		escaped = 1;
	}
	else if(serial_in_index==BUFFER_SIZE) //Buffer overflow
	{
		printf("\r\nERROR: Serial input buffer capacity of %u characters exceeded.\r\n",BUFFER_SIZE);
		serial_in_index = 0;
	}
	else //Everything is fine. Add the character to the buffer.
	{
		serial_in_buffer[serial_in_index] = data;
		serial_in_index++;
		putchar(data);
	}
}

void handle_up_arrow()
{
	//printf("the index position is: %i\r\n", serial_in_index);
	uint8_t temp_index = serial_in_index;
				
	while(serial_in_buffer[temp_index] != '\0')
	{
		putchar(serial_in_buffer[temp_index]);
		temp_index++;
	}
				
	serial_in_index = temp_index;
}

int pc_com_putchar(char c, FILE *stream)
{
	while( (PC_USART.STATUS & USART_DREIF_bm) == 0 ) {}
	PC_USART.DATA = c;
	return 1;
}
 
int pc_com_getchar(FILE *stream)
{
	while( (PC_USART.STATUS & USART_RXCIF_bm) == 0 ) {}
	return PC_USART.DATA;
}

char get_char_nonblocking()
{
		if (PC_USART.STATUS & USART_RXCIF_bm)
			return PC_USART.DATA;
		else
			return 0;
}
