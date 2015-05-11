/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA TWI master driver header file.
 *
 *      This file contains the function prototypes and enumerator definitions
 *      for various configuration parameters for the XMEGA TWI master driver.
 *
 *      The driver is not intended for size and/or speed critical code, since
 *      most functions are just a few lines of code, and the function call
 *      overhead would decrease code performance. The driver is intended for
 *      rapid prototyping and documentation purposes for getting started with
 *      the XMEGA TWI master module.
 *
 *      For size and/or speed critical code, it is recommended to copy the
 *      function contents directly into your application instead of making
 *      a function call.
 *
 * \par Application note:
 *      AVR1308: Using the XMEGA TWI
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1569 $
 * $Date: 2008-04-22 13:03:43 +0200 (ti, 22 apr 2008) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef TWI_MASTER_DRIVER_H
#define TWI_MASTER_DRIVER_H

#include <avr/io.h>
#include "scheduler.h"

//#include "avr_compiler.h"
//
///*! Baud register setting calculation. Formula described in datasheet. */
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)

/*! Transaction status defines. */
#define TWIM_STATUS_READY              0
#define TWIM_STATUS_BUSY               1


/*! Transaction result enumeration. */
typedef enum TWIM_RESULT_enum {
	TWIM_RESULT_UNKNOWN          = (0x00<<0),
	TWIM_RESULT_OK               = (0x01<<0),
	TWIM_RESULT_BUFFER_OVERFLOW  = (0x02<<0),
	TWIM_RESULT_ARBITRATION_LOST = (0x03<<0),
	TWIM_RESULT_BUS_ERROR        = (0x04<<0),
	TWIM_RESULT_NACK_RECEIVED    = (0x05<<0),
	TWIM_RESULT_FAIL             = (0x06<<0),
} TWIM_RESULT_t;

/*! Buffer size defines */
#define TWIM_WRITE_BUFFER_SIZE         8
#define TWIM_READ_BUFFER_SIZE          8


/*! \brief TWI master driver struct
 *
 *  TWI master struct. Holds pointer to TWI module,
 *  buffers and necessary varibles.
 */
typedef struct TWI_Master {
	TWI_t *interface;                  /*!< Pointer to what interface to use */
	register8_t address;                            /*!< Slave address */
	register8_t writeData[TWIM_WRITE_BUFFER_SIZE];  /*!< Data to write */
	register8_t readData[TWIM_READ_BUFFER_SIZE];    /*!< Read data */
	register8_t bytesToWrite;                       /*!< Number of bytes to write */
	register8_t bytesToRead;                        /*!< Number of bytes to read */
	register8_t bytesWritten;                       /*!< Number of bytes written */
	register8_t bytesRead;                          /*!< Number of bytes read */
	register8_t status;                             /*!< Status of transaction */
	register8_t result;                             /*!< Result of transaction */
}TWI_Master_t;

void i2c_init();
void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);
void set_all_ir_powers(uint16_t power);
inline uint16_t get_all_ir_powers(){ return 0; };

void TWI_MasterInit(TWI_Master_t *twi,
                    TWI_t *module,
                    TWI_MASTER_INTLVL_t intLevel,
                    uint8_t baudRateRegisterSetting);
TWI_MASTER_BUSSTATE_t TWI_MasterState(TWI_Master_t *twi);
uint8_t TWI_MasterReady(TWI_Master_t *twi);
uint8_t TWI_MasterWrite(TWI_Master_t *twi,
                     uint8_t address,
                     uint8_t * writeData,
                     uint8_t bytesToWrite);
uint8_t TWI_MasterRead(TWI_Master_t *twi,
                    uint8_t address,
                    uint8_t bytesToRead);
uint8_t TWI_MasterWriteRead(TWI_Master_t *twi,
                         uint8_t address,
                         uint8_t *writeData,
                         uint8_t bytesToWrite,
                         uint8_t bytesToRead);
void TWI_MasterInterruptHandler(TWI_Master_t *twi);
void TWI_MasterArbitrationLostBusErrorHandler(TWI_Master_t *twi);
void TWI_MasterWriteHandler(TWI_Master_t *twi);
void TWI_MasterReadHandler(TWI_Master_t *twi);
void TWI_MasterTransactionFinished(TWI_Master_t *twi, uint8_t result);


void rgb_power_on();

/*! TWI master interrupt service routine.
 *
 *  Interrupt service routine for the TWI master. Copy the needed vectors
 *  into your code.
 *
    ISR(TWIC_TWIM_vect)
    {
      TWI_MasterInterruptHandler(&twiMaster);
    }

 *
 */

#endif /* TWI_MASTER_DRIVER_H */


//#ifndef I2C_H
//#define I2C_H
//#include <avr/io.h>
//#include "scheduler.h"
//
//#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
//volatile uint8_t i2c_count;
//uint16_t thePower;
//void i2c_init();
//
//#define RGB_SENSE_ADDR	(0x29<<1)
//#define I2C_CDATA_L		0x14
//#define I2C_CDATA_H		0x15
//#define I2C_RDATA_L		0x16
//#define I2C_RDATA_H		0x17
//#define I2C_GDATA_L		0x18
//#define I2C_GDATA_H		0x19
//#define I2C_BDATA_L		0x1A
//#define I2C_BDATA_H		0x1B
//
//typedef enum{
	//NONE,
	//IR_POWER,
	//RGB_SENSE_POWER,
	//RGB_SENSE_WRITE,
	//RGB_SENSE_READ,
	//RGB_GAIN
	//} i2c_state_t;
	//
//volatile i2c_state_t i2c_state;
//
//void set_all_ir_powers(uint16_t power);
//void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);
//void set_rgb_gain(uint8_t gain); 
//inline uint16_t get_all_ir_powers(){ return thePower; };
//
//#endif