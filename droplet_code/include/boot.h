/** \file *********************************************************************
 * \deprecated
 * \brief Code for Droplet bootloader. No longer in use.
 *
 *****************************************************************************/
#pragma once

//#include <avr/io.h>
//#include "flash_api.h"
//
//#define BOOTLOADER __attribute__((section(".BOOT")))

// to fix AS6 compile error "section .BOOT loaded at [00005482,000055a7] overlaps section .data loaded at [00005482,00005837]"
// edit project options, under AVR/GNU Linker > Memory Settings > FLASH segment, add the line ".BOOT = 0x010000"
// WHY THIS ERROR OCCURS:
// WHAT THIS FIX DOES:

/*struct
{
	uint8_t curr_part;
	uint8_t curr_page;
	uint8_t num_pages;
	uint32_t addr;
	uint32_t crc_rec_page;
	uint32_t crc_rec_prog;
} flash_pages[6];
*/

//uint8_t page_buffer[512];

//void bootloader_init();

//uint8_t command_program(uint8_t dir);

//void write_to_flash(uint8_t* data, uint32_t address);
//
//void move_flash_and_reset() BOOTLOADER;
//
//uint32_t crc_flash();