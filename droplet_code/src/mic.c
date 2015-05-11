#include "mic.h"

uint32_t edge_times[MIC_DATA_WINDOW_SIZE];
volatile uint8_t curr_idx;

void mic_init()
{
	////port d, pin 5
	PORTD.DIRCLR = PIN5_bm;
	PORTD.INTCTRL |= PORT_INT0LVL_MED_gc;
	PORTD.INT0MASK = PIN5_bm;
	PORTD.PIN5CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_RISING_gc;
	curr_idx=0;
}

void process_mic_data()
{
	for(uint8_t i=1;i<MIC_DATA_WINDOW_SIZE;i++) edge_times[i-1]=edge_times[i]-edge_times[i-1];
	curr_idx=0;
	//do stuff with edge_times
	for(uint8_t i=0;i<MIC_DATA_WINDOW_SIZE;i++) printf("%5u\r\n",edge_times[i]);
	
	//reset edge_times
	for(uint8_t i=0;i<MIC_DATA_WINDOW_SIZE;i++) edge_times[i]=0;
}

ISR(PORTD_INT0_vect)
{
	printf("!");
	edge_times[curr_idx]=get_time();
	curr_idx++;
	if(curr_idx>=MIC_DATA_WINDOW_SIZE) process_mic_data();
}
