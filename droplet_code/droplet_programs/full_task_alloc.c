#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{

}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{

}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
    uint8_t my_index = get_droplet_ord(get_droplet_id()) - 100;
    uint8_t dir = 1; // Because they can't actually move in dir = 1
    uint8_t num_steps = 1000;
    switch (state)
    {
        WALK_FORWARD:
        dir = 0;
        break;
        
        WALK_BACKWARD:
        dir = 3;
        break;
        
        TURN_LEFT_SHORT:
        num_steps = 10;
        TURN_LEFT_LONG:
        dir = 7;
        break;
        
        TURN_RIGHT_SHORT:
        num_steps = 10;
        TURN_RIGHT_LONG:
        dir = 6;
        break;

        LED_ON:
        set_blue_led(50);
        break;
        
        NOTHING:
        led_off();
    }
    
    if (dir != 1)
    {
        if(is_moving() > 0)
        {
            stop_move();
        }
        led_off();
        move_steps(dir, num_steps)
    }
}
