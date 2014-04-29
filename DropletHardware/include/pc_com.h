#ifndef PC_COM_H
#define PC_COM_H

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include "boot.h"
#include "motor.h"
#include "IRcom.h"
#include "serial_handler.h"

#define PC_PORT PORTD
#define PC_USART USARTD1
#define BUFFER_SIZE 64

void pc_com_init();

int pc_com_putchar(char c, FILE *stream);
int pc_com_getchar(FILE *stream);
char get_char_nonblocking();

// If pc_com_to_ir_buffer == 1, send the pc_com input to the IR buffer instead of getchar
uint8_t pc_com_to_ir_buffer;

void handle_serial_input();
void handle_up_arrow();

char serial_in_buffer[BUFFER_SIZE];
uint8_t escaped;
uint8_t escaped_arrow;
uint16_t serial_in_index; //Points to where the next character we get will be added to the buffer.

struct
{
	char data[BUFFER_SIZE];
	uint16_t length;
	
} serial_data_out_buffer;

#endif
