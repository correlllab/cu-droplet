#include "i2c.h"

void i2c_init()
{
	PORTD.DIRSET = PIN0_bm | PIN1_bm;
	PORTD.OUTSET = PIN0_bm | PIN1_bm;
}

void i2c_startbit()
{
	SCL_ON; SDA_ON; 
	delay_us(5);
	SDA_OFF;
	delay_us(5);
	SCL_OFF;
	delay_us(2.5);
}

void i2c_stopbit()
{
	// Stop bit
	SDA_OFF;
	delay_us(2.5);
	SCL_ON;
	delay_us(2.5);
	SDA_ON;
	delay_us(5);
}

uint8_t i2c_sendbyte(uint8_t byte)
{
	// Send data
	for (int8_t i = 7; i >= 0; i--)
	{
		if (byte & (1 << i)) SDA_ON;
		else SDA_OFF;
		delay_us(2.5);
		SCL_ON;
		delay_us(5);
		SCL_OFF;
		delay_us(2.5);
	}
	
	// Ack/Nack
	delay_us(1.5);
	SDA_ON;
	delay_us(1);
	PORTD.DIRCLR = PIN0_bm;
	SCL_ON;
	delay_us(2.5);
	uint8_t ack = PORTD.IN & PIN0_bm;
	delay_us(2.5);
	SCL_OFF;
	SDA_OFF;
	PORTD.DIRSET = PIN0_bm;
	delay_us(2.5);
	
	return ack;
}