#include "flash_api.h"

void load_flash_page_buffer(uint8_t *data)
{
    uint16_t *int_ptr = (uint16_t *)data;
    SP_WaitForSPM(); // Wait until NVM is not busy 
    // Load multiple bytes into page buffer 
    for(uint16_t i=0; i<512; i+=2) 
    { 
		SP_LoadFlashWord(i, int_ptr[i/2]);
		NVM.CMD = NVM_CMD_NO_OPERATION_gc;
    } 

}

uint8_t write_user_signature_row(uint8_t *data)
{
	//uint8_t saved_status = SREG;
	SP_LoadFlashPage(data);
	SP_EraseUserSignatureRow();
	SP_WriteUserSignatureRow();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	
	//SREG = saved_status;
	return XB_SUCCESS;
}

uint8_t read_user_signature_byte(uint16_t index)
{
	cli();
	uint8_t val = SP_ReadUserSignatureByte(index);
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return val;
}

void load_flash_page(const uint8_t * data)
{
	cli(); //prevents interrupts
	SP_LoadFlashPage(data);
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

void erase_application_page(uint32_t address)
{
	SP_EraseApplicationPage(address);
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}

void erase_write_application_page(uint32_t address)
{
	SP_EraseWriteApplicationPage(address);
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;	
}

void erase_flash_buffer()
{
	SP_EraseFlashBuffer();
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;	
}

void read_flash_page(const uint8_t * data, uint32_t address)
{
	SP_ReadFlashPage(data, address);
	SP_WaitForSPM();
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
}