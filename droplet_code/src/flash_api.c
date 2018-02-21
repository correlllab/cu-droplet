#include "flash_api.h"
//
///******************************************************New Code for write/read on flash********************************/
///*! \brief Flush temporary FLASH page buffer. 
 //* 
 //*  This function flushes the FLASH page buffers. 
 //*/ 
//void FLASH_FlushFlasPageBuffer(void) 
//{ 
    //NVM.CMD = NVM_CMD_ERASE_FLASH_BUFFER_gc; 
    //NVM_EXEC(); 
//
    //// Wait until NVM is not busy 
    //FLASH_WaitForNVM(); 
//}   // FLASH_FlushFlasPageBuffer 
//
///*! \brief Load entire page into temporary FLASH page buffer. 
 //* 
 //*  This function loads an entire FLASH page from an SRAM buffer to 
 //*  the FLASH page buffers. Make sure that the buffer is flushed before 
 //*  starting to load bytes. 
 //* 
 //*  \note Only the lower part of the address is used to address the buffer. 
 //*        Therefore, no address parameter is needed. In the end, the data 
 //*        is written to the FLASH page given by the address parameter to the 
 //*        FLASH write page operation. 
 //* 
 //*  \param  values   Pointer to SRAM buffer containing an entire page. 
 //*/ 
//void FLASH_LoadFlashPageBuffer(const uint8_t *ram_buffer_ptr) 
//{ 
    //uint16_t i; 
    //uint16_t *int_ptr; 
//
    //// Wait until NVM is not busy 
    //FLASH_WaitForNVM(); 
    //NVM.CMD = NVM_CMD_LOAD_FLASH_BUFFER_gc; 
//
    //int_ptr = (uint16_t *)ram_buffer_ptr; 
//
    //// Load multiple bytes into page buffer 
    //for (i = 0; i < (FLASH_PAGE_SIZE * 2); i+=2) 
    //{ 
        //LoadR0(*int_ptr); 
        //LoadZ(i); 
        //CPU_CCP = CCP_SPM_gc; 
        //asm("spm"); 
        //int_ptr++; 
    //} 
//}   // FLASH_LoadFlashPageBuffer 
//
///*! \brief Erase entire application section. 
 //* 
 //*  This function erases the entire application and application table section 
 //* 
 //*  \note If the lock bits is set to not allow spm in the application or 
 //*        application table section the erase is not done. 
 //*/ 
//void FLASH_EraseApplicationSections(void) 
//{ 
    //NVM.CMD = NVM_CMD_ERASE_APP_gc; 
    //CPU_CCP = CCP_SPM_gc; 
    //// Self-program. 
    //asm("spm"); 
//
    //// Wait until NVM is not busy 
    //FLASH_WaitForNVM(); 
//}   // FLASH_EraseApplicationSections 
//
//
//
///*! \brief Erase and write page buffer to application or application table section at byte address. 
 //* 
 //*  This function does a combined erase and write to a flash page in the application 
 //*  or application table section. 
 //* 
 //*  \param page_number Flash page number. 
 //*/ 
//void FLASH_EraseWriteApplicationPage(uint16_t page_number) 
//{ 
    //// addresses the page 
    //CPU_RAMPZ = (uint8_t)(page_number >> (16 - FLASH_FWORD_SIZE)); 
    //LoadZ((uint32_t)page_number << FLASH_FWORD_SIZE); 
//
    //// Write the "safety code" to the CCP register 
    //// FLASH write has to be executed within 4 cycles 
    //NVM.CMD = NVM_CMD_ERASE_WRITE_APP_PAGE_gc; 
    //CPU_CCP = CCP_SPM_gc; 
    //// Self-program. 
    //asm("spm"); 
//
    //// Wait until NVM is not busy 
    //FLASH_WaitForNVM(); 
//}   // FLASH_EraseWriteApplicationPage 
//
///*! \brief Read a byte from flash. 
 //* 
 //*  This function reads one byte from the flash. 
 //* 
 //*  \note Both IAR and GCC have functions to do this, but 
 //*        we include the fucntions for easier use. 
 //* 
 //*  \param address Address to the location of the byte to read. 
 //* 
 //*  \retval Byte read from flash. 
 //*/ 
///*uint8_t FLASH_ReadByte(uint32_t flash_address) 
//{ 
    //uint8_t flash_data; 
	//uint32_t page_address = (uint8_t)((flash_address >> 10) & 0xFF); 
    //CPU_RAMPZ = (uint8_t)(page_address >> 16); 
    //LoadZ(flash_address); 
    //flash_data = ReadELPM(); 
//
    //return flash_data; 
//}   // FLASH_ReadByte */
//
///*! \brief Wait for any NVM access to finish, including FLASH. 
 //* 
 //*  This function is blocking and waits for any NVM access to finish, 
 //*  including FLASH. Use this function before any FLSH accesses, 
 //*  if you are not certain that any previous operations are finished yet, 
 //*  like an FLASH write. 
 //*/ 
//void FLASH_WaitForNVM(void) 
//{ 
    //do { 
        ///* Block execution while waiting for the NVM to be ready. */ 
    //} while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm); 
