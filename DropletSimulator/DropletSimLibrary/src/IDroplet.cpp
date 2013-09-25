#include "IDroplet.h"

/* Having access to TrigArray in Droplet code is ONLY for implementing Range and Bearing.
 * This is dangerous bacause DropletSim is where dropletRelPos really gets defined even
 * though the variable is defined in DropletUtil.cpp. Notice that DropletSim.h isn't and
 * cannot be included in IDroplet.h but dropletRelPos is affected by it. 
 */
extern TrigArray *dropletRelPos; // Welcome to the DAINJA' ZONE!
extern std::vector<GPSInfo *> dropletPositions;

float prettyAngle(float ang){
	ang = fmodf(ang,360);
	if(ang<-180){
		ang+=360;
	}else if(ang>180){
		ang-=360;
	}
	return ang;
}


// Constructors and Destructor
IDroplet::IDroplet(ObjectPhysicsData *objPhysics)
{
	this->objPhysics = objPhysics;
	actData		= (DropletActuatorData *)malloc(sizeof(DropletActuatorData));
	senseData	= (DropletSensorData *)malloc(sizeof(DropletSensorData));
	commData	= (DropletCommData *)malloc(sizeof(DropletCommData));
	compData	= (DropletCompData *)malloc(sizeof(DropletCompData));
	timeData	= (DropletTimeData *)malloc(sizeof(DropletTimeData));
	this->global_rx_buffer.buf = (uint8_t *)malloc(sizeof(uint8_t) * IR_BUFFER_SIZE);
}
	
IDroplet::~IDroplet()
{
	free(objPhysics);
	free(actData);
	free(senseData);
	free(commData);
	free(compData);

	objPhysics = NULL;
	actData = NULL;
	senseData = NULL;
	commData = NULL;
	compData = NULL;
}

