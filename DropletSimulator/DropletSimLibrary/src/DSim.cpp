#include "DSim.h"

extern TrigArray *dropletRelPos; // Welcome to the DAINJA' ZONE!
std::vector<GPSInfo *> dropletPositions;

DSim::DSim()
{
	// TODO: Remove this later
	//fh = NULL;

	firstRun = true;
	projSet  = false;

	projector = NULL;
	
	droplets.clear();
	physicalObjects.clear();
	dropletPositions.clear();
	objectPositions.clear();
	dropletRelPos = NULL;
}

DSim::~DSim()
{
	droplets.clear();
	physicalObjects.clear();
	dropletPositions.clear();
}

DS_RESULT DSim::AddCollisionShape(btCollisionShape *colShape, int *colShapeIndex)
{
	simPhysics->collisionShapes->push_back(colShape);
	*colShapeIndex = simPhysics->_colShapeIDCounter++;

	return DS_SUCCESS;
}

DS_RESULT DSim::CreateFloor(
	int floorShapeIndex,
	int wallXShapeIndex, 
	int wallYShapeIndex)
{
	// Set up the Floor
	ObjectPhysicsData floorData;
	floorData.mass = 0.0;	// Static object
	floorData.localInertia = btVector3(0, 0, 0);
	floorData.friction = FLOOR_FRICTION;
	floorData.colShapeIndex = floorShapeIndex;

	btVector3 origin = btVector3(0, 0, 0);
	initPhysicsObject(&floorData, origin);

	// Set up walls - if requested
	if(simSetupData->autoBuildBoundaryWalls)
	{
		if(wallXShapeIndex < 0 || wallYShapeIndex < 0)
		{
			// TODO : Print warning message
			return DS_WARNING;
		}

		for(int i = 0; i < 2; i++)
		{
			btScalar startXPos = powf(-1, (float)i) * simSetupData->tileLength * simSetupData->numColTiles / 2;
			btScalar startYPos = powf(-1, (float)i) * simSetupData->tileLength * simSetupData->numRowTiles / 2;

			ObjectPhysicsData wallData;
			wallData.mass = 0.0f;	// Static object
			wallData.localInertia = btVector3(0, 0, 0);
			wallData.friction = 0.0f;
			wallData.colShapeIndex = wallYShapeIndex;

			origin = btVector3(startXPos,0,0);
			initPhysicsObject(&wallData, origin);

			wallData.colShapeIndex = wallXShapeIndex;
			origin = btVector3(0,startYPos,0);
			initPhysicsObject(&wallData, origin);
		}
	}

	return DS_SUCCESS;
}

DS_RESULT DSim::AddDroplet(		
	DSimDroplet *pDroplet, 
	std::pair<float, float>startPos,
	float startAngle)
{
	DS_RESULT retval = pDroplet->_InitPhysics(simPhysics, startPos, startAngle); // Set up physics for this droplet
	if(retval == DS_SUCCESS)
	{
		droplets.push_back(pDroplet); 
		GPSInfo *tmp = (GPSInfo *)malloc(sizeof(GPSInfo));
		// initialize the droplet when adding it
		pDroplet->DropletInit();

		tmp->posX = startPos.first;
		tmp->posY = startPos.second;
		tmp->posZ = 0.1f; // TODO : This isn't the right z-position but it shouldn't matter
		tmp->rotA = startAngle;
		tmp->rotX = 0;
		tmp->rotY = 0;
		tmp->rotZ = 1;
		dropletPositions.push_back(tmp); // Set up space for storing this droplet's position

		// Set the Droplet's data counter
		ObjectPhysicsData *phyDat;
		AccessPhysicsData(pDroplet, &phyDat);
		phyDat->_dataID = simPhysics->_dropletDataIDCounter++;

	}
	else
	{
		// TODO : Print Error message
	}
	return retval;
}

DS_RESULT DSim::AddPhysicalObject(
		DSimPhysicalObject *pObject,
		std::pair<float, float> startPos,
		float startAngle)
{
	DS_RESULT retval = pObject->_InitPhysics(simPhysics, startPos, startAngle);
	if(retval == DS_SUCCESS)
	{
		physicalObjects.push_back(pObject);

		// Only create tracking information for dynamic objects
		if(pObject->objPhysics->mass != 0.f)
		{
			GPSInfo *tmp = (GPSInfo *)malloc(sizeof(GPSInfo));
			tmp->posX = startPos.first;
			tmp->posY = startPos.second;
			tmp->posZ = 0.1f; // TODO : This isn't the right z-position but it shouldn't matter
			tmp->rotA = startAngle;
			tmp->rotX = 0;
			tmp->rotY = 0;
			tmp->rotZ = 1;
			objectPositions.push_back(tmp); // Set up space for storing this object's position
		}
	}
	else
	{
		// TODO : Print error message
	}

	return retval;
}

