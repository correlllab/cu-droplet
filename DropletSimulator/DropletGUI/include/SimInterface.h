/**
 * \file	\cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\SimInterface.h
 *
 * \brief	Declares the simulation interface class.
 */

#ifndef SIMULATORINTERFACE_H
#define SIMULATORINTERFACE_H

#include <QGlobal.h>
#include <QDebug>
#include <QObject>
#include <QVector>
#include <QTime>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QElapsedTimer>
#include <QValidator>
#include <QIntValidator>
#include <DSimDroplet.h>
#include <DSim.h>

#include <DSimDataLogger.h>
#include <DSimGlobals.h>
#include <DSimDataStructs.h>

#include <DropletCustomOne.h>
#include <DropletCustomTwo.h>
#include <DropletCustomThree.h>
#include <DropletCustomFour.h>
#include <DropletCustomFive.h>
#include <DropletCustomSix.h>
#include <DropletCustomSeven.h>
#include <DropletCustomEight.h>
#include <DropletCustomNine.h>
#include <DropletCustomTen.h>

#include <limits.h>

#include "DropletGUIDefaults.h"
#include "DropletGUIStructs.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdint.h>

#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

/**
*  @defgroup sim Simulator Interface
*  \brief Items related to visualizing the state of the simulation.
*/
/**
 * @ingroup sim
 * \class	SimInterface
 *
 * \brief	Model that contains and controls the droplet simulator.
 */

class SimInterface : public QObject
{
	Q_OBJECT

public:
	SimInterface(QObject *parent = 0);
	~SimInterface();

	/**
	 * \fn	void SimInterface::Init();
	 *
	 * \brief	Initialises this SimInterface object. Creates vectors that track
	 * 			droplet information. Calls functions that create the arena and
	 * 			adds droplets.
	 */

	void Init();

	/**
	 * \fn	float SimInterface::getRandomf(float min, float max);
	 *
	 * \brief	Generates a random float between min and max.
	 *
	 * \param	min	The minimum.
	 * \param	max	The maximum.
	 *
	 * \return	The random float.
	 */

	float getRandomf(float min, float max);

	/**
	 * \fn	bool SimInterface::isPaused(void);
	 *
	 * \brief	Query if this object is paused.
	 *
	 * \return	true if paused, false if not.
	 */

	bool isPaused(void);

	/**
	 * \fn	simSetting_t SimInterface::getDefaultSettings();
	 *
	 * \brief	Returns a simSetting_t that is set to default settings.
	 *
	 * \return	The default settings.
	 */

	simSetting_t getDefaultSettings();

	/**
	 * \fn	simSetting_t SimInterface::getCurrentSettings();
	 *
	 * \brief	Gets current settings.
	 *
	 * \return	The current settings.
	 */

	simSetting_t getCurrentSettings();

	/**
	 * \fn	simSetting_t SimInterface::getSimulatorSettings();
	 *
	 * \brief	Returns the current simulator settings.
	 *
	 * \return	_simSettings, The current simulator settings.
	 */

	simSetting_t getSimulatorSettings();

	/**
	 * \fn	simState_t SimInterface::getSimulatorState();
	 *
	 * \brief	Gets the simulator state.
	 *
	 * \return	_simState, The simulator state.
	 */

	simState_t getSimulatorState();



	/**
	 * \fn	void SimInterface::createArena();
	 *
	 * \brief	Creates the arena.
	 */

	void createArena();

	/**
	 * \fn	simRate_t SimInterface::getSimulatorRate();
	 *
	 * \brief	Gets the simulator rate.
	 *
	 * \return	_simRates, The simulator rate.
	 */

	simRate_t getSimulatorRate();

signals:

	/**
	 * \fn	void SimInterface::arenaChanged(simSetting_t arena);
	 *
	 * \brief	signal emitted in SimInterface::Init(). This signal is connected to the slot
	 * 			RenderWidget::updateArena().
	 *
	 * \param	arena	simulation settings.
	 */

	void arenaChanged(simSetting_t arena);

	/**
	 * \fn	void SimInterface::simulationUpdated(simState_t simInfo);
	 *
	 * \brief	Signal emitted in SimInterface::Init(). This signal is connected to the slot
	 * 			RenderWidget::updateState().
	 *
	 * \param	simInfo	simulation state.
	 */

