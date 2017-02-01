/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA Self-programming driver example source.
 *
 *      This file contains an example application that demonstrates the Self-programming
 *      driver. It shows how to program different values into the Application Table
 *      Section.
 *
 * \par Application note:
 *      AVR1316: XMEGA Self-programming
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1691 $
 * $Date: 2008-07-29 13:25:40 +0200 (ti, 29 jul 2008) $  \n
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
#include "flash_api.h"

/*! \brief Function to erase one page in the Application Table Section.
 *
 *  \note The maximum pageAddress must not be exceeded. The maximum number of 
 *        pages can be found in the datasheet. For the ATxmega128A1, the maximum
 *        number of pages in the application table is 16.
 *
 *  \param pageAddress Page address to the page to erase.
 */
void EraseAppTablePage(uint8_t pageAddress)
{
	/* Calculate actual start address of the page.*/
	uint16_t tableAddress = (pageAddress * FLASH_PAGE_SIZE);
	
	/* Perform page erase. */
	SP_EraseApplicationPage(APP_SECTION_START + tableAddress);

	/* Wait for NVM to finish. */
	SP_WaitForSPM();
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
}

/*! \brief Function to do an atomic erase-write on one page in the Application Table Section.
 *
 *  \note The maximum pageAddress must not be exceeded. The maximum number of 
 *        pages can be found in the datasheet. For the ATxmega128A1, the maximum
 *        number of pages in the application table is 16.
 *
 *  \param pageAddress Page address to the page to erase/write.
 */
void EraseWriteAppTablePage(uint8_t pageAddress)
{
	/* Calculate actual start address of the page.*/
	uint16_t tableAddress = (pageAddress * FLASH_PAGE_SIZE);
	
	/* Perform page erase. */
	SP_EraseWriteApplicationPage(APP_SECTION_START + tableAddress);

	/* Wait for NVM to finish. */
	SP_WaitForSPM();
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
}


/*! \brief Function to program one page in the Application Table Section.
 *
 *  \note The maximum pageAddress must not be exceeded. The maximum number of 
 *        pages can be found in the datasheet. For the ATxmega128A1, the maximum
 *        number of pages in the application table is 16.
 *
 *  \param pageAddress Page address to the page to write.
 */
void WriteAppTablePage(uint8_t pageAddress)
{
	/* Calculate actual start address of the page.*/
	uint16_t tableAddress = (pageAddress * FLASH_PAGE_SIZE);
	
	/* Perform page write. */
	SP_WriteApplicationPage(APP_SECTION_START + tableAddress);

	/* Wait for NVM to finish. */
	SP_WaitForSPM();
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
}


/*! \brief Function to load one word into page buffer.
 *
 *  \param tableAddress Address in buffer to write the word.
 *  \param lowByte      The low byte of the word to load.
 *  \param highByte     The high byte of the word to load.
 */
void LoadAppTableWord(uint16_t tableAddress, uint8_t lowByte, uint8_t highByte)
{
	/* Perform word load. */
	SP_LoadFlashWord(tableAddress, ((uint16_t) highByte << 8) | lowByte);

	/* Wait for NVM to finish. */
	SP_WaitForSPM();
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
}


/*! \brief Function to read a flash page.
 *
 *  \note The maximum pageAddress must not be exceeded. The maximum number of 
 *        pages can be found in the datasheet. For the ATxmega128A1, the maximum
 *        number of pages in the application table is 16.
 *
 *  \param data         Pointer to a data buffer to store the data.
 *  \param pageAddress  Page address to read from. 
 */
void ReadFlashPage(const uint8_t * data, uint8_t pageAddress)
{
	/* Calculate actual start address of the page.*/
	uint16_t tableAddress = (pageAddress * FLASH_PAGE_SIZE);
	
	/* Read the flash page into the buffer. */
	SP_ReadFlashPage(data, APP_SECTION_START + tableAddress);
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
}


/*! \brief Example to show how to read and write to the flash.
 */

void writeRead(uint8_t* WriteBuffer, uint32_t pageTowrite) 
{
		uint8_t success = 1;
		/* Load the flashbuffer with the test buffer. */
		SP_LoadFlashPage(WriteBuffer);
		NVM.CMD = NVM_CMD_NO_OPERATION_gc;

		/* Do a Erase-Write of the page. */
		EraseWriteAppTablePage(pageTowrite);

		/* Read a flashpage into the read buffer. */
		ReadFlashPage(ReadBuffer,pageTowrite);

		/* Verify Flash contents. */
		for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
			//printf(" ReadBuffer %d\n\r",ReadBuffer[i] );
			if (ReadBuffer[i] != WriteBuffer[i]){
				success = 0;
				break;
			}
		}
		
		printf("\n\rsuccess 2nd attempt %hu\n\r", success);
		success = 1;
	
	
}


//void writeRead(void) 
//{
	///* Assume success until proven otherwise. */
	//int success = 1;	
	//
	///*  If success, try with another method using load/read flash page function
	 //*  and an erase-write of the page.
	 //*/
	//if (success) {
//
		///* Fill up a test buffer with 512 bytes with other values. */
		//for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
			//WriteBuffer[i] = (uint8_t) i;
		//}
//
		///* Load the flashbuffer with the test buffer. */
		//SP_LoadFlashPage(WriteBuffer);
		//NVM.CMD = NVM_CMD_NO_OPERATION_gc;
//
		///* Do a Erase-Write of the page. */
		//EraseWriteAppTablePage(16);
//
		///* Read a flashpage into the read buffer. */
		//ReadFlashPage(ReadBuffer, 16);
//
		///* Verify Flash contents. */
		//for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
			//printf(" ReadBuffer %d\n\r",ReadBuffer[i] );
			//if (ReadBuffer[i] != WriteBuffer[i]){
				//success = 0;
				//break;
			//}
		//}
		//
		//printf("\n\rsuccess 2nd attempt %d\n\r", success);
		//success = 1;
	//}
	//
	//
//}

