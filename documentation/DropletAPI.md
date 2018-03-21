## Global Settings/Symbols  
  These are located at the top of (../include/droplet_base.h).
    
  __AUDIO_DROPLET__  
    Should only be #define'd if you are programming an Audio Droplet.
    
  __SYNCHRONIZED__  
      If this is #define'd here, it will turn on the firefly synchronization subsystem.
      At the cost of each Droplet sending a very short message every few seconds, the Droplets will have their real time clocks synchronized such that, if they all have code to perform some action at, for example, `get_time()%1000`, they will all do that action simultaneously.

## General Guidelines  
  Due primarily to memory limitations, I strongly encourage you to use the size-explicit
  data types provided, as you will see throughout the Droplets code:  `uint8_t`, `int8_t`, `uint16_t`, `int16_t`, `uint32_t`, `int32_t`

## Debugging & `printf`
  If you have an FTDI cable set up with a serial connection to a Droplet, you can talk to it! See (../../getting_started.md)  for details on getting there.
  The Droplet will respond to the commands listed in (../src/serial_handler.c), and you can add your own commands by uncommenting and populating the user_handle_command function seen in the template code. Everything up to the first space character is considered the command_word, and everything after that is considered the command_args. You can use the code in serial_handler.c as a template for writing your own command handlers. Remember that the string library expects (and gives) null-terminated strings. 
  Be sure that user_handle_command returns 1 if the command_word is valid, and 0 otherwise.

  To have a Droplet talk back to you, use printf! It works as normal, but is much more sensitive to format chars than normal C. More specifically: all of the variables in your format string for printf will total a certain number of bytes. If the arguments you pass printf after the format string do not add up to the correct number of bytes, it can cause insidious stack-corruption bugs. Most commonly, this occurs when a variable is, say, 8 bits long and your format string specifices a two-byte-long type. 
  
  Brief primer on format strings for the different types:  
    `%[C1]{C2}`  
    `C1`  
      Optional. If not included, indicates the default size for the type specific by C2. For ints, both signed and unsigned, the default size is two bytes. If 'h', size is one byte. If 'l', size is 4 bytes.        
    `C2`  
      Indicates the variable type.  
        `%d` : signed int  
        `%u` : unsigned int  
        `%f` : float  
        `%x` : lower-case hex  
        `%X` : upper-case hex  
    (So use `%hu` for a `uint8_t`, `%ld` for an `int32_t`, `%u` for a `uint16_t`, and `%f` for a `float`).
    
    Note: For consistency with Droplets' labels, I strongly recommend you always print out Droplet IDs with `%04X`.

  Also: you may see `printf_P(PSTR("some format string"), ...)` in the code. This changes the `printf()` to store the string argument in program memory instead of data memory, since the former is much more spacious. In general, users shouldn't need to worry about this, but if your code gets long and complex, and you start running low on memory, it can help to do this yourself.

## Functions

### Communication
  These two functions are used for sending messages to other Droplets. If these functions return '0', it means that your message did not get sent, probably because the ir equipment in at least one of the directions you wanted to send in was still busy sending the previous message you asked to be sent. Otherwise it returns '1'.

  `dir_mask`  
    A dir_mask as described in the IR Directions section at the bottom of this document. If you don't know what direction you want to communicate in, you should use `ALL_DIRS`.
  `data`  
    A char array which contains the message to be sent. Need not be null terminated.  
  `data_length`  
    The number of chars in data. (ie, the size of the array). This should not be greater than 40.  
  `target`
    The ID of the Droplet the message is targeted to. Only that Droplet will get this message.  

  Messages always have a seven-byte header, and take about 2.5ms per byte.  Thus, if your message's data length is 1, the message will take about 20ms. If its data length is 40, it will take about 120ms.
  
```C
uint8_t irSend(uint8_t dir_mask, char* data, uint8_t data_length);
uint8_t irTargetedSend(uint8_t dir_mask, char *data, uint16_t data_length, id_t target);
```

  For each message a Droplet receives, the handleMsg function gets called. This can only happen after the code in loop() has run, and so it's important not to let loop() take too long, or messages will get backed up. handle_msg has a single argument, msg_struct, which contains the received message and other infornmation:
  
  `arrival_time`  
    When the last byte of the message was received.
  `sender_ID`  
    The id of the Droplet which transmitted the message.
  `length`  
    The number of characters in the message.
  `wasTargeted`
    Whether or not the message was targeted at the receiver, or just a general broadcast.
  `msg`
    The address of the first byte in the message, or, equivalently, a pointer to the start of the message.
    Note: This is NOT a null-terminated string.

