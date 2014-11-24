#ifndef I2C_H
#define I2C_H
#include <avr/io.h>
#include "scheduler.h"

#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
uint8_t count;
uint16_t thePower;
void i2c_init();

void set_all_ir_powers(uint16_t power);
inline uint16_t get_all_ir_powers(){ return thePower; };

#endif