/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\SimInfoLogger.cpp
 *
 * \brief	Implements the SimInfoLogger class.
 */

#include "SimInfoLogger.h"

SimInfoLogger::SimInfoLogger(QObject *parent)
	: QObject(parent)
{
	timeInterval = NULL;
	lastPrint = NULL;
	posFlag = NULL;
	colorFlag = NULL;
	rotationFlag = NULL;
	commSAFlag = NULL;
	macroRedFlag = NULL;
	macroSAFlag = NULL;
	QString newFile = NULL;
}
void SimInfoLogger::Init()
{
	newFile = QString(DEFAULT_ASSETDIR).append("output.txt");
    fp = fopen (newFile.toStdString().c_str(),"w");
	if (fp==NULL)
	{
		printf("Cannot create output file\n");
	}
	timeInterval = .5;
	//lastPrint = -timeInterval - 1;
    lastPrint = 0;

	if(macroRedFlag)
	{
		redTally = 0;
	}
	if(macroSAFlag)
	{
		SATally = 0;
	}

	// Messages printed one at the top of the file
	fprintf(fp, "Format:\n");
	fprintf(fp, "Droplet#: ");
	if(posFlag)
	{
		fprintf(fp,"(xPos,yPos,zPos)  |  ");
	}
	if(colorFlag)
	{
		fprintf(fp,"(rColor,gColor,bColor)  |  ");
	}
	if(rotationFlag)
	{
		fprintf(fp,"(xRot,yRot,zRot)  |  ");
	}
	if(commSAFlag)
	{
		fprintf(fp,"SAbool  |  ");
	}
	fprintf(fp, "\n\n");
}

SimInfoLogger::~SimInfoLogger()
{

}

void SimInfoLogger::close()
{
	// If newFile == NULL, then Init has not been called.
	// and thus a file has not been opened.
	if(newFile != NULL)
	{	
		printf("File had been opened");
		fclose(fp);
	}
}

void SimInfoLogger::setPosFlag(bool flag)
{
	posFlag = flag;
}
void SimInfoLogger::setColorFlag(bool flag)
{
	colorFlag = flag;
}
void SimInfoLogger::setRotationFlag(bool flag)
{
	rotationFlag = flag;
}
void SimInfoLogger::setCommSAFlag(bool flag)
{
	commSAFlag = flag;
}
void SimInfoLogger::setMacroRedFlag(bool flag)
{
	macroRedFlag = flag;
}
void SimInfoLogger::setMacroSAFlag(bool flag)
{
	macroSAFlag = flag;
}

void SimInfoLogger::printDropletData(simState_t stateInfo)
{
	if(macroRedFlag){
		redTally = 0;
	}
	if(macroSAFlag){
		SATally = 0;	
	}

	// Messages printed at the start of each step
	fprintf(fp, "Simiulation time: %.3f | Real time: %.3f\n", stateInfo.simTime, stateInfo.realTime);
	fprintf(fp, "----------------------------------------------\n");
	
	// Messages printed once per droplet per step
	foreach(dropletStruct_t droplet, stateInfo.dropletData)
	{
		fprintf(fp,"%i: ", droplet.dropletID);
		if(posFlag)
		{
			fprintf(fp,"(%3.4f,%3.4f,%3.4f)  |  ",droplet.origin.x,droplet.origin.y,droplet.origin.z);
		}
		if(colorFlag)
		{
			fprintf(fp,"(%03i,%03i,%03i)  |  ",droplet.color.r,droplet.color.g,droplet.color.b);
			if(macroRedFlag && droplet.color.r > 240 && droplet.color.g < 15 && droplet.color.b < 15){
				redTally++;
			}
		}
		if(rotationFlag)
		{
			fprintf(fp,"(%3.4f,%3.4f,%3.4f)  |  ",droplet.rotation.x,droplet.rotation.y,droplet.rotation.z);
		}
		if(commSAFlag)
		{
			if(droplet.commData.sendActive)
			{
				fprintf(fp,"TRUE  |  ");
				if(macroSAFlag){
					SATally++;
				}
			}
			else
			{
				fprintf(fp,"FALSE  |  ");
			}
		}
		fprintf(fp,"\n");
	}
	fprintf(fp, "\n");
	if (macroRedFlag)
	{
		fprintf(fp, "# of red Droplets: %d  |  ", redTally);
	}
	if (macroSAFlag)
	{
		fprintf(fp, "# of Droplets communicating: %d  |  ", SATally);
	}
	fprintf(fp, "\n");
	fprintf(fp, "----------------------------------------------\n\n");
	fflush(fp);
}

void SimInfoLogger::timeCheck(simState_t stateInfo)
{
	double currentTime;
	currentTime = stateInfo.simTime;
	if((currentTime - lastPrint) > timeInterval)
	{
		lastPrint = currentTime;
		printDropletData(stateInfo);
	}
}