### Light
  These functions set the intensity of the red, green, and blue LEDs respectively.
  '0' turns an LED off, '255' is full brightness.
```C
void setRedLED(uint8_t intensity); 
void setGreenLED(uint8_t intensity);
void setBlueLED(uint8_t intensity);
```

  This function simply calls the three functions above with the values passed it. If r=g=b, the light will be white.
```C
void setRGB(uint8_t r, uint8_t g, uint8_t b);
```

   This function simply calls set_rgb, after performing a color conversion from hsv color space to rgb color space. For more information on HSV color space, see: (http://www.wikipedia.org/wiki/HSL_and_HSV)  
       h : The hue. Should be between 0 and 360.  
       s : Saturation. Should be between 0 (0%) and 255 (100%)  
       v :  Value, or Brightness. Should be beteen 0 (0%) and 255 (100%)
```C
void setHSV(uint16_t h, uint8_t s, uint8_t v);
```

  These functions are used to get the intensity that the red, green, or blue LED is currently set to.
```C 
uint8_t getRedLED(void);
uint8_t getGreenLED(void);
uint8_t getBlueLED(void);
```

### Color Sensing.
  This function returns the sensed red, green, and blue values. It is pass by reference: you should pass pointers to `int16_t`'s where thecolor measurement will be stored. For example:
```C
void getRGB(int16_t* r, int16_t* g, int16_t* b);
```
For Example:
```C
  int16_t r,g,b;
  getRGB(&r, &g, &b);
 ```

### Range and Bearing
  'rnb' is short for 'range and bearing', and getting this data is made a little bit more complicated because it requires two Droplets to do something in unison (one turning its IR lights on, the other making measurements). When you call `broadcastRnbData()` on a Droplet, it causes the Droplet do perform an an 'rnb broadcast': a carefully timed sequence of turning on its IR lights. Before it does so, it broadcasts a message so other Droplets know that this is going to happen. (If you call `irSend` right before `broadcastRnbData`, the thing you tried to send probably won't get out because broadcast_rnb_data will stomp all over your message) As a result of the rnb broadcast, every other Droplet nearby will get new measurements of range, bearing, and heading for the Droplet which performed the broadcast. In general, the 'standard' way to use this system is to have every Droplet periodically call `broadcastRnbData()`. (You don't want to do this /too/ frequently. Try around every 5 seconds.)
  
  When a Droplet gets a good range and bearing measurement, the measurement is queued in the background until the next time loop returns. Then, the user-defined `handleMeas` function will be called once for each queued measurement.
  
  Note: An rnb broadcast takes ~150ms.
```C
void broadcastRnbData(void); 
```


### Movement
  This function has the Droplet move in the specified direction for the specific number of 'steps'. There isn't a great way to map from steps to actual distances. Try 30-100 as a starting place.
  ```C
  uint8_t	moveSteps(uint8_t direction, uint16_t num_steps);
  ```

  Droplets can be calibrated for how far they move each step. If they have been, this function lets you specify how far you want the robot to move and converts the distance in mm to a number of steps.
  ```C 
  void walk(uint8_t direction, uint16_t mm);
  ```

  This function has the Droplet stop all movement.
  ```C
  void stopMove(void);
  ```
  This function returns '-1' (NOT '0') if the Droplet is not moving, and otherwise returns the direction the Droplet is moving in.
  ```C
  int8_t isMoving(void);
  ```
  
### Power
  The functions below can be used to check power levels on each leg, and on the capacitor.
  ```C
  uint8_t capStatus(void);
  int8_t legStatus(uint8_t leg);
  uint8_t legsPowered(void);
  ```

### Utilities
  These functions return random values:
  ```C
  uint8_t  randByte(void);  //from 0 to 255
  uint16_t randShort(void); //from 0 to 65535
  uint32_t randQuad(void);  //from 0 to 4294967295
  float    randReal(void);  //from 0.0 to 1.0
  float    randNorm(float mean, float stdDev); //normally-distributed random value with the specified properties.
  ```
  
  Every droplet has a unique, sixteen-bit ID number.
  This function returns the ID number of the Droplet which calls it.
  ```C
  id_t getDropletID(void);
  ```
  
  Restart the Droplet:
  ```C
  void dropletReboot(void);
  ```
  
### Time
  ```C
  void delayMS(uint16_t ms).
  uint32_t get_time(void); //returns time in ms since Droplet started.
  ```
  
  `scheduleTask` causes the Droplet to call the specified function.
  
  `time`: The number of ms in the future function will be called.
  `function`: The name of the function to be called. This function must take either no argument, or one argument which is two bytes wide. (ie, `uint16_t`). You'll get a warning unless you cast your function to the type `arg_func_t` or `noarg_func_t`, as appropriate. If function has an argument, you'll get a different warning, unless the type of that argument is `void*`. There's no way for `function` to return anything.
  `arg`: The argument you want to be passed to `function`, when it is called. If `function` has no arguments, use `NULL`. You'll get a warning unless you cast `arg` to type `void*`.
  
  Note that `scheduleTask` returns a `Task_t` pointer. This is a pointer to the one of the elements in the task queue; specifically the one which corresponds to the requested scheduled task. There are two reasons you might want this return value:
  If the returned `Task_t` pointer is null, it indicates that there was an error and the task was not scheduled. And, if you might want to `cancel' the scheduled task, before it is run, you can do that by calling `removeTask` and passing the `Task_t` pointer this function returns. 
  ```C
  volatile Task_t* scheduleTask(uint32_t time, flex_function function, void* arg);
  ```
  
  `schedulePeriodicTask` causes the Droplet to call the specified function periodically.
  The arguments and return value for `schedulePeriodicTask` are identical to those of `scheduleTask`, with one exception:
  
  `period`: The specified `function` will be called first `period` ms in the future, and then every `period` ms after that, forever. It is particularly important to store the returned `Task_t` pointer if you might want to stop periodically calling the task at some point.
  ```C
  volatile Task_t* scheduleTask(uint32_t period, flex_function function, void* arg);
  ```  
  
  `removeTask` removes a task from the list of scheduled tasks. The argument should be the `Task_t` pointer which the associated call to `scheduleTask` returned.
  ```C
  void removeTask(volatile Task_t*);
  ```  

## IR Directions
  For several different IR functions, we use a 'dir mask', a single byte (ie, uint8_t) which can mark each of the six directions as 'on' or 'off'. A few values are defined in the code to make using these dir_masks easier. There is a direction for each set of IR hardware. To help describe each direction unambiguously, imagine the top of a Droplet as a 12-hour clock face, with the arrow on the shell at 12 o'clock. With this, each direction has the hour position on the clock which corresponds with that direction.

| Dir Name  | Binary     | Hex  |
|:---------:|:----------:|:----:|
| DIR0      | 0b00000001 | 0x01 |
| DIR1      | 0b00000010 | 0x02 |
| DIR2      | 0b00000100 | 0x04 |
| DIR3      | 0b00001000 | 0x08 |
| DIR4      | 0b00010000 | 0x10 |
| DIR5      | 0b00100000 | 0x20 |
| ALL_DIRS  | 0b00111111 | 0x3F |
  ```
       _--------_
     /            \
    /DIR5     DIR0 \
   /                \
  | DIR4        DIR1 |
   \    	           /
    \DIR3      DIR2/
     \            /
       -_________-
  ```
  If you wanted a dir mask for directions 1 and 4, say, you could just write `(DIR1|DIR4)`.

## Motor Directions
  Unfortunately, the directions for this function are different than for the IR stuff. We didn't do this out of spite, but the directions are determined by where the legs and motors are positioned. This time, the direction is just a number (no fancy binary stuff, since the Droplet can only move in one direction at a time). Directions 0-5 are walking straight in a certain direction. Direction 6 is clockwise rotation. Direction 7 is counterclockwise rotation. Referring to a clock face as we did for the IR directions, we have:
  ```
        direction 0: 12 o'clock (forward)
        direction 1: 2 o'clock
        direction 2: 4 o'clock
        direction 3: 6 o'clock (backward)
        direction 4: 8 o'clock
        direction 5: 10 o'clock
  ```
  Note that, since one of their motors has been replaced with a speaker, Audio Droplets can only move in directions 0, 3, 6, and 7. They don't break if you tell them to move in direction 1, they just don't do anything.
