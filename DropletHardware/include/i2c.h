#ifndef I2C_H
#define I2C_H
#define F_CPU 32000000UL
#include <avr/io.h>
#include <util/delay.h>

void i2c_init();

void i2c_startbit();
void i2c_stopbit();
uint8_t i2c_sendbyte(uint8_t byte);

#define SDA_ON (PORTD.OUTSET = PIN0_bm)
#define SDA_OFF (PORTD.OUTCLR = PIN0_bm)
#define SCL_ON (PORTD.OUTSET = PIN1_bm)
#define SCL_OFF (PORTD.OUTCLR = PIN1_bm)

#endif