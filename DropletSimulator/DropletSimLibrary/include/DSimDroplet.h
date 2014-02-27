#pragma once

#ifndef _I_DROPLET
#define _I_DROPLET

#ifdef _WIN32
#include "inttypes.h"	// inttypes.h is included in UNIX systems
#else
#include <inttypes.h>
#endif

#include "btBulletDynamicsCommon.h"
#include "DSimGlobals.h"
#include "DSimDataStructs.h"
#include "DSimUtil.h"

#include <cstdlib>
#include <utility>
#include <time.h>
#include <vector>

/**
 * DSimDroplet models the behavior and state of and individual droplet.
 */

class DSimDroplet
{
private :

	ObjectPhysicsData *objPhysics;
	DropletActuatorData *actData;
	DropletSensorData *senseData;
	DropletCommData *commData;
	DropletCompData *compData;
	DropletTimeData *timeData;
	
	/** \name Simulator backend functions  
	 *  
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 */
	///@{
	/**
	 * Access physics data.
	 *
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 *
	 * \param [in,out]	pDroplet  	If non-null, the droplet.
	 * \param [in,out]	objPhysics	If non-null, the object physics.
	 */

	friend void AccessPhysicsData(DSimDroplet *pDroplet, ObjectPhysicsData **objPhysics)
	{
		*objPhysics = pDroplet->objPhysics;
	}

	/**
	 * Access actuator data.
	 *  
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 *
	 * \param [in,out]	pDroplet	If non-null, the droplet.
	 * \param [in,out]	actData 	If non-null, information describing the act.
	 */

	friend void AccessActuatorData(DSimDroplet *pDroplet, DropletActuatorData **actData)
	{
		*actData = pDroplet->actData;
	}

	/**
	 * Access sensor data.
	 *
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 * 
	 * \param [in,out]	pDroplet 	If non-null, the droplet.
	 * \param [in,out]	senseData	If non-null, information describing the sense.
	 */

	friend void AccessSensorData(DSimDroplet *pDroplet, DropletSensorData **senseData)
	{
		*senseData = pDroplet->senseData;
	}

	/**
	 * Access communications data.
	 *
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 * 
	 * \param [in,out]	pDroplet	If non-null, the droplet.
	 * \param [in,out]	commData	If non-null, information describing the communications.
	 */

	friend void AccessCommData(DSimDroplet *pDroplet, DropletCommData **commData)
	{
		*commData = pDroplet->commData;
	}

	/**
	 * Access component data.
	 *
	 * NOTE : These friend functions are meant for use by the simulator backend ONLY.
	 * DO NOT USE THESE IN ANY CODE WRITTEN FOR A CLASS DERIVED FROM DSimDroplet!!!
	 *
	 * \param [in,out]	pDroplet	If non-null, the droplet.
	 * \param [in,out]	compData	If non-null, information describing the component.
	 */

	friend void AccessCompData(DSimDroplet *pDroplet, DropletCompData **compData)
	{
		*compData = pDroplet->compData;
	}


	friend void AccessTimeData(DSimDroplet *pDroplet, DropletTimeData **timeData)
	{
		*timeData = pDroplet->timeData;
	}

protected :

	DropletStatData *statData;
	friend void AccessStatData(DSimDroplet *pDroplet, DropletStatData **statData)
	{
		*statData = pDroplet->statData;
	}
///@}
/** @name Subsystem setup functions.
 */
///@{
	/**
	 * Resets all systems.
	 */
	void init_all_systems(void);

	/**
	 * /todo Reboots the droplet. The DropletInit() function will be run again on next simulator step.
	 */
	void droplet_reboot(void);

	/**
	 * Resets the RGB LED.
	 */
	void reset_rgb_led(void);

	/**
	 * Resets the IR LED.
	 */
	void reset_ir_sensor(uint8_t sensor_num);

	/**
	 * Resets the RGB sensor.
	 */
	void reset_rgb_sensor(void);

	/**
	 * Resets the motors.
	 */
	void reset_motors(void);

	/**
	 * Resets the timers.
	 */
	void reset_timers(void);

	/**
	 * Gets droplet identifier.
	 *
	 * \return	The droplet identifier.
	 */
	droplet_id_type get_droplet_id(void);

