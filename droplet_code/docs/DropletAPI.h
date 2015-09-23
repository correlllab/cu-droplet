// SUPER AWESOME DROPLET API!

/* 
 *  IR (Infrared) Directions:
 *      For several different IR functions, we use a 'dir mask', a single byte
 *      (ie, uint8_t) which can mark each of the six directions as 'on' or 'off'. 
 *      A few values are defined in the code to make using these dir_masks easier. 
 *      To help describe what the direction is, imagine the top of a Droplet as
 *      a 12-hour clock face, with the arrow on the shell at 12 o'clock. With 
 *      this, each direction has the hour position on the clock which corresponds 
 *      with that direction.
 *          DIR0: 0b00000001 = 0x01     (1 o'clock)           _--------_
 *          DIR1: 0b00000010 = 0x02     (3 o'clock)         /            \
 *          DIR2: 0b00000100 = 0x04     (5 o'clock)        /DIR5     DIR0 \
 *          DIR3: 0b00001000 = 0x08     (7 o'clock)       /                \
 *          DIR4: 0b00010000 = 0x10     (9 o'clock)      | DIR4        DIR1 |
 *          DIR5: 0b00100000 = 0x20     (11 o'clock)      \    	           /
 *          ALL_DIRS: 0b00111111 = 0x3F                    \DIR3      DIR2/
 *                                                          \            /
 *                                                            -_________-
 *
 *      If you wanted a dir mask for directions 1 and 4, say, you could just write:
 *          (DIR1|DIR4)
 */

/*
 * This function returns a dir_mask as described above, where a position is '1' if 
 * a collision is detected in the corresponding direction, and '0' otherwise. Here,
 * a collision being detected indicates that there is some object within a cm or two
 * of the ir hardware indicated by that direction. 
 * For example, if you wanted the Droplet's light to turn red when a collision was 
 * detected in direction 3, you would want the following code in loop():
 *      if(DIR3&check_collisions()){
 *          set_rgb(255,0,0);
 *      }
 */
uint8_t check_collisions();

/*
 *      These two functions are used for communicating with other Droplets. If these 
 *  functions return '0', it means that your message did not get sent because the
 *  ir equipment in at least one of the directions you wanted to send in was still
 *  busy sending the previous message you asked to be sent. Otherwise it returns '1'.
 *
 *      Depending on the length of a message, it takes 15-80ms to send a message, 
 *  for each direction you want to send it in.
 *  
 *  dir_mask: a dir_mask as described above. If you don't know what direction you 
 *      want to communicate in, you can use ALL_DIRS.
 *  data: A char array which contains the message to be sent.
 *  data_length: The number of chars in data. (ie, the size of the array).
 *                  This should not be greater than 32.
 *  target: (only for ir_targeted_send) The ID of the Droplet the message is 
 *          targeted to. Only that Droplet will get this message.
 */
uint8_t ir_send(uint8_t dir_mask, char* data, uint8_t data_length);
uint8_t ir_targeted_send(uint8_t dir_mask, char *data, uint16_t data_length, uint16_t target);


/*
 * Functions below are used to set the intensity of the red, green, and blue
 * LEDs respectively. Range is 0-255. Setting an LED to 0 turns it off.
 */
void set_red_led(uint8_t intensity); 
void set_green_led(uint8_t intensity);
void set_blue_led(uint8_t intensity);

/*
 * These functions are used to get the intensity that the red, green, or blue
 * LED was last set to.
 */
uint8_t get_red_led();
uint8_t get_green_led();
uint8_t get_blue_led();
void led_off();

/*
 * This function simply calls set_red_led, set_green_led, and set_blue_led with
 * the values passed it. If r=g=b, the light will be white.
 */
void set_rgb(uint8_t r, uint8_t g, uint8_t b);
/*
 *  set_hsv simply calls set_rgb, after performing a color conversion
 *  from hsv color space to rgb color space. For more information on HSV
 * color space, see: wikipedia.org/wiki/HSL_and_HSV
 *  h:  The hue. Should be between 0 and 360.
 *  s:  Saturation. Should be between 0 (0%) and 255 (100%)
 *  v:  Value, or Brightness. Should be beteen 0 (0%) and 255 (100%)
 */
