#include "DSimDroplet.h"

/* Having access to TrigArray in Droplet code is ONLY for implementing Range and Bearing.
 * This is dangerous bacause DSim is where dropletRelPos really gets defined even
 * though the variable is defined in DSimUtil.cpp. Notice that DSim.h isn't and
 * cannot be included in DSimDroplet.h but dropletRelPos is affected by it. 
 */
extern TrigArray *dropletRelPos; // Welcome to the DAINJA' ZONE!
extern std::vector<ObjectPositionData *> dropletPositions;

float prettyAngle(float ang)
{
	ang = fmodf ( ang, 360 );
	
    if(ang < -180)
    {
		ang += 360;
	}
    else if(ang > 180)
    {
		ang -= 360;
	}
	
    return ang;
}


// Constructors and Destructor
DSimDroplet::DSimDroplet(ObjectPhysicsData *objPhysics)
{
	this->objPhysics = objPhysics;
	actData		= (DropletActuatorData *)malloc(sizeof(DropletActuatorData));
	senseData	= (DropletSensorData *)malloc(sizeof(DropletSensorData));
	commData	= (DropletCommData *)malloc(sizeof(DropletCommData));
	compData	= (DropletCompData *)malloc(sizeof(DropletCompData));
	timeData	= (DropletTimeData *)malloc(sizeof(DropletTimeData));
	statData	= (DropletStatData *)malloc(sizeof(DropletStatData));
	this->global_rx_buffer.buf = (uint8_t *)malloc(sizeof(uint8_t) * IR_BUFFER_SIZE);
}
	
DSimDroplet::~DSimDroplet()
{
	free(objPhysics);
	free(actData);
	free(senseData);
	free(commData);
	free(compData);
	free(statData);

	objPhysics = NULL;
	actData = NULL;
	senseData = NULL;
	commData = NULL;
	compData = NULL;
}

DS_RESULT DSimDroplet::_InitPhysics(
	SimPhysicsData *simPhysics, 
	std::pair<float, float> startPosition,
	float startAngle)
{
	// Set up the bullet physics object for this Droplet
	btCollisionShape *colShape = simPhysics->collisionShapes->at(objPhysics->colShapeIndex);

	colShape->calculateLocalInertia(objPhysics->mass, objPhysics->localInertia);
	btTransform temp;
	temp.setIdentity();
	btVector3 tmpHolder1, tmpHolder2; // Don't really need this for anything besides calling getBoundingSphere
	colShape->getAabb(temp,tmpHolder1, tmpHolder2);

	float height = 0.f - tmpHolder1.z();

	// Set the Droplet's initial position in the world. Remember that bullet uses a Y-Up coord system
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(
		btScalar(startPosition.first), 
		btScalar(startPosition.second),
		btScalar(height + 0.01f))
	);

	// Set this Droplet's angular orientation using input parameter startAngle
	btQuaternion rotationQuat; 
	rotationQuat.setRotation(btVector3(
			btScalar(0.0), 
			btScalar(0.0),
			btScalar(1.0)
		), 
		btScalar(startAngle)
	);
	startTransform.setRotation(rotationQuat);

	// Add this Droplet as a rigid body in the physics sim
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		objPhysics->mass, 
		motionState, 
		colShape, 
		objPhysics->localInertia
	);
	rbInfo.m_friction = objPhysics->friction;
	rbInfo.m_linearDamping = DROPLET_LINEAR_DAMPING;		// Add a damping coefficient
	rbInfo.m_angularDamping = DROPLET_ANGULAR_DAMPING;

	btRigidBody *body = new btRigidBody(rbInfo);
	body->activate(true);
	body->setActivationState(DISABLE_DEACTIVATION);

	
	// Constrain movement to XY axes and rotations along the Z axis only
	//body->setLinearFactor(btVector3(1, 1, 1));
	//body->setAngularFactor(btVector3(0, 0, 1));

	simPhysics->dynWorld->addRigidBody(		
		body, 
		SimPhysicsData::_dynObjCollisionBM,	// Droplet Collision bitmask
		SimPhysicsData::_staticObjCollisionBM | 
			SimPhysicsData::_dynObjCollisionBM); // Droplet collides with these objects

	// Set the Droplet's ID
	objPhysics->_worldID = simPhysics->_physicsWorldObjCounter++;
	compData->dropletID = static_cast<droplet_id_type>(objPhysics->_worldID + DROPLET_ID_START);

	return DS_SUCCESS;
}

// Droplet Subsystems Setup Functions
void DSimDroplet::init_all_systems() 
{
	reset_motors();
	reset_rgb_led();
	reset_rgb_sensor();
	for(uint8_t i = 0; i < NUM_COMM_CHANNELS; i++)
		reset_ir_sensor(i);

	// Initialize variables
	commData->sendActive = false;
	compData->capStatus = 0;
	msg_return_order = OLDEST_MSG_FIRST;
    timeData->time_since_start = 0;
}

void DSimDroplet::droplet_reboot(void)
{
	init_all_systems ();
}

void DSimDroplet::reset_rgb_led() 
{
	actData->rOut = 250;
	actData->gOut = 250;
	actData->bOut = 250;
}