	void simulationUpdated(simState_t simInfo);

	/**
	 * \fn	void SimInterface::pauseChanged(bool paused);
	 *
	 * \brief	Signal emitted in SimInterface::pause() and SimInterface::resume(). 
	 * 			Signal is connected to the slot RenderWidget::updatePause().
	 *
	 * \param	paused	true if paused.
	 */

	void pauseChanged(bool paused);

	/**
	 * \fn	void SimInterface::ratesChanged(simRate_t rates);
	 *
	 * \brief	Signal emitted in SimInterface::setUpdateRate(), SimInterface::increaseUpdateRate(),
	 * 			SimInterface::decreaseUpdateRate(), SimInterface::enableUpdateLimit(), 
	 * 			SimInterface::disableUpdateLimit(), and SimInterface::toggleUpdateLimit(). 
	 * 			Signal is connected to the slot RenderWidget::updateRate().
	 *
	 * \param	rates	simulation rate.
	 */

	void ratesChanged(simRate_t rates);

    void resetLogger ( void );

public slots:

		/**
	 * \fn	void SimInterface::loadTilePositions();
	 *
	 * \brief	Creates the default rectangle arena based on the 
	 * 			numRowTiles and numColTiles parameters
	 */

	void loadTilePositions();

	/**
	 * \fn	void SimInterface::createArena(QString filename);
	 *
	 * \brief	Creates an arena based on an input file.
	 * 			The input file contains lines of the following format:
	 * 			xPos yPos up right down left
	 * 			xPos and yPos are grid coordinates. up right down left 
	 * 			are replaced with yes or no, booleans representing if 
	 * 			a wall is created for that tile at each position.
	 * 			
	 *
	 * \param	filename	Filename of the floor file.
	 */

	void loadTilePositions(QString filename);

	/**
	 * \fn	void SimInterface::pause(void);
	 *
	 * \brief	emits the SimInterface::pauseChanged() signal. Pauses the simulation
	 */

	void pause(void);

	/**
	 * \fn	void SimInterface::resume(void);
	 *
	 * \brief	emits the SimInterface::pauseChanged() signal. Resumes the simulation
	 */

	void resume(void);

	/**
	 * \fn	void SimInterface::togglePause(void);
	 *
	 * \brief	Calls SimInterface::resume() or SimInterface::pause() depending on 
	 * 			the current status of the simulation.
	 */

	void togglePause(void);

	/**
	 * \fn	void SimInterface::Update(float timeSinceLastUpdate);
	 *
	 * \brief	Steps the DSim object, and updates the DSimDataLogger object.
	 * 			Uses the DSimDataLogger object to update droplet and object positions.
	 *
	 * \param	timeSinceLastUpdate	The time since last update.
	 */

	void Update(float timeSinceLastUpdate);

	/**
	 * \fn	void SimInterface::reset(void);
	 *
	 * \brief	Resets this object. Calls SimInterface::Init()
	 */

	void reset(void);

	/**
	 * \fn	void SimInterface::addDroplet(float x, float y, droplet_t dType = CustomOne,
	 * 		int dropletID = 0);
	 *
	 * \brief	Adds a droplet.
	 *
	 * \param	x		 	The x coordinate.
	 * \param	y		 	The y coordinate.
	 * \param	dType	 	(Optional) The droplet program type.
	 * \param	dropletID	(Optional) The Droplet ID. If 0, adds one to the current number of droplets.
	 */

	void addDroplet(float x, float y, float a, droplet_t dType = CustomOne, int dropletID = 0);

	/**
	 * \fn	int SimInterface::addNewShape(object_t objectType, float radius, vec3 scale);
	 *
	 * \brief	Adds collision objects.
	 *
	 * \param	objectType	Type of the object.
	 * \param	radius	  	The radius.
	 * \param	scale	  	The scale.
	 *
	 * \return	The index of the corresponding shape in the
	 * 			list of collision objects.
	 */

	int addNewShape(object_t objectType, float radius, vec3 scale);

