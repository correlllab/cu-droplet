#include "DSimPhysicalObject.h"

DSimPhysicalObject::DSimPhysicalObject(ObjectPhysicsData *objPhysics)
{
	this->objPhysics = objPhysics;
}

DS_RESULT DSimPhysicalObject::_InitPhysics(
		SimPhysicsData *simPhysics,
		std::pair<float, float> startPosition,
		float startAngle)
{
	btCollisionShape *colShape = simPhysics->collisionShapes->at(objPhysics->colShapeIndex);
	
	btTransform temp;
	temp.setIdentity();
	btVector3 tmpHolder1, tmpHolder2; // Don't really need this for anything besides calling getBoundingSphere
	colShape->getAabb(temp,tmpHolder1, tmpHolder2);

	float height = -1.f * tmpHolder1.z();
	// Set the Droplet's initial position in the world. Remember that bullet uses a Y-Up coord system
	
	return this->_InitPhysics(simPhysics,startPosition,(height + 0.01f),startAngle);
}


DS_RESULT DSimPhysicalObject::_InitPhysics(
		SimPhysicsData *simPhysics,
		std::pair<float, float> startPosition,
		float startHeight,
		float startAngle)
{
	// Set up the bullet physics object for this Object
	btCollisionShape *colShape = simPhysics->collisionShapes->at(objPhysics->colShapeIndex);

	//rigidbody is dynamic if and only if objPhysics->mass is non zero, otherwise static
	bool isDynamic = (objPhysics->mass != 0.f);
	if (isDynamic)
		colShape->calculateLocalInertia(objPhysics->mass, objPhysics->localInertia);

	// Set the Object's initial position in the world. Remember that bullet uses a Y-Up coord system
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(
		btScalar(startPosition.first), 
		btScalar(startPosition.second),
		btScalar(startHeight))
	);

	// Set this Object's angular orientation using input parameter startAngle
	btQuaternion rotationQuat; 
	rotationQuat.setRotation(btVector3(
			btScalar(0.0), 
			btScalar(0.0),
			btScalar(1.0)
		), 
		btScalar(startAngle)
	);
	startTransform.setRotation(rotationQuat);

	// Add this Object as a rigid body in the physics sim
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		objPhysics->mass, 
		motionState, 
		colShape, 
		objPhysics->localInertia
	);
	rbInfo.m_friction = objPhysics->friction;

	if(isDynamic)
	{
		rbInfo.m_linearDamping = OBJECT_LINEAR_DAMPING;		// Add a damping coefficient
		rbInfo.m_angularDamping = OBJECT_ANGULAR_DAMPING;
	}

	btRigidBody *body = new btRigidBody(rbInfo);
	body->activate(true);
	body->setActivationState(DISABLE_DEACTIVATION);

	if(isDynamic)
	{
		// Constrain movement to XY axes and rotations along the Z axis only
		//body->setLinearFactor(btVector3(1, 1, 0));
		//body->setAngularFactor(btVector3(0, 0, 1));

		simPhysics->dynWorld->addRigidBody(		
			body, 
			SimPhysicsData::_dynObjCollisionBM,	// Dynamic Object Collision bitmask
			SimPhysicsData::_dynObjCollisionBM | 
				SimPhysicsData::_staticObjCollisionBM); // Object collides with static and dynamic objects
	}
	else // Static object
	{
		simPhysics->dynWorld->addRigidBody(		
			body, 
			SimPhysicsData::_staticObjCollisionBM,	// Static Object Collision bitmask
			SimPhysicsData::_dynObjCollisionBM); // Object collides with only dynamic objects
	}

	// Set the Object's world ID
	objPhysics->_worldID = simPhysics->_physicsWorldObjCounter++;

	return DS_SUCCESS;
}