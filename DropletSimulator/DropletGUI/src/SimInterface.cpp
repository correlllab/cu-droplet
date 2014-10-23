/**
* \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\SimInterface.cpp
*
* \brief	Implements the SimInterface class.
*/
#define SIMPLIFY_COLLISION_SHAPES
#include "SimInterface.h"

SimInterface::SimInterface(QObject *parent)
	: QObject(parent)
{	
	_sim = NULL;
	_dropletColors = NULL;
	_dropletPos = NULL;
	_dropletComm = NULL;
	_objectPos = NULL;
	_timer = NULL;
	_simSettings = getDefaultSettings();
	_simStatus.paused = false;
	_simRates.timeScale = 1.0;
	_simRates.limitRate = false;
	_timerID = 0;
	_timeUntilNextUpdate = 0;
	_simStatus.dropletShape = NULL;

	// TODO - abstract this better
	makeDropletCollisionShapeFromFile(QString("assets/Models/HQDroplet.obj"));
	_objectNames.single << "cube" << "sphere";
	_objectNames.multiple << "cubes" << "spheres";

	// Set default reset timer values here
	_simState.useResetTime = false;
    _simState.resetTime = DEFAULT_RESET_TIME;
	_simState.simTime = 0.0;
	_simState.realTime = 0.0;
}

SimInterface::~SimInterface()
{

	if (_timerID != 0)
	{
		killTimer(_timerID);
		_timerID = 0;
	}

	teardownSim();

}

void SimInterface::teardownSim()
{
	if (_sim != NULL)
	{
		_sim->Cleanup();
		delete _sim;
	}

	if (_dropletColors != NULL)
	{
		std::vector<uint8_t *>::iterator it;
		for (it = _dropletColors->begin(); it != _dropletColors->end() ; it++)
		{
			uint8_t *bob = *it;
			free(bob); 
		}
		delete _dropletColors;
		_dropletColors = NULL;

	}
	if (_dropletPos != NULL)
	{
		std::vector<ObjectPositionData *>::iterator it;
		for (it = _dropletPos->begin(); it != _dropletPos->end() ; it++)
		{
			ObjectPositionData *bob = *it;
			free(bob); 
		}
		delete _dropletPos;
		_dropletPos = NULL;
	}

	if (_dropletComm != NULL)
	{
		std::vector<DropletCommData *>::iterator commIt;
		for (commIt = _dropletComm->begin(); commIt != _dropletComm->end(); commIt++)
		{
			DropletCommData *bob = *commIt;
			free(bob);
		}
		delete _dropletComm;

		_dropletComm = NULL;
	}

	if (_objectPos != NULL)
	{
		std::vector<ObjectPositionData *>::iterator it;
		for (it = _objectPos->begin(); it != _objectPos->end() ; it++)
		{
			ObjectPositionData *bob = *it;
			free(bob); 

		}
		delete _objectPos;
		_objectPos = NULL;
	}

	if (_timer != NULL)
	{
		free(_timer);
		_timer = NULL;
	}

    _simState.simTime = 0.0;
    _simState.realTime = 0.0;
}

