#include "rgb_sensor.h"
#define RGB_SENSE_ADDR 0x29

void rgb_sensor_init()
{
	uint8_t power_on_sequence[8] = {0x80, 0x01,  // Write 0x01 to ENABLE register, activating the device's oscillator.
									0x8F, 0x01,  // Write 0x01 to CONTROL register, setting the gain to x4.
									0x81, 0xD5,	 // Write 0xD5 to ATIME register, setting the integration time to 2.4ms*(256-ATIME)
									0x80, 0x03};  // Write 0x03 to ENABLE register, activating the ADC (and leaving the oscillator on);  
							

	uint8_t result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[0]), 2);
	if(!result)	printf("RGB sense power-on failed (1).\r\n");
	delay_ms(5);
	result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[2]), 2);
	if(!result)	printf("RGB sense power-on failed (2).\r\n");
	delay_ms(5);
	result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[4]), 2);
	if(!result)	printf("RGB sense power-on failed (3).\r\n");
	delay_ms(5);	
	result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[6]), 2);
	if(!result)	printf("RGB sense power-on failed (4).\r\n");
	delay_ms(5);
}

void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
{
	uint8_t write_sequence = 0xB4;
	uint8_t result = TWI_MasterWriteRead(RGB_SENSE_ADDR, &write_sequence, 1, 8);
	uint16_t* temp_values = (uint16_t*)(twi->readData);
	if(result)
	{
		*c=temp_values[0];
		*r=temp_values[1];
		*g=temp_values[2];
		*b=temp_values[3];
	}
	else printf("Read failed.\r\n");
}