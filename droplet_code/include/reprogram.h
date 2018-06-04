#pragma once

#include "droplet_base.h"
#include "ir_comm.h"
#include "nvm.h"

typedef struct prog_data_struct{
	uint16_t secAaddr;
	uint16_t secAsize;
	uint16_t secBaddr;
	uint16_t secBsize;
	uint16_t secCaddr;
	uint16_t secCsize;
}ProgData;

typedef struct prog_msg_struct{
	char cmdWord[6];
	ProgData data;
}ProgMsg;


uint8_t reprogramming;
ProgData progData;

void handleReprogMsg(irMsg* msgStruct);

char dataHEX[64];
char initial_msg[20];
uint8_t number_of_hex;

void send_hex(void);
void send_initial(void);
