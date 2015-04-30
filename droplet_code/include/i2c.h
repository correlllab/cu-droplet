#ifndef I2C_H
#define I2C_H
#include <avr/io.h>
#include "scheduler.h"

#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
volatile uint8_t i2c_count;
uint16_t thePower;
void i2c_init();

#define RGB_SENSE_ADDR	(0x29<<1)
#define I2C_CDATA_L		0x14
#define I2C_CDATA_H		0x15
#define I2C_RDATA_L		0x16
#define I2C_RDATA_H		0x17
#define I2C_GDATA_L		0x18
#define I2C_GDATA_H		0x19
#define I2C_BDATA_L		0x1A
#define I2C_BDATA_H		0x1B

typedef enum{
	NONE,
	IR_POWER,
	RGB_SENSE_POWER,
	RGB_SENSE_WRITE,
	RGB_SENSE_READ,
	RGB_GAIN
	} i2c_state_t;
	
volatile i2c_state_t i2c_state;

void set_all_ir_powers(uint16_t power);
void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);
void set_rgb_gain(uint8_t gain); 
inline uint16_t get_all_ir_powers(){ return thePower; };

#endif