void SimInterface::Init()
{
	bool isPaused = _simStatus.paused;
	if (!isPaused)
		pause();

	teardownSim();

	// TIMER
	updateTiming();
	_timeUntilNextUpdate = 0;

	QTime time = QTime::currentTime();
	srand((uint)time.msec());

	_sim = new DSim();
	SimSetupData setupData(
		_simSettings.numRowTiles, 
		_simSettings.numColTiles, 
		_simSettings.tileLength, 
		_simSettings.dropletRadius,
		_simSettings.fps,
		true);
	_sim->Init(setupData);

	if (_simSettings.projecting)
	{
		QString fileDir = QString(DEFAULT_ASSETDIR).append(DEFAULT_PROJECTDIR);
		QFileInfo test(QString(fileDir).append(_simSettings.projTexture));
		if (test.exists())
		{
			qDebug() << "Found projection texture" << test.absoluteFilePath();
			if (_sim->SetUpProjector(fileDir.toStdString(),_simSettings.projTexture.toStdString()) == DS_WARNING)
			{
				qDebug() << "Error setting up projector";
				_simSettings.projecting = false;
				_simState.projTextureChanged = false;
			} else {
				_simState.projTexture = QImage(test.absoluteFilePath());
				_simState.projTextureChanged = true;
			}
		} else {
			qDebug() << "Error: file not found" << test.absoluteFilePath();
			_simSettings.projecting = false;
			_simState.projTextureChanged = false;

		}
	}

	btCollisionShape *floorShape = new btStaticPlaneShape(btVector3(0, 0, 1) , 0);
	btCollisionShape *xWallShape = new btBoxShape(btVector3(
		btScalar(_simSettings.tileLength * _simSettings.numRowTiles),  
		btScalar(_simSettings.wallWidth),
		btScalar(_simSettings.wallHeight)));
	btCollisionShape *yWallShape = new btBoxShape(btVector3(
		btScalar(_simSettings.wallWidth), 
		btScalar(_simSettings.tileLength * _simSettings.numColTiles),
		btScalar(_simSettings.wallHeight)));

	_simSettings.dropletOffset = _simStatus.dropletOffset;
	_simStatus.dropletShape->setLocalScaling(btVector3(
		_simSettings.dropletRadius,
		_simSettings.dropletRadius,
		_simSettings.dropletRadius));


	_sim->AddCollisionShape(floorShape, &_simStatus.btFloorShapeID);
	_sim->AddCollisionShape(xWallShape, &_simStatus.btXWallShapeID);
	_sim->AddCollisionShape(yWallShape, &_simStatus.btYWallShapeID);
	_sim->AddCollisionShape(_simStatus.dropletShape, &_simStatus.btDropletShapeID);

	_dropletColors = new std::vector<unsigned char *>();
	_dropletPos = new std::vector<ObjectPositionData *>();

	_dropletComm = new std::vector<DropletCommData *>();

	_objectPos = new std::vector<ObjectPositionData *>();

	_timer = (DSimTimeControl *)malloc(sizeof(DSimTimeControl));

	_simState.dropletData.clear();
	_simState.dynamicObjectData.clear();
	_simState.staticObjectData.clear();
	_simState.collisionShapes.clear();

	qDebug() << "floor file" << _simSettings.floorFile;
	createArena();

	if (_simSettings.startingObjects.count() > 0)
	{
		float floorWidth = _simSettings.tileLength * _simSettings.numColTiles;
		float floorLength = _simSettings.tileLength * _simSettings.numRowTiles;
		float posRangeWidth = floorWidth / 2.0f;
		float posRangeLength = floorLength / 2.0f;
		foreach(QStringList list,_simSettings.startingObjects)
		{	
			QString iType = list[0].toLower();
			if (_objectNames.multiple.contains(iType))
			{
				if (list.count() >= 2)
				{
					int num = list[1].toInt();
					object_t oType = Cube;
					float radius = DEFAULT_OBJECT_RADIUS;

					if (list.count() >= 3)
					{
						float temp = list[2].toFloat();
						if (temp > 0)
							radius = temp;
					}

					if (iType == QString("spheres"))
					{
						oType = Sphere;
					} else if (iType == QString("cubes"))
					{
						oType = Cube;
						radius *= 2.0;
					} 
					float mass = DEFAULT_OBJECT_MASS;
					float friction = DEFAULT_OBJECT_MASS;
					if (list.count() >= 4)
					{
						mass = list[3].toFloat();
					} 

					if (list.count() >= 5)
					{
						friction = list[4].toFloat();
					}
					
					for(int i = 0; i < num; i++)
					{
						// prevents index out of bound errors
						int tileIndex = rand()%_tilePositions.size();

						float xPos = getRandomf(_tilePositions[tileIndex].x - _simSettings.tileLength/2.0 + _simSettings.dropletRadius,
							_tilePositions[tileIndex].x + _simSettings.tileLength/2.0 - _simSettings.dropletRadius);
						float yPos = getRandomf(_tilePositions[tileIndex].y - _simSettings.tileLength/2.0 + _simSettings.dropletRadius,
							_tilePositions[tileIndex].y + _simSettings.tileLength/2.0 - _simSettings.dropletRadius);

						addObject(oType, xPos,yPos,radius,mass,friction);
					}
				}
			} else if (_objectNames.single.contains(iType))
			{
				if (list.count() >= 3)
				{
					object_t oType = Cube;
					float radius = DEFAULT_OBJECT_RADIUS;

					if (list.count() >= 4)
					{
						float temp = list[3].toFloat();
						if (temp > 0)
							radius = temp;
					}

					if (iType == QString("sphere"))
					{
						oType = Sphere;
					} else if (iType == QString("cube"))
					{
						oType = Cube;
						radius *= 2.0;
					} 
					float xPos = list[1].toFloat();
					float yPos = list[2].toFloat();
					float mass = DEFAULT_OBJECT_MASS;
					float friction = DEFAULT_OBJECT_MASS;
					if (list.count() >= 4)
					{
						mass = list[3].toFloat();
					}
					if (list.count() >= 5)
					{
						friction = list[4].toFloat();
					}
					
					addObject(oType, xPos,yPos,radius,mass,friction);

				}
			}
		}
	}
	_sim->Step();

	emit simulationUpdated(_simState);
	emit arenaChanged(_simSettings);
	emit ratesChanged(_simRates);


	float floorWidth = _simSettings.tileLength * _simSettings.numColTiles;
	float floorLength = _simSettings.tileLength * _simSettings.numRowTiles;
	float posRangeWidth = floorWidth / 2.0f;
	float posRangeLength = floorLength / 2.0f;

	if (_simSettings.startingDroplets.count() > 0)
	{
		float floorWidth = _simSettings.tileLength * _simSettings.numColTiles;
		float floorLength = _simSettings.tileLength * _simSettings.numRowTiles;
		float posRangeWidth = floorWidth / 2.0f;
		float posRangeLength = floorLength / 2.0f;
		foreach(QStringList list,_simSettings.startingDroplets)
		{
			if (list.count() >= 2)
			{

				droplet_t dType;

				QString iType = list[0].toLower();
				if (iType == QString("customone"))
				{
					dType = CustomOne;
				} else if (iType == QString("customtwo"))
				{
					dType = CustomTwo;
				} else if (iType == QString("customthree"))
				{
					dType = CustomThree;
				} else if (iType == QString("customfour"))
				{
					dType = CustomFour;
				} else if (iType == QString("customfive"))
				{
					dType = CustomFive;
				} else if (iType == QString("customsix"))
				{
					dType = CustomSix;
				} else if (iType == QString("customseven"))
				{
					dType = CustomSeven;
				} else if (iType == QString("customeight"))
				{
					dType = CustomEight;
				} else if (iType == QString("customnine"))
				{
					dType = CustomNine;
				} else if (iType == QString("customten"))
				{
					dType = CustomTen;
				} else 
				{
					dType = CustomOne;
				}

				if (list.count() == 2)
				{
					int num = list[1].toInt();
					for(int i = 0; i < num; i++)
					{
						// prevents index out of bound errors
						int tileIndex = rand()%_tilePositions.size();

						float xPos = getRandomf(_tilePositions[tileIndex].x - _simSettings.tileLength/2.0 + _simSettings.dropletRadius,
							_tilePositions[tileIndex].x + _simSettings.tileLength/2.0 - _simSettings.dropletRadius);
						float yPos = getRandomf(_tilePositions[tileIndex].y - _simSettings.tileLength/2.0 + _simSettings.dropletRadius,
							_tilePositions[tileIndex].y + _simSettings.tileLength/2.0 - _simSettings.dropletRadius);
                        float angle = rand() % 360;

						addDroplet(xPos,yPos,angle,dType);
					}
				} else if (list.count() >= 3) 
				{
					float xPos  = list[1].toFloat();
					float yPos  = list[2].toFloat();
                    float angle = rand() % 360;
                    if (list.count() > 3 )
                    {
                        angle = list[3].toFloat();
                    }
					addDroplet(xPos,yPos,angle,dType);
				}
			}

		}
	}

	// step the simulator
	_sim->Step();
	_simStatus.paused = false;
	Update(1000.0/_simSettings.fps);
	_simStatus.paused = isPaused;
	// restore state
	if (!isPaused)
		resume();
}