void DSimDroplet::reset_ir_sensor(uint8_t sensor_num) 
{
	if(sensor_num >= 0 && sensor_num < NUM_COMM_CHANNELS)
	{
		memset(commData->commChannels[sensor_num].inBuf, 0, IR_BUFFER_SIZE);
		memset(commData->commChannels[sensor_num].outBuf, 0, IR_BUFFER_SIZE);
		commData->commChannels[sensor_num].inMsgLength = 0;
		commData->commChannels[sensor_num].outMsgLength = 0;
		commData->commChannels[sensor_num].lastMsgInTimestamp = 0;
		commData->commChannels[sensor_num].lastMsgOutTimestamp = 0;
	}
}

void DSimDroplet::reset_rgb_sensor() 
{
	senseData->rIn = 0;
	senseData->gIn = 0;
	senseData->bIn = 0;
}

void DSimDroplet::reset_motors() 
{
	actData->moveTimeRemaining  = 0;
	actData->_oscillator        = true;
}

droplet_id_type DSimDroplet::get_droplet_id()
{
	return compData->dropletID;
}

uint8_t DSimDroplet::rand_byte(void)
{
	return static_cast<uint8_t>(255 * ((float)rand() / RAND_MAX));
}


// Droplet Motion Subsystem Functions
uint32_t DSimDroplet::rotate_degrees(int16_t deg)
{
    uint32_t retval = cancel_move();

	deg = static_cast<int16_t>(prettyAngle(static_cast<float>(deg)));
	if(deg > 0)
	{
		move_duration(TURN_COUNTERCLOCKWISE, ROTATE_RATE_MS_PER_DEG * deg);
	}
	else if(deg < 0)
	{
		move_duration(TURN_CLOCKWISE, -1 * ROTATE_RATE_MS_PER_DEG * deg);
	}

    return retval;
}

uint32_t DSimDroplet::move_duration(move_direction dir, uint32_t duration)
{
    uint32_t retval = cancel_move();
	actData->moveTimeRemaining = static_cast<float>(duration);
	actData->currMoveDir = dir;

    return retval;
}

uint32_t DSimDroplet::move_steps(move_direction dir, uint16_t num_steps)
{
    return move_duration ( dir, num_steps * WALK_STEP_TIME );
}

uint8_t DSimDroplet::is_moving(move_direction *dir)
{
    uint8_t retval = 0;
    if( actData->moveTimeRemaining > 0.f )
    {
        if ( dir != NULL )
            *dir = actData->currMoveDir;

        retval = 1;
    }

    return retval;
}

uint32_t DSimDroplet::cancel_move() 
{
	float tmp = 0.0f;
    if ( actData->moveTimeRemaining > 0 )
		tmp = actData->moveTimeRemaining;

    reset_motors();

    return static_cast<uint32_t>(tmp);
}

int8_t DSimDroplet::leg1_status()
{
	return compData->leg1Status;
}

int8_t DSimDroplet::leg2_status()
{
	return compData->leg2Status;
}

int8_t DSimDroplet::leg3_status()
{
	return compData->leg3Status;
}

int8_t DSimDroplet::leg_status(uint8_t leg)
{
	switch(leg)
	{
	case 1:
		return compData->leg1Status;
		break;

	case 2:
		return compData->leg2Status;
		break;

	case 3:
		return compData->leg3Status;
		break;

	default:
		return 2;
	}

	return 2;
}
int8_t DSimDroplet::cap_status(void)
{
	return compData->capStatus;
}

void DSimDroplet::set_rgb_led(uint8_t r, uint8_t g, uint8_t b)
{
	actData->rOut = r;
	actData->gOut = g;
	actData->bOut = b;
}

void DSimDroplet::set_red_led(uint8_t saturation)
{
	actData->rOut = saturation;
}

void DSimDroplet::set_green_led(uint8_t saturation)
{
	actData->gOut = saturation;
}

void DSimDroplet::set_blue_led(uint8_t saturation)
{
	actData->bOut = saturation;
}

uint8_t DSimDroplet::get_red_led(void)
{
	return actData->rOut;
}

uint8_t DSimDroplet::get_green_led(void)
{
	return actData->gOut;
}

uint8_t DSimDroplet::get_blue_led(void)
{
	return actData->bOut;
}

void DSimDroplet::led_off(void)
{
	reset_rgb_led();
}

void DSimDroplet::get_rgb_sensor(uint8_t *r, uint8_t *g, uint8_t *b)
{
	*r = senseData->rIn;
	*g = senseData->gIn;
	*b = senseData->bIn;
}

uint8_t DSimDroplet::get_red_sensor(void)
{
	return senseData->rIn;
}

uint8_t DSimDroplet::get_green_sensor(void)
{
	return senseData->gIn;
}

uint8_t DSimDroplet::get_blue_sensor(void)
{
	return senseData->bIn;
}

uint8_t DSimDroplet::ir_broadcast(const char *send_buf, uint8_t length)
{
	return ir_send(0, send_buf, length);
}