	/**
	 * \fn	void SimInterface::addSphere( float x, float y, int objectID, float radius, float mass,
	 * 		float friction);
	 *
	 * \brief	Adds a sphere object.
	 *
	 * \param	x			The x coordinate.
	 * \param	y			The y coordinate.
	 * \param	objectID	Identifier for the object.
	 * \param	radius  	The radius.
	 * \param	mass		The mass.
	 * \param	friction	The friction.
	**/

	void addSphere( float x, float y, int objectID, float radius, float mass, float friction);

	/**
	 * \fn	void SimInterface::addCube( float x, float y, int objectID, vec3 scale, float mass,
	 * 		float friction);
	 *
	 * \brief	Adds a cube.
	 *
	 * \param	x			The x coordinate.
	 * \param	y			The y coordinate.
	 * \param	objectID	Identifier for the object.
	 * \param	scale   	The scale.
	 * \param	mass		The mass.
	 * \param	friction	The friction.
	**/

	void addCube( float x, float y, int objectID, vec3 scale, float mass, float friction);

	/**
	 * \fn	void SimInterface::addWall( float x, float y, int objectID, vec3 scale);
	 *
	 * \brief	Adds a wall. This is the same as addCube but the object created is static.
	 *
	 * \param	x			The x coordinate.
	 * \param	y			The y coordinate.
	 * \param	objectID	Identifier for the object.
	 * \param	scale   	The scale.
	 */

	void addWall( float x, float y, int objectID, vec3 scale);

	/**
	 * \fn	void SimInterface::addFloor( float x, float y, int objectID, vec3 scale);
	 *
	 * \brief	Adds a floor. This is the same as addCube but the object created is static.
	 *
	 * \param	x			The x coordinate.
	 * \param	y			The y coordinate.
	 * \param	objectID	Identifier for the object.
	 * \param	scale   	The scale.
	 */

	void addFloor( float x, float y, int objectID, vec3 scale);

	/**
	 * \fn	void SimInterface::addObject(object_t kind, float x, float y, float radius = 1.0f,
	 * 		float mass = DEFAULT_OBJECT_MASS, float friction = DEFAULT_OBJECT_FRICTION);
	 *
	 * \brief	Adds an object. Calls either addSphere or addCube.
	 *
	 * \param	kind		The kind (Sphere or Cube)
	 * \param	x			The x coordinate.
	 * \param	y			The y coordinate.
	 * \param	radius  	(Optional) the radius.
	 * \param	mass		(Optional) the mass.
	 * \param	friction	(Optional) the friction.
	**/

	void addObject(object_t kind, float x, float y, float radius = 1.0f, float mass = DEFAULT_OBJECT_MASS, float friction = DEFAULT_OBJECT_FRICTION);

	/**
	 * \fn	void SimInterface::loadArena(simSetting_t arena);
	 *
	 * \brief	Updates sim settings to settings stored in simSetting_t arena.
	 * 			Then calls SimInterface::Init()
	 *
	 * \param	arena	The simulation settings.
	 */

	void loadArena(simSetting_t arena);

	/**
	 * \fn	void SimInterface::setUpdateRate(float rate);
	 *
	 * \brief	Sets the update rate.
	 * 			Emits the SimInterface::ratesChanged() signal. 
	 *
	 * \param	rate	The update rate.
	 */

	void setUpdateRate(float rate);

	/**
	 * \fn	void SimInterface::increaseUpdateRate(void);
	 *
	 * \brief	Increase update rate.
	 * 			Emits the SimInterface::ratesChanged() signal.
	 */

	void increaseUpdateRate(void);

	/**
	 * \fn	void SimInterface::decreaseUpdateRate(void);
	 *
	 * \brief	Decrease update rate.
	 * 			Emits the SimInterface::ratesChanged() signal.
	 */

	void decreaseUpdateRate(void);

	/**
	 * \fn	void SimInterface::enableUpdateLimit(void);
	 *
	 * \brief	Enables the update limit.
	 * 			Emits the SimInterface::ratesChanged() signal.
	 */

	void enableUpdateLimit(void);

	/**
	 * \fn	void SimInterface::disableUpdateLimit(void);
	 *
	 * \brief	Disables the update limit.
	 * 			Emits the SimInterface::ratesChanged() signal.
	 */

