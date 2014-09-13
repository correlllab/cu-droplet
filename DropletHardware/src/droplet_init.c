
//TODO: cut back on the number of calls to get_id_number()
//		reverse the roles of calculate_id_number and get_id_number	(TODO)

#include "droplet_init.h"
//#include "sp_driver.h"

static uint8_t INIT_DEBUG_MODE = 0;

uint16_t droplet_ID = 0;

void Config32MHzClock(void)
{
	// Set system clock to 32 MHz
	CCP = CCP_IOREG_gc;
	OSC.CTRL = OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = 0x01;
	
	// Set up real-time clock
	CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;	// per Dustin: RTCSRC is a 1 kHz oscillator, needs to be verified
	//RTC.INTCTRL = RTC_OVFINTLVL_LO_gc;	
	while (RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.PER = 0xFFFF;		//	0xFFFF == 0b1111111111111111 = (2^16)-1
							// (2^16)-1 milliseconds is 65.535 seconds

	RTC.CTRL = RTC_PRESCALER_DIV1_gc;

	// reset RTC to 0, important for after a reboot:
	while(RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.CNT = 0;
}

void init_all_systems()
{
	Config32MHzClock();
	
	calculate_id_number();
	
	pc_com_init();
	if(INIT_DEBUG_MODE >= 1)	printf("\r\n\nPC COM INIT\r\n");

	motor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("MOTOR INIT\r\n");

	RGB_LED_init();
	if(INIT_DEBUG_MODE >= 1)	printf("LED INIT\r\n");

	rgb_sensor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RGB SENSE INIT\r\n");

	IR_sensor_init();
	//IR_sensor_enable();
	if(INIT_DEBUG_MODE >= 1)	printf("IR SENSE INIT\r\n");

	leg_monitor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("LEG INIT\r\n");

	cap_monitor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("CAP INIT\r\n");

	i2c_init();
	if(INIT_DEBUG_MODE >= 1)	printf("I2C INIT\r\n");

	//ir_com_init(72); // Proper size for IR reprogramming
	ir_com_init(IR_BUFFER_SIZE);
	//ir_com_init(IR_SMALL_BUFFER_SIZE);
	if(INIT_DEBUG_MODE >= 1)	printf("IR COM INIT\r\n");

	random_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RAND INIT\r\n");
	
	scheduler_init();
	if(INIT_DEBUG_MODE >= 1)	printf("SCHEDULER INIT\r\n");
	
	range_algorithms_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RANGE ALGORITHMS INIT\r\n");


	
	last_serial_command_time = get_32bit_time();

	for(uint8_t i = 0 ; i< 6 ;i++)
	{
		set_ir_power(i,256);
	}

	rnb_updated = 0; //The user code can check this flag to see if we have new rnb_data in last_good_rnb

	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;	// enable low level interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;	// enable medium level interrupts	(e.g. TXCIF)
	PMIC.CTRL |= PMIC_HILVLEN_bm;	// enable high level interrupts		(e.g. RTC_OVF)
	sei();
	//printf("Droplet Initialized\r\n\r\n");
}


uint16_t get_16bit_time()
{
	return RTC.CNT;
}

void set_current_time(uint16_t count)
{
	while(RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.CNT = count;
}

// Delay ms milliseconds
// (the built-in _delay_ms only takes constant arguments, not variables)
void delay_ms(uint16_t ms)
{
	uint32_t cur_time, end_time;
	cli(); cur_time = get_32bit_time(); sei();
	end_time = cur_time + ms;
	while (1)
	{
		cli();
		if (get_32bit_time() >= end_time)
		{
			sei();
			return;
		}
		sei();
		_delay_us(10);
	}
}

void calculate_id_number()
{
	if(INIT_DEBUG_MODE >= 1)	printf("get id number\r\n");

	/*
	uint8_t hi_byte = SP_ReadUserSignatureByte(0);
	uint8_t lo_byte = SP_ReadUserSignatureByte(1);
	
	if((((uint16_t)hi_byte << 8) | (uint16_t)lo_byte) != 0xFFFF)	// the assigned unique IDs appear to be randomly assigned??
	{																// lets not use this until we are prepared to assign them all
		// then a unique ID# has been assigned, you may use it...
		
		printf("Assigned ID: %x  %x\r\n", hi_byte, lo_byte);
		
		return ((uint16_t)hi_byte << 8) | (uint16_t)lo_byte;
	}
	
	// ... otherwise, calculate the default ID obtained from the manufacturer chip stamp
	*/
	
	/// 11/2012: NEW CODE (requires <avr/pgmspace.h>, <util/crc16.h>)
	// source: xgrid.cpp (abt. line 80)
	uint32_t pgm_bytes = 0;
	uint32_t crc = 0;
	
	// calculate local id
	// simply crc of user sig row
	// likely to be unique and constant for each chip
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	
	/*
	Q: what is in memory at addresses 8 to 15??

	A:	("code" taken from internet: http://savannah.nongnu.org/bugs/?32723)
	LOTNUM0_offset = 0x08, ///< Lot Number Byte 0, ASCII
	LOTNUM1_offset = 0x09, ///< Lot Number Byte 1, ASCII
	LOTNUM2_offset = 0x0A, ///< Lot Number Byte 2, ASCII
	LOTNUM3_offset = 0x0B, ///< Lot Number Byte 3, ASCII
	LOTNUM4_offset = 0x0C, ///< Lot Number Byte 4, ASCII
	LOTNUM5_offset = 0x0D, ///< Lot Number Byte 5, ASCII
	WAFNUM_offset = 0x10, ///< Wafer Number
	COORDX0_offset = 0x12, ///< Wafer Coordinate X Byte 0
	COORDX1_offset = 0x13, ///< Wafer Coordinate X Byte 1
	COORDY0_offset = 0x14, ///< Wafer Coordinate Y Byte 0
	COORDY1_offset = 0x15, ///< Wafer Coordinate Y Byte 1

	see line(s) # 2734 - 2755, & 2938 - 2948 of iox128a3u.h for clarification
	*/

	for (uint32_t i = 0x08; i <= 0x15; i++)			
	{
		pgm_bytes = pgm_read_word_far(i);		// Q: why is pgm_read_word_far() being used instead of SP_ReadCalibrationByte()??
		//pgm_bytes = PGM_READ_BYTE(i);
		//if(INIT_DEBUG_MODE >= 2)	printf("pgm_byte: %u\r\n",pgm_bytes);
		//fprintf_P(&usart_stream, PSTR("%i:%i\r\n"),i,b);	//	<--- hey! LOOK INTO THIS IMPLEMENTATION OF printf

		crc = _crc16_update(crc, pgm_bytes);
	}

	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	
/*	
	Also, this code doesn't yet work, but something similar was done with an earlier version of Droplet code,
	back when NDF was doing R&B and communication code Nov 2011 for Swarm Intelligence class.

	CCP = CCP_IOREG_gc;
	ID = PRODSIGNATURES_LOTNUM0;
	printf("Droplet Lot0: %X\r\n",ID);
	printf("Droplet Lot1: %X\r\n",PRODSIGNATURES_LOTNUM1);
	printf("Droplet Lot2: %X\r\n",PRODSIGNATURES_LOTNUM2);
	printf("Droplet Lot3: %X\r\n",PRODSIGNATURES_LOTNUM3);
	printf("Droplet Lot4: %X\r\n",PRODSIGNATURES_LOTNUM4);
	printf("Droplet Lot5: %X\r\n",PRODSIGNATURES_LOTNUM5);
	printf("\r\n");
	CCP = CCP_IOREG_gc;
	ID = PRODSIGNATURES_WAFNUM;
	printf("Droplet Wafer: %X\r\n",ID);
	printf("Droplet X0: %X\r\n",PRODSIGNATURES_COORDX0);
	printf("Droplet X1: %X\r\n",PRODSIGNATURES_COORDX1);
	printf("Droplet Y0: %X\r\n",PRODSIGNATURES_COORDY0);	
	printf("Droplet Y1: %X\r\n",PRODSIGNATURES_COORDY1);	
*/

	droplet_ID = crc;	// this is a globally available field
}

/*
DEPRECATED
*/
uint16_t get_droplet_id(){
	return droplet_ID;
}