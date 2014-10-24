// SUPER AWESOME DROPLET API!

// Include all the appropriate files

// IR Sensor
uint8_t check_collisions()

// IR Communication
void ir_targeted_send(uint8_t dirs, char *data, uint16_t data_length, uint16_t target);
void ir_send(uint8_t dirs, char *data, uint8_t data_length);

// RGB LED
uint8_t get_red_led();
void set_red_led(uint8_t saturation);
uint8_t get_blue_led();
void set_blue_led(uint8_t saturation);
uint8_t get_green_led();
void set_green_led(uint8_t saturation);
void led_off();
void set_rgb(uint8_t r, uint8_t g, uint8_t b);
void set_hsv(uint16_t h, uint8_t s, uint8_t v);

// RGB Sensor
int8_t get_red_sensor();
int8_t get_green_sensor();
int8_t get_blue_sensor();

// Range and Bearing
broadcast_rnb_data();
collect_rnb_data(uint8_t power);

// Motors
uint8_t	move_steps(uint8_t direction, uint16_t num_steps);
void walk(uint8_t direction, uint16_t mm);
void stop();
uint8_t is_moving(void);

// Power
uint8_t cap_status();
int8_t leg_status(uint8_t leg);
uint8_t legs_powered();

// Utilities
uint8_t rand_byte()
uint16_t get_droplet_id();

// Scheduler
void delay_ms(uint16_t ms);
void delay_us(double us);
uint32_t get_32bit_time();

// Initialization
void droplet_reboot();
