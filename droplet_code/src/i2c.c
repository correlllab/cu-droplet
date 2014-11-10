#include "i2c.h"

void i2c_init()
{
	count=0;
	thePower=0;
	PORTE.DIRSET = PIN0_bm | PIN1_bm;
	TWIE_MASTER_CTRLA = TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
	TWIE_MASTER_BAUD = TWI_BAUD(F_CPU, 400000);
	TWIE_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

ISR(TWIE_TWIM_vect)
{
	switch(count)
	{
		case 0:	TWIE_MASTER_DATA = (0x00 | ((uint8_t)((thePower>>8)&1))); break;
		case 1: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;
		case 2:	TWIE_MASTER_DATA = (0x10 | ((uint8_t)((thePower>>8)&1))); break;
		case 3: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;	
		case 4:	TWIE_MASTER_DATA = (0x60 | ((uint8_t)((thePower>>8)&1))); break;
		case 5: TWIE_MASTER_DATA = ((uint8_t)(thePower&0xFF)); break;		
		case 6: TWIE_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;
	}
	count = (count+1)%7;
}

void set_all_ir_powers(uint16_t power)
{
	thePower=power;
	TWIE_MASTER_ADDR = 0x58;
	delay_us(1200);
	TWIE_MASTER_ADDR = 0x5c;
	delay_us(1200);	
}