void SimInterface::loadTilePositions()
{
	//Floor Tiles
	_tilePositions.clear();
	_wallBools.clear();
	for(int i = 0; i < _simSettings.numColTiles; i++)
	{
		for(int j = 0; j < _simSettings.numRowTiles; j++)
		{
			vec2 currentTile;
			currentTile.x = ((float)i - (_simSettings.numColTiles-1)/2.0f)* _simSettings.tileLength;
			currentTile.y = ((float)j - (_simSettings.numRowTiles-1)/2.0f) * _simSettings.tileLength;
		
			_tilePositions.push_back(currentTile);

			//currentWallBool represents which sides of the tile need walls
			vec4 currentWallBool;
			currentWallBool.v[0] = 0;
			currentWallBool.v[1] = 0;
			currentWallBool.v[2] = 0;
			currentWallBool.v[3] = 0;
			
			// Top Wall
			if(j == _simSettings.numRowTiles - 1)
			{
				currentWallBool.v[0] = 1;
			}
			// Right Wall
			if(i == _simSettings.numColTiles - 1)
			{
				currentWallBool.v[1] = 1;
			}
			// Bottom Wall
			if(j == 0)
			{
				currentWallBool.v[2] = 1;
			}
			// Left Wall
			if(i == 0)
			{
				currentWallBool.v[3] = 1;
			}
			_wallBools.push_back(currentWallBool);

		}
	}
}

void SimInterface::loadTilePositions(QString filename)
{
	_simSettings.floorFile = QString(filename).append(".txt");
	_tilePositions.clear();
	_wallBools.clear();
	if(filename.operator!=("Default (Rectangle)"))
	{
		QFile file(QString(DEFAULT_ASSETDIR).append(DEFAULT_FLOORDIR).append(_simSettings.floorFile));

		if (file.exists())
		{
			if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
			{
				QTextStream in(&file);
				while (!in.atEnd())
				{
					QString line = in.readLine(0);
					if (line.count() > 0)
					{
						if (!line.startsWith("#"))
						{
							QStringList list = line.split(" ",QString::SkipEmptyParts);
							//Store tile position in a QVector of tile positions
							vec2 currentTile;
							currentTile.x = list[0].toFloat() * _simSettings.tileLength;
							currentTile.y = list[1].toFloat() * _simSettings.tileLength;
							_tilePositions.push_back(currentTile);

							//currentWallBool represents which sides of the tile need walls
							vec4 currentWallBool;
							currentWallBool.v[0] = 0;
							currentWallBool.v[1] = 0;
							currentWallBool.v[2] = 0;
							currentWallBool.v[3] = 0;
							// Top Wall
							if(list[2] == QString("yes"))
							{
								currentWallBool.v[0] = 1;
							}
							// Right Wall
							if(list[3] == QString("yes"))
							{
								currentWallBool.v[1] = 1;
							}
							// Bottom Wall
							if(list[4] == QString("yes"))
							{
								currentWallBool.v[2] = 1;
							}
							// Left Wall
							if(list[5] == QString("yes"))
							{
								currentWallBool.v[3] = 1;
							}
							//Store the wallBools
							// Needs to match up with _tilePositions indices wise
							// i.e. _tilePositions[0] and _wallBools[0] refer to same tile
							_wallBools.push_back(currentWallBool);
						}
					}
				}

			} 
			else
			{
				qDebug() << "Error: " << file.error();	
				file.close();
			}
			file.close();
			//Find boundin box custom arena lies in
			float xMin = std::numeric_limits<float>::max();
			float xMax = std::numeric_limits<float>::min();
			float yMin = std::numeric_limits<float>::max();
			float yMax = std::numeric_limits<float>::min();
			foreach(vec2 tilePosition, _tilePositions){
				xMin = qMin(xMin, tilePosition.x);
				xMax = qMax(xMax, tilePosition.x);
				yMin = qMin(yMin, tilePosition.y);
				yMax = qMax(yMax, tilePosition.y);
			}
			//For projection purposes
			// min and max are at the center of each tile, so xMax - xMin isn't accounting for a full tileLength
			_simSettings.numColTiles = qAbs(xMax - xMin)/_simSettings.tileLength + 1;
			_simSettings.numRowTiles = qAbs(yMax - yMin)/_simSettings.tileLength + 1;
			//xOffset and yOffset represent the center of the custom arena
			float xOffset = (xMax + xMin)/2;
			float yOffset = (yMax + yMin)/2;
			for(int i = 0; i < _tilePositions.size(); i++)
			{
				//Modify tilePosition to center the arena to the origin
				_tilePositions[i].x -= xOffset;
				_tilePositions[i].y -= yOffset;
			}
		} 
		else
		{
			qDebug() << "Error: file does not exist - using compiled defaults";
		}
	}
}

