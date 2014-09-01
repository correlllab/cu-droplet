#include "i2c.h"

void i2c_init()
{
	count=0;
	nextWiper=0;
	thePower=0;
	PORTE.DIRSET = PIN0_bm | PIN1_bm;
	TWIE_MASTER_CTRLA = TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
	TWIE_MASTER_BAUD = TWI_BAUD(F_CPU, 400000);
	TWIE_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
	//PORTE.OUTSET = PIN0_bm | PIN1_bm;
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

//uint8_t i2c_sendbyte2(uint8_t wiper, uint16_t val)
//{
	//lastWiper=wiper;
	//lastVal=val;
	//TWIE_MASTER_ADDR = 0x58;
	////while(!((TWIE_MASTER_STATUS&TWI_MASTER_WIF_bm)||(TWIE_MASTER_STATUS&TWI_MASTER_RIF_bm)));
	////TWIE_MASTER_DATA = (wiper | ((uint8_t)((val>>8)&1)));
	////while(!((TWIE_MASTER_STATUS&TWI_MASTER_WIF_bm)||(TWIE_MASTER_STATUS&TWI_MASTER_RIF_bm)));
	////TWIE_MASTER_DATA = ((uint8_t)(val&0xFF));
	////while(!((TWIE_MASTER_STATUS&TWI_MASTER_WIF_bm)||(TWIE_MASTER_STATUS&TWI_MASTER_RIF_bm)));
	////TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
//}

ISR(TWIE_TWIM_vect)
{
	switch(count)
	{
		case 0:	TWIE_MASTER_DATA = (nextWiper | ((uint8_t)((thePower>>8)&1))); break;
		case 1: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;
		case 2: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
	}
	count = (count+1)%3;
}

void set_all_ir_powers(uint16_t power)
{
	thePower=power;
	nextWiper=0x00;
	TWIE_MASTER_ADDR = 0x58;
	delay_ms(1);
	nextWiper=0x10;
	TWIE_MASTER_ADDR = 0x58;
	delay_ms(1);
	nextWiper=0x60;
	TWIE_MASTER_ADDR = 0x58;
}

//ISR(TWIE_TWIM_vect)
//{
	//if(TWIE_MASTER_STATUS&TWI_MASTER_WIF_bm)
	//{
		//if(count==2)
		//{
			//count=0;
			//if(nextWiper==96)
			//{
				//TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
				//nextWiper=0;
				//return;
			//}
			//else
			//{
				//nextWiper=((nextWiper*6)+16);
			//}
		//}
		//
		//if(count==0) TWIE_MASTER_DATA = (nextWiper | ((uint8_t)((thePower>>8)&1)));
		//else if(count==1) TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF));
		//count++;
	//}
//}

//uint8_t i2c_sendbyte(uint8_t byte)
//{
	//// Send data
	//for (int8_t i = 7; i >= 0; i--)
	//{
		//if (byte & (1 << i)) SDA_ON;
		//else SDA_OFF;
		//delay_us(2.5);
		//SCL_ON;
		//delay_us(5);
		//SCL_OFF;
		//delay_us(2.5);
	//}
	//
	//// Ack/Nack
	//delay_us(1.5);
	//SDA_ON;
	//delay_us(1);
	//PORTE.DIRCLR = PIN0_bm;
	//SCL_ON;
	//delay_us(2.5);
	//uint8_t ack = PORTE.IN & PIN0_bm;
	//delay_us(2.5);
	//SCL_OFF;
	//SDA_OFF;
	//PORTE.DIRSET = PIN0_bm;
	//delay_us(2.5);
	//
	//return ack;
//}