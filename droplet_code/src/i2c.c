#include "i2c.h"

uint16_t* global_r;
uint16_t* global_g;
uint16_t* global_b;
uint16_t* global_c;


void i2c_init()
{
	//init varaibles
	i2c_count=0;
	i2c_state=NONE;
	thePower=0;
	
	//init I2C functionality on xmega.
	PORTE.DIRSET = PIN0_bm | PIN1_bm;
	TWIE_MASTER_BAUD = TWI_BAUD(F_CPU, 400000);
	TWIE_MASTER_CTRLA = TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;	
	//TWIE_MASTER_CTRLB = TWI_MASTER_SMEN_bm; //automatically do something when the data register is read.
	//TWIE_MASTER_CTRLC = TWI_MASTER_ACKACT_bm; //make it so the thing that is automatically done is an 'ack'
	TWIE_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
		
	PORTB.DIRCLR = 0;
	
	//init RGB sensor.
	//i2c_count=0;
	//state=RGB_SENSE_POWER;
	//TWIE_MASTER_ADDR = RGB_SENSE_ADDR;
	//delay_ms(500);
	//state=NONE;
}

ISR(TWIE_TWIM_vect)
{
	if(i2c_state==IR_POWER)
	{			
		printf("I%01d:\t%3hx\r\n", i2c_count, TWIE_MASTER_STATUS);
		switch(i2c_count)
		{
			case 0:	TWIE_MASTER_DATA = (0x00 | ((uint8_t)((thePower>>8)&1))); break;
			case 1: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;
			case 2:	TWIE_MASTER_DATA = (0x10 | ((uint8_t)((thePower>>8)&1))); break;
			case 3: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;
			case 4:	TWIE_MASTER_DATA = (0x60 | ((uint8_t)((thePower>>8)&1))); break;
			case 5: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;
			default: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
		}
	}
	else if(i2c_state==RGB_SENSE_POWER)
	{
		printf("P%01d:\t%3hx\r\n", i2c_count, TWIE_MASTER_STATUS);		
		switch(i2c_count)
		{
			case 0: TWIE_MASTER_DATA = (0x00); break; //(0x00): The ENABLE register.
			case 1: TWIE_MASTER_DATA = (0x01); break; //Setting Power ON bit to 1.
			default: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
		}
	}
	else if(i2c_state==RGB_SENSE_WRITE)
	{
		printf("W%01d:\t%3hx\r\n", i2c_count, TWIE_MASTER_STATUS);
		switch(i2c_count)
		{
			case 0: TWIE_MASTER_DATA = (0x00); break; //(0x00): The ENABLE register.
			case 1: TWIE_MASTER_DATA = (0x01<<1); break; //Setting RGBC enable bit to 1.
			case 2: TWIE_MASTER_DATA = ((0x01<<7) | (0x01<<5) | 0x15); break; // (0x01<<7): CMD Register Select | (0x01<<5): auto-increment protocol | (0x15) address of first value to read;
			default: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
		}
	}
	else if(i2c_state==RGB_SENSE_READ)
	{
		printf("R%01d:\t%3hx\r\n", i2c_count, TWIE_MASTER_STATUS);
		switch(i2c_count)
		{
			case 0: *global_c = TWIE_MASTER_DATA; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 1: *global_c = (((uint16_t)TWIE_MASTER_DATA)<<8)|*global_c; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 2: *global_r = TWIE_MASTER_DATA; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 3: *global_r = (((uint16_t)TWIE_MASTER_DATA)<<8)|*global_r; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 4: *global_g = TWIE_MASTER_DATA; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 5: *global_g = (((uint16_t)TWIE_MASTER_DATA)<<8)|*global_g; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 6: *global_b = TWIE_MASTER_DATA; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			case 7: *global_b = (((uint16_t)TWIE_MASTER_DATA)<<8)|*global_b; TWIE_MASTER_CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; break;
			default: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;			
		}
	}
	else
	{
		printf("?%01d:\t%3hx\r\n", i2c_count, TWIE_MASTER_STATUS);
	}
	i2c_count++;
}

void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
{
	global_r = r;
	global_g = g;
	global_b = b;
	global_c = c;
	i2c_count=0;
	i2c_state=RGB_SENSE_WRITE;
	TWIE_MASTER_ADDR = RGB_SENSE_ADDR;
	delay_ms(1000);
	i2c_count=0;
	i2c_state=RGB_SENSE_READ;
	TWIE_MASTER_ADDR = RGB_SENSE_ADDR | 0x1;
	delay_ms(1000);
	printf("%5u | %5u | %5u | %5u\r\n",*r, *g, *b, *c);
	i2c_state=NONE;
}

void set_rgb_gain(uint8_t gain) //need to implement this.
{
	
}


void set_all_ir_powers(uint16_t power)
{
	//i2c_state=IR_POWER;
	//i2c_count=0;
	//thePower=power;
	//TWIE_MASTER_ADDR = 0x58;
	//delay_us(1200);
	//TWIE_MASTER_ADDR = 0x5c;
	//delay_us(1200);
	//
	//i2c_state=NONE;	
}