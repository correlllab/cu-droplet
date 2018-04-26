/** \file *********************************************************************
 * \brief Code for reading and writing to flash memory directly.
 *
 * This file largely derived from the 
 * <a href="https://github.com/alexforencich/xboot">xboot</a> project, on GitHub.
 *
 ************************************************************************/
#pragma once
#include "droplet_base.h"
#include "sp_driver.h"
#include "eeprom_driver.h"

/******************************************************New Code for write/read on flash********************************/
// XMega 128A3 

#define FLASH_FWORD_SIZE                    9 
#define FRAZIONI_DI_PAGINA_FLASH            4 
#define MAX_PAGE_NUMBER						256
#define MIN_PAGE_NUMBER						0
#define IMX_MAX								20

//status of page numbers if it contains all FFs or not
#define PAGE_TRUE 1
#define PAGE_FALSE 0

/*! \brief Non-Volatile Memory Execute Command 
 * 
 *  This macro set the CCP register before setting the CMDEX bit in the 
 *  NVM.CTRLA register. 
 * 
 *  \note The CMDEX bit must be set within 4 clock cycles after setting the 
 *        protection byte in the CCP register. 
 */ 
#ifndef NVM_EXEC 
#define NVM_EXEC()  asm("push r30"      "\n\t"  \
                        "push r31"      "\n\t"  \
                        "push r16"      "\n\t"  \
                        "push r18"      "\n\t"  \
                        "ldi r30, 0xCB" "\n\t"  \
                        "ldi r31, 0x01" "\n\t"  \
                        "ldi r16, 0xD8" "\n\t"  \
                        "ldi r18, 0x01" "\n\t"  \
                        "out 0x34, r16" "\n\t"  \
                        "st Z, r18"     "\n\t"  \
                        "pop r18"       "\n\t"  \
                        "pop r16"       "\n\t"  \
                        "pop r31"       "\n\t"  \
                        "pop r30"       "\n\t"  \
                       ) 
#endif // NVM_EXEC 

void LoadR0(uint16_t);
void LoadZ(uint16_t);

uint8_t FLASH_ReadByte(uint32_t); 
void FLASH_FlushFlasPageBuffer(void); 
void FLASH_LoadFlashPageBuffer(const uint8_t *); 
void FLASH_EraseApplicationSections(void); 
void FLASH_EraseWriteApplicationPage(uint16_t); 
uint32_t FLASH_ApplicationCRC(void); 
uint32_t FLASH_RangeCRC(uint32_t, uint32_t); 
void FLASH_WaitForNVM(void); 
void FLASH_ReadFlashPage(uint8_t *, uint32_t); 
/******************************************************End of New Code*******************************************************/

// status codes
#define XB_SUCCESS 0
#define XB_ERR_NO_API 1
#define XB_ERR_NOT_FOUND 2
#define XB_INVALID_ADDRESS 3

uint8_t write_user_signature_row(uint8_t *data);

uint8_t read_user_signature_byte(uint16_t index);
void load_flash_page(const uint8_t * data);
void erase_write_application_page(uint32_t address);
void erase_flash_buffer(void);
void read_flash_page(const uint8_t * data, uint32_t address);
void erase_application_page(uint32_t address);