void SimInterface::createArena()
{
	if(_simSettings.floorFile.operator==("Default (Rectangle)"))
	{
		loadTilePositions();
	}
	vec3 tileScale;
	tileScale.x = _simSettings.tileLength;
	tileScale.y = _simSettings.tileLength;
	tileScale.z = 0.1f;
	vec3 hWallScale;
	hWallScale.x = _simSettings.tileLength;
	hWallScale.y = _simSettings.wallWidth;
	hWallScale.z = _simSettings.wallHeight;
	vec3 vWallScale;
	vWallScale.x = _simSettings.wallWidth;
	vWallScale.y = _simSettings.tileLength;
	vWallScale.z = _simSettings.wallHeight;

	for(int i = 0; i < _tilePositions.size(); i++)
		{
			addFloor(_tilePositions[i].x, _tilePositions[i].y, 0, tileScale);
			//Top Wall
			if(_wallBools[i].v[0] == 1)
			{
				float xPos = _tilePositions[i].x;
				float yPos = _tilePositions[i].y + _simSettings.tileLength/2.0f;
				addWall(xPos, yPos,0,hWallScale);
			}
			//Right Wall
			if(_wallBools[i].v[1] == 1)
			{
				float xPos = _tilePositions[i].x + _simSettings.tileLength/2.0f;
				float yPos = _tilePositions[i].y;
				addWall(xPos,yPos,0,vWallScale);
			}
			//Bottom Wall
			if(_wallBools[i].v[2] == 1)
			{
				float xPos = _tilePositions[i].x;
				float yPos = _tilePositions[i].y - _simSettings.tileLength/2.0f;
				addWall(xPos,yPos,0,hWallScale);
			}
			//Left Wall
			if(_wallBools[i].v[3] == 1)
			{
				float xPos = _tilePositions[i].x -_simSettings.tileLength/2.0f;
				float yPos = _tilePositions[i].y;
				addWall(xPos,yPos,0,vWallScale);
			}
		}
}

void SimInterface::addDroplet( float x, float y, float angle, droplet_t dType, int dropletID )
{
	bool isPaused = _simStatus.paused;
	_simStatus.paused = true;
	unsigned char *tmp1 = (unsigned char *)malloc(sizeof(unsigned char) * 3);
	_dropletColors->push_back(tmp1);
	ObjectPositionData *tmp2 = (ObjectPositionData *)malloc(sizeof(ObjectPositionData));
	_dropletPos->push_back(tmp2);

	DropletCommData *tmp3 = (DropletCommData *)malloc(sizeof(DropletCommData));
	_dropletComm->push_back(tmp3);

	dropletStruct_t droplet;

	if (dropletID == 0)
	{
		// make up a droplet ID
		droplet.dropletID = _simState.dropletData.count() + 1;
	} else {

		droplet.dropletID = dropletID;
	}

	vec3 zero = {0.0f,0.0f,0.0f};
	droplet.color = vec3imake(0,0,0);
	droplet.origin = vec3make(x,y,0);
	droplet.rotation = zero;
	droplet.commData.sendActive = false;
	droplet.changed = true;

	_simState.dropletData.append(droplet);

	// Set up the simulator/physics model
	ObjectPhysicsData *dropletPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
	dropletPhyDat->colShapeIndex = _simStatus.btDropletShapeID;
	dropletPhyDat->mass = DEFAULT_DROPLET_MASS;
	dropletPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
	dropletPhyDat->friction = DEFAULT_DROPLET_FRICTION;

	DSimDroplet *newDroplet = newDropletOfType(dType,dropletPhyDat);

	_sim->AddDroplet(
		newDroplet, 
		std::make_pair(droplet.origin.x, droplet.origin.y),
		angle
		);
	_simStatus.paused = isPaused;

}

//Adding collision objects. Returns the index of the corresponding shape in the list
//of collision objects.
int SimInterface::addNewShape(object_t objectType, float radius, vec3 scale){
	if(objectType == Sphere){
		if(_simState.collisionShapes.count() > 0){
			collisionShapeStruct_t shape;
			int i;
			for(i = 0; i < _simState.collisionShapes.count(); i++){
				shape = _simState.collisionShapes.at(i);
				if((shape.oType == Sphere) && (shape.objectRadius == radius)){
					return i;
				}
			}
		}

		//Add new shape to list
		vec3 one = {1.0, 1.0, 1.0};
		collisionShapeStruct_t newShape;
		_simState.collisionShapes.append(newShape);

		_simState.collisionShapes[_simState.collisionShapes.count() - 1].oType = Sphere;
		_simState.collisionShapes[_simState.collisionShapes.count() - 1].objectRadius = radius;
		_simState.collisionShapes[_simState.collisionShapes.count() - 1].scale = one;

		// Add collision shape
		btCollisionShape *sphereShape = new btSphereShape(
			btScalar(radius));

		// Get indeces
		_sim->AddCollisionShape(sphereShape, &(_simState.collisionShapes[_simState.collisionShapes.count() - 1].collisionID));

		return _simState.collisionShapes.count() - 1;
	}

	if((objectType == Cube) || (objectType == Wall) || (objectType == Floor)){
		if(_simState.collisionShapes.count() > 0){
			collisionShapeStruct_t shape;
			int i;
			for(i = 0; i < _simState.collisionShapes.count(); i++){
				shape = _simState.collisionShapes.at(i);
				if(((shape.oType == Cube) && (objectType == Cube)) && (shape.scale.x == scale.x) && (shape.scale.y == scale.y) && (shape.scale.z == scale.z)){
					return i;
				}
				if(((shape.oType == Wall) && (objectType == Wall)) && (shape.scale.x == scale.x) && (shape.scale.y == scale.y) && (shape.scale.z == scale.z)){
					return i;
				}
				if(((shape.oType == Floor) && (objectType == Floor)) && (shape.scale.x == scale.x) && (shape.scale.y == scale.y) && (shape.scale.z == scale.z)){
					return i;
				}
			}
		}

		//Add new shape to list
		vec3 one = {1.0, 1.0, 1.0};
		collisionShapeStruct_t newShape;
		_simState.collisionShapes.append(newShape);

		if(objectType == Cube){_simState.collisionShapes[_simState.collisionShapes.count() - 1].oType = Cube;}
		if(objectType == Wall){_simState.collisionShapes[_simState.collisionShapes.count() - 1].oType = Wall;}
		if(objectType == Floor){_simState.collisionShapes[_simState.collisionShapes.count() - 1].oType = Floor;}
		_simState.collisionShapes[_simState.collisionShapes.count() - 1].objectRadius = one.x;
		_simState.collisionShapes[_simState.collisionShapes.count() - 1].scale = scale;

		// Add collision shape
		btCollisionShape *cubeShape = new btBoxShape(btVector3(
			btScalar(0.5*scale.x), 
			btScalar(0.5*scale.y),
			btScalar(0.5*scale.z)));

		// Get indeces
		_sim->AddCollisionShape(cubeShape, &(_simState.collisionShapes[_simState.collisionShapes.count() - 1].collisionID));

		return _simState.collisionShapes.count() - 1;
	}

	return NULL;
}