DS_RESULT IDroplet::_InitPhysics(
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
void IDroplet::init_all_systems() 
{
	reset_motors();
	reset_rgb_led();
	reset_rgb_sensor();
	reset_timers();
	for(uint8_t i = 0; i < 7; i++)
		reset_ir_sensor(i);

	// Initialize variables
	commData->sendActive = false;
	compData->capStatus = 0;
	msg_return_order = OLDEST_MSG_FIRST;
}

void droplet_reboot(void)
{
	// TODO
}

void IDroplet::reset_rgb_led() 
{
	actData->rOut = 250;
	actData->gOut = 250;
	actData->bOut = 250;
}


void IDroplet::reset_ir_sensor(uint8_t sensor_num) 
{
	if(sensor_num >= 0 && sensor_num < 7)
	{
		memset(commData->commChannels[sensor_num].inBuf, 0, IR_BUFFER_SIZE);
		memset(commData->commChannels[sensor_num].outBuf, 0, IR_BUFFER_SIZE);
		commData->commChannels[sensor_num].inMsgLength = 0;
		commData->commChannels[sensor_num].outMsgLength = 0;
		commData->commChannels[sensor_num].lastMsgInTimestamp = 0;
		commData->commChannels[sensor_num].lastMsgOutTimestamp = 0;
	}
}

void IDroplet::reset_rgb_sensor() 
{
	senseData->rIn = 0;
	senseData->gIn = 0;
	senseData->bIn = 0;
}

void IDroplet::reset_motors() 
{
	actData->currMoveDir			= MOVE_OFF;
	actData->currTurnDir			= TURN_OFF;
	actData->moveTimeRemaining		= 0;
	actData->rotateTimeRemaining	= 0;
	actData->moveStepRemaining		= 0;
	actData->rotateStepRemaining	= 0;
	actData->_oscillator			= true;
}

void IDroplet::reset_timers()
{
	for(int i = 0; i < DROPLET_NUM_TIMERS; i++)
	{
		timeData->timer[i] = 0;
		timeData->trigger[i] = 1;
	}
}

droplet_id_type IDroplet::get_droplet_id()
{
	return compData->dropletID;
}

uint8_t IDroplet::rand_byte(void)
{
	return static_cast<uint8_t>(255 * ((float)rand() / RAND_MAX));
}

// Droplet Motion Subsystem Functions
void IDroplet::move_duration(move_direction direction, uint16_t duration)
{
	actData->moveTimeRemaining = static_cast<float>(duration);
	actData->currMoveDir = direction;
}

void IDroplet::move_steps(move_direction direction, uint16_t num_steps)
{
	actData->moveStepRemaining = static_cast<float>(num_steps * WALK_STEP_TIME);
	actData->currMoveDir = direction;
}
void IDroplet::rotate_duration(turn_direction direction, uint16_t duration)
{
	actData->rotateTimeRemaining = static_cast<float>(duration);
	actData->currTurnDir = direction;
}

void IDroplet::rotate_steps(turn_direction direction, uint16_t num_steps)
{
	actData->rotateStepRemaining = static_cast<float>(num_steps * WALK_STEP_TIME);
	actData->currTurnDir = direction;
}

void IDroplet::rotate_degrees(int16_t deg)
{
	deg = static_cast<int16_t>(prettyAngle(static_cast<float>(deg)));
	if(deg > 0)
	{
		rotate_duration(TURN_COUNTERCLOCKWISE, ROTATE_RATE_MS_PER_DEG * deg);
	}
	else if(deg < 0)
	{
		rotate_duration(TURN_CLOCKWISE, -1 * ROTATE_RATE_MS_PER_DEG * deg);
	}
}

uint32_t IDroplet::cancel_move() 
{
	float tmp;
	if ( actData->moveStepRemaining > 0){
		tmp = actData->moveStepRemaining * ( 60.f / 1000.f);
		actData->moveStepRemaining = 0;
		actData->moveTimeRemaining = 0;
		return static_cast<uint32_t>(tmp);
	}
	else{
		tmp = actData->moveTimeRemaining;
		actData->moveTimeRemaining = 0;
		actData->moveStepRemaining = 0;
		return static_cast<uint32_t>(tmp);
	}
}

uint32_t IDroplet::cancel_rotate() 
{
	float tmp;
	if ( actData->rotateStepRemaining > 0){
		tmp = actData->rotateStepRemaining * ( 60.f / 1000.f);
		actData->rotateStepRemaining = 0;
		actData->rotateTimeRemaining = 0;
		return static_cast<uint32_t>(tmp);
	}
	else{
		tmp = actData->rotateTimeRemaining;
		actData->rotateTimeRemaining = 0;
		actData->rotateStepRemaining = 0;
		return static_cast<uint32_t>(tmp);
	}
}

uint8_t IDroplet::is_moving()
{
	if(actData->moveTimeRemaining <= 0.f && 
		actData->moveStepRemaining <= 0.f)
		return 0;
	else
		return actData->currMoveDir;
}

//uint8_t IDroplet::is_rotating()
turn_direction IDroplet::is_rotating() 
{
	if(actData->rotateTimeRemaining <= 0.f &&
		actData->rotateStepRemaining <= 0.f)
		return 0;
	else
		return actData->currTurnDir;
}

int8_t IDroplet::leg1_status()
{
	return compData->leg1Status;
}

int8_t IDroplet::leg2_status()
{
	return compData->leg2Status;
}

int8_t IDroplet::leg3_status()
{
	return compData->leg3Status;
}

int8_t IDroplet::leg_status(uint8_t leg)
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
int8_t IDroplet::cap_status(void)
{
	return compData->capStatus;
}

void IDroplet::set_rgb_led(uint8_t r, uint8_t g, uint8_t b)
{
	actData->rOut = r;
	actData->gOut = g;
	actData->bOut = b;
}

void IDroplet::set_red_led(uint8_t saturation)
{
	actData->rOut = saturation;
}

void IDroplet::set_green_led(uint8_t saturation)
{
	actData->gOut = saturation;
}

void IDroplet::set_blue_led(uint8_t saturation)
{
	actData->bOut = saturation;
}

uint8_t IDroplet::get_red_led(void)
{
	return actData->rOut;
}

uint8_t IDroplet::get_green_led(void)
{
	return actData->gOut;
}

uint8_t IDroplet::get_blue_led(void)
{
	return actData->bOut;
}

void IDroplet::led_off(void)
{
	reset_rgb_led();
}

void IDroplet::get_rgb_sensor(uint8_t *r, uint8_t *g, uint8_t *b)
{
	*r = senseData->rIn;
	*g = senseData->gIn;
	*b = senseData->bIn;
}

uint8_t IDroplet::get_red_sensor(void)
{
	return senseData->rIn;
}

uint8_t IDroplet::get_green_sensor(void)
{
	return senseData->gIn;
}

uint8_t IDroplet::get_blue_sensor(void)
{
	return senseData->bIn;
}

uint8_t IDroplet::ir_broadcast(const char *send_buf, uint8_t length)
{
	return ir_send(0, send_buf, length);
}

uint8_t IDroplet::ir_send(uint8_t channel, const char *send_buf, uint8_t length)
{
	// We only have 6 channels, 1 - 6. Channel 0 is reserved for broadcast
	if(channel > 6)
		return 0;

	// Reset the out comm buffer
	memset(commData->commChannels[channel].outBuf, 0, IR_BUFFER_SIZE);
	commData->commChannels[channel].outMsgLength = 0;

	// Store the sending droplet's id as part of the message header
	memcpy(commData->commChannels[channel].outBuf, &(compData->dropletID), sizeof(droplet_id_type));
	

	// Store the rest of the message header, then body
	uint8_t msgLength = length < IR_MAX_DATA_SIZE ? length : IR_MAX_DATA_SIZE;
	memcpy(&commData->commChannels[channel].outBuf[sizeof(droplet_id_type)], &msgLength, sizeof(uint8_t));
	memcpy(&commData->commChannels[channel].outBuf[IR_MSG_HEADER], send_buf, msgLength);
	commData->commChannels[channel].outMsgLength = msgLength + IR_MSG_HEADER;

	// Set Send to active
	commData->sendActive = true;
	return 1;
}

uint8_t IDroplet::check_for_new_messages(void)
{
	int newMsgCh = -1;
	for(int i = 0; i < 7; i++)
	{
		if(commData->commChannels[i].inMsgLength > 0)
		{
			if(newMsgCh == -1)
				newMsgCh = i;

			else
			{
				if(commData->commChannels[i].lastMsgInTimestamp <
					commData->commChannels[newMsgCh].lastMsgInTimestamp)
				{
					if(msg_return_order == OLDEST_MSG_FIRST)
						newMsgCh = i;
				}
				else
				{
					if(msg_return_order == NEWEST_MSG_FIRST)
						newMsgCh = i;
				}
			}
		}
	}

	// If there is indeed a new message then we should copy it into the global_rx_buffer
	if(newMsgCh != -1)
	{
		/* Note : Here global_rx_buffer.size, global_rx_buffer.data_len & 
		   DropletCommChannelData::inMsgLength are being used to store the same value,
		   the length of the actual body of the message and not the whole message (including
		   header). It is a bit confusing, fix it later.
		*/
		global_rx_buffer.size = commData->commChannels[newMsgCh].inMsgLength;
		global_rx_buffer.message_time = commData->commChannels[newMsgCh].lastMsgInTimestamp;
		memset(global_rx_buffer.buf, 0, IR_BUFFER_SIZE);
		memcpy( 
			global_rx_buffer.buf,
			&commData->commChannels[newMsgCh].inBuf[IR_MSG_HEADER],
			global_rx_buffer.size);
		memcpy(
			&global_rx_buffer.data_len,
			&commData->commChannels[newMsgCh].inBuf[sizeof(droplet_id_type)],
			sizeof(uint8_t));
		memcpy(
			&global_rx_buffer.sender_ID,
			commData->commChannels[newMsgCh].inBuf,
			sizeof(droplet_id_type));

		// Clean out this message from the in buffer
		commData->commChannels[newMsgCh].inMsgLength = 0;
	}

	return (newMsgCh == -1) ? 0 : 1;
}

uint8_t IDroplet::set_timer(uint16_t time, uint8_t index)
{
	if(index >= DROPLET_NUM_TIMERS) return 0;

	timeData->timer[index] = static_cast<float>(time);
	timeData->trigger[index] = 0;

	return 1;
}
 
uint8_t IDroplet::check_timer(uint8_t index)
{
	if(index >= DROPLET_NUM_TIMERS) return 0;

	return timeData->trigger[index];
}

uint8_t IDroplet::range_and_bearing(uint16_t partner_id, float *dist, float *theta, float *phi)
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



void IDroplet::DropletInit(void) { return; }
void IDroplet::DropletMainLoop(void) { return; }