DS_RESULT DSim::AddPhysicalObject(
		DSimPhysicalObject *pObject,
		std::pair<float, float> startPos,
		float startHeight,
		float startAngle)
{
	DS_RESULT retval = pObject->_InitPhysics(simPhysics, startPos, startHeight, startAngle);
	if(retval == DS_SUCCESS)
	{
		physicalObjects.push_back(pObject);

		// Only create tracking information for dynamic objects
		if(pObject->objPhysics->mass != 0.f)
		{
			GPSInfo *tmp = (GPSInfo *)malloc(sizeof(GPSInfo));
			tmp->posX = startPos.first;
			tmp->posY = startPos.second;
			tmp->posZ = startHeight;
			tmp->rotA = startAngle;
			tmp->rotX = 0;
			tmp->rotY = 0;
			tmp->rotZ = 1;
			objectPositions.push_back(tmp); // Set up space for storing this object's position
		}
	}
	else
	{
		// TODO : Print error message
	}

	return retval;
}

DS_RESULT DSim::SetUpProjector(std::string imgDir, std::string imgName)
{
	DS_RESULT retval = DS_SUCCESS;

	if(projSet)
	{
		SAFE_DELETE(projector);
		fprintf(stderr, "[WARNING] Projector has been reset.\n");
		retval = DS_WARNING;
	}

	projector = new DSimProjection(
		static_cast<int>(simSetupData->numRowTiles * simSetupData->tileLength),
		static_cast<int>(simSetupData->numColTiles * simSetupData->tileLength));

	retval = projector->SetDirectory(imgDir);
	retval = projector->LoadFile(imgName);

	// TODO : Error checking before setting projSet to true
	projSet = true;

	return retval;
}

DS_RESULT DSim::SetUpProjector(
		std::string imgDir, 
		std::string imgName,
		int projWidth, 
		int projLength)
{
	DS_RESULT retval = DS_SUCCESS;

	if(projSet)
	{
		SAFE_DELETE(projector);
		fprintf(stderr, "[WARNING] Projector has been reset.\n");
		retval = DS_WARNING;
	}

	projector = new DSimProjection(projWidth, projLength);
	
	retval = projector->SetDirectory(imgDir);
	retval = projector->LoadFile(imgName);

	// TODO : Error checking before setting projSet to true
	projSet = true;

	return retval;
}

DS_RESULT DSim::Init(const SimSetupData &setupData)
{
	// TODO: Remove this later
	//fh = fopen("SimMsgDat.txt","w");

	DS_RESULT retval;

	// Set up simulator init parameters
	simSetupData = new SimSetupData(setupData);
	halfArenaWidth = simSetupData->tileLength * simSetupData->numColTiles / 2.0f;
	halfArenaLength = simSetupData->tileLength * simSetupData->numRowTiles / 2.0f;
	goodRand = new Ran(time(NULL));

	// Set up communication init parameters
	for(unsigned int i = 0; i < dropletPositions.size(); i++)
	{
		dropletPositions[i]->lastRelPosUpdate = 0;
		dropletPositions[i]->movedSinceLastUpdate = true;
	}


	// Initialize Physics engine
	retval = initPhysics();

	// Initialize the time control
	timer.initTimer(1.0/double(simSetupData->fps));

	return retval;
}

/*
 * The main body of the simulator goes here. The Step() method is responsible for
 * updating all state variables of the simulator and running the physics engine for
 * a single timestep. Order of operations performed in this method are organized
 * as follows :
 * 
 * 1. Run the main loop on each droplet
 * 2. Update position information for objects in sim
 * 3. Apply physical forces to objects in sim, if required
 * 4. Gather and update sensor data on each droplet, if required
 * 5. Gather and update communication data on each droplet, if required
 * 6. Step the physics engine
 *
 */
