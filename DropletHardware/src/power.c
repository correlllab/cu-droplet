#include "power.h"

void power_init()
{
	cap_monitor_init();
	//leg_monitor_init();
	programming_mode_init();
}


void cap_monitor_init()
{
	PORTB.DIRCLR = PIN0_bm | PIN1_bm;
	
	ACB.AC0CTRL = AC_ENABLE_bm;
	ACB.AC1CTRL = AC_ENABLE_bm;
	
	ACB.AC0MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_BANDGAP_gc;
	ACB.AC1MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_PIN0_gc;
	
	ACB.WINCTRL = AC_WEN_bm;
}

void leg_monitor_init()
{
	PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
	
	ACA.AC0CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	ACA.AC1CTRL = AC_HSMODE_bm | AC_ENABLE_bm;
	
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc;			// PA0 is VREF_HI
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc;			// PA1 is VREF_LO
	
	ACA.AC0MUXCTRL |= AC_MUXPOS_PIN2_gc;		// Initially, look at leg1
	ACA.AC1MUXCTRL |= AC_MUXPOS_PIN2_gc;
	
	ACA.WINCTRL = AC_WEN_bm;					// Enable window mode
}

void programming_mode_init()
{
	prog_in_buffer = (char*)malloc(PROG_BUFFER_SIZE);
	prog_num_chars = 0;
	time_old = 0;
	PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;		//sets pins as inputs
	//PORTA.INT0MASK = PIN2_bm | PIN3_bm | PIN4_bm;	//set the pin for leg so an interrupt is generated
	PORTA.INT0MASK = PIN2_bm;
	PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;			//edge detection settings, leg 1
	//PORTA.PIN3CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;			//edge detection settings, leg 2
	//PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;			//edge detection settings, leg 3
	PORTA.INTCTRL = PORT_INT0LVL_LO_gc;			//interrupt control is set to low level
	printf("\n\rProgramming mode initialized");
	//schedule_task(1000/PROG_BUFFER_CHECK_FREQ, print_prog_buffer, NULL);
}

//void print_prog_buffer()
//{
	////uint8_t checksum = 0;
	////uint8_t checksum_tgt = 0;
	//uint8_t data_bytes_in_line = 0;
	//
	//
	//
	////for(uint8_t i=0; i<prog_num_chars; i++)
	////{
		////if(i==0) data_bytes_in_line = prog_in_buffer[i];
		//printf("In print prog buffer");
		//////printf("%hhx",prog_in_buffer[i]);
			////
		////
		//////if(i==data_bytes_in_line+4)
		//////{
			//////checksum_tgt = prog_in_buffer[i];
			//////break;
		//////}
		//////checksum+=prog_in_buffer[i];
		//////(uint8_t*)(0x3001)=
	////}
	////checksum = (~checksum)+1;
	//
	//
	////if(prog_num_chars)
	////{
		////printf("\r\n");
		////if(checksum!=checksum_tgt) printf("checksum error. got: %hhx, should be: %hhx\r\n",checksum, checksum_tgt);
	////}
	////prog_num_chars = 0;
	//schedule_task(1000/PROG_BUFFER_CHECK_FREQ, print_prog_buffer, NULL);
//}

