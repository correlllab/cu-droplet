#pragma once

#include "droplet_base.h"
#include "serial_handler.h"

#define PC_PORT PORTD
#define PC_USART USARTD1
#define SRL_BUFFER_SIZE 64u

void pcCommInit(void);