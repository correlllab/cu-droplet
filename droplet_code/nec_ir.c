#include "nec_ir.h"

void sendIRBurst(uint8_t data){
	//NEC IR Transmission Protocol:
	//9ms leading pulse burst (16 times pulse burst length used for a logical data bit)
	//a 4.5ms space
	//the 8-bit address for the receiving device
	//the 8-bit logical inverse of the address
	//the 8-bit command
	//the 8-bit logical inverse of the command
	//a final 562.5us pulse burst to signify the end of message transmission.
	//four bytes of data bits should be sent LSB first.
}