DS_RESULT DSim::Step()
{
	// The following if() statement is just run on the first step of the simulator.
	std::vector<DSimDroplet *>::iterator it;
	if(firstRun)
	{
		// Step the simulation a few times first
		for(int i = 0; i < 100; i++)
			simPhysics->dynWorld->stepSimulation(1.0f / simSetupData->fps);

		// Stagger each droplet's starting time, if required
		if(simSetupData->staggeredStart)
		{
			for(it = droplets.begin(); it != droplets.end(); it++)
			{

				int randSteps = static_cast<int>(goodRand->doub() * 100);
				for(int i = 0; i < randSteps; i++) 
				{
					std::vector<DSimDroplet *>::iterator n_it;
					for(n_it = droplets.begin(); n_it <= it; n_it++)
					{
						DSimDroplet *pDroplet = *n_it;
						pDroplet->DropletMainLoop();
					}

					// Call some of the simulation controllers
					gatherPositionsAndCorrect();
					motionController();
					timerController();

					// Step the physics simulation.
					simPhysics->dynWorld->stepSimulation(1.0f / simSetupData->fps);
				}
			}
		}

		firstRun = false;
		return DS_SUCCESS;
	}

	for(it = droplets.begin(); it != droplets.end(); it++)
	{
		DSimDroplet *pDroplet = *it;
		pDroplet->DropletMainLoop();
	}

	// Gather up all current droplet world positions
	gatherPositionsAndCorrect();
	
	// Call the simulation controllers
	motionController();
	sensorController();
	timerController();
	commController();

	// Step the physics simulation. Default run at 60fps = default for Bullet.
	// To understand how stepSimulation() works, 
	// ref : http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_The_World

	simPhysics->dynWorld->stepSimulation(1.0f / simSetupData->fps);
	timer.updateTimer();

	return DS_SUCCESS;
}

DS_RESULT DSim::Cleanup()
{
	// TODO: Remove this later
	//if(fh != NULL)
		//fclose(fh);

	// Clean up droplet data.
	std::vector<DSimDroplet *>::reverse_iterator d_rit;
	for(d_rit = droplets.rbegin(); d_rit != droplets.rend(); d_rit++)
		delete *d_rit;

	std::vector<GPSInfo *>::reverse_iterator p_rit;
	for(p_rit = dropletPositions.rbegin(); p_rit != dropletPositions.rend(); p_rit++)
		free((GPSInfo *)*p_rit);

	// Clean up physical object data
	std::vector<DSimPhysicalObject *>::reverse_iterator po_rit;
	for(po_rit = physicalObjects.rbegin(); po_rit != physicalObjects.rend(); po_rit++)
		delete *po_rit;

	if(projSet)
		SAFE_DELETE(projector);

	delete simSetupData;
	delete goodRand;

	return endPhysics();
}

DS_RESULT DSim::initPhysics()
{
	simPhysics = (SimPhysicsData *)malloc(sizeof(SimPhysicsData));

	//collision configuration contains default setup for memory, collision setup
	simPhysics->collisionConfig		= new btDefaultCollisionConfiguration();
	
	// Use the default collision dispatcher. 
	// For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)	
	simPhysics->collisionDispatch	= new btCollisionDispatcher(simPhysics->collisionConfig);
	simPhysics->broadphase			= new btDbvtBroadphase();

	// The default constraint solver. 
	// For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)	
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;	
	simPhysics->constraintSolver	= sol;
	
	simPhysics->dynWorld			= new btDiscreteDynamicsWorld(
		simPhysics->collisionDispatch,
		simPhysics->broadphase,
		simPhysics->constraintSolver,
		simPhysics->collisionConfig
	);

	simPhysics->dynWorld->setGravity(btVector3(0, 0, -PHYSICS_GRAVITY));
	simPhysics->_physicsWorldObjCounter = 0;
	simPhysics->_dropletDataIDCounter = 0;

	// Create the Collision Shapes for the Droplets and Floor
	simPhysics->collisionShapes = new btAlignedObjectArray<btCollisionShape *>();
	simPhysics->collisionShapes->clear();
	simPhysics->_colShapeIDCounter = 0;

	return DS_SUCCESS;
}

DS_RESULT DSim::endPhysics()
{
	// TODO : Delete simPhysics->collisionShapes object array
	delete simPhysics->dynWorld;
	delete simPhysics->constraintSolver;
	delete simPhysics->broadphase;
	delete simPhysics->collisionDispatch;
	delete simPhysics->collisionConfig;

	free(simPhysics);

	return DS_SUCCESS;
}

