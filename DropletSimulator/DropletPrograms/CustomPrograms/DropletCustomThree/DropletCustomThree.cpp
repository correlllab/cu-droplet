#include "CustomPrograms/DropletCustomThree/DropletCustomThree.h"

const int circle_layers[] = { 1, 6, 12, 18, 24, 30 };


DropletCustomThree::DropletCustomThree(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{   return; }

DropletCustomThree::~DropletCustomThree() 
{  return; }

void DropletCustomThree::DropletInit()
{
	init_all_systems();
	state = WAIT;
	layer_num = 0;
	counter = 0; 
	is_set_rgb = false;
	move_rgb = false;
	wait_rgb = false;
	call_rgb = false;
	layer_set = false;
	first_set = false;
	mover = false;
	corner = false;
}

void DropletCustomThree::DropletMainLoop()
{
	switch(state)
	{
	case WAIT:
		if(!wait_rgb)
		{
			set_rgb_led(255,0,255);
			wait_rgb = true;
		}
		if(!is_moving())
		{
			move_steps((rand_byte() % 6) + 1, rand_byte() * 50);
		}
		if(!first_set)
		{
			uint8_t r, g, b;
			get_rgb_sensor(&r, &g, &b);
			if(r >200)
			{
				first_set = true;
				char msg[1];
				msg[0] = 'R';
				ir_broadcast(&msg[0], sizeof(msg));
				layer_num = 0;
				id = 0;
				mover = false;
				cancel_move();
				cancel_rotate();
				state = CALL;
				break;
			}
		}
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			char let2;
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			uint16_t idnum;
			switch(let)
			{
			case 'N':
				float dist, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &dist, &theta, &phi);
				if(dist < 20)
				{
					char msg;
					memset(&msg, 0, 1);
					msg = 'C';
					ir_broadcast(&msg, sizeof(msg));
				}
				break;
			case 'S':  
				memcpy(&idnum, &global_rx_buffer.buf[1], 2);
				if(idnum == get_droplet_id())
				{
					target = global_rx_buffer.sender_ID;
					id = (uint8_t)global_rx_buffer.buf[3];
					layer_num = (uint8_t)global_rx_buffer.buf[4];
					let2 = (uint8_t)global_rx_buffer.buf[5];
					if(let2 == 'C')
						corner = true;
					cancel_move();
					cancel_rotate();
					state = MOVE;
					break;
				}
				break;
			case 'R':
				first_set = true;
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
		if(id>0)
		{
			float dist, theta, phi;
			range_and_bearing(target, &dist, &theta, &phi);
			if(abs(theta) > 2.5f)
			{
				rotate_degrees(static_cast<int16_t>(theta));
			}
			if(dist>5)
			{
				move_steps(NORTH, 5);
			}
		}
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[1];
			if(let == 'L')
			{
				if(num == id)
				{
					call_rgb = false; 
					state = CALL;
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
			mover = false;
			if(layer_num == 0)
			{
				needed = 6;
			}
			else if(corner)
			{
				needed = 2;
			}
			else
			{
				needed = 1;
			}
		}
		if(!mover || layer_num>0)
		{
			char msg[1];
			memset(&msg[0], 0, 1);
			msg[0] = 'N';
			ir_broadcast(&msg[0], sizeof(msg));
		}
		
		while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint16_t send_id;
			uint8_t new_id, new_layer;
			switch(let)
			{
			case 'C':
				send_id  = global_rx_buffer.sender_ID;
				new_id = get_next_id();
				new_layer = layer_num + 1;
				char msg[6];
				memset(&msg[0], 0, 5);
				msg[0] = 'S';
				memcpy(&msg[1], &send_id, sizeof(uint16_t));
				memcpy(&msg[3], &new_id, sizeof(uint8_t));
				memcpy(&msg[4], &new_layer, sizeof(uint8_t));
				if((layer_num==0) || (corner==true && counter==0))
					msg[5] = 'C';
				else
					msg[5] = 0;
				ir_broadcast(&msg[0], sizeof(msg));
				mover = true;
				break;
			case 'F':
				mover = false;
				break;
			case 'D':
				counter++;
				mover = false;
				break;
			default:
				break;
			}
		}
		if(counter == needed)
		{
			char msg[3];
			memset(&msg[0], 0, 2);
			msg[0] = 'L';
			msg[1] = (id+1);
			msg[2] = layer_num+1;
			ir_broadcast(&msg[0], sizeof(msg));
			is_set_rgb = false;
			state = SET;
		}
		break;
	case MOVE:
		if(!move_rgb)
		{
			set_timer(60000,0);
			move_rgb = true;
			set_rgb_led(150,150,150);
		}
		if(!is_moving())
		{
			float dist, theta, phi;
			if(!range_and_bearing(target, &dist, &theta, &phi))
			{
				state = WAIT;
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
			if(dist <= 8)
			{
				rotate_degrees(static_cast<int16_t>(phi));
				char msg[1];
				memset(&msg[0], 0, 1);
				msg[0] = 'D';
				ir_broadcast(&msg[0], sizeof(msg));
				state = SET;
			}
			if(check_timer(0))
			{
				set_rgb_led(255,0,0);
				char msg;
				memset(&msg, 0, 1);
				msg = 'F';		
				ir_broadcast(&msg, (sizeof(msg)));
				cancel_move();
				cancel_rotate();
				state = WAIT;
			}
		}
		/*while(check_for_new_messages())
		{
			char let = (char)global_rx_buffer.buf[0];
			uint8_t num = (uint8_t)global_rx_buffer.buf[2];
			if(let=='L')
			{
				if(num==layer_num)
				{
					wait_rgb = false;
					cancel_rotate();
					cancel_move();
					state = WAIT;
				}
			}
		}*/
		break;
	}
}
uint8_t DropletCustomThree::get_next_id()
{
	switch(layer_num)
	{
	case 0:
		return counter + 1;
		break;
	case 1:
		if(counter>0)
			return id + circle_layers[layer_num] + (id-1) + counter;
		else
			return id + circle_layers[layer_num] + (id-1);
		break;
	/*case 2:
		if(corner)
		{
			if(counter>0)
				return id + circle_layers[layer_num];
		}*/
	default:
		break;
	}
}
