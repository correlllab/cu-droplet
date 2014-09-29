/**
 * \file	cu-droplet\DropletSimulator\DropletSimLibrary\include\DSimDataLogger.h
 *
 * \brief	Declares the droplet simulation information class that gives access to 
 * 			info stored by the structs declared in DSimDataStructs and timing info
 *			calculated by DSimTimeControl. Detailed info on the data structs can be
 *			found in:
 *				cu-droplet\DropletSimulator\DropletSimLibrary\include\DSimDataStructs.h
 *			Details on Droplet Simulator return codes (DS_RESULT) can be found in:
 *				cu-droplet\DropletSimulator\DropletSimLibrary\include\DSimGlobals.h
 */

#pragma once

#include "DSimGlobals.h"
#include "DSim.h"
#include "DSimDataStructs.h"
#include <vector>
#include <cstdio>
#include <cstdlib>


/**
 * \typedef	struct Droplet_Motion_Direction_Data
 *
 * \brief	Defines an alias representing information describing the droplet motion direction.
 *			Details on move_direction and turn_direction can be found in:
 *			cu-droplet\DropletSimulator\DropletSimLibrary\include\DSimGlobals.h
 */

typedef struct Droplet_Motion_Direction_Data // Droplet move and turn direction Info
{
	move_direction currMoveDir;
}DirInfo;


/**
 * \class	DSimDataLogger
 *
 * \brief	Helper class used for retrieving information from the simulator. Retrieves any
 *			interesting info stored in DSimDataStructs
 *
 */

class DSimDataLogger
{	
friend class DSim;
public:
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetDropletPositions(std::vector<ObjectPositionData *> *outPosData,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet position info into a supplied vector.
	 *
	 * \param [in,out]	outPosData	If non-null, information describing the droplet positions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetDropletPositions(std::vector<ObjectPositionData *> *outPosData, DSim& simulator);

	/**
	 * \fn	DS_RESULT DSimDataLogger::GetObjectPositions(std::vector<ObjectPositionData *> *outPosData,
	 * 		DSim& simulator);
	 *
	 * \brief	puts object position info into a supplied vector.
	 *
	 * \param [in,out]	outPosData	If non-null, information describing the object positions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetObjectPositions(std::vector<ObjectPositionData *> *outPosData, DSim& simulator);

	/**
	 * \fn	DS_RESULT DSimDataLogger::GetDropletColors(std::vector<uint8_t *> *colors,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet LED color info into a supplied vector. color data is RGB ranging
	 *			from 0 to 255
	 *
	 * \param [in,out]	colors		If non-null, information describing the droplet colors.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetDropletColors(std::vector<uint8_t *> *colors, DSim& simulator);

	/**
	 * \fn	DS_RESULT DSimDataLogger::GetRemainingMotionTimes(std::vector<float *> *times,
	 * 		DSim& simulator);
	 *
	 * \brief	puts remaining droplet motion (move and rotate) time info into a supplied vector.
	 *
	 * \param [in,out]	times		If non-null, information describing the remaining times.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetRemainingMotionTimes(std::vector<float *> *times, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetMotionDirections(std::vector<DirInfo *> *directions,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet motion (move and rotate) direction info into a supplied vector.
	 *
	 * \param [in,out]	directions	If non-null, information describing the move directions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetMotionDirections(std::vector<DirInfo *> *directions, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetSensorColors(std::vector<uint8_t *> *colors,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet RGB sensor color info into a supplied vector. colors range from
	 *			0 to 255.
	 *
	 * \param [in,out]	colors		If non-null, information describing the colors.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetSensorColors(std::vector<uint8_t *> *colors, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetPhysData(std::vector<ObjectPhysicsData *> *phys,
	 * 		DSim& simulator);
	 *
	 * \brief	puts object physics info into a supplied vector.
	 *
	 * \param [in,out]	phys		If non-null, information describing the physics data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetPhysData(std::vector<ObjectPhysicsData *> *phys, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetCommData(std::vector<DropletCommData *> *comm,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet communication info into a supplied vector.
	 *
	 * \param [in,out]	comm		If non-null, information describing the communications.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetCommData(std::vector<DropletCommData *> *comm, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetCompData(std::vector<DropletCompData *> *comp,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet component info into a supplied vector.
	 *
	 * \param [in,out]	comp		If non-null, information describing the component data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetCompData(std::vector<DropletCompData *> *comp, DSim& simulator);
	
	/**
	 * \fn	DS_RESULT DSimDataLogger::GetActuationData(std::vector<DropletActuatorData *> *act,
	 * 		DSim& simulator);
	 *
	 * \brief	puts droplet actuator info into a supplied vector.
	 *
	 * \param [in,out]	act			If non-null, information describing actuator data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */
	DS_RESULT GetActuationData(std::vector<DropletActuatorData *> *act, DSim &simulator);

	/* GetNumCollisions() returns the number of objects colliding at any given time.
	 * To understand the implmenentation of this function refer to the wiki page,
	 * http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers
	 */
	uint64_t GetNumCollisions(DSim &simulator);

	/**
	 * \fn	double DSimDataLogger::GetTotalRT(DSim& simulator);
	 *
	 * \brief	gets the total real time elapsed since the simulator has started from
	 *			the DSimTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total real time elapsed.
	 */
	double GetTotalRT(DSim& simulator);
	
	/**
	 * \fn	double DSimDataLogger::GetTotalST(DSim& simulator);
	 *
	 * \brief	gets the total time simulated since the simulator has started from
	 *			the DSimTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total time simulated.
	 */
	double GetTotalST(DSim& simulator);
	
	/**
	 * \fn	double DSimDataLogger::GetStepRT(DSim& simulator);
	 *
	 * \brief	gets the real time elapsed since the simulator calculated the last step 
	 *			from the DSimTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The real time since the last step.
	 */
	double GetStepRT(DSim& simulator);
	
	/**
	 * \fn	double DSimDataLogger::GetTotalDiff(DSim& simulator);
	 *
	 * \brief	gets the difference between the total real time and simulator time elapsed from
	 *			the DSimTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The difference between total real time and simulator time.
	 */
	double GetTotalDiff(DSim& simulator);
	
	/**
	 * \fn	double DSimDataLogger::GetTimeRatio(DSim& simulator);
	 *
	 * \brief	gets the ratio of time simulated in one step to real time elapsed since the 
	 *			last step from the DSimTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total real time elapsed.
	 */
	double GetTimeRatio(DSim& simulator);
};