DS_RESULT DSim::initPhysicsObject(ObjectPhysicsData *objPhysics, btVector3 &origin)
{
	btCollisionShape *colShape = simPhysics->collisionShapes->at(objPhysics->colShapeIndex);
	
	// Set object frame origin
	btTransform objTransform;
	objTransform.setIdentity();
	objTransform.setOrigin(origin);

	//rigidbody is dynamic if and only if objPhysics->mass is non zero, otherwise static
	bool isDynamic = (objPhysics->mass != 0.f);
	if (isDynamic)
		colShape->calculateLocalInertia(objPhysics->mass, objPhysics->localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(objTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		objPhysics->mass,
		myMotionState,
		colShape,
		objPhysics->localInertia);
	rbInfo.m_friction = objPhysics->friction;
	btRigidBody* body = new btRigidBody(rbInfo);

	//body->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
	body->activate(true);
	body->setActivationState(DISABLE_DEACTIVATION);

	//add the body to the dynamics world
	simPhysics->dynWorld->addRigidBody(
		body, 
		SimPhysicsData::_staticObjCollisionBM,	// Wall Collision Bitmask
		SimPhysicsData::_dynObjCollisionBM); // Wall collides with these objects
	simPhysics->_physicsWorldObjCounter++;

	return DS_SUCCESS;
}

// ----- Control and Update Functions ----- //

void DSim::gatherPositionsAndCorrect()
{
	btCollisionObjectArray objs = simPhysics->dynWorld->getCollisionObjectArray();
	std::vector<DSimDroplet *>::iterator d_it;
	std::vector<DSimPhysicalObject *>::iterator o_it;
	std::vector<GPSInfo *>::iterator p_it;
	p_it = dropletPositions.begin();

	for(d_it = droplets.begin(); d_it != droplets.end(); d_it++)
	{
		DSimDroplet *pDroplet = *d_it;
		ObjectPhysicsData *objPhysics;
		AccessPhysicsData(pDroplet, &objPhysics);

		btRigidBody *body = btRigidBody::upcast(objs[objPhysics->_worldID]);
		if (body && body->getMotionState())
		{
			GPSInfo *gpsDat = (GPSInfo *)*p_it;
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);
			btVector3 physicsPos = trans.getOrigin();

			if (physicsPos.getZ() < 0.0)
			{
				btVector3 tmpHolder1, tmpHolder2; // Don't really need this for anything besides calling getBoundingSphere
				body->getAabb(tmpHolder1, tmpHolder2);
				float height = tmpHolder2.z() - tmpHolder1.z();
				float xlength = simSetupData->numColTiles * simSetupData->tileLength;
				float ylength = simSetupData->numRowTiles * simSetupData->tileLength;
				float randx = (rand() / (float) RAND_MAX) * xlength - xlength / 2.0f;
				float randy = (rand() / (float) RAND_MAX) * ylength - ylength / 2.0f;
				btVector3 trans(randx,randy,height);

				physicsPos = trans;
				btTransform fixTransform;
				fixTransform.setIdentity();
				fixTransform.setOrigin(trans);
				btQuaternion rotation(btVector3(
					btScalar(0.0), 
					btScalar(0.0),
					btScalar(1.0)
					), 
					btScalar(0.0)
					);		
				fixTransform.setRotation(rotation);
				body->setCenterOfMassTransform(fixTransform);

				body->clearForces();
				body->setAngularVelocity(btVector3(0,0,0));
				body->setLinearVelocity(btVector3(0,0,0));
			}
			btQuaternion physicsRot = trans.getRotation();	
			physicsRot.normalize();
			btVector3 rotAxis = physicsRot.getAxis();
			gpsDat->rotA = static_cast<float>(physicsRot.getAngle());
			gpsDat->rotX = static_cast<float>(rotAxis.x());
			gpsDat->rotY = static_cast<float>(rotAxis.y());
			gpsDat->rotZ = static_cast<float>(rotAxis.z());
			gpsDat->posX = static_cast<float>(physicsPos.getX());
			gpsDat->posY = static_cast<float>(physicsPos.getY());
			gpsDat->posZ = static_cast<float>(physicsPos.getZ());
		}
		else
		{
			// TODO : We have a problem. Print error message. Handle error.
			break;
		}

		if(p_it != dropletPositions.end())
			p_it++;
		else
		{
			// TODO : We have a problem. Print error message. Handle error.
			break;
		}
	}

	// Set up communication data for the first time if needed
	if(dropletRelPos == NULL)
	{
		for(unsigned int i = 0; i < dropletPositions.size(); i++)
			calcRelativePos(i);
	}

	// Gather other physical object positions
	p_it = objectPositions.begin();

	for(o_it = physicalObjects.begin(); o_it != physicalObjects.end(); o_it++)
	{
		DSimPhysicalObject *pPhysicalObj = *o_it;
		if(pPhysicalObj->objPhysics->mass == 0.f) continue;

		btRigidBody *body = btRigidBody::upcast(objs[pPhysicalObj->objPhysics->_worldID]);
		if (body && body->getMotionState())
		{
			GPSInfo *gpsDat = (GPSInfo *)*p_it;
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);
			btVector3 physicsPos = trans.getOrigin();
			btQuaternion physicsRot = trans.getRotation();	

			if (physicsPos.getZ() < 0.0)
			{
				btVector3 tmpHolder1, tmpHolder2; // Don't really need this for anything besides calling getBoundingSphere
				body->getAabb(tmpHolder1, tmpHolder2);
				float height = tmpHolder2.z() - tmpHolder1.z();
				float xlength = simSetupData->numColTiles * simSetupData->tileLength;
				float ylength = simSetupData->numRowTiles * simSetupData->tileLength;
				float randx = (rand() / (float) RAND_MAX) * xlength - xlength / 2.0f;
				float randy = (rand() / (float) RAND_MAX) * ylength - ylength / 2.0f;
				btVector3 trans(randx,randy,height);

				physicsPos = trans;
				btTransform fixTransform;
				fixTransform.setIdentity();
				fixTransform.setOrigin(trans);
				btQuaternion rotation(btVector3(
					btScalar(0.0), 
					btScalar(0.0),
					btScalar(1.0)
					), 
					btScalar(0.0)
					);
				fixTransform.setRotation(rotation);
				body->setCenterOfMassTransform(fixTransform);

				body->clearForces();
				body->setAngularVelocity(btVector3(0,0,0));
				body->setLinearVelocity(btVector3(0,0,0));
				//physicsPos.setZ(btScalar(physicsPos.getZ() + height));
			}

			physicsRot.normalize();
			btVector3 rotAxis = physicsRot.getAxis();
			gpsDat->rotA = static_cast<float>(physicsRot.getAngle());
			gpsDat->rotX = static_cast<float>(rotAxis.x());
			gpsDat->rotY = static_cast<float>(rotAxis.y());
			gpsDat->rotZ = static_cast<float>(rotAxis.z());
			gpsDat->posX = static_cast<float>(physicsPos.getX());
			gpsDat->posY = static_cast<float>(physicsPos.getY());
			gpsDat->posZ = static_cast<float>(physicsPos.getZ());
		}
		else
		{
			// TODO : We have a problem. Print error message. Handle error.
			break;
		}

		if(p_it != objectPositions.end())
			p_it++;
		else
		{
			// TODO : We have a problem. Print error message. Handle error.
			break;
		}
	}
}

