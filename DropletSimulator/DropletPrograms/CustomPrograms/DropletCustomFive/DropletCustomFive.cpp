#include "CustomPrograms/DropletCustomFive/DropletCustomFive.h"

/* for smaller guitar: */
const int guitar_rows[] = {
        /* Body */ 3, 4, 5, 6, 5, 4, 5, 4, 3,
        /* Neck */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
        /* Head */ 3, 2, 0};

/* for larger guitar:
const int guitar_rows[] = {
	/* Body  6, 7, 8, 9, 10, 9, 8, 7, 6, 7, 6, 5,
	/* Neck  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/* Head  3, 4, 3, 2, 0}; */

DropletCustomFive::DropletCustomFive(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{   neighbors = (uint8_t*)malloc(sizeof(uint8_t) * 6);
	return; }

DropletCustomFive::~DropletCustomFive() { 
	free(neighbors);
		return; }

void DropletCustomFive::DropletInit()
{
	init_all_systems();

	state = WANDER;
	slots_set = 0;
	first_set = false;
	first_search = false;
	wander_rgb = false;
	call_rgb = false;
	dock_rgb = false;
	align_rgb = false;
	is_set_rgb = false;
	clock_wise = false;
	preset_rgb = false;
}

void DropletCustomFive::DropletMainLoop()
{
	switch(state)
	{
	case WANDER:
		if(!wander_rgb)
		{
			set_rgb_led(255,0,255);
			wander_rgb = true;
		}
		if(!is_moving())
		{
			move_steps((rand_byte() % 6) + 1, rand_byte() * 50);
		}
		if(!first_set)
		{
			uint8_t r, g, b;
			get_rgb_sensor(&r, &g, &b);
			if(r > 200)
			{
				first_set = true;
				char msg = 'R';
				ir_broadcast(&msg, sizeof(msg));
				cancel_move();
				cancel_rotate();
				guitar_id = 0;
				row_num = 0;
				num_in_row = 1;
				state = CALL;
				break;
			}
		}
		while(check_for_new_messages())
		{
			char let = global_rx_buffer.buf[0];
			uint8_t num;
			uint16_t idnum;
			switch(let)
			{
			case 'V':
				float dist, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				num = global_rx_buffer.buf[1];
				if(dist<20 && dist>10)
				{
					char msg[2];
					memset(&msg[0], 0, 2);
					msg[0] = 'C';
					msg[1] = num;
					ir_broadcast(&msg[0], sizeof(msg));
				}
				break;
			case 'S':
				memcpy(&idnum, &global_rx_buffer.buf[1], 2);
				if(idnum == get_droplet_id())
				{
					target = global_rx_buffer.sender_ID;
					slot_num = (uint8_t)global_rx_buffer.buf[3];
					guitar_id = (uint8_t)global_rx_buffer.buf[4];
					cancel_move();
					cancel_rotate();
					state = DOCK;
					break;
				}
				break;
			case 'R':
				first_set = true;
				break;
			case 'D':
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				if(dist<30)
				{
					rotate_degrees(static_cast<int16_t>(theta+160));
					move_steps(NORTH, 30);
				}
				break;
			default:
				break;
			}
			global_rx_buffer.read=true;
		}
		break;
	case DOCK:
		if(!dock_rgb)
		{
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
			if(dist <= 20)
			{
				if(!clock_wise) // rotate around target counter clockwise
				{
					rotate_degrees(static_cast<int16_t>(theta-90));
					move_steps(NORTH, 2);
					char msg = 'D';
					ir_broadcast(&msg, 1);
					if(theta>85)
					{
						switch(slot_num)
						{
							case 0:
								if(phi < -75 && phi > -180)
								{
									cancel_move();
									cancel_rotate();
									rotate_degrees(static_cast<int16_t>(-180));
									clock_wise = true;
									break;
								}
								if(phi<-59 && phi>-61)
								{
									cancel_move();
									cancel_rotate();
									state = ALIGN;
									break;
								}
								break;
							case 1:
								if(phi < -75 && phi > -180)
								{
									cancel_move();
									cancel_rotate();
									rotate_degrees(static_cast<int16_t>(-180));
									clock_wise = true;
									break;
								}
								if(phi>-1 && phi<1)
								{
									cancel_move();
									cancel_rotate();
									state = ALIGN;
									break;
								}
								break;
							case 2:
								if(phi>59 && phi<61)
								{
									cancel_move();
									cancel_rotate();
									state = ALIGN;
									break;
								}
								break;
							case 3:
								if(phi<121 && phi>119)
								{
									cancel_move();
									cancel_rotate();
									state = ALIGN;
									break;
								}
								break;
							case 4:
								if((phi>179 && phi<181)||(phi>-181 && phi<-179))
								{
									cancel_move();
									cancel_rotate();
									state = ALIGN;
									break;
								}
								break;
							case 5:
								if(phi>-121 && phi<-119)
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
				else // rotate around target clockwise 
				{
					rotate_degrees(static_cast<int16_t>(theta+90));
					move_steps(NORTH, 2);
					char msg = 'D';
					ir_broadcast(&msg, 1);
					if(theta<-85)
					{
						switch(slot_num)
						{
						case 0:
							if(phi < -30 && phi > -75)
							{
								cancel_move();
								cancel_rotate();
								rotate_degrees(static_cast<int16_t>(180));
								clock_wise = false;
								break;
							}
							if(phi<121 && phi>119)
							{
								cancel_move();
								cancel_rotate();
								state = ALIGN;
								break;
							}
							break;
						case 1:
							if(phi < -50 && phi > -75)
							{
								cancel_move();
								cancel_rotate();
								rotate_degrees(static_cast<int16_t>(180));
								clock_wise = false;
								break;
							}
							if((phi>177 && phi<181)||(phi<-177 && phi>-181))
							{
								cancel_move();
								cancel_rotate();
								state = ALIGN;
								break;
							}
							break;
						case 2:
							if(phi>-121 && phi<-119)
							{
								cancel_move();
								cancel_rotate();
								state = ALIGN;
								break;
							}
							break;
						case 3:
							if(phi>-31 && phi<-29)
							{
								cancel_move();
								cancel_rotate();
								state = ALIGN;
								break;
							}
							break;
						case 4:
							if(phi>-1 && phi<1)
							{
								cancel_move();
								cancel_rotate();
								state = ALIGN;
								break;
							}
							break;
						case 5:
							if(phi>59 && phi<61)
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
			}
		}
		if(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			switch(let)
			{
			case 'A':
				if(num == guitar_id)
				{
					wander_rgb = false;
					state = WANDER;
				}
			case 'F':
				if(num == guitar_id)
				{
					wander_rgb = false;
					state = WANDER;
				}
				break;
			case 'T':
				float dist, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				if(dist<5)
				{
					move_steps(SOUTH, 5);
					if(clock_wise)
					{
						cancel_move();
						cancel_rotate();
						clock_wise = false;
						rotate_degrees(static_cast<int16_t>(theta+165));
						move_steps(NORTH, 20);
						break;
					}
					else
					{
						cancel_move();
						cancel_rotate();
						clock_wise = true;
						rotate_degrees(static_cast<int16_t>(theta+165));
						move_steps(NORTH, 20);
						break;
					}
				}
				break;
			default:
				break;
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
			if(phi < 2 && phi >-2)
			{
				char msg[4];
				msg[0] = 'F';
				memcpy(&msg[1], &target, sizeof(uint16_t));
				msg[3] = slot_num;
				ir_broadcast(&msg[0], sizeof(msg));
				state = PRESET;
				break;
			}
		}
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			switch(let)
			{
			case 'A':
				if(num == guitar_id)
				{
					wander_rgb = false;
					state = WANDER;
				}
			case 'F':
				if(num == guitar_id)
				{
					wander_rgb = false;
					state = WANDER;
				}
				break;
			default:
				break;
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
			find_empty_neighbors(guitar_id, row_num, num_in_row);
			first_search = true;
			set_timer(10000, 0);
		}
		if(check_timer(0))
		{
			for(uint8_t i=0; i<6; i++)
			{
				if((neighbors[i]>0) && (neighbors[i]<200))
				{
					/* this message calls needed droplets */
					char msg[2];
					memset(&msg[0], 0, 2);
					msg[0] = 'V';
					msg[1] = i;
					ir_broadcast(&msg[0], sizeof(msg));
				}
			}
		}
		else
		{
			for(uint8_t i=0; i<6; i++)
			{
				if((neighbors[i]>0))
				{
					/* timer hasn't gone off, we're list building and calling out
					   all possible neighbors to see which ones are already there */
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
			uint8_t num;
			uint16_t send_id;
			switch(let)
			{
			case 'C':
				send_id  = global_rx_buffer.sender_ID;
				num = (uint8_t)global_rx_buffer.buf[1];
				if((neighbors[num]>0) && (neighbors[num]<200))
				{
					char msg[5];
					memset(&msg[0], 0, 3);
					msg[0] = 'S';
					memcpy(&msg[1], &send_id, sizeof(uint16_t));
					msg[3] = num;
					msg[4] = neighbors[num];
					ir_broadcast(&msg[0], sizeof(msg));
					neighbors[num] = 255;
				}
				break;
			case 'F':
				memcpy(&send_id, &global_rx_buffer.buf[1], sizeof(uint16_t));
				num = (uint8_t)global_rx_buffer.buf[3];
				if(send_id == get_droplet_id())
				{
					set_rgb_led(255, 0, 0);
					call_rgb = false;
					neighbors[num] = 0;
					slots_set++;
				}
				break;
			case 'A':
				num = (uint8_t)global_rx_buffer.buf[1];
				for(uint8_t i=0; i<6; i++)
				{
					if((neighbors[i] > 0) && (neighbors[i] == num))
					{
						neighbors[i] = 0;
						slots_needed--;
						break;
					}
				}
				break;
			default:
				break;
			}
			global_rx_buffer.read = true;
		}
		if(slots_set == slots_needed)
		{
			char msg[2];
			memset(&msg[0], 0, 2);
			msg[0] = 'L';
			msg[1] = (guitar_id + 1);
			ir_broadcast(&msg[0], sizeof(msg));
			is_set_rgb = false;
			state = SET;
			break;
		}
		/*if((neighbors[1])==0 && (neighbors[2]==0) && (neighbors[3]==0) && (neighbors[4]==0) && (neighbors[5]==0))
		{
			char msg[2];
			memset(&msg[0], 0, 2);
			msg[0] = 'L';
			msg[1] = (guitar_id + 1);
			ir_broadcast(&msg[0], sizeof(msg));
			is_set_rgb = false;
			state = SET;
			break;
		}*/
		break;
	case PRESET:
		if(!preset_rgb)
		{
			set_rgb_led(0, 255, 0);
			preset_rgb = true;
		}
		set_rgb_led(0, 255, 0);
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			uint8_t num2;
			switch(let)
			{
			case 'S':
				num2 = (uint8_t)global_rx_buffer.buf[4];
				if(num2 == guitar_id)
				{
					set_rgb_led(255, 0, 0);
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
					row_num = get_row_num(guitar_id);
					num_in_row = get_num_in_row(guitar_id);
					call_rgb = false;
					state = CALL;
				}
				break;
			case 'N':
				if(num==guitar_id)
				{
					set_rgb_led(255, 0, 0);
					char msg[2];
					memset(&msg[0], 0, 2);
					msg[0] = 'A';
					msg[1] = guitar_id;
					ir_broadcast(&msg[0], sizeof(msg));
				}
				break;
			case 'D':
				float dist, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				if(dist<5)
				{
					char msg = 'T';
					ir_broadcast(&msg, 1);
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
			set_rgb_led(0,0,255);
			is_set_rgb = true;
		}
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			switch(let)
			{
			case 'N':
				if(num==guitar_id)
				{
					set_rgb_led(255, 0 , 0);
					is_set_rgb = false;
					char msg[2];
					memset(&msg[0], 0, 2);
					msg[0] = 'A';
					msg[1] = guitar_id;
					ir_broadcast(&msg[0], sizeof(msg));
				}
				break;
			case 'D':
				float dist, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				if(dist<5)
				{
					char msg = 'T';
					ir_broadcast(&msg, 1);
				}
				break;
			default:
				break;
			}
			global_rx_buffer.read = true;
		}
		break;
		
	}
}

void DropletCustomFive::find_empty_neighbors(uint8_t guitar_id, uint8_t row_num, uint8_t num_in_row)
{
	/* there are 6 neighbors */
	bool neck_row;
	/* for smaller guitar: */
	if((row_num > 8) && (row_num < 19))
	{
		neck_row = true;
	}
	else
	{
		neck_row = false; 
	}
	/* for larger guitar: 
	if((row_num > 11) && (row_num < 26))
		neck_row = true;
	else
		neck_row = false; */
	/* left and right neighbors same for all rows */
	if(!(num_in_row==1))					
	{                    
		neighbors[4] = guitar_id -1;  
	}
	else
	{
		neighbors[4] = 0; // no left neighbor
	}
	if(num_in_row==guitar_rows[row_num])  
	{
		neighbors[1] = 0; // no right neighbor
	}
	else
	{
		neighbors[1] = guitar_id + 1;
	}
	/* the neck rows are different */
	if(neck_row){
		if(row_num%2==0) // larger guitar change 0 to 1
		{
			neighbors[3] = guitar_id-2;
			neighbors[5] = guitar_id+2;
			if(num_in_row==2)
			{
				neighbors[2] = 0;
				if(row_num < 18)
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
				if(row_num>9)
				{
					neighbors[3] = 0;
				}
				else
				{
					neighbors[3] = guitar_id-3;
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
			neighbors[0] = 0;
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
			neighbors[5]=0;
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
	uint8_t count;
	count = 0;
	for(uint8_t i = 0; i<6; i++)
	{
		if(neighbors[i]>0)
			count++;
	}
	if(guitar_id == 3)
	{
		slots_needed = 2;
	}
	else
	{
	slots_needed = count;
	}
}


uint8_t DropletCustomFive::get_row_num(uint8_t guitar_id) 
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


uint8_t DropletCustomFive::get_num_in_row(uint8_t guitar_id) 
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