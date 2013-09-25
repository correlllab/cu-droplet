/**
 * \file	cu-droplet\DropletSimulator\DropletSimLibrary\include\DropletSimInfo.h
 *
 * \brief	Declares the droplet simulation information class that gives access to 
 * 			info stored by the structs declared in DropletDataStructs and timing info
 *			calculated by DropletTimeControl. Detailed info on the data structs can be
 *			found in:
 *				cu-droplet\DropletSimulator\DropletSimLibrary\include\DropletDataStructs.h
 *			Details on Droplet Simulator return codes (DS_RESULT) can be found in:
 *				cu-droplet\DropletSimulator\DropletSimLibrary\include\DropletSimGlobals.h
 */

#pragma once

#include "DropletSimGlobals.h"
#include "DropletSim.h"
#include "DropletDataStructs.h"
#include <vector>
#include <cstdio>
#include <cstdlib>


/**
 * \typedef	struct Droplet_Motion_Direction_Data
 *
 * \brief	Defines an alias representing information describing the droplet motion direction.
 *			Details on move_direction and turn_direction can be found in:
 *			cu-droplet\DropletSimulator\DropletSimLibrary\include\DropletSimGlobals.h
 */

typedef struct Droplet_Motion_Direction_Data // Droplet move and turn direction Info
{
	move_direction currMoveDir;
	turn_direction currTurnDir;
}DirInfo;


/**
 * \class	DropletSimInfo
 *
 * \brief	Helper class used for retrieving information from the simulator. Retrieves any
 *			interesting info stored in DropletDataStructs
 *
 */

class DropletSimInfo
{	
friend class DropletSim;
public:

	/**
	 * \fn	DS_RESULT DropletSimInfo::GetDropletPositions(std::vector<GPSInfo *> *outPosData,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet position info into a supplied vector.
	 *
	 * \param [in,out]	outPosData	If non-null, information describing the droplet positions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetDropletPositions(std::vector<GPSInfo *> *outPosData, DropletSim& simulator);

	/**
	 * \fn	DS_RESULT DropletSimInfo::GetObjectPositions(std::vector<GPSInfo *> *outPosData,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts object position info into a supplied vector.
	 *
	 * \param [in,out]	outPosData	If non-null, information describing the object positions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetObjectPositions(std::vector<GPSInfo *> *outPosData, DropletSim& simulator);

	/**
	 * \fn	DS_RESULT DropletSimInfo::GetDropletColors(std::vector<uint8_t *> *colors,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet LED color info into a supplied vector. color data is RGB ranging
	 *			from 0 to 255
	 *
	 * \param [in,out]	colors		If non-null, information describing the droplet colors.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetDropletColors(std::vector<uint8_t *> *colors, DropletSim& simulator);

	/**
	 * \fn	DS_RESULT DropletSimInfo::GetRemainingMotionTimes(std::vector<float *> *times,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts remaining droplet motion (move and rotate) time info into a supplied vector.
	 *
	 * \param [in,out]	times		If non-null, information describing the remaining times.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetRemainingMotionTimes(std::vector<float *> *times, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetMotionDirections(std::vector<DirInfo *> *directions,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet motion (move and rotate) direction info into a supplied vector.
	 *
	 * \param [in,out]	directions	If non-null, information describing the move directions.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetMotionDirections(std::vector<DirInfo *> *directions, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetSensorColors(std::vector<uint8_t *> *colors,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet RGB sensor color info into a supplied vector. colors range from
	 *			0 to 255.
	 *
	 * \param [in,out]	colors		If non-null, information describing the colors.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetSensorColors(std::vector<uint8_t *> *colors, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetPhysData(std::vector<ObjectPhysicsData *> *phys,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts object physics info into a supplied vector.
	 *
	 * \param [in,out]	phys		If non-null, information describing the physics data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetPhysData(std::vector<ObjectPhysicsData *> *phys, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetCommData(std::vector<DropletCommData *> *comm,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet communication info into a supplied vector.
	 *
	 * \param [in,out]	comm		If non-null, information describing the communications.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetCommData(std::vector<DropletCommData *> *comm, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetCompData(std::vector<DropletCompData *> *comp,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet component info into a supplied vector.
	 *
	 * \param [in,out]	comp		If non-null, information describing the component data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetCompData(std::vector<DropletCompData *> *comp, DropletSim& simulator);
	
	/**
	 * \fn	DS_RESULT DropletSimInfo::GetActuationData(std::vector<DropletActuatorData *> *act,
	 * 		DropletSim& simulator);
	 *
	 * \brief	puts droplet actuator info into a supplied vector.
	 *
	 * \param [in,out]	act			If non-null, information describing actuator data.
	 * \param [in,out]	simulator 	The simulator.
	 *
	 * \return	A Droplet Simulator error code.
	 */

	DS_RESULT GetActuationData(std::vector<DropletActuatorData *> *act, DropletSim &simulator);

	/* GetNumCollisions() returns the number of objects colliding at any given time.
	 * To understand the implmenentation of this function refer to the wiki page,
	 * http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers
	 */
	uint64_t GetNumCollisions(DropletSim &simulator);

	/**
	 * \fn	double DropletSimInfo::GetTotalRT(DropletSim& simulator);
	 *
	 * \brief	gets the total real time elapsed since the simulator has started from
	 *			the DropletTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total real time elapsed.
	 */

	double GetTotalRT(DropletSim& simulator);
	
	/**
	 * \fn	double DropletSimInfo::GetTotalST(DropletSim& simulator);
	 *
	 * \brief	gets the total time simulated since the simulator has started from
	 *			the DropletTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total time simulated.
	 */

	double GetTotalST(DropletSim& simulator);
	
	/**
	 * \fn	double DropletSimInfo::GetStepRT(DropletSim& simulator);
	 *
	 * \brief	gets the real time elapsed since the simulator calculated the last step 
	 *			from the DropletTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The real time since the last step.
	 */

	double GetStepRT(DropletSim& simulator);
	
	/**
	 * \fn	double DropletSimInfo::GetTotalDiff(DropletSim& simulator);
	 *
	 * \brief	gets the difference between the total real time and simulator time elapsed from
	 *			the DropletTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The difference between total real time and simulator time.
	 */

	double GetTotalDiff(DropletSim& simulator);
	
	/**
	 * \fn	double DropletSimInfo::GetTimeRatio(DropletSim& simulator);
	 *
	 * \brief	gets the ratio of time simulated in one step to real time elapsed since the 
	 *			last step from the DropletTimeControl class.
	 *
	 * \param [in,out]	simulator	The simulator.
	 *
	 * \return	The total real time elapsed.
	 */
	double GetTimeRatio(DropletSim& simulator);
};