void DSim::motionController()
{
	btCollisionObjectArray objs = simPhysics->dynWorld->getCollisionObjectArray();

	std::vector<GPSInfo *>::iterator d_it = dropletPositions.begin();
	std::vector<DSimDroplet *>::iterator it;
	for(it = droplets.begin (); it < droplets.end(); it++) 
	{
		DSimDroplet *pDroplet = *it;
		GPSInfo *gpsInfo = *d_it;
		DropletActuatorData *actData;
		AccessActuatorData(pDroplet, &actData);

		// Move/Rotate all the droplets
		if(actData->moveTimeRemaining > 0 || actData->rotateTimeRemaining > 0 ||
			actData->moveStepRemaining > 0 || actData->rotateStepRemaining > 0)
		{
			// The motors are positioned 2*PI/3 radians apart
			// Rotate the relative position of the first motor
			btVector3 motor1RelativePos = 
				btVector3(0, simSetupData->dropletRadius * MOTOR_POS_SCALING, 0.1f).rotate(
				btVector3(gpsInfo->rotX, gpsInfo->rotY, gpsInfo->rotZ), gpsInfo->rotA);
			btVector3 motor2RelativePos = 
				motor1RelativePos.rotate(btVector3(0, 0, 1), 2.f * SIMD_PI / 3);
			btVector3 motor3RelativePos = 
				motor2RelativePos.rotate(btVector3(0, 0, 1), 2.f * SIMD_PI / 3);

			/* Add a small error variance to each motor since they are never identical.
			 * The error variance value can be changed using the MOTOR_ERROR constant in 
			 * DSimGlobals.h */
			btVector3 motor1CounterClockImpulse =
				(btScalar(IMPULSE_SCALING * ((goodRand->doub() * MOTOR_ERROR * 2.f) + 
					(1.f - MOTOR_ERROR))) * 
						motor1RelativePos.normalize()).rotate(
							btVector3(0, 0, 1), SIMD_PI / 2);
			btVector3 motor2CounterClockImpulse =
				(btScalar(IMPULSE_SCALING * ((goodRand->doub() * MOTOR_ERROR * 2.f) + 
					(1.f - MOTOR_ERROR))) * 
						motor2RelativePos.normalize()).rotate(
							btVector3(0, 0, 1), SIMD_PI / 2);
			btVector3 motor3CounterClockImpulse = 
				(btScalar(IMPULSE_SCALING * ((goodRand->doub() * MOTOR_ERROR * 2.f) + 
					(1.f - MOTOR_ERROR))) * 
						motor3RelativePos.normalize()).rotate(
							btVector3(0, 0, 1), SIMD_PI / 2);

			btVector3 motor1ClockImpulse = -1. * motor1CounterClockImpulse;
			btVector3 motor2ClockImpulse = -1. * motor2CounterClockImpulse;
			btVector3 motor3ClockImpulse = -1. * motor3CounterClockImpulse;

			ObjectPhysicsData *objPhysics;
			AccessPhysicsData(pDroplet, &objPhysics);
			btRigidBody *body = btRigidBody::upcast(objs[objPhysics->_worldID]);
		
			if(actData->moveTimeRemaining > 0 || actData->moveStepRemaining > 0)
			{
				move_direction md = actData->currMoveDir;
				bool updateRelPos = true;
				switch(md)
				{
				case NORTH :
					if(actData->_oscillator) body->applyImpulse(motor2ClockImpulse, motor2RelativePos);
					else body->applyImpulse(motor3CounterClockImpulse, motor3RelativePos);
					break;

				case NORTH_EAST :
					if(actData->_oscillator) body->applyImpulse(motor1ClockImpulse, motor1RelativePos);
					else body->applyImpulse(motor3CounterClockImpulse, motor3RelativePos);
					break;

				case SOUTH_EAST :
					if(actData->_oscillator) body->applyImpulse(motor1ClockImpulse, motor1RelativePos);
					else body->applyImpulse(motor2CounterClockImpulse, motor2RelativePos);
					break;

				case SOUTH :			
					if(actData->_oscillator) body->applyImpulse(motor3ClockImpulse, motor3RelativePos);
					else body->applyImpulse(motor2CounterClockImpulse, motor2RelativePos);
					break;

				case SOUTH_WEST :
					if(actData->_oscillator) body->applyImpulse(motor3ClockImpulse, motor3RelativePos);
					else body->applyImpulse(motor1CounterClockImpulse, motor1RelativePos);
					break;

				case NORTH_WEST :
					if(actData->_oscillator) body->applyImpulse(motor2ClockImpulse, motor2RelativePos);
					else body->applyImpulse(motor1CounterClockImpulse, motor1RelativePos);
					break;

				default :
					updateRelPos = false;
					break;
				}

				gpsInfo->movedSinceLastUpdate = updateRelPos;
				if (actData->moveTimeRemaining > 0)
				{
					if((actData->moveTimeRemaining -= simSetupData->timestep) <= 0)
					{
						actData->moveTimeRemaining = 0;
						actData->currMoveDir = MOVE_OFF;
					}
				}
				else if(actData->moveStepRemaining > 0)
				{
					if((actData->moveStepRemaining -= simSetupData->timestep) <= 0)
					{
						actData->moveStepRemaining = 0;
						actData->currMoveDir = MOVE_OFF;
					}
				}
			}

			if(actData->rotateTimeRemaining > 0 || actData->rotateStepRemaining > 0)
			{
				turn_direction td = actData->currTurnDir;
				switch(td)
				{
				case TURN_CLOCKWISE :
					if(actData->_oscillator)
					{
						body->applyImpulse(motor1ClockImpulse, motor1RelativePos);
						body->applyImpulse(motor2ClockImpulse, motor2RelativePos);
						body->applyImpulse(motor3ClockImpulse, motor3RelativePos);
					}
					break;

				case TURN_COUNTERCLOCKWISE :
					if(!actData->_oscillator)
					{
						body->applyImpulse(motor1CounterClockImpulse, motor1RelativePos);
						body->applyImpulse(motor2CounterClockImpulse, motor2RelativePos);
						body->applyImpulse(motor3CounterClockImpulse, motor3RelativePos);
					}
					break;
				}
				if (actData->rotateTimeRemaining > 0)
				{
					if((actData->rotateTimeRemaining -= simSetupData->timestep) <= 0)
					{
						actData->rotateTimeRemaining = 0;
						actData->currTurnDir = MOVE_OFF;
					}
				}
				else if(actData->rotateStepRemaining >0)
				{
					if((actData->rotateStepRemaining -= simSetupData->timestep) <= 0)
					{
						actData->rotateStepRemaining = 0;
						actData->currTurnDir = MOVE_OFF;
					}
				}

			}
			actData->_oscillator = !actData->_oscillator;

			if (actData->moveTimeRemaining > 0 || actData->moveStepRemaining > 0 ||
				actData->rotateTimeRemaining > 0 || actData->rotateStepRemaining > 0)
				setLegPower(pDroplet, gpsInfo);
		}
		d_it++;
	}
}

