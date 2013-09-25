#include "CustomPrograms/DropletCustomTwo/DropletCustomTwo.h"

const int guitar_rows[] = {
	/* Body */ 6, 7, 8, 9, 10, 9, 8, 7, 6, 7, 6, 5,
	/* Neck */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/* Head */ 3, 4, 3, 2, 0};

DropletCustomTwo::DropletCustomTwo(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ neighbors = (uint8_t*)malloc(sizeof(uint8_t) * 6);
	return; }

DropletCustomTwo::~DropletCustomTwo() { 
	free(neighbors);
	return; }

void DropletCustomTwo::DropletInit()
{
	init_all_systems();
	state = WANDER;
	ack = 1;
	is_set_rgb = false;
	wander_rgb = false;
	dock_rgb = false;
	align_rgb = false;
	call_rgb = false;
	first_set = false;
	first_search = false;
	mover = false;
}

void DropletCustomTwo::DropletMainLoop()
{
	switch(state)
	{
		case WANDER:
			if(!is_moving())
			{
				move_steps((rand_byte() % 6) + 1, rand_byte() * 50);
			}
			if(!wander_rgb){
				set_rgb_led(255,0,255);
				wander_rgb = true;
			}
			if(!first_set)
			{
				uint8_t r, g, b;
				get_rgb_sensor(&r, &g, &b);
				if(r >200)
				{
					first_set = true;
					guitar_id = 0;
					num_in_row = 1;
					row_num = 0;
					state = CALL;
					char msg = 'R';
					ir_send(0,&msg,1);
					cancel_move();
					cancel_rotate();
					break;
				}
			}
			while(check_for_new_messages())
			{
				char ltr = (char)global_rx_buffer.buf[0];
				uint8_t num = (uint8_t)global_rx_buffer.buf[1];
				switch(ltr)
				{
				case 'N':  
					float dist, theta, phi;
					range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
					if(dist < 20){
						char msg[2];
						memset(&msg[0], 0, 2);
						msg[0] = 'C';
						msg[1] = num;
						ir_broadcast(&msg[0], sizeof(msg));
					}
					break;
				case 'S':  
					uint16_t idnum;
					memcpy(&idnum, &global_rx_buffer.buf[1], sizeof(uint16_t));
					if(idnum == get_droplet_id())
					{
						target = global_rx_buffer.sender_ID;
						guitar_id = (uint8_t)global_rx_buffer.buf[3];
						slot_num = (uint8_t)global_rx_buffer.buf[4];
						row_num = get_row_num(guitar_id);
						num_in_row = get_num_in_row(guitar_id);
						state = DOCK;
						cancel_move();
						cancel_rotate();
						char msg[2];
						memset(msg, 0, 2);
						msg[0] = 'M';
						msg[1] = guitar_id;
						ir_broadcast(&msg[0], sizeof(msg));  
						break;
					}
					break;
				case 'R':
					first_set = true;
					break;
				case 'D':
					range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
					if(dist<20)
					{
						rotate_degrees(static_cast<int16_t>(theta+160));
						move_steps(NORTH, 30);
					}
					break;
				default:
					break;
				}
				global_rx_buffer.read = true;
			}
			break;
		case SET:
			if(!is_set_rgb)
			{
				set_rgb_led(0, 0, 255);
				is_set_rgb = true;
			}
			while(check_for_new_messages())
			{
				char let = global_rx_buffer.buf[0];
				uint8_t num = global_rx_buffer.buf[1];
				switch(let)
				{
				case 'N':
					if(num==guitar_id)
					{
						char msg[2];
						memset(&msg[0], 0, 2);
						msg[0] = 'A';
						msg[1] = guitar_id;
						ir_broadcast(&msg[0], sizeof(msg));
					}
					break;
				case 'L':
					if(num==guitar_id)
					{
						call_rgb = false;
						state = CALL;
					}
					break;
				}
				global_rx_buffer.read = true;
			}
			break;
		case DOCK:
			if(!dock_rgb)
			{
				set_timer(60000,0); 
				set_rgb_led(150,150,150);
				dock_rgb = true;
			}
			if(!is_moving())
			{
				float dist, theta, phi;

				if(!range_and_bearing(target, &dist, &theta, &phi))
				{
					state = WANDER;
					set_rgb_led(0, 0, 0);
					move_steps((rand_byte() % 6) + 1, rand_byte()*50);
					break;
				}
			
				if(!is_rotating() && abs(theta) > 2.5f)
				{
					rotate_degrees(static_cast<int16_t>(theta));
				}
				if(!is_rotating() && abs(theta) <= 2.5f)
				{
					move_steps(NORTH, static_cast<uint16_t>(dist));
				}
				if(dist <= 15)
				{
					rotate_degrees(static_cast<int16_t>(theta+90));
					move_steps(NORTH,5);
					char msg = 'D';
					ir_broadcast(&msg, 1);
					switch(slot_num)
					{
					case 0:
						if(phi<125 && phi>115)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					case 1:
						if(phi>175 && phi<185)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					case 2:
						if(phi>-125 && phi<-115)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					case 3:
						if(phi>-35 && phi<-25)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					case 4:
						if(phi>-5 && phi<5)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					case 5:
						if(phi>50 && phi<55)
						{
							cancel_move();
							cancel_rotate();
							state = ALIGN;
							break;
						}
						break;
					default:
						break;
					}
				}
			}
			while(check_for_new_messages())
			{
				char let = global_rx_buffer.buf[0];
				uint8_t num = global_rx_buffer.buf[3];
				if(let=='F')
				{
					if(num==guitar_id)
					{
						wander_rgb = false;
						cancel_move();
						cancel_rotate();
						state = WANDER;
						break;
					}
				}
				global_rx_buffer.read = true;
			}
			break;
		case ALIGN:
			if(!align_rgb)
			{
				set_rgb_led(255,0,0);
				align_rgb = true;
			}
			float dist, theta, phi;
			range_and_bearing(target, &dist, &theta, &phi);
			rotate_degrees(static_cast<int16_t>(theta));
			move_steps(NORTH, static_cast<uint16_t>(dist));
			if(dist<5)
			{
				cancel_move();
				rotate_degrees(static_cast<int16_t>(phi));
				if(phi<2 && phi>-2)
				{
					state = SET;
					char msg[4];
					msg[0] = 'F';
					memcpy(&msg[1], &target, sizeof(uint16_t));
					msg[3] = guitar_id;
					ir_broadcast(&msg[0], sizeof(msg));
					break;
				}

			}
			while(check_for_new_messages())
			{
				char let = global_rx_buffer.buf[0];
				uint8_t num = global_rx_buffer.buf[3];
				if(let=='F')
				{
					if(num==guitar_id)
					{
						wander_rgb = false;
						cancel_move();
						cancel_rotate();
						state = WANDER;
					}
				}
				global_rx_buffer.read = true;
			}
			break;
		case CALL:
			if(!call_rgb)
			{
				set_rgb_led(0,255,255);
				call_rgb = true;
			}
			if(!first_search)
			{
				find_neighbors(guitar_id, row_num, num_in_row);
				first_search=true;
			}
			if(!mover)
			{
				for(uint8_t i=0; i<6; i++)
				{
					if(neighbors[i]>0)
					{
						char msg[2];
						memset(&msg[0], 0, 2);
						msg[0] = 'N';
						msg[1] = neighbors[i];
						ir_broadcast(&msg[0], sizeof(msg));
					}
				}
			}
			while(check_for_new_messages())
			{
				char let = global_rx_buffer.buf[0];
				uint8_t num, orientation;
				uint16_t send_id;
				switch(let)
				{
				case 'C':
					send_id  = global_rx_buffer.sender_ID;
					num = global_rx_buffer.buf[1];
					for(uint8_t i=0; i<6; i++)
					{
						if(neighbors[i] == num)
							orientation = i;
					}
					char msg[5];
					memset(&msg[0], 0, 3);
					msg[0] = 'S';
					memcpy(&msg[1], &send_id, sizeof(uint16_t));
					memcpy(&msg[3], &num, sizeof(uint8_t));
					memcpy(&msg[4], &orientation, sizeof(uint8_t));
					ir_broadcast(&msg[0], sizeof(msg));
					mover = true;
					break;
				case 'A':
					num = global_rx_buffer.buf[1];
					for(uint8_t i=0; i<6; i++)
					{
						if(neighbors[i]==num)
						{
							slots_set++;
							neighbors[i] = 0;
						}
					}
					break;
				case 'F':
					mover = false;
					slots_set++;
					break;
				default:
					break;
				}
				global_rx_buffer.read=true;
			}
			if(slots_needed==slots_set)
			{
				char msg[2];
				memset(&msg[0], 0, 2);
				msg[0] = 'L';
				msg[1] = (guitar_id+1);
				ir_broadcast(&msg[0], sizeof(msg));
				is_set_rgb = false;
				state = SET;
				break;
			}
			break;
	}	
}

