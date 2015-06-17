#include "droplet_init.h"
//#include "sp_driver.h"

static uint8_t INIT_DEBUG_MODE = 0;

uint16_t droplet_ID = 0;

void init_all_systems()
{
	Config32MHzClock();
	
	calculate_id_number();
	
	scheduler_init();			if(INIT_DEBUG_MODE) printf("SCHEDULER INIT\r\n"); //This will probably never print, since you need pc_com for printf to happen, but pc_com needs the scheduler.
	pc_comm_init();				if(INIT_DEBUG_MODE) printf("PC COM INIT\r\n");
	rgb_led_init();				if(INIT_DEBUG_MODE) printf("LED INIT\r\n");
	rgb_sensor_init();			if(INIT_DEBUG_MODE) printf("RGB SENSE INIT\r\n");
	power_init();				if(INIT_DEBUG_MODE) printf("POWER INIT\r\n");
	random_init();				if(INIT_DEBUG_MODE) printf("RAND INIT\r\n");
	i2c_init();					if(INIT_DEBUG_MODE) printf("I2C INIT\r\n");	
	ir_comm_init();				if(INIT_DEBUG_MODE) printf("IR COM INIT\r\n");
	ir_sensor_init();			if(INIT_DEBUG_MODE) printf("IR SENSE INIT\r\n");
	motor_init();				if(INIT_DEBUG_MODE) printf("MOTOR INIT\r\n");
	range_algs_init();			if(INIT_DEBUG_MODE) printf("RANGE ALGORITHMS INIT\r\n");

	enable_interrupts();
	startup_light_sequence();

}

int main()
{
	init_all_systems();
	init();
	while(1)
	{
		loop();
		check_messages();
	}
	return 0;
}

void check_messages ()
{
	ir_msg* msg_struct;	
	char actual_struct[sizeof(ir_msg)]; //It's like malloc, but on the stack.
	char actual_msg[IR_BUFFER_SIZE+1];
	msg_struct = (ir_msg*)actual_struct;
	msg_struct->msg = actual_msg;
	uint8_t i;
	
	if(user_facing_messages_ovf)
	{
		num_waiting_msgs=MAX_USER_FACING_MESSAGES;
		user_facing_messages_ovf=0;
		printf("Error: Messages overflow. Too many messages received. Try speeding up your loop if you see this a lot.\r\n");
	}
	//if(num_waiting_msgs>0) printf("num_msgs: %hu\r\n",num_waiting_msgs);
	while(num_waiting_msgs>0)
	{
		i=num_waiting_msgs-1;
		//We don't want this block to be interrupted by perform_ir_upkeep because the 
		//list of messages could get corrupted.
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			memcpy(msg_struct->msg, (const void*)msg_node[i].msg, msg_node[i].msg_length);
			msg_struct->arrival_time					= msg_node[i].arrival_time;
			msg_struct->sender_ID						= msg_node[i].sender_ID;
			msg_struct->dir_received					= msg_node[i].arrival_dir;
			msg_struct->length							= msg_node[i].msg_length;
		}			
		msg_struct->msg[msg_node[i].msg_length]	= '\0';		
		num_waiting_msgs--;


		handle_msg(msg_struct);
	}
}

void calculate_id_number()
{
	if(INIT_DEBUG_MODE >= 1)	printf("get id number\r\n");

	uint32_t pgm_bytes = 0;
	uint16_t crc = 0;
	
	// calculate local id
	// simply crc of user sig row
	// likely to be unique and constant for each chip
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;

	uint32_t addrs[16] = {0x00,0x01,0x02,0x03,0x04,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x10,0x12,0x13,0x14,0x15};

	for (uint8_t i = 0; i < 16; i++)			
	{
		pgm_bytes = pgm_read_word_far(addrs[i]);
		crc = _crc16_update(crc, (uint16_t)(pgm_bytes&0xFF));
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
	RST.CTRL = 0x1;
}

uint8_t get_droplet_ord(uint16_t id)
{
	switch(id)
	{
		case 0x0029: return 0;
		case 0x0120: return 1;
		case 0x01A9: return 2;
		case 0x086B: return 3;
		case 0x0B68: return 4;
		case 0x1064: return 5;
		case 0x11D3: return 6;
		case 0x1266: return 7;
		case 0x12AD: return 8;
		case 0x1361: return 9;
		case 0x14AA: return 10;
		case 0x1562: return 11;
		case 0x1767: return 12;
		case 0x18A2: return 13;
		case 0x1927: return 14;
		case 0x2668: return 15;
		case 0x2826: return 16;
		case 0x2C92: return 17;
		case 0x3062: return 18;
		case 0x32A7: return 19;
		case 0x3493: return 20;
		case 0x382E: return 21;
		case 0x392B: return 22;
		case 0x392C: return 23;
		case 0x3B61: return 24;
		case 0x3D6C: return 25;
		case 0x3F6D: return 26;
		case 0x3F9D: return 27;
		case 0x4327: return 28;
		case 0x46A1: return 29;
		case 0x4E2E: return 30;
		case 0x4ED3: return 31;
		case 0x5161: return 32;
		case 0x5264: return 33;
		case 0x5A2F: return 34;
		case 0x5B2F: return 35;
		case 0x5C68: return 36;
		case 0x5D61: return 37;
		case 0x5E60: return 38;
		case 0x5F2D: return 39;
		case 0x5FEC: return 40;
		case 0x6597: return 41;
		case 0x6B6F: return 42;
		case 0x6C66: return 43;
		case 0x6C6F: return 44;
		case 0x6E67: return 45;
		case 0x7022: return 46;
		case 0x7066: return 47;
		case 0x73AF: return 48;
		case 0x75A1: return 49;
		case 0x7D13: return 50;
		case 0x7EDF: return 51;
		case 0x8521: return 52;
		case 0x8625: return 53;
		case 0x896F: return 54;
		case 0x8F9C: return 55;
		case 0x9029: return 56;
		case 0x9261: return 57;
		case 0x92DA: return 58;
		case 0x9363: return 59;
		case 0x9420: return 60;
		case 0x9463: return 61;
		case 0x9495: return 62;
		case 0x9564: return 63;
		case 0x9669: return 64;
		case 0x97A0: return 65;
		case 0xA0D8: return 66;
		case 0xA165: return 67;
		case 0xA250: return 68;
		case 0xA52F: return 69;
		case 0xAF6A: return 70;
		case 0xAFD8: return 71;
		case 0xB122: return 72;
		case 0xB36F: return 73;
		case 0xB41B: return 74;
		case 0xB561: return 75;
		case 0xBC63: return 76;
		case 0xBC6E: return 77;
		case 0xBCB5: return 78;
		case 0xBD2D: return 79;
		case 0xC051: return 80;
		case 0xC32D: return 81;
		case 0xCB64: return 82;
		case 0xCBAB: return 83;
		case 0xCCD1: return 84;
		case 0xCD6B: return 85;
		case 0xCFA1: return 86;
		case 0xD0AE: return 87;
		case 0xD2D7: return 88;
		case 0xD766: return 89;
		case 0xD76C: return 90;
		case 0xD86C: return 91;
		case 0xD913: return 92;
		case 0xDC62: return 93;
		case 0xDC64: return 94;
		case 0xDC9E: return 95;
		case 0xDD21: return 96;
		case 0xDF64: return 97;
		case 0xFA6F: return 98;
		case 0xFCD0: return 99;
	}
	return 0xFF;
}