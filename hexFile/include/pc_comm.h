#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "scheduler.h"
#include "serial_handler.h"

#define PC_PORT PORTD
#define PC_USART USARTD1
#define BUFFER_SIZE 64

char serial_in_buffer[BUFFER_SIZE];
uint8_t escaped;
uint8_t escaped_arrow;
uint16_t serial_in_index; //Points to where the next character we get will be added to the buffer.
struct
{
	char data[BUFFER_SIZE];
	uint16_t length;
} serial_data_out_buffer;

void pc_comm_init();
void handle_serial_input();
void handle_up_arrow();
int pc_comm_putchar(char c, FILE *stream);
int pc_comm_getchar(FILE *stream);
char get_char_nonblocking();