	/**
	 * Gets a random number between 0 and 255 (inclusive).
	 *
	 * \return	The random number.
	 */
	uint8_t rand_byte(void);

///@}
/** @name Actuator subsystem functions.
 */
///@{
	/**
	 * Rotate duration.
	 *
	 * \param	direction	The direction (1 = CW, -1 = CCW, or use macros).
	 * \param	duration	The duration in ms.
	 */
	void rotate_duration(turn_direction direction, uint16_t duration);

	/**
	 * Rotate steps.
	 *
	 * \param	direction	The direction (1 = CW, -1 = CCW, or use macros).
	 * \param	num_steps The number of steps.
	 */
	void rotate_steps(turn_direction direction, uint16_t num_steps);

	/**
	 * Rotate degrees.
	 *
	 * \param	deg	The number of degrees (between -180 and +180) to rotate.
	 */
	void rotate_degrees(int16_t deg);

	/**
	 * Move duration.
	 *
	 * \param	direction	The direction (1 through 6, or use macros).
	 * \param	duration	The duration in ms.
	 */
	void move_duration(uint8_t direction, uint16_t duration);

	/**
	 * Move steps.
	 *
	 * \param	dir	The direction (1 through 6, or use macros).
	 * \param	num_steps The number of steps.
	 */
	void move_steps(uint8_t direction, uint16_t num_steps);

	/**
	 * Returns droplet movement status.
	 * 
	 * \return 0 if droplet is not moving.
	 * \return 1 through 6 depending on movement direction.
	 */
	move_direction is_moving(void);

	/**
	 * Returns droplet rotation status.
	 * 
	 * \return 0 if droplet is not rotating.
	 * \return 1 if droplet is rotating CW.
	 * \return -1 if droplet is rotating CCW.
	 */
	turn_direction is_rotating(void);

	/**
	 * Cancel move.
	 *
	 * \return number of steps taken
	 * \return duration 
	 */
	uint32_t cancel_move(void);

	/**
	 * Cancel rotate.
	 *
	 * \return number of steps taken
	 * \return duration
	 */
	uint32_t cancel_rotate(void);

///@}
/** @name RGB LED subsystem functions.
 */
///@{
	/**
	 * Sets a RGB value.
	 *
	 * \param	r	The uint8_t to process.
	 * \param	g	The uint8_t to process.
	 * \param	b	The uint8_t to process.
	 */
	void set_rgb_led(uint8_t r, uint8_t g, uint8_t b);

	/**
	 * Sets red saturation value.
	 *
	 * \param	saturation	The uint8_t to process.
	 */
	void set_red_led(uint8_t saturation);

	/**
	 * Sets green saturation value.
	 *
	 * \param	saturation	The uint8_t to process.
	 */
	void set_green_led(uint8_t saturation);

	/**
	 * Sets blue saturation value.
	 *
	 * \param	saturation	The uint8_t to process.
	 */
	void set_blue_led(uint8_t saturation);

	/**
	 * Gets saturation value.
	 *
	 * /returns The current red color saturation value for the rgb led
	 */
	uint8_t get_red_led(void);

	/**
	 * Gets saturation value.
	 *
	 * /returns The current green color saturation value for the rgb led
	 */
	uint8_t get_green_led(void);

	/**
	 * Gets saturation value.
	 *
	 * /returns The current blue color saturation value for the rgb led
	 */
	uint8_t get_blue_led(void);

	/**
	 * Turns the rgb LED off
	 */
	void led_off(void);

///@}
/** @name Power monitoring subsystem functions.
 */
///@{
	/**
	 * /deprecated Use leg_status(uint8_t leg) instead. 
	 * Returns droplet leg 1 status.
	 * 
	 * /return	1 if leg is on power.
	 * /return -1 if leg is on ground.
	 * /return 0 if leg is floating.
	 */
	int8_t leg1_status(void);

	/**
	 * /deprecated Use leg_status(uint8_t leg) instead. 
	 * Returns droplet leg 2 status.
	 * 
	 * /return	1 if leg is on power.
	 * /return -1 if leg is on ground.
	 * /return 0 if leg is floating.
	 */
	int8_t leg2_status(void);

	/**
	 * /deprecated Use leg_status(uint8_t leg) instead. 
	 * Returns droplet leg 3 status.
	 * 
	 * /return +1 if leg is on power.
	 * /return -1 if leg is on ground.
	 * /return 0 if leg is floating.
	 */
	int8_t leg3_status(void);

