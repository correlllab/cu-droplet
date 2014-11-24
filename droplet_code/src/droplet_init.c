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
	range_algs_init();	if(INIT_DEBUG_MODE) printf("RANGE ALGORITHMS INIT\r\n");

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
	while ( last_ir_msg != NULL )
	{
		//We don't want this block to be interrupted by perform_ir_upkeep because the 
		//list of messages could get corrupted.
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			msg_struct			= (ir_msg*)malloc(sizeof(ir_msg));
			msg_struct->msg		= (char*)malloc(last_ir_msg->msg_length+1);
			
			memcpy(msg_struct->msg, last_ir_msg->msg, last_ir_msg->msg_length);
			
			msg_struct->msg[last_ir_msg->msg_length]	= '\0';
			msg_struct->arrival_time					= last_ir_msg->arrival_time;
			msg_struct->sender_ID						= last_ir_msg->sender_ID;
			msg_struct->dir_received					= last_ir_msg->arrival_dir;
			msg_struct->length							= last_ir_msg->msg_length;
			
			msg_node* temp = last_ir_msg;
			last_ir_msg = last_ir_msg->prev;
			free(temp->msg);
			free((ir_msg*)temp);
		}

		handle_msg(msg_struct);
		free(msg_struct->msg);
		free(msg_struct);
	}
}

////This function adds your message to the linked list of messages to be sent out.
//void send_message(out_ir_msg* msg_struct)
//{
	//out_msg_node* msg_struct_node = (out_msg_node*)malloc(sizeof(out_msg_node));
	//msg_struct_node->msg = (char*)malloc(msg_struct->length+1);
	//memcpy(msg_struct_node->msg, msg_struct->msg, msg_struct->length);
	//msg_struct_node->target = msg_struct->target;
	//msg_struct_node->length = msg_struct->length;
	//msg_struct_node->dir_mask = msg_struct->dir_mask;
	//msg_struct_node->prev = outbound_msgs;
	//outbound_msgs = msg_struct_node;
//}

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