void DSim::setLegPower(DSimDroplet *pDroplet, GPSInfo *gpsInfo)
{
	DropletCompData *compData;
	AccessCompData(pDroplet, &compData);

	// compute inital variables for leg power
	float bandWidth = simSetupData->tileLength / BANDS_PER_TILE;
	float leg1PosX = gpsInfo->posX + sin(gpsInfo->rotZ) * (simSetupData->dropletRadius * MOTOR_POS_SCALING);
	float leg2PosX = gpsInfo->posX + sin(gpsInfo->rotZ + (2.f * SIMD_PI / 3.f)) * (simSetupData->dropletRadius * MOTOR_POS_SCALING);
	float leg3PosX = gpsInfo->posX + sin(gpsInfo->rotZ + (4.f * SIMD_PI / 3.f)) * (simSetupData->dropletRadius * MOTOR_POS_SCALING);
	
	// leg 1
	float totalDistance1 = leg1PosX + (( simSetupData->numColTiles * simSetupData->tileLength ) / 2.f);
	float distanceGap1 = fmod(totalDistance1 - (bandWidth / 2.f), bandWidth);
	// on gap: floating
	if (distanceGap1 == 0) {
		compData->leg1Status = 0;
	}
	// leg on power or ground
	else 
	{
		int numBands1 = static_cast<int>(((totalDistance1 - (bandWidth / 2.f)) / bandWidth));
		if ((numBands1 % 2) == 0)
			compData->leg1Status = -1;
		else
			compData->leg1Status = 1;
	}

	// leg 2
	float totalDistance2 = leg2PosX + (( simSetupData->numColTiles * simSetupData->tileLength ) / 2.f);
	float gapPos2 = fmod(totalDistance2, bandWidth);
	// on gap: floating
	if (gapPos2 == 0) {
		compData->leg2Status = 0;
	}
	else 
	{
		int numBands2 = static_cast<int>((totalDistance2 - (bandWidth / 2.f)) / bandWidth);
		if ((numBands2 % 2) == 0)
			compData->leg2Status = -1;
		else
			compData->leg2Status = 1;
	}

	// leg 3
	float totalDistance3 = leg3PosX + (( simSetupData->numColTiles * simSetupData->tileLength ) / 2.f);
	float gapPos3 = fmod(totalDistance3, bandWidth);
	// on gap: floating
	if (gapPos3 == 0) {
		compData->leg3Status = 0;
	}
	else 
	{
		int numBands3 = static_cast<int>((totalDistance3 - (bandWidth / 2.f)) / bandWidth);
		if ((numBands3 % 2) == 0)
			compData->leg3Status = -1;
		else
			compData->leg3Status = 1;
	}
}


