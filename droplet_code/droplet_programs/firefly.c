#include "firefly.h"

void init()
{
    b = 2.;
    eps = .25;
    change_state(SHINE);
}

void loop()
{
	switch (state)
    {
    case SHINE:
        if (get_time() - state_start_time >= REF_DELAY)
        {
            led_off();
            change_state(LISTEN);
        }
        break;
    case LISTEN:
        if (listen_time >= BASE_LISTEN_TIME)
        {
            change_state(TRANSMIT);
        }
        listen_time += (double)(get_time() - last_update_time);
        last_update_time = get_time();
        break;
    case TRANSMIT:
        change_state(SHINE);
        break;
    }

}

void handle_msg(ir_msg* msg_struct)
{
	if (state == LISTEN) // Only parse messages when in LISTEN state
    {
        if(strcmp(msg_struct->msg, "<3") == 0)
        {
            double alpha = exp(b * eps);
            double beta = (alpha - 1) / (exp(b) - 1);
            listen_time = fmin(alpha * listen_time/BASE_LISTEN_TIME + beta, 1.) * BASE_LISTEN_TIME;
        }
    }
}

/*
* The function below is optional, and need not be included. If included,
* it should return '1' if the command_word is a valid command, and '0', 
* otherwise.
*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
//
//}

void change_state(State new_state)
{
    state_start_time = get_time();
    state = new_state;
    
	switch (state)
	{
    case SHINE:
        set_hsv(30, 255, 120);
        break;
    case LISTEN:
        //set_hsv(120, 255, 120);
        listen_time = 0.;
        last_update_time = state_start_time;
        break;
    case TRANSMIT:
        // Send a sync message
        //set_hsv(210, 255, 120);
        ir_send(ALL_DIRS, "<3", 2);
        break;
	}    
}