void DropletCustomTwo::find_neighbors(uint8_t guitar_id, uint8_t row_num, uint8_t num_in_row)
{
	/* there are 6 neighbors */
	/* left and right neighbors same for all rows */
	if(num_in_row==1)					
	{                    
		neighbors[4] = 0;  // no left neighbor
	}
	else
	{
		neighbors[4] = guitar_id - 1;
	}
	if(num_in_row==guitar_rows[row_num])  
	{
		neighbors[1] = 0; // no right neighbor
	}
	else
	{
		neighbors[1] = guitar_id +1;
	}
	/* the neck rows are different */
	if(row_num > 11 && row_num < 26){
		if(row_num%2==1)
		{
			neighbors[3] = guitar_id-2;
			neighbors[5] = guitar_id+2;
			if(num_in_row==2)
			{
				neighbors[2] = 0;
				if(row_num < 26)
				{
					neighbors[0] = 0;
				}
				else
				{
					neighbors[0] = guitar_id+3;
				}
			}
			else
			{
				neighbors[0] = guitar_id+3;
				neighbors[2] = guitar_id-1;
			}
		}
		else
		{
			neighbors[0] = guitar_id+2;
			neighbors[2] = guitar_id-2;
			if(num_in_row==1)
			{
				neighbors[5] = 0;
				if(row_num>13)
				{
					neighbors[3] = 0;
				}
				else
				{
					neighbors[3] = guitar_id-4;
				}
			}
			else
			{
				neighbors[3] = guitar_id-3;
				neighbors[5] = guitar_id+1;
			}
		}
	}
	else  // not a neck row
	{
		if(row_num==30)
		{
			neighbors[0]=0;
		}
		else
		{
			if(((guitar_rows[row_num]>guitar_rows[row_num+1]) && num_in_row==guitar_rows[row_num]))
			{
				neighbors[0] = 0;  // no upright neighbor
			}
			else
			{
				if(guitar_rows[row_num]>guitar_rows[row_num+1])
				{
					neighbors[0] = guitar_id+(guitar_rows[row_num]-num_in_row)+num_in_row;
				}
				else
				{
					neighbors[0] = guitar_id+(guitar_rows[row_num]-num_in_row)+num_in_row+1;
				}
			}
		}
		if(row_num>0)
		{
			if(((guitar_rows[row_num]>guitar_rows[row_num-1]) && num_in_row==guitar_rows[row_num]))
			{
				neighbors[2] = 0; // no lowright neighbor
			}
			else
			{
				if(guitar_rows[row_num]>guitar_rows[row_num-1])
				{
					neighbors[2] = guitar_id - (num_in_row) - (guitar_rows[row_num-1]-num_in_row);
				}
				else
				{
					neighbors[2] = guitar_id - (num_in_row) - (guitar_rows[row_num-1]-(num_in_row+1));
				}
			}
		}
		else
		{
			neighbors[2] = 0;
		}
		if(row_num>0)
		{
			if(((guitar_rows[row_num]>guitar_rows[row_num-1]) && num_in_row==1))
			{
				neighbors[3] = 0; // no lowleft neighbor
			}
			else
			{
				if(guitar_rows[row_num]<guitar_rows[row_num-1])
				{
					neighbors[3] = guitar_id - (num_in_row) - (guitar_rows[row_num-1]-num_in_row);
				}
				else
				{
					neighbors[3] = guitar_id - (num_in_row) - (guitar_rows[row_num-1]-(num_in_row-1));
				}
			}
		}
		else
		{
			neighbors[3] = 0;
		}
		if(row_num==30)
		{
			neighbors[5] = 0;
		}
		else
		{
			if(((guitar_rows[row_num]>guitar_rows[row_num+1]) && num_in_row==1))
			{
				neighbors[5] = 0; // no upleft neighbor
			}
			else
			{
				if(guitar_rows[row_num]<guitar_rows[row_num+1])
				{
					neighbors[5] = guitar_id+(guitar_rows[row_num]-num_in_row)+num_in_row;
				}
				else
				{
					neighbors[5] = guitar_id+(guitar_rows[row_num]-num_in_row)+num_in_row-1;
				}
			}
		}
	}
	uint8_t counter = 0;
	for(uint8_t i=0; i<6; i++)
	{
		if(neighbors[i]>0)
			counter++;
	}
	slots_needed = counter;
}
uint8_t DropletCustomTwo::get_row_num(uint8_t guitar_id) 
{
	uint8_t a = 0;
	uint8_t row = 0;
	uint8_t num = 1;
	while(a!=guitar_id)
	{
		a++;
		num++;
		if(guitar_rows[row]<num)
		{
			row++;
			num=1;
		}
	}
	return row;
}
uint8_t DropletCustomTwo::get_num_in_row(uint8_t guitar_id) 
{
	uint8_t a = 0;
	uint8_t row = 0;
	uint8_t num = 1;
	while(a!=guitar_id)
	{
		a++;
		num++;
		if(guitar_rows[row]<num)
		{
			row++;
			num=1;
		}
	}
	return num;
}