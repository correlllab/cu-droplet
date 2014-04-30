#include "main.h"

int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");
	// Set theta and tau initially
	theta = 2.f;
	tau = 10;
	
	group_root = (group_item*)malloc(sizeof(group_item));
	group_root->ms_age = 0;
	group_root->ID = droplet_ID;
	group_root->next = group_root;
	group_root->prev = group_root;
	
	uint32_t last_time = get_32bit_time();
	uint32_t cur_time = get_32bit_time();
	uint16_t prev_gap=0;
	
	yes_count = 0;
	broadcast_heartbeat();
	check_votes();
	while (1)
	{	
		cur_time = get_32bit_time();
		prev_gap = (uint16_t)(cur_time - last_time);
		last_time = cur_time;
		current_group_size = update_group_size(prev_gap);
		check_messages();	
	}
}

uint8_t roll_that_sigmoid(int16_t group_size)
{
	theta = (theta + 0.1);
	if(theta>2.0){ theta=0.0; tau=(tau+1)%10;}
	
	double sig_value = 1/(1+exp(theta*(tau-group_size)));
	//double sig_value = powf(M_E, theta);
	uint8_t random_byte_h = rand_byte();
	uint8_t random_byte_l = rand_byte();
	uint16_t random_short = ((uint16_t)random_byte_l | (((uint16_t)random_byte_h)<<8));
	uint16_t int_sig = ((uint16_t)(sig_value * 65535.0));
	int16_t int_theta = (int16_t)(theta*1000);
	printf("int_sig: %hu, int_theta: %hd, tau: %hd\r\n",int_sig,int_theta, tau);
	//printf("sig_val: %hu, rand_val: %hu, rand_byte_h: %hhu, rand_byte_l: %hhu\r\n",(uint16_t)sig_value*65535,random_short, random_byte_h, random_byte_l);
	if(random_short <= int_sig) return 1;
	return 0;
}

void check_votes()
{
	if(!collaborating)
	{
		if(yes_count*2>=current_group_size)
		{
			char* msg = "GO";
			ir_broadcast(msg,2);
			collaborative_task();
			//Do collab stuff.
		}
		yes_count=0;
	}
	schedule_task(HEART_RATE, check_votes, NULL);
}

void collaborative_task()
{	
	collaborating=1;
	set_rgb(150,200,0);	
	delay_ms(3000);
	set_rgb(0,0,0);	
	collaborating=0;
	reset_experiment();
}

void reset_experiment(){
	while(check_for_new_messages()>0) global_rx_buffer.read=1; //Throw out any messages we got.
	yes_count=0;
}

void broadcast_heartbeat()
{
	if(!collaborating)
	{
		set_rgb(200,0,0);
		char* msg;
		if(roll_that_sigmoid(current_group_size))
		{
			 msg = "<3Y";
			 yes_count+=1;
		}	    
		else msg = "<3N";
		ir_broadcast(msg, 3);
		set_rgb(0,0,0);
	}
	schedule_task(HEART_RATE, broadcast_heartbeat, NULL);		
}

/* 
 * This function traverses the group list, adding time_to_add to the age of each item.
 * If, after adding, the ms_age is greater than the timeout, that item is removed from the list.
 * Otherwise, we increase our group size by one.
 */
uint16_t update_group_size(uint16_t time_to_add)
{
	group_item* gi=group_root;
	uint16_t group_size = 0;
	do 
	{
		if(gi==group_root) group_size++;
		else
		{
			gi->ms_age+=time_to_add;
			if(gi->ms_age>GROUP_MEMBERSHIP_TIMEOUT)
			{
				group_item* temp = gi;
				gi->prev->next=gi->next;
				gi->next->prev=gi->prev;
				gi=gi->next;
				free(temp);
			}
			else
			{
				group_size++;
			}
		}
	} while (gi!=group_root);
	return group_size;
}

// If the senderID is already in our group, this function resets its age to 0.
// Otherwise, this function adds it to the group list.
void add_group_member(uint16_t senderID)
{
	group_item* gi=group_root;
	do
	{
		if(senderID==gi->ID)
		{
			gi->ms_age=0;
			return;	
		}		
		gi=gi->next;	
	}
	while(gi!=group_root);
	
	group_item* new_node = (group_item*)malloc(sizeof(group_item));
	new_node->ms_age=0;
	new_node->ID = senderID;
	
	new_node->prev = gi->prev;
	gi->prev->next = new_node;
	gi->prev = new_node;
	new_node->next = gi;
	current_group_size++;
}

void set_tau(int16_t t)
{
	tau = t;
}

void set_theta(double th)
{
	theta = th;
}

void check_messages()
{
	if(check_for_new_messages()==1)
	{	
		/* Do whatever you want with incoming message, stored in array
		 * global_rx_buffer.buf[i] from i=0 to i<global_rx_buffer.data_len
		 */
		char tempStr[64];
		memcpy(tempStr, global_rx_buffer.buf, global_rx_buffer.data_len);
		tempStr[global_rx_buffer.data_len] = NULL;
		uint16_t senderID = global_rx_buffer.sender_ID;
		if(strcmp(tempStr,"<3Y")==0||strcmp(tempStr,"<3N")==0)
		{
			add_group_member(senderID);
			if(tempStr[2]=='Y') yes_count++;
		}
		else if(strcmp(tempStr,"GO")==0)
		{
			char* msg = "GO";
			ir_broadcast(msg,2);
			collaborative_task();
		}
		global_rx_buffer.read = 1;
	}
	if(rnb_updated==1)
	{
		//do stuff with last_good_rnb
		rnb_updated=0;
	}
}

void color_cycle()
{
	for(uint16_t i=0;i<=360;i++)
	{
		set_hsv(i, 255, 10);
		delay_ms(10);
	}
}

