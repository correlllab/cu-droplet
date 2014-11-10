#include "boot.h"
//#include "IRcom.h"
//#include <stdio.h>
//
//uint8_t command_program(uint8_t dir)
//{
	//ir_rx[dir].ir_status &= ~IR_RX_STATUS_PACKET_DONE_bm;
//
	//if ((ir_rx[dir].data_len == 144) && (ir_rx[dir].buf[0] == 'P'))
	//{
		//// Process Header
		//uint8_t curr_part = ir_rx[dir].buf[1];
		//uint8_t curr_page = ir_rx[dir].buf[2];
		//uint8_t num_pages = ir_rx[dir].buf[3];
//
		//uint32_t addr = ((uint32_t)ir_rx[dir].buf[4]) << 8;
		//addr |= ((uint32_t)ir_rx[dir].buf[5]) << 16;
//
		//uint32_t crc_rec_prog = ((uint32_t)ir_rx[dir].buf[6]) << 0;
		//crc_rec_prog |= ((uint32_t)ir_rx[dir].buf[7]) << 8;
		//crc_rec_prog |= ((uint32_t)ir_rx[dir].buf[8]) << 16;
		//crc_rec_prog |= ((uint32_t)ir_rx[dir].buf[9]) << 24;
//
		//uint32_t crc_rec_page = ((uint32_t)ir_rx[dir].buf[10]) << 0;
		//crc_rec_page |= ((uint32_t)ir_rx[dir].buf[11]) << 8;
		//crc_rec_page |= ((uint32_t)ir_rx[dir].buf[12]) << 16;
		//crc_rec_page |= ((uint32_t)ir_rx[dir].buf[13]) << 24;
		//
		//uint16_t crc_rec_part = ((uint16_t)ir_rx[dir].buf[142]) << 0;
		//crc_rec_part |= ((uint16_t)ir_rx[dir].buf[143]) << 8;
//
		//// Check CRC of packet
		//CRC.CHECKSUM0 = CRC.CHECKSUM1 = CRC.CHECKSUM2 = CRC.CHECKSUM3 = 0;
		//CRC.CTRL = CRC_SOURCE_IO_gc;
		//for (uint16_t i = 0; i < 142; i++)
			//CRC.DATAIN = ir_rx[dir].buf[i];
		//CRC.STATUS = CRC_BUSY_bm;
		//uint16_t crc_comp_part = ((uint32_t)CRC.CHECKSUM0) << 0;
		//crc_comp_part |= ((uint32_t)CRC.CHECKSUM1) << 8;
//
		//if (crc_comp_part == crc_rec_part)
		//{
			//for (uint16_t i = 0; i < 128; i++)
				//page_buffer[128*curr_part+i] = ir_rx[dir].buf[14+i];
			//
			//if (curr_part == 3)			// Received the last part of the flash page
			//{
				//// Check CRC of page
				//CRC.CHECKSUM0 = CRC.CHECKSUM1 = CRC.CHECKSUM2 = CRC.CHECKSUM3 = 0;
				//CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_IO_gc;
				//for (uint16_t i = 0; i < 512; i++)
					//CRC.DATAIN = page_buffer[i];
				//CRC.STATUS = CRC_BUSY_bm;
//
				//uint32_t crc_comp_page = (((uint32_t)CRC.CHECKSUM0) << 0) | (((uint32_t)CRC.CHECKSUM1) << 8) | (((uint32_t)CRC.CHECKSUM2) << 16) | (((uint32_t)CRC.CHECKSUM3) << 24);
				//if (crc_rec_page == crc_comp_page)
				//{
					//SP_LoadFlashPage(page_buffer);
					//SP_EraseWriteApplicationPage(addr+0x10000);
					//SP_EraseFlashBuffer();
//
					//if (curr_page+1 == num_pages)
					//{
						//uint32_t crc_comp_prog = crc_flash();
				//
						//if (crc_comp_prog == crc_rec_prog)
						//{
							//printf("Received valid flash program.  Moving it to application space and reseting.\r\n");
							//for (uint8_t i = num_pages; i < 128; i++) 
							//{
								//SP_WaitForSPM();
								//SP_EraseApplicationPage(i*512+0x10000);
							//}
							//move_flash_and_reset();
							//return 0; // <-- never executes
						//}
						//return 1; // program crc error
					//}
					//return 0; // valid page
				//}
				//return 2; // page crc error
			//}								
			//return 0; // valid part
		//}
		//return 3; // part crc error
	//}
	//return 4; // length error
//}
//
//void write_to_flash(uint8_t* data, uint32_t address)
//{
	//load_flash_page(data);
	//erase_write_application_page(address + 0x10000);
	//erase_flash_buffer();
	//for (int16_t j = 0; j < APP_SECTION_PAGE_SIZE; j++)
	//data[j] = 0xFF;
//}
////
//void move_flash_and_reset()
//{
	//uint8_t data[APP_SECTION_PAGE_SIZE];
	//for (uint8_t page = 0; page < 128; page++)
	//{
		////while(NVM.STATUS & NVM_FBUSY_bm);
		//erase_flash_buffer();
		////while(NVM.STATUS & NVM_FBUSY_bm);
		//read_flash_page(data, (uint32_t)page * APP_SECTION_PAGE_SIZE + 0x10000);
		////while(NVM.STATUS & NVM_FBUSY_bm);
		//load_flash_page(data);
		////while(NVM.STATUS & NVM_FBUSY_bm);
		//erase_write_application_page(page * APP_SECTION_PAGE_SIZE);
	//}
	//CCP = CCP_IOREG_gc;
	//RST.CTRL = RST_SWRST_bm;
//}
//
//uint32_t crc_flash()
//{
	//CRC.CHECKSUM0 = CRC.CHECKSUM1 = CRC.CHECKSUM2 = CRC.CHECKSUM3 = 0;
	//CRC.CTRL = CRC_CRC32_bm | CRC_SOURCE_FLASH_gc;
	//
	//NVM.ADDR0 = 0;
	//NVM.ADDR1 = 0;
	//NVM.ADDR2 = 1;
	//
	//NVM.DATA0 = (uint8_t)(APP_SECTION_END >> 0);
	//NVM.DATA1 = (uint8_t)(APP_SECTION_END >> 8);
	//NVM.DATA2 = (uint8_t)(APP_SECTION_END >> 16);
//
	//NVM.CMD = NVM_CMD_FLASH_RANGE_CRC_gc;
	//
	//CCP = CCP_IOREG_gc;
	//NVM.CTRLA |= NVM_CMDEX_bm;
//
	//while (CRC.STATUS & CRC_BUSY_bm) {asm("nop");}
	//uint32_t chksum = ((((uint32_t)CRC.CHECKSUM3) << 24) | (((uint32_t)CRC.CHECKSUM2) << 16) | (((uint32_t)CRC.CHECKSUM1) << 8) | (((uint32_t)CRC.CHECKSUM0) << 0));
//
	//return chksum;
//}
//
//// Non Volatile Memory Controller
///*
//ISR( NVM_EE_vect )
//{
	//printf("VECTOR 32/r/n");
//}*/	// having both (NVM_EE_vect & NVM_SPM_vect) defined caused compile error
//
//ISR( NVM_SPM_vect )
//{
	//printf("VECTOR 33\r\n");
//}