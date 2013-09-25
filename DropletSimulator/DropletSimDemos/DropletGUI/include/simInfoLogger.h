/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\simInfoLogger.h
 *
 * \brief	Declares the simulation information logger class. This class logs info
 * 			while the simulator is running and prints the info into a specified output
 * 			file.
 */

#include <DropletSimInfo.h>
#include <SimInterface.h>
#include <structs.h>
#include <DropletDataStructs.h>
#include <QGlobal.h>
#include <QTime>
#include <stdio.h>
#include "defaults.h"
#include <glm/glm.hpp>

/**
*  @defgroup info Info Logging
*  \brief Items related to logging information from the simulator.
*/

/**
 * @ingroup info 
 * \class	simInfoLogger
 *
 * \brief	Simulation information logger.
 *
 */

class simInfoLogger : public QObject
{

	Q_OBJECT

public:
	simInfoLogger(QObject *parent = 0);

	/**
	 * \fn	void simInfoLogger::Init();
	 *
	 * \brief	opens the output file and initializes some vars.
	 */

	void Init();

	/**
	 * \fn	void simInfoLogger::printDropletData(simState_t stateInfo);
	 *
	 * \brief	prints out droplet data to a file
	 *
	 *
	 * \param	stateInfo	Information describing the state.
	 */

	void printDropletData(simState_t stateInfo);

	/**
	 * \fn	void simInfoLogger::setPosFlag(bool flag);
	 *
	 * \brief	Sets flag.
	 *
	 * \param	flag	boolean
	 */

	void setPosFlag(bool flag);
	void setColorFlag(bool flag);
	void setRotationFlag(bool flag);
	void setCommSAFlag(bool flag);
	void setMacroRedFlag(bool flag);
	void setMacroSAFlag(bool flag);
	~simInfoLogger();

private:

	/**
	 * \property	bool posFlag,colorFlag,rotationFlag,commSAFlag, macroRedFlag, macroSAFlag
	 *
	 * \brief	Flags store what data to print.
	 *
	 */

	bool posFlag,colorFlag,rotationFlag,commSAFlag, macroRedFlag, macroSAFlag;

	/**
	 * \property	int redTally, SATally
	 *
	 * \brief	Tallies to count macro info.
	 *
	 */

	int redTally, SATally;

	/**
	 * \property	int lastPrint
	 *
	 * \brief	Time of the last print.
	 */

	double lastPrint;

	/**
	 * \property	int timeInterval
	 *
	 * \brief	Time between prints.
	 */
	double timeInterval;

	/**
	 * \property	QString newFile
	 *
	 * \brief	The output file path.
	 */

	QString newFile;

	/**
	 * \property	FILE * fp
	 *
	 * \brief	The output file pointer.
	 */
	FILE * fp;

public slots:
	/**
	 * \fn	void simInfoLogger::close();
	 *
	 * \brief	Closes output file.
	 */
	void close();

	/**
	 * \fn	void simInfoLogger::timeCheck(simState_t stateInfo);
	 *
	 * \brief	Checks to see if enough time has elapsed since last print.
	 *
	 * \param	stateInfo	Information describing the state.
	 */
	void timeCheck(simState_t stateInfo);

};