void SimInterface::addSphere( float x, float y, int objectID, float radius, float mass, float friction)
{

	bool isPaused = _simStatus.paused;
	_simStatus.paused = true;
	ObjectPositionData *tmp2 = (ObjectPositionData *)malloc(sizeof(ObjectPositionData));
	_objectPos->push_back(tmp2);
	objectStruct_t object;

	if (objectID == 0)
	{
		// make up an object ID
		object.objectID = _simState.dynamicObjectData.count() + 1;
	} else {

		object.objectID = objectID;
	}

	vec3 scale = {1.0f,1.0f,1.0f};
	vec4 zero = {0.0f,0.0f,0.0f,0.0f};
	object.color = vec3imake(rand()%256,rand()%256,rand()%256);
	object.origin = vec3make(x,y,radius);

	object.objectRadius = radius;
	object.scale = scale;
	object.quaternion = zero;
	object.changed = true;
	object.oType = Sphere;

	_simState.dynamicObjectData.append(object);

	//Get shape colShapeIndex
	int colIndex = addNewShape(object.oType, object.objectRadius, scale);

	// Set up the simulator/physics model
	ObjectPhysicsData *objectPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
	objectPhyDat->colShapeIndex = _simState.collisionShapes[colIndex].collisionID;
	objectPhyDat->mass = mass;
	objectPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
	objectPhyDat->friction = friction;

	//Create physical object
	DSimPhysicalObject *newobject = new DSimPhysicalObject(objectPhyDat);

	//Add to sim
	_sim->AddPhysicalObject(
		newobject, 
		std::make_pair(object.origin.x,object.origin.y),
		0.0f
		);

	_simStatus.paused = isPaused;
}

void SimInterface::addCube( float x, float y, int objectID, vec3 scale, float mass, float friction)
{

	bool isPaused = _simStatus.paused;
	_simStatus.paused = true;
	ObjectPositionData *tmp2 = (ObjectPositionData *)malloc(sizeof(ObjectPositionData));
	_objectPos->push_back(tmp2);
	objectStruct_t object;

	if (objectID == 0)
	{
		// make up an object ID
		object.objectID = _simState.dynamicObjectData.count() + 1;
	} else {

		object.objectID = objectID;
	}

	vec4 zero = {0,0,0,0};
	object.color = vec3imake(rand()%256,rand()%256,rand()%256);
	object.origin = vec3make(x,y,0.5*scale.z);

	object.objectRadius = 1.0;
	object.scale = scale;
	object.quaternion = zero;
	object.changed = true;
	object.oType = Cube;

	_simState.dynamicObjectData.append(object);

	//Get shape colShapeIndex
	int colIndex = addNewShape(object.oType, object.objectRadius, scale);

	// Set up the simulator/physics model
	ObjectPhysicsData *objectPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
	objectPhyDat->colShapeIndex = _simState.collisionShapes.at(colIndex).collisionID;
	objectPhyDat->mass = mass;
	objectPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
	objectPhyDat->friction = friction;

	//Create physical object
	DSimPhysicalObject *newobject = new DSimPhysicalObject(objectPhyDat);

	//Add to sim
	_sim->AddPhysicalObject(
		newobject, 
		std::make_pair(object.origin.x,object.origin.y),
		0.0f
		);

	_simStatus.paused = isPaused;
}

void SimInterface::addWall( float x, float y, int objectID, vec3 scale)
{

	bool isPaused = _simStatus.paused;
	_simStatus.paused = true;
	objectStruct_t object;

	if (objectID == 0)
	{
		// make up an object ID
		object.objectID = _simState.staticObjectData.count() + 1;
	} else {

		object.objectID = objectID;
	}

	vec4 zero = {0,0,0,0};
	object.color = vec3imake(rand()%256,rand()%256,rand()%256);
	object.origin = vec3make(x,y,0.5*scale.z);
	object.objectRadius = 1.0;
	object.scale = scale;
	object.quaternion = zero;
	object.changed = true;
	object.oType = Wall;

	_simState.staticObjectData.append(object);

	//Get shape colShapeIndex
	int colIndex = addNewShape(object.oType, object.objectRadius, scale);

	// Set up the simulator/physics model
	ObjectPhysicsData *objectPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
	objectPhyDat->colShapeIndex = _simState.collisionShapes[colIndex].collisionID;
	objectPhyDat->mass = DEFAULT_WALL_MASS;
	objectPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
	objectPhyDat->friction = DEFAULT_WALL_FRICTION;

	//Create physical object
	DSimPhysicalObject *newobject = new DSimPhysicalObject(objectPhyDat);

	//Add to sim
	_sim->AddPhysicalObject(
		newobject, 
		std::make_pair(object.origin.x,object.origin.y),
		object.origin.z,
		0.0f
		);

	_simStatus.paused = isPaused;
}

