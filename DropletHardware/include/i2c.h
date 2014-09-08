#ifndef I2C_H
#define I2C_H
#include <avr/io.h>
#include "scheduler.h"

#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
uint8_t count;
uint8_t nextWiper;
uint16_t thePower;
void i2c_init();

void i2c_startbit();
void i2c_stopbit();
uint8_t i2c_sendbyte(uint8_t byte);
uint8_t i2c_sendbyte2(uint8_t wiper, uint16_t val);
void set_all_ir_powers(uint16_t power);

#define SDA_ON (PORTE.OUTSET = PIN0_bm)
#define SDA_OFF (PORTE.OUTCLR = PIN0_bm)
#define SCL_ON (PORTE.OUTSET = PIN1_bm)
#define SCL_OFF (PORTE.OUTCLR = PIN1_bm)

#endif