/**
 * \file
 *
 * \brief Non Volatile Memory controller driver
 *
 * Copyright (c) 2010-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#pragma once
#include "droplet_base.h"

#define  MSB(u16)          (((uint8_t* )&u16)[1])
#define  LSB(u16)          (((uint8_t* )&u16)[0])


/**
 * \defgroup nvm_group NVM driver
 *
 * See \ref xmega_nvm_quickstart
 *
 * \brief Low-level driver implementation for the AVR XMEGA Non Volatile
 *        Memory Controller (NVM).
 *
 * The XMEGA NVM controller interfaces the internal non-volatile memories
 * in the XMEGA devices. Program memory, EEPROM and signature row is can be
 * interfaced by the module. See the documentation of each sub-module for
 * more information.
 *
 * \note If using GCC and the flash sub-module, remember to configure
 *       the boot section in the make file. More information in the sub-module
 *       documentation.
 *
 * \section xmega_nvm_quickstart_section Quick Start Guide
 * See \ref xmega_nvm_quickstart
 */

extern void ccp_write_io(void *addr, uint8_t value);

/**
 * \defgroup nvm_generic_group NVM driver generic module handling
 * \ingroup nvm_group
 * \brief Support functions for the NVM driver.
 *
 * These functions are helper functions for the functions of the
 * \ref nvm_group "NVM driver".
 *
 * @{
 */

/**
 * \brief Wait for any NVM access to finish.
 *
 * This function is blocking and waits for any NVM access to finish.
 * Use this function before any NVM accesses, if you are not certain that
 * any previous operations are finished yet.
 */
static inline void nvm_wait_until_ready( void )
{
	do {
		// Block execution while waiting for the NVM to be ready
	} while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
}

/**
 * \brief Non-Volatile Memory Execute Command
 *
 * This function sets the CCP register before setting the CMDEX bit in the
 * NVM.CTRLA register.
 *
 * \note The correct NVM command must be set in the NVM.CMD register before
 *       calling this function.
 */
static inline void nvm_exec(void)
{
	ccp_write_io((uint8_t *)&NVM.CTRLA, NVM_CMDEX_bm);
}

/**
 * \brief Non-Volatile Memory Execute Specific Command
 *
 * This function sets a command in the NVM.CMD register, then performs an
 * execute command by writing the CMDEX bit to the NVM.CTRLA register.
 *
 * \note The function saves and restores the NVM.CMD register, but if this
 *       function is called from an interrupt, interrupts must be disabled
 *       before this function is called.
 *
 * \param nvm_command NVM Command to execute.
 */
static inline void nvm_issue_command(NVM_CMD_t nvm_command)
{
	uint8_t old_cmd;

	old_cmd = NVM.CMD;
	NVM.CMD = nvm_command;
	ccp_write_io((uint8_t *)&NVM.CTRLA, NVM_CMDEX_bm);
	NVM.CMD = old_cmd;
}

/**
 * \brief Read one byte using the LDI instruction
 * \internal
 *
 * This function sets the specified NVM_CMD, reads one byte using at the
 * specified byte address with the LPM instruction. NVM_CMD is restored after
 * use.
 *
 * \note Interrupts should be disabled before running this function
 *       if program memory/NVM controller is accessed from ISRs.
 *
 * \param nvm_cmd NVM command to load before running LPM
 * \param address Byte offset into the signature row
 */
uint8_t nvm_read_byte(uint8_t nvm_cmd, uint16_t address);


/**
 * \brief Perform SPM write
 * \internal
 *
 * This function sets the specified NVM_CMD, sets CCP and then runs the SPM
 * instruction to write to flash.
 *
 * \note Interrupts should be disabled before running this function
 *       if program memory/NVM controller is accessed from ISRs.
 *
 * \param addr Address to perform the SPM on.
 * \param nvm_cmd NVM command to use in the NVM_CMD register
 */
void nvm_common_spm(uint32_t addr, uint8_t nvm_cmd);

//! @}

/**
 * \defgroup nvm_flash_group NVM driver flash handling
 * \ingroup nvm_group
 * \brief Functions for handling internal flash memory.
 *
 * The internal flash memory on the XMEGA devices consists of the application
 * section, the application table section and the bootloader section.
 * All these sections can store program code for the MCU, but if there is
 * available space, they can be used for storing other persistent data.
 *
 * Writing the flash memory can only be done one page at a time. It consists
 * of loading the data to the internal page buffer and then running one of
 * the write commands. If the page has not been erased before writing, the
 * data will not be written correctly.
 *
 * In order to be able to write to flash memory the programming commands need
 * to be run from the boot section.
 * - When using IAR this is handled automatically by the linker script.
 * - When using GCC this needs to be specified manually in the make files. For
 *   example the ATxmega128A1 has the boot section at the word address 0x10000
 *   the corresponding byte address of 0x20000 needs to be added to the
 *   config.mk makefile:
 *   LDFLAGS += -Wl,--section-start=.BOOT=0x20000
 *   See the device datasheet for the correct address for other devices.
 *
 * \note If using GCC and the flash sub-module, remember to configure
 *       the boot section in the make file.
 *
 * \note The functions in this module are modifying the NVM.CMD register.
 *       If the application are using program space access in interrupts
 *       (__flash pointers in IAR EW or pgm_read_byte in GCC) interrupts
 *       needs to be disabled when running EEPROM access functions. If not
 *       the program space reads will be corrupted.
 * @{
 */
#define FLASH_SIZE (128*1024L)
#define FLASH_PAGE_SIZE (512)
/**
 * Data type for holding flash memory addresses.
 *
 */