void SimInterface::addFloor( float x, float y, int objectID, vec3 scale)
{

	bool isPaused = _simStatus.paused;
	_simStatus.paused = true;
	objectStruct_t object;

	if (objectID == 0)
	{
		// make up an object ID
		object.objectID = _simState.staticObjectData.count() + 1;
	} else {

		object.objectID = objectID;
	}
	vec4 zero = {0,0,0,0};
	object.color = vec3imake(rand()%256,rand()%256,rand()%256);
	object.origin = vec3make(x,y,0);
	object.objectRadius = 1.0;
	object.scale = scale;
	object.quaternion = zero;
	object.changed = true;
	object.oType = Floor;

	_simState.staticObjectData.append(object);

	//Get shape colShapeIndex
	int colIndex = addNewShape(object.oType, object.objectRadius, scale);

	// Set up the simulator/physics model
	ObjectPhysicsData *objectPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
	objectPhyDat->colShapeIndex = _simState.collisionShapes[colIndex].collisionID;
	objectPhyDat->mass = DEFAULT_FLOOR_MASS;
	objectPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
	objectPhyDat->friction = DEFAULT_FLOOR_FRICTION;

	//Create physical object
	DSimPhysicalObject *newobject = new DSimPhysicalObject(objectPhyDat);

	//Add to sim
	_sim->AddPhysicalObject(
		newobject, 
		std::make_pair(object.origin.x,object.origin.y),
		object.origin.z,
		0.0f
		);

	_simStatus.paused = isPaused;
}


void SimInterface::addObject(object_t kind, float x, float y, float radius, float mass, float friction)
{
	switch(kind)
	{
	case Sphere:
		addSphere(x,y,0,radius, mass, friction);
		break;
	case Cube:
		addCube(x,y,0,vec3make(radius,radius,radius), mass, friction);
		break;
	}
}

float SimInterface::getRandomf(float min, float max)
{
	uint randNum = rand();
	float range = max - min;
	return (min + ((range * randNum) / RAND_MAX));
}


void SimInterface::Update(float timeSinceLastUpdate)
{
	if (_sim != NULL)
	{
		if (!_simStatus.paused)
		{
			_sim->Step();
			_simInfo.GetDropletColors(_dropletColors,*_sim);
			_simInfo.GetDropletPositions(_dropletPos,*_sim);

			_simInfo.GetCommData(_dropletComm,*_sim);

			_simInfo.GetObjectPositions(_objectPos, *_sim);

			_simStatus.runTime = _simInfo.GetTotalST(*_sim);
		}

		std::vector<ObjectPositionData *>::iterator it;
		it = _dropletPos->begin();
		std::vector<DropletCommData *>::iterator commIt;
		commIt = _dropletComm->begin();

		for(int i = 0 ; i < _dropletPos->size(); i++)
		{

			dropletStruct_t droplet = _simState.dropletData[i];
			ObjectPositionData *ObjectPositionData = *it;
			DropletCommData *dropletCommData = *commIt;
			bool posChanged = false;


			glm::quat quaternion = glm::angleAxis(ObjectPositionData->rotA * 180.f / SIMD_PI,ObjectPositionData->rotX,ObjectPositionData->rotY,ObjectPositionData->rotZ);
			glm::vec3 rotation = glm::eulerAngles(quaternion);

			vec4 newQuaternion = {
				quaternion.x,
				quaternion.y,
				quaternion.z,
				quaternion.w
			};

			vec3 newRotation = {
				rotation.x,
				rotation.y,
				rotation.z
			};


			vec3 newOrigin = {
				ObjectPositionData->posX,
				ObjectPositionData->posY,
				ObjectPositionData->posZ
			};

			vec3i newColor = {
				_dropletColors->at(i)[0],
				_dropletColors->at(i)[1],
				_dropletColors->at(i)[2]
			};

			droplet.commData.sendActive = dropletCommData->sendActive;
			int j;
			for (j = 0; j < 6; j++)
			{
				droplet.commData.commChannels[j].lastMsgInTimestamp = 
					dropletCommData->commChannels[j].lastMsgInTimestamp;
				droplet.commData.commChannels[j].inMsgLength =
					dropletCommData->commChannels[j].inMsgLength;
				droplet.commData.commChannels[j].lastMsgOutTimestamp =
					dropletCommData->commChannels[j].lastMsgOutTimestamp;
				droplet.commData.commChannels[j].outMsgLength =
					dropletCommData->commChannels[j].outMsgLength;
				memcpy(droplet.commData.commChannels[j].inBuf,
					dropletCommData->commChannels[j].inBuf, IR_BUFFER_SIZE);
				memcpy(droplet.commData.commChannels[j].outBuf,
					dropletCommData->commChannels[j].outBuf, IR_BUFFER_SIZE);
			}

			if (newQuaternion.v != droplet.quaternion.v)
			{
				droplet.quaternion = newQuaternion;
				droplet.rotation = newRotation;
				droplet.changed = true;

			}

			if (newOrigin.v != droplet.origin.v)
			{
				droplet.origin = newOrigin;
				droplet.changed = true;
			}

			if (droplet.color.v != newColor.v)
			{
				droplet.color = newColor;
				droplet.changed = true;
			}
			if (droplet.changed == true)
			{
				_simState.dropletData[i]=droplet;
			}		

			it++;
			commIt++;
		}

		//Update object positions
		std::vector<ObjectPositionData *>::iterator itobj;
		itobj = _objectPos->begin();

		for(int i = 0 ; i < _objectPos->size(); i++)
		{

			objectStruct_t object = _simState.dynamicObjectData[i];
			ObjectPositionData *ObjectPositionData = *itobj;
			bool posChanged = false;

			glm::quat quaternion = glm::angleAxis(ObjectPositionData->rotA * 180.f / SIMD_PI,ObjectPositionData->rotX,ObjectPositionData->rotY,ObjectPositionData->rotZ);
			glm::vec3 rotation = glm::eulerAngles(quaternion);

			vec4 newQuaternion = {
				quaternion.x,
				quaternion.y,
				quaternion.z,
				quaternion.w
			};

			vec3 newRotation = {
				rotation.x,
				rotation.y,
				rotation.z
			};


			vec3 newOrigin = {
				ObjectPositionData->posX,
				ObjectPositionData->posY,
				ObjectPositionData->posZ
			};


			if (newQuaternion.v != object.quaternion.v)
			{
				object.quaternion = newQuaternion;
				object.rotation = newRotation;
				object.changed = true;

			}

			if (newOrigin.v != object.origin.v)
			{
				object.origin = newOrigin;
				object.changed = true;
			}

			if (object.changed == true)
			{
				_simState.dynamicObjectData[i]=object;
			}		
			itobj++;
		}

		_simState.simTime = _simStatus.runTime;
		_simState.realTime = _simInfo.GetTotalRT(*_sim);
		_simState.timeRatio = _simInfo.GetTimeRatio(*_sim);
		emit simulationUpdated(_simState);

		if (_simState.projTextureChanged)
			_simState.projTextureChanged = false;
	}

	// reset the simulation if the reset timer is on
	if(_simState.useResetTime && (_simState.resetTime <= _simState.simTime))
	{
		reset();
	}
}

