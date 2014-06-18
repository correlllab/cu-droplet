#include "flash_api.h"

uint8_t write_user_signature_row(uint8_t *data)
{
	uint8_t saved_status = SREG;
	cli();
	
	SP_LoadFlashPage(data);
	SP_EraseUserSignatureRow();
	SP_WaitForSPM();
	SP_WriteUserSignatureRow();
	SP_WaitForSPM();
	
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	
	SREG = saved_status;
	return XB_SUCCESS;
}

uint8_t read_user_signature_byte(uint16_t index)
{
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