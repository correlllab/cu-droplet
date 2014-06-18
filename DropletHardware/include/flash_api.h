#include <avr/io.h>
#include <avr/interrupt.h>
#include "sp_driver.h"
//This file largely derived from xboot project, on git

// status codes
#define XB_SUCCESS 0
#define XB_ERR_NO_API 1
#define XB_ERR_NOT_FOUND 2
#define XB_INVALID_ADDRESS 3

uint8_t write_user_signature_row(uint8_t *data);

uint8_t read_user_signature_byte(uint16_t index);
void load_flash_page(const uint8_t * data);
void erase_write_application_page(uint32_t address);
void erase_flash_buffer();
void read_flash_page(const uint8_t * data, uint32_t address);
void erase_application_page(uint32_t address);