void SimInterface::pause()
{
	_simStatus.paused = true;
	if (_timerID != 0)
	{
		killTimer(_timerID);
		_timerID = 0;
	}
	emit pauseChanged(_simStatus.paused);
}

void SimInterface::resume()
{
	_simStatus.paused = false;
	_updateTimer.start();
	_timerID = startTimer(0);
	emit pauseChanged(_simStatus.paused);
}

void SimInterface::togglePause()
{
	if (_simStatus.paused)
	{
		resume();
	} else
	{
		pause();
	}
}

bool SimInterface::isPaused()
{
	return _simStatus.paused;
}


DSimDroplet* SimInterface::newDropletOfType(droplet_t dType, ObjectPhysicsData *dropletPhyDat)
{
	DSimDroplet* result = NULL;

	switch (dType)
	{
	case CustomOne:
		result = new DropletCustomOne(dropletPhyDat);
		break;
	case CustomTwo:
		result = new DropletCustomTwo(dropletPhyDat);
		break;
	case CustomThree:
		result = new DropletCustomThree(dropletPhyDat);
		break;
	case CustomFour:
		result = new DropletCustomFour(dropletPhyDat);
		break;
	case CustomFive:
		result = new DropletCustomFive(dropletPhyDat);
		break;
	case CustomSix:
		result = new DropletCustomSix(dropletPhyDat);
		break;
	case CustomSeven:
		result = new DropletCustomSeven(dropletPhyDat);
		break;
	case CustomEight:
		result = new DropletCustomEight(dropletPhyDat);
		break;
	case CustomNine:
		result = new DropletCustomNine(dropletPhyDat);
		break;
	case CustomTen:
		result = new DropletCustomTen(dropletPhyDat);
		break;
	default:
		result = new DropletCustomOne(dropletPhyDat);

	}
	return result;
}

void SimInterface::reset()
{
    emit resetLogger();
	Init();
}

simSetting_t SimInterface::getSimulatorSettings()
{
	return _simSettings;
}

simState_t SimInterface::getSimulatorState()
{
	return _simState;
}

void SimInterface::updateTiming()
{	
	if (_simRates.limitRate)
	{
		if (_simSettings.fps > 0 && _simRates.timeScale > 0)
		{
			float rate = 1000.0 / (_simSettings.fps * _simRates.timeScale);
			if (rate != _targetUpdateTime)
			{
				_targetUpdateTime = rate;
				qDebug() << "Set update goal for" <<  _targetUpdateTime << "ms";
			}
		}
	} else
	{
		_targetUpdateTime = 0;
	}
}

void SimInterface::timerEvent ( QTimerEvent * event ) 
{
	qint64 nsElapsed = _updateTimer.nsecsElapsed();
	_timeUntilNextUpdate += nsElapsed / 1000000.0;
	_updateTimer.start();

	if (_timeUntilNextUpdate >= _targetUpdateTime)
	{
		Update(1000.0/_simSettings.fps);
		if (_targetUpdateTime != 0)
		{
			_timeUntilNextUpdate -= _targetUpdateTime;
		} else {
			_timeUntilNextUpdate = 0;
		}
	}



}

void SimInterface::loadArena(simSetting_t arena)
{
	_simSettings = arena;
	Init();
}

simSetting_t SimInterface::getDefaultSettings()
{
	simSetting_t newSettings;
	newSettings.dropletRadius = DEFAULT_DROPLET_RADIUS;
	newSettings.fps = DEFAULT_FPS;
	newSettings.numColTiles = DEFAULT_COL_TILES;
	newSettings.numRowTiles = DEFAULT_ROW_TILES;
	newSettings.tileLength = DEFAULT_TILE_LENGTH;
	newSettings.wallWidth = DEFAULT_WALL_WIDTH;
	newSettings.wallHeight = DEFAULT_WALL_HEIGHT;
	newSettings.dropletRadius = DEFAULT_DROPLET_RADIUS;
	newSettings.dropletOffset = 0.0f;
	newSettings.projecting = false;
	newSettings.projTexture	= QString("none.bmp");
	newSettings.floorFile = QString("Default (Rectangle)");
	return newSettings;
}