	void disableUpdateLimit(void);

	/**
	 * \fn	void SimInterface::toggleUpdateLimit(void);
	 *
	 * \brief	Toggle update limit.
	 * 			Emits the SimInterface::ratesChanged() signal.
	 */

	void toggleUpdateLimit(void);

	/**
	 * \fn void SimInterface::updateResetTimer(int resetTime)
	 *
	 * \brief	Toggle reset timer, on/off
	 */
	void useResetTimer(int resetTime);

	/**
	 * \fn void SimInterface::updateResetTimer(int resetTime)
	 *
	 * \brief	Update the reset timer, if it is enabled.
	 */
	void updateResetTimer(const QString &text);



protected:

	/**
	 * \fn	void SimInterface::timerEvent ( QTimerEvent * event );
	 *
	 * \brief	Manages when to call SimInterface::Update().
	 *
	 * \param [in,out]	event	If non-null, the event.
	 */

	void timerEvent ( QTimerEvent * event );


private:
	/**
	 * \fn	DSimDroplet* SimInterface::newDropletOfType(droplet_t dType,
	 * 		ObjectPhysicsData *dropletPhyDat);
	 *
	 * \brief	Creates a new droplet of type.
	 *
	 * \param	dType				 	The type.
	 * \param [in,out]	dropletPhyDat	If non-null, the droplet phy dat.
	 *
	 * \return	null if it fails, else.
	 */

	DSimDroplet* newDropletOfType(droplet_t dType,ObjectPhysicsData *dropletPhyDat);

	/**
	 * \fn	btCollisionShape* SimInterface::makeCollisionShapeFromFile(QString file);
	 *
	 * \brief	Makes collision shape from file.
	 *
	 * \param	file	The file.
	 *
	 * \return	null if it fails, else.
	 */

	btCollisionShape* makeCollisionShapeFromFile(QString file);

	/**
	 * \fn	void SimInterface::makeDropletCollisionShapeFromFile(QString file);
	 *
	 * \brief	Makes droplet collision shape from file.
	 *
	 * \param	file	The file.
	 */

	void makeDropletCollisionShapeFromFile(QString file);

	/**
	 * \fn	void SimInterface::teardownSim();
	 *
	 * \brief	Teardown the simulator.
	 */

	void teardownSim();

	//variables
	DSim *_sim;
	DSimDataLogger _simInfo;

	/**
	 * \brief	The simulation settings.
	 */

	simSetting_t _simSettings;
	simRate_t _simRates;
	
	struct {
		bool paused;
		double runTime;
		btCollisionShape *dropletShape;
		float dropletOffset;
		int btXWallShapeID;
		int btYWallShapeID;
		int btFloorShapeID;
		int btDropletShapeID;
	} _simStatus;

	/**
	 * \brief	State of the simulation.
	 */

	simState_t _simState;

	/**
	 * \fn	void SimInterface::updateTiming();
	 *
	 * \brief	Updates the timing.
	 */

	void updateTiming();

	std::vector<unsigned char *> *_dropletColors;

	/**
	 * \brief	The droplet position.
	 */

	std::vector<ObjectPositionData *> *_dropletPos;
	std::vector<DropletCommData *> *_dropletComm;

	/**
	 * \brief	The object position.
	 */

	std::vector<ObjectPositionData *> *_objectPos;
	DSimTimeControl *_timer;

	/**
	 * \brief	The tile positions and corresponding wall booleans for each tile.
	 */

	QVector<vec2> _tilePositions; 
	QVector<vec4> _wallBools;

	// TIMER
	//QElapsedTimer _updateTimer,_realTime;
	QElapsedTimer _updateTimer;

	/**
	 * \brief	Identifier for the timer.
	 */

	int _timerID;

	double _realTime;

	/**
	 * \brief	The simulation time scale.
	 */

	double _simTimeScale;

	/**
	 * \brief	Time of the target update.
	 */

	double _targetUpdateTime;

	/**
	 * \brief	The time until next update.
	 */

	double _timeUntilNextUpdate;

	struct {
		QStringList single;
		QStringList multiple;
	} _objectNames;


};

#endif // SIMULATORINTERFACE_H