//}   // FLASH_WaitForNVM 
//
//
///*! \brief Read entire Flash page into SRAM buffer. 
 //* 
 //*  This function reads an entire flash page and puts it to SRAM. 
 //* 
 //*  \param data        Pointer to where to store the data. 
 //*  \param page_number Flash page number. 
 //*/ 
///*void FLASH_ReadFlashPage(uint8_t *ram_buffer, uint32_t page_number) 
//{ 
    //uint16_t idx; 
    //uint32_t base_address; 
//
    //NVM.CMD = NVM_CMD_NO_OPERATION_gc; 
//
    //CPU_RAMPZ = (uint8_t)(page_number >> (16 - FLASH_FWORD_SIZE)); 
//
    //base_address = page_number << FLASH_FWORD_SIZE; 
//
    //for (idx = 0; idx < (FLASH_PAGE_SIZE * 2); idx++) 
    //{ 
        //LoadZ((uint32_t)(base_address + idx)); 
        //*ram_buffer = ReadELPM(); 
        //ram_buffer++; 
    //} 
//}   // FLASH_ReadFlashPage */
//
//
///************************************************End of New Code*****************************************/
//
//
//void load_flash_page_buffer(uint8_t *data)
//{
    //uint16_t *int_ptr = (uint16_t *)data;
    //SP_WaitForSPM(); // Wait until NVM is not busy 
    //// Load multiple bytes into page buffer 
    //for(uint16_t i=0; i<512; i+=2) 
    //{ 
		//SP_LoadFlashWord(i, int_ptr[i/2]);
		//NVM.CMD = NVM_CMD_NO_OPERATION_gc;
    //} 
//
//}
//
//uint8_t write_user_signature_row(uint8_t *data)
//{
	////uint8_t saved_status = SREG;
	//load_flash_page(data);
	//SP_EraseUserSignatureRow();
	//SP_WriteUserSignatureRow();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	//
	////SREG = saved_status;
	//return XB_SUCCESS;
//}
//
//uint8_t read_user_signature_byte(uint16_t index)
//{
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		//uint8_t val = SP_ReadUserSignatureByte(index);
		//SP_WaitForSPM();
		//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
		//return val;
	//}
//}
//
//void load_flash_page(const uint8_t * data)
//{
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		//SP_LoadFlashPage(data);
		//SP_WaitForSPM();
		//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	//}
//}
//
//void erase_application_page(uint32_t address)
//{
	//SP_EraseApplicationPage(address);
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
//}
//
//void erase_write_application_page(uint32_t address)
//{
	//SP_EraseWriteApplicationPage(address);
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;	
//}
//
//void erase_flash_buffer()
//{
	//SP_EraseFlashBuffer();
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;	
//}
//
//void read_flash_page(const uint8_t * data, uint32_t address)
//{
	//SP_ReadFlashPage(data, address);
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
//}
//
//uint8_t FLASH_ReadByte(uint32_t flash_address)
//{
	//uint8_t val = SP_ReadByte(flash_address);
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	//return val;
//}
//
//void FLASH_ReadFlashPage(uint8_t *ram_buffer, uint32_t page_number)
//{
	//uint32_t base_address = page_number*FLASH_PAGE_SIZE;
	////printf("\n\rbase_address=%lx",base_address);
	//SP_ReadFlashPage(ram_buffer,base_address);
	//SP_WaitForSPM();
	//NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	//
//}
///*! \brief Function to read a flash page.
 //*
 //*  \note The maximum pageAddress must not be exceeded. The maximum number of 
 //*        pages can be found in the datasheet. For the ATxmega128A1, the maximum
 //*        number of pages in the application table is 16.
 //*
 //*  \param data         Pointer to a data buffer to store the data.
 //*  \param pageAddress  Page address to read from. 
 //*/
//void ReadFlashPage(const uint8_t * data, uint8_t pageAddress)
//{
	///* Calculate actual start address of the page.*/
	//uint32_t tableAddress = (pageAddress * FLASH_PAGE_SIZE);
	//
	///* Read the flash page into the buffer. */
	//SP_ReadFlashPage(data, APP_SECTION_START + tableAddress);
	//NVM.CMD = NVM_CMD_NO_OPERATION_gc;
//}
//
///*! \brief Example to show how to read and write to the flash.
 //*/
//
//void writeRead(uint8_t* WriteBuffer, uint32_t pageNumber) 
//{
		/////* Load the flashbuffer with the test buffer. */
		///* Calculate actual start address of the page.*/
		//uint32_t tableAddress = (pageNumber * FLASH_PAGE_SIZE);
		//
		//SP_LoadFlashPage(WriteBuffer);
		//SP_WaitForSPM();
		//NVM.CMD = NVM_CMD_NO_OPERATION_gc;
		//
		//SP_EraseWriteApplicationPage(tableAddress);
		//NVM.CMD = NVM_CMD_ERASE_WRITE_APP_PAGE_gc;
		//NVM_EXEC();
		//NVM.CMD = NVM_CMD_NO_OPERATION_gc;
//
//}