uint8_t DSimDroplet::ir_send(uint8_t channel, const char *send_buf, uint8_t length)
{
	// Channel 0 is reserved for broadcast
	if(channel >= NUM_COMM_CHANNELS)
		return 0;

	/* TODO : This section of code finds an empty channel to fill the send buffer in.
	 * Once directed communication is implemented, this block of code should be removed 
	 * and the channel input parameter should be used instead.
	 */
	unsigned int sendChannel;
	for(sendChannel = 0; sendChannel < NUM_COMM_CHANNELS; sendChannel++)
	{
		if(commData->commChannels[sendChannel].outMsgLength == 0)
			break;
	}

	// Reset the out comm buffer
	memset(commData->commChannels[sendChannel].outBuf, 0, IR_BUFFER_SIZE);
	//commData->commChannels[sendChannel].outMsgLength = 0;

	// Store the sending droplet's id as part of the message header
	memcpy(commData->commChannels[sendChannel].outBuf, &(compData->dropletID), sizeof(droplet_id_type));

	// Store the rest of the message header, then body
	uint16_t msgLength = length < IR_MAX_DATA_SIZE ? length : IR_MAX_DATA_SIZE;
	memcpy(&commData->commChannels[sendChannel].outBuf[sizeof(droplet_id_type)], send_buf, msgLength);
	commData->commChannels[sendChannel].outMsgLength = msgLength + sizeof(droplet_id_type);

	// Set Send to active
	commData->sendActive = true;
	return 1;
}

uint8_t DSimDroplet::check_for_new_messages(void)
{
	int newMsgCh = -1;
	for(int i = 0; i < NUM_COMM_CHANNELS; i++)
	{
		if(commData->commChannels[i].inMsgLength > 0)
		{
			newMsgCh = i;
			break;
		}
	}

	// If there is indeed a new message then we should copy it into the global_rx_buffer
	if(newMsgCh != -1)
	{
		global_rx_buffer.size = commData->commChannels[newMsgCh].inMsgLength;
		global_rx_buffer.data_len = global_rx_buffer.size - sizeof(droplet_id_type);
		global_rx_buffer.message_time = commData->commChannels[newMsgCh].lastMsgInTimestamp;
		memset(global_rx_buffer.buf, 0, IR_BUFFER_SIZE);
		memcpy(
			&global_rx_buffer.sender_ID,
			commData->commChannels[newMsgCh].inBuf,
			sizeof(droplet_id_type));
		memcpy( 
			global_rx_buffer.buf,
			&commData->commChannels[newMsgCh].inBuf[sizeof(droplet_id_type)],
			global_rx_buffer.data_len);

		// Clean out this message from the in buffer
		commData->commChannels[newMsgCh].inMsgLength = 0;
	}

	return (newMsgCh == -1) ? 0 : 1;
}

uint32_t DSimDroplet::get_32bit_time()
{
	return static_cast<uint32_t>(timeData->time_since_start);
}

uint8_t DSimDroplet::range_and_bearing(uint16_t partner_id, float *dist, float *theta, float *phi)
{
	float angle;
	unsigned int partner_world_id = static_cast<unsigned int>(partner_id - DROPLET_ID_START);
	unsigned int data_id_diff = this->objPhysics->_worldID - this->objPhysics->_dataID;
	unsigned int partner_data_id = partner_world_id - data_id_diff;

	// Get distance and relative angle
	if(dropletRelPos->GetData(
		this->objPhysics->_dataID, 
		partner_data_id, 
		dist, 
		&angle) != DS_SUCCESS)
	{ 
		return 0;
	}
	angle *= 180 / SIMD_PI; // Radians to Degrees

	//START JOHN ADD DIST FIX HACK?
	float xDist = (dropletPositions[this->objPhysics->_dataID]->posX - dropletPositions[partner_data_id]->posX);
	float yDist = (dropletPositions[this->objPhysics->_dataID]->posY - dropletPositions[partner_data_id]->posY);
	float zDist = (dropletPositions[this->objPhysics->_dataID]->posZ - dropletPositions[partner_data_id]->posZ);
	*dist = sqrtf(xDist*xDist+yDist*yDist+zDist*zDist);
	//END JOHN ADD DIST FIX HACK

	// Find Theta
	float tauRX;
	if(dropletPositions[this->objPhysics->_dataID]->rotZ < 0)
		tauRX = dropletPositions[this->objPhysics->_dataID]->rotA * -180 / SIMD_PI;
	else
		tauRX = dropletPositions[this->objPhysics->_dataID]->rotA * 180 / SIMD_PI;
	*theta = prettyAngle(angle - tauRX);

	// Find Phi
	float tauTX;
	if(dropletPositions[partner_data_id]->rotZ < 0)
		tauTX = dropletPositions[partner_data_id]->rotA * -180 / SIMD_PI;
	else
		tauTX = dropletPositions[partner_data_id]->rotA * 180 / SIMD_PI;
	*phi = prettyAngle(tauTX - tauRX); 

	return 1;
}

void DSimDroplet::DropletInit(void) { return; }
void DSimDroplet::DropletMainLoop(void) { return; }
