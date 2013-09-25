#include "DefaultPrograms/DropletAnts/DropletAnts.h"

DropletAnts::DropletAnts(ObjectPhysicsData *objPhysics) 
	: IDroplet(objPhysics) 
{ return; }

DropletAnts::~DropletAnts() { return; }

void DropletAnts::DropletInit()
{
	init_all_systems();
	set_rgb_led(0,0,0);
	get_rgb_sensor(&r, &g, &b);
	new_r = r;
	new_g = g;
	new_b = b;
	phase = 0;
	action = 0;
	turnDir = 1;

	turns_left = 10;

	cleared_buff = 0;
	group_size = 1;
	group_thresh = 15;
	gpx2 = group_thresh * 2;
	recv_ids = (droplet_id_type *)malloc(sizeof(droplet_id_type) * gpx2);
	uint16_t i;
	for(i = 0; i < gpx2; i++)
		recv_ids[i] = 0;
}

void DropletAnts::DropletMainLoop()
{
	if(is_moving() == 0 && is_rotating() == 0)
	{

		get_rgb_sensor(&new_r, &new_g, &new_b);

		// --------------- PHASE 0-1: searching for object-------------------
		if (phase < 2)
		{
			// found object
			if (new_r > 250)
			{
				phase += 1;
				set_rgb_led(new_r,new_g,new_b);
				move_steps(NORTH, 400);
				action = 0;
			}
			// on trail
			else if (new_r > 0) {
				if (action == 1) {
					if (new_r > r)
					{
						if ( (new_r - r) < 1 ) {
							rotate_steps(turnDir, 100);
						}
						else {
							rotate_steps(turnDir, 100/(new_r - r));
						}
					}
					else if (new_r < r)
					{
						rotate_steps(turnDir, 10*(r - new_r));
					}
					turnDir = -1 * turnDir;
					action = 0;
				}
				else {
					move_steps(NORTH, 150);
					action = 1;
				}
			}
			// just left trail
			else if (new_r == 0 && r > 0) {
				move_steps(SOUTH, 300);
				rotate_steps(TURN_CLOCKWISE, 200);
				action = 0;
			}
			// no trail or object
			else
			{
				if (action == 1) {
					rotate_steps(turnDir, 10*(new_g%10));
					turnDir = (2*(new_b%2)) - 1;
					action = 0;
				}
				else if (new_r == r && new_g == g && new_b == b)
				{
					move_steps((rand_byte()%6) + 1, 300);
					action = 1;
				}
				else {
					move_steps(NORTH, 300);
					action = 1;
				}
			}
		}
		
		// --------------- PHASE 2: orienting north-------------------
		if (phase == 2) {
			get_rgb_sensor(&new_r, &new_g, &new_b);

			// rotating
			if (action == 1) {

				// moving S
				if (new_g >= g) {
					// W 
					if (new_b > b) {
						rotate_steps(TURN_CLOCKWISE, 200);
					}
					// E
					else if (new_b < b) {
						rotate_steps(TURN_COUNTERCLOCKWISE, 200);
					}
					// stuck
					else {
						move_steps(SOUTH, 50);
					}
				}
				// N
				else {
					// W
					if (new_b > b) {
						rotate_steps(TURN_CLOCKWISE, 20);
					}
					// E
					else if (new_b < b) {
						rotate_steps(TURN_COUNTERCLOCKWISE, 20);
					}
					// wiggle if straight north
					else {
						move_steps(SOUTH, 50);
					}
				}
				action = 0;
			}

			// moving
			else {
				move_steps(NORTH, 100);
				action = 1;
			}
			turns_left--;
			if (turns_left == 0) {
				phase = 3;
			}
		}
		
		// --------------- PHASE 3: waiting for group-------------------
		if (phase == 3) {
			if (cleared_buff == 0) {
				// clear message buffers
				while (check_for_new_messages());
				cleared_buff = 1;
			}
			else {
				// send your droplet ID every 100ms
				if (check_timer(0)) {
					set_timer(100, 0);
					char send = '0';
					ir_broadcast(&send, sizeof(char));
				}
				// check for incoming messages
				else {
					if (check_for_new_messages()) {
						uint16_t i;
						uint8_t found = 0;

						char *data = (char *)malloc(sizeof(char));
						memcpy(data, global_rx_buffer.buf, sizeof(char));
						// given the go signal
						if ( data[0] == '1' ){
							char send = '1';
							ir_broadcast(&send, sizeof(char));
							phase = 4;
						}

						// check to see if you have met the sender droplet before
						for(i = 0; i < gpx2; i++)
						{
							if(global_rx_buffer.sender_ID == recv_ids[i])
							{
								found = 1;
								break;
							}
							if(recv_ids[i] == 0)
								break;
						}
						// New guy! Add it's id to the list
						if(!found)
						{
							uint16_t l = i < gpx2 ? i : 0;
							recv_ids[l] = global_rx_buffer.sender_ID;
							group_size++;
						}
					}

					// Check to see if we have reached group threshold
					if(group_size >= group_thresh) {
						char send = '1';
						ir_broadcast(&send, sizeof(char));
						phase = 4;
					}
				}
			}
		}

		// --------------- PHASE 4: moving with group-------------------
		if (phase == 4) {
			set_rgb_led(0, 0, 255);
			get_rgb_sensor(&new_r, &new_g, &new_b);

			// rotating
			if (action == 1) {

				// moving S
				if (new_g >= g) {
					// W 
					if (new_b > b) {
						rotate_steps(TURN_CLOCKWISE, 200);
					}
					// E
					else if (new_b < b) {
						rotate_steps(TURN_COUNTERCLOCKWISE, 200);
					}
					// stuck
					else {
						move_steps(SOUTH, 50);
					}
				}
				// N
				else {
					// W
					if (new_b > b) {
						rotate_steps(TURN_CLOCKWISE, 20);
					}
					// E
					else if (new_b < b) {
						rotate_steps(TURN_COUNTERCLOCKWISE, 20);
					}
					// if straight north
					else {
						move_steps(NORTH, 300);
					}
				}
				action = 0;
			}

			// moving
			else {
				move_steps(NORTH, 300);
				action = 1;
			}
		}

		// --------------- PHASE 5: DONE-------------------
		if (phase == 5) {
			set_rgb_led(0, 255, 0);
		}
		
		r = new_r;
		g = new_g;
		b = new_b;
	}

	// done moving when it hits the top wall
	else if (is_moving() == 1 && phase == 4 && new_g < 10) {
		phase = 5;
	}
}