simSetting_t SimInterface::getCurrentSettings()
{
	return _simSettings;
}

void SimInterface::makeDropletCollisionShapeFromFile(QString fileName)
{
	if (_simStatus.dropletShape != NULL)
	{
		delete _simStatus.dropletShape;
		_simStatus.dropletShape = NULL;
	}
	btConvexHullShape *shape = new btConvexHullShape();

	QVector<btVector3> points;
	float highest = std::numeric_limits<float>::min();
	float lowest = std::numeric_limits<float>::max();
	float range;
	btVector3 offset;

	QString inLine;
	int n;

	QFile file(fileName);


	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
		{
			QTextStream in(&file);

			while (!in.atEnd())
			{
				inLine = in.readLine(0);
				if(inLine.length() == 0)
					continue;
				// qDebug() << inLine;

				QStringList list = inLine.split(" ",QString::SkipEmptyParts);

				if(list[0] == QString("v") && list.size() == 4)
				{
					btVector3 vertex;
					float z = list[2].toFloat();

					if (z > highest)
						highest = z;
					if (z < lowest)
						lowest = z;

					vertex.setX(list[1].toFloat());
					vertex.setY(list[3].toFloat());
					vertex.setZ(z);
					points.append(vertex);
				}
			}
			if (highest >= 0)
				range = highest - lowest;
			else
				range = highest + lowest;

			_simStatus.dropletOffset = -(lowest + DEFAULT_DROPLET_CENTER_OF_MASS * range);
			offset = btVector3(0,0,_simStatus.dropletOffset);
			qDebug() << "Droplet offset" << _simStatus.dropletOffset;
			foreach(btVector3 vert,points)
			{
				btVector3 newvert;
				newvert = vert + offset;
				shape->addPoint(newvert);
			}

#ifdef SIMPLIFY_COLLISION_SHAPES
			btShapeHull* hull = new btShapeHull(shape);
			btScalar margin = shape->getMargin();
			hull->buildHull(margin);
			btConvexHullShape* simplifiedConvexShape = new btConvexHullShape((btScalar *) hull->getVertexPointer(),hull->numVertices());
			delete shape;
			_simStatus.dropletShape = (btCollisionShape *) simplifiedConvexShape;
#else
			_simStatus.dropletShape = (btCollisionShape *) shape;
#endif
		} else {
			qDebug() << "Error: cannot read from" << fileName;
		}
	} else {
		qDebug() << "Error: file" << fileName << "does not exist";
	}


}


btCollisionShape* SimInterface::makeCollisionShapeFromFile(QString fileName)
{

	btConvexHullShape *shape = new btConvexHullShape();

	QString inLine;
	int n;

	QFile file(fileName);

	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly  |QIODevice::Text))
		{
			QTextStream in(&file);

			while (!in.atEnd())
			{
				inLine = in.readLine(0);
				if(inLine.length() == 0)
					continue;

				QStringList list = inLine.split(" ",QString::SkipEmptyParts);

				if(list[0] == QString("v") && list.size() == 4)
				{
					btVector3 vertex;
					vertex.setX(list[1].toFloat());
					vertex.setY(list[3].toFloat());
					vertex.setZ(list[2].toFloat());
					shape->addPoint(vertex);
				}
				//obj files include face information, the following parses through
				//three vertices at a time (for each triangle) storing indices
			}


		} else {
			qDebug() << "Error: cannot read from" << fileName;
			return NULL;
		}
	} else {
		qDebug() << "Error: file" << fileName << "does not exist";
		return NULL;
	}
#ifdef SIMPLIFY_COLLISION_SHAPES
	btShapeHull* hull = new btShapeHull(shape);
	btScalar margin = shape->getMargin();
	hull->buildHull(margin);
	btConvexHullShape* simplifiedConvexShape = new btConvexHullShape((btScalar *) hull->getVertexPointer(),hull->numVertices());
	delete shape;
	return (btCollisionShape *) simplifiedConvexShape;
#else
	return (btCollisionShape *) shape;
#endif
}

void SimInterface::setUpdateRate(float rate)
{
	_simRates.timeScale = rate;
	if (_simRates.timeScale < 0.1)
		_simRates.timeScale = 0.1;
	updateTiming();
	emit ratesChanged(_simRates);

}

void SimInterface::increaseUpdateRate()
{

	_simRates.timeScale -= 0.1;
	if (_simRates.timeScale < 0.1)
		_simRates.timeScale = 0.1;
	updateTiming();
	emit ratesChanged(_simRates);
}

void SimInterface::decreaseUpdateRate()
{
	_simRates.timeScale += 0.1;
	updateTiming();
	emit ratesChanged(_simRates);

}


void SimInterface::enableUpdateLimit()
{
	if (!_simRates.limitRate)
	{
		_simRates.limitRate = true;
		updateTiming();
		emit ratesChanged(_simRates);
	}
}

void SimInterface::disableUpdateLimit()
{	
	if (_simRates.limitRate)
	{
		_simRates.limitRate = false;
		updateTiming();
		emit ratesChanged(_simRates);
	}
}

void SimInterface::toggleUpdateLimit()
{
	_simRates.limitRate = !_simRates.limitRate;
	updateTiming();
	emit ratesChanged(_simRates);
}

void SimInterface::useResetTimer(int buttonState)
{
	if (buttonState == 2)
		_simState.useResetTime = true;
	else
		_simState.useResetTime = false;
}

void SimInterface::updateResetTimer(const QString &text)
{
	bool ok;
	_simState.resetTime = text.toDouble(&ok);
}

simRate_t SimInterface::getSimulatorRate()
{
	return _simRates;
}

