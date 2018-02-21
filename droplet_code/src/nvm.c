/**
 * \file
 *
 * \brief Non Volatile Memory controller driver
 *
 * Copyright (C) 2010-2015 Atmel Corporation. All rights reserved.
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
//#include "compiler.h"
//#include "ccp.h"
#include "nvm.h"
//#include <string.h>

/**
 * \weakgroup nvm_flash_group
 * @{
 */

/**
 * \brief Issue flash range CRC command
 *
 * This function sets the FLASH range CRC command in the NVM.CMD register.
 * It then loads the start and end byte address of the part of FLASH to
 * generate a CRC-32 for into the ADDR and DATA registers and finally performs
 * the execute command.
 *
 * \note Should only be called from the CRC module. The function saves and
 *       restores the NVM.CMD register, but if this
 *       function is called from an interrupt, interrupts must be disabled
 *       before this function is called.
 *
 * \param start_addr  end byte address
 * \param end_addr    start byte address
 */
void nvm_issue_flash_range_crc(flash_addr_t start_addr, flash_addr_t end_addr)
{
	uint8_t old_cmd;
	// Save current nvm command
	old_cmd = NVM.CMD;

	// Load the NVM CMD register with the Flash Range CRC command
	NVM.CMD = NVM_CMD_FLASH_RANGE_CRC_gc;

	// Load the start byte address in the NVM Address Register
	NVM.ADDR0 = start_addr & 0xFF;
	NVM.ADDR1 = (start_addr >> 8) & 0xFF;
#if (FLASH_SIZE >= 0x10000UL)
	NVM.ADDR2 = (start_addr >> 16) & 0xFF;
#endif

	// Load the end byte address in NVM Data Register
	NVM.DATA0 = end_addr & 0xFF;
	NVM.DATA1 = (end_addr >> 8) & 0xFF;
#if (FLASH_SIZE >= 0x10000UL)
	NVM.DATA2 = (end_addr >> 16) & 0xFF;
#endif

	// Execute command
	ccp_write_io((uint8_t *)&NVM.CTRLA, NVM_CMDEX_bm);

	// Restore command register
	NVM.CMD = old_cmd;
}

/**
 * \brief Read buffer within the application section
 *
 * \param address	the address to where to read
 * \param buf		pointer to the data
 * \param len		the number of bytes to read
 */
void nvm_flash_read_buffer(flash_addr_t address, void *buf, uint16_t len)
{
#if (FLASH_SIZE>0x10000)
	uint32_t opt_address = address;
#else
	uint16_t opt_address = (uint16_t)address;
#endif
	nvm_wait_until_ready();
	while ( len ) {
		*(uint8_t*)buf = nvm_flash_read_byte(opt_address);
		buf=(uint8_t*)buf+1;
		opt_address++;
		len--;
	}
}

/**
 * \brief Erase and write specific parts of application flash section
 *
 * \param address        the address to where to write
 * \param buf            pointer to the data
 * \param len            the number of bytes to write
 * \param b_blank_check  if True then the page flash is checked before write
 *                       to run or not the erase page command.
 *
 * Set b_blank_check to false if all application flash is erased before.
 */
void nvm_flash_erase_and_write_buffer(flash_addr_t address, const void *buf,
	uint16_t len, bool b_blank_check)
{
	volatile uint16_t w_value;
	uint16_t page_pos;
	bool b_flag_erase;
#if (FLASH_SIZE>0x10000)
	volatile uint32_t page_address;
	uint32_t opt_address = address;
#else
	uint16_t page_address;
	uint16_t opt_address = (uint16_t)address;
#endif

	// Compute the start of the page to be modified
	page_address = opt_address-(opt_address%FLASH_PAGE_SIZE);

	// For each page
	while ( len ) {
		b_flag_erase = false;

		nvm_wait_until_ready();
		for (page_pos=0; page_pos<FLASH_PAGE_SIZE; page_pos+=2 ) {
			if (b_blank_check) {
				// Read flash to know if the erase command is mandatory
				w_value = nvm_flash_read_word(page_address);
				if (w_value!=0xFFFF) {
					b_flag_erase = true; // The page is not empty
				}
			}else{
				w_value = 0xFFFF;
			}

			// Update flash buffer
			if (len) {
				if (opt_address == page_address) {
					// The MSB of flash word must be changed
					// because the address is even
					len--;
					opt_address++;
					LSB(w_value)=*(uint8_t*)buf;
					buf=(uint8_t*)buf+1;
				}
			}
			if (len) {
				if (opt_address == (page_address+1)) {
					// The LSB of flash word must be changed
					// because the user buffer is not empty
					len--;
					opt_address++;
					MSB(w_value)=*(uint8_t*)buf;
					buf=(uint8_t*)buf+1;
				}
			}
			// Load flash buffer
			nvm_flash_load_word_to_buffer(page_address,w_value);
			page_address+=2;
		}

		// Write flash buffer
		if (b_flag_erase) {
			nvm_flash_atomic_write_app_page(page_address-FLASH_PAGE_SIZE);
		}else{
			nvm_flash_split_write_app_page(page_address-FLASH_PAGE_SIZE);
		}
	}
	printf("End of NVM eraseWrite.\r\n");
}