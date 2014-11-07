/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\include\SimInfoLogger.h
 *
 * \brief	Declares the simulation information logger class. This class logs info
 * 			while the simulator is running and prints the info into a specified output
 * 			file.
 */

#include <DSimDataLogger.h>
#include <SimInterface.h>
#include <DropletGUIStructs.h>
#include <DSimDataStructs.h>
#include <QGlobal.h>
#include <QTime>
#include <stdio.h>
#include "DropletGUIDefaults.h"
#include <glm/glm.hpp>

/**
*  @defgroup info Info Logging
*  \brief Items related to logging information from the simulator.
*/

/**
 * @ingroup info 
 * \class	SimInfoLogger
 *
 * \brief	Simulation information logger.
 *
 */

class SimInfoLogger : public QObject
{
	Q_OBJECT

public:
	SimInfoLogger(QObject *parent = 0);

	/**
	 * \fn	void SimInfoLogger::printDropletData(simState_t stateInfo);
	 *
	 * \brief	prints out droplet data to a file
	 *
	 *
	 * \param	stateInfo	Information describing the state.
	 */

	void printDropletData(simState_t stateInfo);

	/**
	 * \fn	void SimInfoLogger::setPosFlag(bool flag);
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
	~SimInfoLogger();

private:

	/**
	 * \property	bool posFlag,colorFlag,rotationFlag,commSAFlag, macroRedFlag, macroSAFlag
	 *
	 * \brief	Flags store what data to print.
	 *
	 */

	bool posFlag,colorFlag,rotationFlag,commSAFlag, macroRedFlag, macroSAFlag;

	bool firstTime;

	/**
	 * \property	int redTally, SATally
	 *
	 * \brief	Tallies to count macro info.
	 *
	 */

	int redTally, SATally;

	/**
	 * \property	double lastPrint
	 *
	 * \brief	Time of the last print.
	 */

	double lastPrint;

	/**
	 * \property	double timeInterval
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
	 * \fn	void SimInfoLogger::Init();
	 *
	 * \brief	opens the output file and initializes some vars.
	 */

	void Init();

    /**
	 * \fn	void SimInfoLogger::close();
	 *
	 * \brief	Closes output file.
	 */
	void close();

	/**
	 * \fn	void SimInfoLogger::timeCheck(simState_t stateInfo);
	 *
	 * \brief	Checks to see if enough time has elapsed since last print.
	 *
	 * \param	stateInfo	Information describing the state.
	 */
	void timeCheck(simState_t stateInfo);

};