void set_hsv(uint16_t h, uint8_t s, uint8_t v);

/*
 *  You should pass this function pointers to uint16_ts where the
 *  color measurement will be stored. For example:
 *      uint16_t r,g,b;
 *      get_rgb_sensor(&r, &g, &b);
 *      //use r,g,b as you desire.
 */
get_rgb_sensor(uint16_t* r, uint16_t* g, uint16_t* b);

// Range and Bearing
/*
 *  'rnb' is short for 'range and bearing', and getting this data is made a little bit more
 *  complicated because it requires two Droplets to do something in unison (one turning its 
 *  IR lights on, the other making measurements). When you call broadcast_rnb_data() on a 
 *  Droplet, it causes the Droplet do perform what we call an rnb broadcast: a carefully timed
 *  sequence of turning on its IR lights. Before it does so, it broadcasts a message so other
 *  Droplets know that this is going to happen. (If you call ir_send right before
 *  broadcast_rnb_data, the thing you tried to send probably won't get out because
 *  broadcast_rnb_data will stomp all over your message) As a result of the rnb broadcast,
 *  every other Droplet nearby will get new measurements of range, bearing, and heading for 
 *  the Droplet which performed the broadcast. In general, the 'standard' way to use this 
 *  system is:
 *      Periodically have every Droplet call broadcast_rnb_data(). 
 *      You don't want to do this /too/ frequently. Try around every 5 seconds.
 *      In every Droplet's loop(), have:
 *          if(rnb_updated){
 *             //new data in last_good_rnb, ie.:
 *             last_good_rnb.id_number;
 *             last_good_rnb.range;
 *             last_good_rnb.bearing;
 *             last_good_rnb.heading;
 *             rnb_updated = 0; //Note! This line must be included for things to work properly.
 *          }
 *      So if Droplets A,B,C, and D all have this in their code, and Droplet A
 *      does an rnb broadcast, Droplets B,C, and D will all get new rnb data for Droplet A.
 */
broadcast_rnb_data();
collect_rnb_data(uint8_t power);

/*
 *  Unfortunately, the directions for this function are different than 
 *  for the IR stuff.We didn't do this out of spite, but the directions
 *  are determined by where the legs and motors are positioned.
 *  This time, the direction is just a number (no fancy binary stuff, 
 *  since the Droplet can only move in one direction at a time).
 *  Directions 0-5 are walking straight in a certain direction. 
 *  Direction 6 is clockwise rotation. Direction 7 is counterclockwise rotation. 
 *  Referring to a clock face as we did for the IR directions, we have:
 *      direction 0: 12 o'clock (forward)
 *      direction 1: 2 o'clock
 *      direction 2: 4 o'clock
 *      direction 3: 6 o'clock (backward)
 *      direction 4: 8 o'clock
 *      direction 5: 10 o'clock
 *  Note that, since one of their motors has been replaced with a speaker, 
 *  the audio droplets can only move in directions 0, 3, 6, and 7. They 
 *  don't break if you tell them to move in direction 1, they just don't 
 *  do anything.
 */
uint8_t	move_steps(uint8_t direction, uint16_t num_steps);
void walk(uint8_t direction, uint16_t mm);
void stop();

//This function returns -1 if the Droplet is not moving, or the direction 
//the Droplet is moving in otherwise.
int8_t is_moving(void);

// Power
uint8_t cap_status();
int8_t leg_status(uint8_t leg);
uint8_t legs_powered();

// Utilities
/*
 * This function returns a single, psuedorandom byte, uniformly distributed between 0 and 255 inclusive.
 */
uint8_t rand_byte();
/*
 * Every droplet has a unique, sixteen-bit ID number. 
 * This function returns the ID number of the Droplet which calls it.
 */
uint16_t get_droplet_id();

// Scheduler
void delay_ms(uint16_t ms);
void schedule_task(uint32_t time, (void *)fn_name, void *args); // maybe?

//This function returns the time in ms since the Droplet last powered on.
uint32_t get_time();

// Initialization
void droplet_reboot();

