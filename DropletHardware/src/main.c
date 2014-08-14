#include "main.h"
#include "flash_api.h"

volatile float blah;

int main(void)
{
	//uint8_t ram_buff[512];
	//uint16_t idx =0, page_number = 0;
	uint16_t pg_no = 0;
	init_all_systems();
	printf("Initialized.\r\n");
	//pg_no = binary_search(MIN_PAGE_NUMBER,MAX_PAGE_NUMBER);
	//printf("\n\rIn main");
	//printf("\n\rPage Number = %d",pg_no);
	//uint8_t val = FLASH_ReadByte(0x0002);
	//printf("\n\rval_read = %x",val);
	
	/*FLASH_ReadFlashPage(&ram_buff,74);
	while(idx < FLASH_PAGE_SIZE * 2)
	{
		//printf("idx=%d",idx);
		printf("%01X",ram_buff[idx]);
		if(idx!=0 && idx%16 == 0)
		printf("\n\r");
		idx++;
	}
	printf("\n\rpage_num=%d\n\r",page_number);
	page_number++;
	//idx=0;*/
	
	while (1)
	{	
		check_messages();
		delay_ms(100);
	}
}

void check_messages()
{
	uint8_t num_msgs = 0;
	while(last_ir_msg!=NULL)
	{
		printf("\tGot: \"");
		for(uint8_t i=0; i<last_ir_msg->msg_length; i++)
		{
			printf("%c",last_ir_msg->msg[i]);
		}
		printf("\"\r\n\tfrom %hx %ums ago.\r\n",last_ir_msg->sender_ID, get_32bit_time()-last_ir_msg->arrival_time);
		msg_node* temp = last_ir_msg;
		last_ir_msg = last_ir_msg->prev;
		free(temp->msg);
		free(temp);
		num_msgs++;
	}
	if(num_msgs>0) printf("Got %hhu messages.\r\n",num_msgs);
}

void color_cycle()
{
	for(uint16_t i=0;i<=360;i++)
	{
		set_hsv(i, 255, 10);
		delay_ms(10);
	}
}