	/**
	 * Returns droplet leg status.
	 * 
	 * /param leg a value between 1 and 3, for each droplet leg. See hardware diagram for which value corresponds to which leg.
	 * /return +1 if leg is on power.
	 * /return -1 if leg is on ground.
	 * /return 0 if leg is floating.
	 * /return other value on error.
	 */
	int8_t leg_status(uint8_t leg);

	/**
	 * Returns droplet capacitor status. (Note : This function always returns 0 for now.)
	 * /return 0 if cap voltage within normal operating range (2.8v to 5v)
	 * /return +1 if cap voltage too high (>5v)
	 * /return -1 if cap voltage too low (<2.8v)
	 */
	int8_t cap_status(void);

///@}
/** @name RGB sensing subsystem functions.
 */
///@{
	/**
	 * Returns rgb values.
	 * 
	 * /return	0-255 for each rgb value.	
	 */
	void get_rgb_sensor(uint8_t *r, uint8_t *g, uint8_t *b);

	/**
	 * Returns and rgb sensor value.
	 * /return The brightness values read from the rgb sensor for red light.
	 */
	uint8_t get_red_sensor(void);

	/**
	 * Returns and rgb sensor value.
	 * /return The brightness values read from the rgb sensor for green light.
	 */
	uint8_t get_green_sensor(void);

	/**
	 * Returns and rgb sensor value.
	 * /return The brightness values read from the rgb sensor for blue light.
	 */
	uint8_t get_blue_sensor(void);

///@}
/** @name Communication subsystem functions.
 */
///@{
	uint8_t ir_send(uint8_t channel, const char *send_buf, uint8_t length);
	
	uint8_t ir_broadcast(const char *send_buf, uint8_t length);

	uint8_t check_for_new_messages(void);

	uint8_t range_and_bearing(uint16_t partner_id, float *dist, float *theta, float *phi);

///@}
/** @name Timer subsystem functions.
 */
///@{
	/**
	 * Set one of the 5 available timers.
	 * /param time the time to fire in ms
	 * /param index 0 to 4, for one of the available timers.
	 */
	uint8_t set_timer(uint16_t time, uint8_t index); // time in ms

	/**
	 * Check the status of available timers.
	 * /return 0 when the timer is on and hasn't fired yet. 1 if the
	 * timer is off or has fired after being set.
	 */
	uint8_t check_timer(uint8_t index);
///@}

public :

	msg_order msg_return_order; // See DSimGlobals.h NEWEST_MSG_FIRST and OLDEST_MSG_FIRST

	/// The global incoming message buffer for the droplet.
	/* TODO : Here global_rx_buffer.size, global_rx_buffer.data_len are 
	 * being used to store the same value, the length of the actual
	 * body of the message and not the whole message (including
	 * header). It is a bit confusing, fix it later.
	*/
	struct  
	{
			uint8_t *buf;
			uint16_t size;       // Stores size of msg + header                           

			uint8_t receivers_used;	/* set flags of the receiver numbers which 
									 * successfully received the message */

			uint16_t data_len;	// Stores size of data in buf, i.e. just msg size
			droplet_id_type sender_ID;	// to be copied from the header

			uint16_t message_time;

			uint8_t read;
	} global_rx_buffer;

	/**
	 * Constructor.
	 *
	 * \param [in,out]	objPhysics	If non-null, the object physics.
	 */
	DSimDroplet(ObjectPhysicsData *objPhysics);

	/**
	 * Destructor.
	 */
	virtual ~DSimDroplet();

	/**
	 * Initialises the initialise physics.
	 *
	 * \param [in,out]	simPhysics	If non-null, the simulation physics.
	 * \param	startPosition	  	The start position.
	 * \param	startAngle		  	The start angle.
	 *
	 * \return	.
	 */
	DS_RESULT _InitPhysics(
		SimPhysicsData *simPhysics,
		std::pair<float, float> startPosition,
		float startAngle);

	/**
	 * Droplet Simulator Function to be overridden.
	 */
	virtual void DropletInit(void);

	/**
	 * Droplet Simulator Function to be overridden.
	 */
	virtual void DropletMainLoop(void);

};

#endif