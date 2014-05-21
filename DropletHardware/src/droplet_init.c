#include "droplet_init.h"

static uint8_t INIT_DEBUG_MODE = 0;

uint16_t droplet_ID = 0;

void init_all_systems()
{
	Config32MHzClock();
	
	calculate_id_number();
	
	scheduler_init();			if(INIT_DEBUG_MODE) printf("SCHEDULER INIT\r\n"); //This will probably never print, since you need pc_com for printf to happen, but pc_com needs the scheduler.
	pc_com_init();				if(INIT_DEBUG_MODE) printf("PC COM INIT\r\n");
	RGB_LED_init();				if(INIT_DEBUG_MODE) printf("LED INIT\r\n");	
	rgb_sensor_init();			if(INIT_DEBUG_MODE) printf("RGB SENSE INIT\r\n");
	power_init();				if(INIT_DEBUG_MODE) printf("POWER INIT\r\n");
	random_init();				if(INIT_DEBUG_MODE) printf("RAND INIT\r\n");
	ir_com_init();				if(INIT_DEBUG_MODE) printf("IR COM INIT\r\n");
	IR_sensor_init();			if(INIT_DEBUG_MODE) printf("IR SENSE INIT\r\n");
	i2c_init();					if(INIT_DEBUG_MODE) printf("I2C INIT\r\n");
	motor_init();				if(INIT_DEBUG_MODE) printf("MOTOR INIT\r\n");
	range_algorithms_init();	if(INIT_DEBUG_MODE) printf("RANGE ALGORITHMS INIT\r\n");

	last_serial_command_time = get_32bit_time();

	enable_interrupts();
	startup_light_sequence();

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

void enable_interrupts()
{
	PMIC.CTRL |= PMIC_LOLVLEN_bm;	// enable low level interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;	// enable medium level interrupts	(e.g. TXCIF)
	PMIC.CTRL |= PMIC_HILVLEN_bm;	// enable high level interrupts		(e.g. RTC_OVF)
	sei();
}

void startup_light_sequence()
{
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
	set_rgb(100,0,0); delay_ms(100); set_rgb(0,100,0); delay_ms(100); set_rgb(0,0,100); delay_ms(100); led_off();
}


void droplet_reboot()
{
	CPU_CCP=CCP_IOREG_gc;
	RST.CTRL = 0b00000001;
}