ISR(PORTA_INT0_vect)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		unsigned char in_byte=0;
		uint8_t new_crc;
		busy_delay_us(HALF_BIT_DURATION);
		//curr_pos=0;
		for(uint8_t i=0;i<8;i++)
		{
			busy_delay_us(FULL_BIT_DURATION);
			//printf("\n\rIn ISR for loop");
			if((((PORTA.IN>>2)|(PORTA.IN>>3)|(PORTA.IN>>4))&0x1)) { in_byte |= (0x1<<i); 
				if(i==7)	pwr_curr_pos++;
				//printf("\n\rpwr_curr_pos=%hhu",pwr_curr_pos);
				//printf("\n\rIn ISR if loop");	
				}
			else												  in_byte |= (0x0<<i);
		}	
	time_new = get_32bit_time();
	time_diff = time_new - time_old;
	time_old = time_new;
	
	if(time_diff>15)	pwr_curr_pos = 0;
	
	if(pwr_curr_pos==POS_BYTE_COUNT)	data_length = in_byte;
	else if(pwr_curr_pos==POS_ADDR_LOW)	    byte_addr_low = in_byte;
	else if(pwr_curr_pos==POS_ADDR_HIGH)	    byte_addr_high  =  in_byte;
	else if(pwr_curr_pos==POS_RECORD_TYPE)	{ rec_type = in_byte; POS_CRC = (POS_RECORD_TYPE+data_length); }	
	else if(pwr_curr_pos<POS_CRC)	prog_temp_buffer[prog_num_chars-4] = in_byte;			
	else {
		crc = in_byte;
		new_crc = data_length + byte_addr_low + byte_addr_high + rec_type;
		for(uint8_t j=0;j<(prog_num_chars-4);j++)	new_crc += prog_temp_buffer[j];
		new_crc = (~new_crc) + 0x01;
		//if(new_crc==crc)
		{
			for(uint8_t ijk=0;ijk<data_length;ijk++)
				prog_in_buffer[ijk] = prog_temp_buffer[ijk];
			printf("data_length=%hhx",data_length);
			printf("addrlow=%hhx",byte_addr_low);
			printf("addrhigh=%hhx",byte_addr_high);
			printf("rectype=%hhx",rec_type);
			for(uint8_t ijk=0;ijk<data_length;ijk++)
				printf("%hhx",prog_in_buffer[ijk]);
		}	
		if(new_crc!=crc)
			printf("checksum error. got: %hhx, should be: %hhx\r\n",crc, new_crc);	
	}			
	
	//byte_addr = byte_addr_low | (byte_addr_high<<8);
	prog_num_chars++;
	//printf("\n\rIn ISR End ");
	//printf("%hhx\r\n",in_byte);
	}
PORTA.INTFLAGS = PORT_INT0IF_bm; //clear any interrupts that happened while we were reading the byte.
}

uint8_t cap_status()
{
	switch (ACB.STATUS & AC_WSTATE_gm)
	{
		case AC_WSTATE_ABOVE_gc:  return 1;
		case AC_WSTATE_INSIDE_gc: return 0;
		case AC_WSTATE_BELOW_gc:  return -1;
	}
	return -2;
}

int8_t leg_status(uint8_t leg)
{
	switch (leg)
	{
		case 1:
		return leg1_status();
		case 2:
		return leg2_status();
		case 3:
		return leg3_status();
	}
	return 0x80;
}

int8_t leg1_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN2_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN2_gc;
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t leg2_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN3_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN3_gc;
	
	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t leg3_status()
{
	ACA.AC0MUXCTRL = AC_MUXNEG_PIN0_gc | AC_MUXPOS_PIN4_gc;
	ACA.AC1MUXCTRL = AC_MUXNEG_PIN1_gc | AC_MUXPOS_PIN4_gc;

	uint8_t status = ACA.STATUS;
	
	if ((status & AC_WSTATE_gm) == AC_WSTATE_ABOVE_gc) { return 1; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_INSIDE_gc) { return 0; }
	if ((status & AC_WSTATE_gm) == AC_WSTATE_BELOW_gc) { return -1; }
	return -2;
}

int8_t legs_powered()
{
	if ((leg1_status() == 1 || leg2_status() == 1 || leg3_status() == 1) &&
		(leg1_status() == -1 || leg2_status() == -1 || leg3_status() == -1))
		return 1;
	return 0;
}

uint8_t light_if_unpowered(uint8_t r, uint8_t g, uint8_t b)
{
	if (!legs_powered())
	set_rgb(r,g,b);
	else set_rgb(0,0,0);
}