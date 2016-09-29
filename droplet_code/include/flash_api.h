#ifndef __FLASH_H
#define __FLASH_H

#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef __AVR_XMEGA__
#include "sp_driver.h"
#else // __AVR_XMEGA__
#include <avr/boot.h>
#endif // __AVR_XMEGA__

//#include "xboot.h"

// offsets and addresses
#ifndef PROGMEM_SIZE
#define PROGMEM_SIZE (FLASHEND + 1UL)
#endif

#ifndef BOOT_SECTION_SIZE
#error BOOT_SECTION_SIZE not defined!
#endif

#ifndef BOOT_SECTION_START
#define BOOT_SECTION_START (PROGMEM_SIZE - BOOT_SECTION_SIZE)
#endif

#ifndef APP_SECTION_START
#define APP_SECTION_START 0
#endif

#ifndef APP_SECTION_SIZE
#define APP_SECTION_SIZE (PROGMEM_SIZE - BOOT_SECTION_SIZE)
#endif

#ifndef APP_SECTION_END
#define APP_SECTION_END (APP_SECTION_START + APP_SECTION_SIZE - 1UL)
#endif

#if PROGMEM_SIZE > 0x010000
#define PGM_READ_BYTE pgm_read_byte_far
#define PGM_READ_WORD pgm_read_word_far
#define PGM_READ_DWORD pgm_read_dword_far
#else
#define PGM_READ_BYTE pgm_read_byte_near
#define PGM_READ_WORD pgm_read_word_near
#define PGM_READ_DWORD pgm_read_dword_near
#endif
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


#ifdef __AVR_XMEGA__

// XMega functions
// (sp_driver wrapper)

#define Flash_ReadByte SP_ReadByte
#define Flash_ReadWord SP_ReadWord
#define Flash_LoadFlashWord SP_LoadFlashWord
#define Flash_EraseApplicationSection SP_EraseApplicationSection
#define Flash_EraseApplicationPage SP_EraseApplicationPage
#define Flash_EraseWriteApplicationPage SP_EraseWriteApplicationPage
#define Flash_WriteApplicationPage SP_WriteApplicationPage
#define Flash_EraseUserSignatureRow SP_EraseUserSignatureRow
#define Flash_WriteUserSignatureRow SP_WriteUserSignatureRow
#define Flash_LoadFlashPage SP_LoadFlashPage
#define Flash_ReadFlashPage SP_ReadFlashPage
#define Flash_WaitForSPM SP_WaitForSPM
//printf("Namaste thambi\n\r");

#else

// ATMega Functions
//printf("Namaste anna\n\r");
#define Flash_ReadByte PGM_READ_BYTE
#define Flash_ReadWord PGM_READ_WORD
#define Flash_LoadFlashWord boot_page_fill
void Flash_EraseApplicationSection(void);
#define Flash_EraseApplicationPage boot_page_erase
void Flash_EraseWriteApplicationPage(uint32_t addr);
#define Flash_WriteApplicationPage boot_page_write
void Flash_LoadFlashPage(uint8_t *data);
void Flash_ReadFlashPage(uint8_t *data, uint32_t addr);
#define Flash_WaitForSPM boot_spm_busy_wait

#endif // __AVR_XMEGA__

void Flash_ProgramPage(uint32_t page, uint8_t *buf, uint8_t erase);


#endif // __FLASH_H
