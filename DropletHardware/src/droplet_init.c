#include "droplet_init.h"
//#include "sp_driver.h"

static uint8_t INIT_DEBUG_MODE = 0;

uint16_t droplet_ID = 0;

void init_all_systems()
{
	Config32MHzClock();
	
	calculate_id_number();
	
	pc_com_init();
	if(INIT_DEBUG_MODE >= 1)	printf("\r\n\nPC COM INIT\r\n");
	
	scheduler_init();
	if(INIT_DEBUG_MODE >= 1)	printf("SCHEDULER INIT\r\n");
	
	RGB_LED_init();
	if(INIT_DEBUG_MODE >= 1)	printf("LED INIT\r\n");
	
	rgb_sensor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RGB SENSE INIT\r\n");
	
	cap_monitor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("CAP INIT\r\n");
	
	leg_monitor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("LEG INIT\r\n");
	
	random_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RAND INIT\r\n");
	
	ir_com_init(IR_BUFFER_SIZE);
	if(INIT_DEBUG_MODE >= 1)	printf("IR COM INIT\r\n");
	
	IR_sensor_init();
	//IR_sensor_enable();
	if(INIT_DEBUG_MODE >= 1)	printf("IR SENSE INIT\r\n");
	
	i2c_init();
	if(INIT_DEBUG_MODE >= 1)	printf("I2C INIT\r\n");

	motor_init();
	if(INIT_DEBUG_MODE >= 1)	printf("MOTOR INIT\r\n");
	
	range_algorithms_init();
	if(INIT_DEBUG_MODE >= 1)	printf("RANGE ALGORITHMS INIT\r\n");

	last_serial_command_time = get_32bit_time();

	for(uint8_t i = 0 ; i< 6 ;i++)
	{
		set_ir_power(i,256);
	}
//
	//rnb_updated = 0; //The user code can check this flag to see if we have new rnb_data in last_good_rnb

	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;	// enable low level interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;	// enable medium level interrupts	(e.g. TXCIF)
	PMIC.CTRL |= PMIC_HILVLEN_bm;	// enable high level interrupts		(e.g. RTC_OVF)
	sei();
	//printf("Droplet Initialized\r\n\r\n");
}

void calculate_id_number()
{
	if(INIT_DEBUG_MODE >= 1)	printf("get id number\r\n");

	uint32_t pgm_bytes = 0;
	uint32_t crc = 0;
	
	// calculate local id
	// simply crc of user sig row
	// likely to be unique and constant for each chip
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;

	for (uint32_t i = 0x08; i <= 0x15; i++)			
	{
		pgm_bytes = pgm_read_word_far(i);
		crc = _crc16_update(crc, pgm_bytes);
	}

	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	droplet_ID = crc;
}

void droplet_reboot()
{
	CPU_CCP=CCP_IOREG_gc;
	RST.CTRL = 0b00000001;
}