void DSim::sensorController()
{
	// RGB Sensing active only when projector is set up.
	if(projSet)
	{
		std::vector<DSimDroplet *>::iterator d_it;
		std::vector<GPSInfo *>::iterator p_it;
		p_it = dropletPositions.begin();

		for(d_it = droplets.begin(); d_it < droplets.end(); d_it++)
		{
			DSimDroplet *pDroplet = *d_it;
			DropletSensorData *senseData;
			AccessSensorData(pDroplet, &senseData);
			uint8_t rgbaVal[4];

			GPSInfo *gpsInfo = *p_it;
			float xyPos[2]; xyPos[0] = gpsInfo->posX; xyPos[1] = gpsInfo->posY;

			projector->GetPixel(xyPos, rgbaVal);
			
			senseData->rIn = rgbaVal[0];
			senseData->gIn = rgbaVal[1];
			senseData->bIn = rgbaVal[2];

			if(p_it != dropletPositions.end())
				p_it++;
		}		 
	}
}


void DSim::calcRelativePos(unsigned int dID)
{
	if(dropletRelPos == NULL)
		dropletRelPos = new TrigArray(dropletPositions.size());

	GPSInfo *gpsData = dropletPositions[dID];

	for(unsigned int i = 0; i < dropletPositions.size(); i++)
	{
		if(i == dID) continue;

		dropletRelPos->AddData(
			dID, 
			i, 
			dropletPositions[i]->posX - gpsData->posX,
			dropletPositions[i]->posY - gpsData->posY,
			dropletPositions[i]->posZ - gpsData->posZ);
	}

	gpsData->lastRelPosUpdate = timer.getTotalST();
	gpsData->movedSinceLastUpdate = false;
}