#if (FLASH_SIZE >= 0x10000UL) // Note: Xmega with 64KB of flash have 4KB boot flash
typedef uint32_t flash_addr_t;
#else
typedef uint16_t flash_addr_t;
#endif

/**
 * \brief Load byte from flash memory
 *
 * Load one word of flash using byte addressing. IAR has __flash pointers
 * and GCC have pgm_read_byte_xx functions which load data from flash memory.
 * This function used for compatibility between the compilers.
 *
 * \param addr Byte address to load
 * \return Byte from program memory
 */
static inline uint8_t nvm_flash_read_byte(flash_addr_t addr){
	return pgm_read_byte_far(addr);
}

/**
 * \brief Load word from flash memory
 *
 * Load one word of flash using byte addressing. IAR has __flash pointers
 * and GCC have pgm_read_byte_xx functions which load data from flash memory.
 * This function used for compatibility between the compilers.
 *
 * \param addr Byte address to load (last bit is ignored)
 * \return Word from program memory
 */
static inline uint16_t nvm_flash_read_word(flash_addr_t addr){
	return pgm_read_word_far(addr);
}


/**
 * \brief Flush flash page buffer
 *
 * Clear the NVM controller page buffer for flash. This needs to be called
 * before using \ref nvm_flash_load_word_to_buffer if it has not already been
 * cleared.
 *
 */
static inline void nvm_flash_flush_buffer(void){
	nvm_wait_until_ready();
	nvm_common_spm(0, NVM_CMD_ERASE_FLASH_BUFFER_gc);
}


/**
 * \brief Load word into flash page buffer
 *
 * Clear the NVM controller page buffer for flash. This needs to be called
 * before using \ref nvm_flash_load_word_to_buffer if it has not already been
 * cleared.
 *
 * \param word_addr Address to store data. The upper bits beyond the page size
 *                  is ignored. \ref FLASH_PAGE_SIZE
 * \param data Data word to load into the page buffer
 */
void nvm_flash_load_word_to_buffer(uint32_t word_addr, uint16_t data);


/**
 * \brief Erase entire application section
 *
 * Erase all of the application section.
 */
static inline void nvm_flash_erase_app(void)
{
	nvm_wait_until_ready();
	nvm_common_spm(0, NVM_CMD_ERASE_APP_gc);
}

/**
 * \brief Erase a page within the application section
 *
 * Erase one page within the application section
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_erase_app_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	nvm_common_spm(page_addr, NVM_CMD_ERASE_APP_PAGE_gc);
}

/**
 * \brief Write a page within the application section
 *
 * Write a page within the application section with the data stored in the
 * page buffer. The page needs to be erased before the write to avoid
 * corruption of the data written.
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_split_write_app_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	nvm_common_spm(page_addr, NVM_CMD_WRITE_APP_PAGE_gc);
}

/**
 * \brief Erase and write a page within the application section
 *
 * Erase and the write a page within the application section with the data
 * stored in the page buffer. Erase and write is done in an atomic operation.
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_atomic_write_app_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	printf("\n------- erase write func : addr passed : %lu\n", page_addr);
	nvm_common_spm(page_addr, NVM_CMD_ERASE_WRITE_APP_PAGE_gc);
}

void nvm_issue_flash_range_crc(flash_addr_t start_addr, flash_addr_t end_addr);

void nvm_flash_read_buffer(flash_addr_t address, void *buf, uint16_t len);

void nvm_flash_erase_and_write_buffer(flash_addr_t address, const void *buf,
	uint16_t len, bool b_blank_check);

/**
 * \brief Erase a page within the boot section
 *
 * Erase one page within the boot section
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_erase_boot_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	nvm_common_spm(page_addr, NVM_CMD_ERASE_BOOT_PAGE_gc);
}

/**
 * \brief Write a page within the boot section
 *
 * Write a page within the boot section with the data stored in the
 * page buffer. The page needs to be erased before the write to avoid
 * corruption of the data written.
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_split_write_boot_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	nvm_common_spm(page_addr, NVM_CMD_WRITE_BOOT_PAGE_gc);
}

/**
 * \brief Erase and write a page within the boot section
 *
 * Erase and the write a page within the boot section with the data
 * stored in the page buffer. Erase and write is done in an atomic operation.
 *
 * \param page_addr Byte address to the page to delete
 */
static inline void nvm_flash_atomic_write_boot_page(flash_addr_t page_addr)
{
	nvm_wait_until_ready();
	nvm_common_spm(page_addr, NVM_CMD_ERASE_WRITE_BOOT_PAGE_gc);
}

void nvm_user_sig_read_buffer(flash_addr_t address, void *buf, uint16_t len);
void nvm_user_sig_write_buffer(flash_addr_t address, const void *buf,
	uint16_t len, bool b_blank_check);

/**
 * \brief Erase the user calibration section page
 *
 * Erase the user calibration section page. There is only one page, so no
 * parameters are needed.
 */
static inline void nvm_flash_erase_user_section(void)
{
	nvm_wait_until_ready();
	nvm_common_spm(0, NVM_CMD_ERASE_USER_SIG_ROW_gc);
}

/**
 * \brief Write the user calibration section page
 *
 * Write a the user calibration section page with the data stored in the
 * page buffer. The page needs to be erased before the write to avoid
 * corruption of the data written. There is only one page, so no
 * parameters are needed.
 */
static inline void nvm_flash_write_user_page(void)
{
	nvm_wait_until_ready();
	nvm_common_spm(0, NVM_CMD_WRITE_USER_SIG_ROW_gc);
}