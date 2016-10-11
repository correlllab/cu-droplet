#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "scheduler.h"
#include "serial_handler.h"

#define PC_PORT PORTD
#define PC_USART USARTD1
#define BUFFER_SIZE 64u

void pc_comm_init();