void DSim::commController()
{
	unsigned int d_id = 0;
	std::vector<DSimDroplet *>::iterator it;

	for(unsigned int i = 0; i < dropletPositions.size(); i++)
	{
		GPSInfo *gpsData = dropletPositions[i];
		if(gpsData->movedSinceLastUpdate && 
			((timer.getTotalST() - gpsData->lastRelPosUpdate) > 
			DROPLET_REL_POS_UPDATE_TIME))
		{
			calcRelativePos(i);
		}
	}

	for(it = droplets.begin(); it != droplets.end(); it++)
	{
		DSimDroplet *sendDroplet = *it;
		DropletCommData *sendCommData;
		AccessCommData(sendDroplet, &sendCommData);

		// Check if this droplet wants to send a message
		if(sendCommData->sendActive)
		{

			/* If droplets are within the threshold distance then copy over msg buffers.
			*/
			for(unsigned int i = 0; i < dropletPositions.size(); i++)
			{
				if(i == d_id) continue;

				if(dropletRelPos->GetDistance(d_id, i) <= BROADCAST_THRESHOLD)
				{
					DSimDroplet *recvDroplet = droplets[i];
					DropletCommData *recvCommData;
					AccessCommData(recvDroplet, &recvCommData);

					for(unsigned int sendChannel = 0; sendChannel < NUM_COMM_CHANNELS; sendChannel++)
					{
						if(sendCommData->commChannels[sendChannel].outMsgLength > 0)
						{
							/* TODO : For now we just find an empty buffer to put the new
								message in. Eventually we will fill the buffers based on
								droplet range and bearing. By default we put the new
								message in channel 0 if all other channels are full.
							*/
							unsigned int recvChannel = 0;
							for(unsigned int c = 0; c < NUM_COMM_CHANNELS; c++)
							{
								if(recvCommData->commChannels[c].inMsgLength == 0)
								{
									recvChannel = c;
									break;
								}
							}

							recvCommData->commChannels[recvChannel].inMsgLength = 
								sendCommData->commChannels[sendChannel].outMsgLength;
							memcpy(recvCommData->commChannels[recvChannel].inBuf,
								sendCommData->commChannels[sendChannel].outBuf,
								IR_BUFFER_SIZE);

							recvCommData->commChannels[recvChannel].lastMsgInTimestamp = 
								sendCommData->commChannels[sendChannel].lastMsgOutTimestamp = 
								static_cast<uint16_t>(timer.getTotalST() * 1000);

							// TODO: Remove this later
							/*if(fh != NULL)
							{
								DropletCompData *sendDat, *recvDat;
								AccessCompData(sendDroplet, &sendDat);
								AccessCompData(recvDroplet, &recvDat);
								fprintf(fh, "Sent a msg from %u ---> %u\n", 
									sendDat->dropletID,
									recvDat->dropletID);
							}*/
						}
					}
				}
			}
			sendCommData->sendActive = false;

			// Clear out-going message buffers
			for(unsigned int sendChannel = 0; sendChannel < NUM_COMM_CHANNELS; sendChannel++)
			{
				// Clear the send message buffer on the sender droplet
				memset(
					sendCommData->commChannels[sendChannel].outBuf, 
					0, 
					IR_BUFFER_SIZE);
				sendCommData->commChannels[sendChannel].outMsgLength = 0;
			}
		}
		d_id++;
	}
}

void DSim::timerController()
{
	std::vector<DSimDroplet *>::iterator it;
	for(it = droplets.begin(); it != droplets.end(); it++)
	{
		DSimDroplet *pDroplet = *it;
		DropletTimeData *timeData;
		AccessTimeData(pDroplet, &timeData);

		for(int i = 0; i < DROPLET_NUM_TIMERS; i++)
		{
			if(timeData->trigger[i] == 0)
			{
				if((timeData->timer[i] -= simSetupData->timestep) <= 0.f)
					timeData->trigger[i] = 1;
			}
		}
	}
}

// ====== SimSetupData Class Functions ====== //

SimSetupData::SimSetupData(
	int numRowTiles,
	int numColTiles,
	float tileLength,
	float dropletRadius,
	float fps,
	bool autoBuildBoundaryWalls)
{
	this->fps = fps;
	this->timestep = 1000.0f / this->fps;
	this->numRowTiles = numRowTiles;
	this->numColTiles = numColTiles;
	this->tileLength  = tileLength;
	this->dropletRadius = dropletRadius;
	this->autoBuildBoundaryWalls = autoBuildBoundaryWalls;

	// TODO : Add STAGGERED_START as an input arg
	this->staggeredStart = STAGGERED_START;
}


SimSetupData::SimSetupData(const SimSetupData& setupData)
{
	this->fps = setupData.fps;
	this->timestep = setupData.timestep;
	this->numRowTiles = setupData.numRowTiles;
	this->numColTiles = setupData.numColTiles;
	this->tileLength  = setupData.tileLength;
	this->dropletRadius = setupData.dropletRadius;
	this->autoBuildBoundaryWalls = setupData.autoBuildBoundaryWalls;

	this->staggeredStart = setupData.staggeredStart;
}