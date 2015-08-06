#include "droplet_programs/full_task_alloc.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
    state = NOTHING;
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
    uint8_t *commands = (uint8_t *)(msg_struct->msg);
    state = commands[my_index] - '0';
    printf("Got msg!\r\n\t");
    //for(uint8_t i=0;i<msg_struct->length;i++) printf("%c",msg_struct->msg[i]);
    printf("\r\nMy_index: %hu\r\n\tState: %hu\r\n",my_index,(uint8_t)state);
    stop_move();
    led_off();
    
    uint8_t dir = 1; // Because they can't actually move in dir = 1
    uint8_t num_steps = 1000;
    switch (state)
    {
        case WALK_FORWARD:
        dir = 0;
        break;
        
        case WALK_BACKWARD:
        dir = 3;
        break;
        
        case TURN_LEFT_SHORT:
        num_steps = 10;        
        case TURN_LEFT_LONG:
        dir = 7;
        break;
        
        case TURN_RIGHT_SHORT:
        num_steps = 10;
        case TURN_RIGHT_LONG:
        dir = 6;
        break;

        case LED_ON:
        set_blue_led(50);
        break;
               
    }
    
    if (dir != 1)
    {
        move_steps(dir, num_steps);
    }
}
