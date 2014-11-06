/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\SimInfoLogger.cpp
 *
 * \brief	Implements the SimInfoLogger class.
 */

#include "SimInfoLogger.h"
#include <ctime>

SimInfoLogger::SimInfoLogger(QObject *parent)
	: QObject(parent),
    timeInterval ( 0.0 ),
    lastPrint ( 0.0 ),
    posFlag ( false ),
    colorFlag ( false ),
    rotationFlag ( false ),
    commSAFlag ( false ),
    macroRedFlag ( false ),
    macroSAFlag ( false ),
    fp ( NULL )
{ }


SimInfoLogger::~SimInfoLogger()
{ }

void SimInfoLogger::Init()
{	
    // close any open file first
    close();

	char sdate[9];
    char stime[9];
	_strdate_s(sdate);
    _strtime_s(stime);
	timeInterval = .5;
    lastPrint = 0;
	firstTime = true;

	newFile = QString(DEFAULT_ASSETDIR).append("output_");
	newFile.append(stime);
    newFile.append("_");
    newFile.append(sdate);
    newFile.append(".txt");
    for(int i = 10; i<newFile.length(); ++i){
        if (newFile[i] == '/' || newFile[i] == ':')
            newFile[i] = '-';
    }
    fp = fopen (newFile.toStdString().c_str(),"w");
	if (fp==NULL)
	{
		printf("Cannot create output file\n");
	}

	timeInterval = .5;
    lastPrint = 0;
	firstTime = true;

	if(macroRedFlag)
	{
		redTally = 0;
	}
	if(macroSAFlag)
	{
		SATally = 0;
	}

	// Messages printed one at the top of the file
	fprintf(fp, "(*\nFormat:\n{\n");
	fprintf(fp, "{simulationTime, realTime");
	if(macroRedFlag)
	{
		fprintf(fp, ", redTally");
		redTally = 0;
	}
	if(macroSAFlag)
	{
		fprintf(fp, ", saTally");
		SATally = 0;
	}
	fprintf(fp, ", \n\t{dropletID");
	if(posFlag)
	{
		fprintf(fp,", {xPos,yPos,zPos}");
	}
	if(colorFlag)
	{
		fprintf(fp,", {rColor,gColor,bColor}");
	}
	if(rotationFlag)
	{
		fprintf(fp,", {xRot,yRot,zRot}");
	}
	if(commSAFlag)
	{
		fprintf(fp,", {saBool}");
	}
	fprintf(fp, "},\n\t...\n},\n...\n}\n*)\n");
	fprintf(fp, "rawDat={\n");
	fflush(fp);

}

void SimInfoLogger::close()
{
	// If newFile == NULL, then Init has not been called.
	// and thus a file has not been opened.
	if(fp != NULL)
	{	
		fprintf(fp,"}");
		fclose(fp);
        fp = NULL;
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
	if(firstTime) firstTime=false;
	else fprintf(fp, ", ");
	fprintf(fp, "{%.3f, %.3f", stateInfo.simTime, stateInfo.realTime);
	if (macroRedFlag)
	{
		fprintf(fp, ", %d", redTally);
	}
	if (macroSAFlag)
	{
		fprintf(fp, ", %d", SATally);
	}
	fprintf(fp, ", {\n");
	// Messages printed once per droplet per step
	bool first_bot = true;
	foreach(dropletStruct_t droplet, stateInfo.dropletData)
	{
		if(first_bot) first_bot=false;
		else fprintf(fp, ", ");
		fprintf(fp,"{%i", droplet.dropletID);
		if(posFlag)
		{
			fprintf(fp,", {%3.4f, %3.4f, %3.4f}",droplet.origin.x,droplet.origin.y,droplet.origin.z);
		}
		if(colorFlag)
		{
			fprintf(fp,", {%03i, %03i, %03i}",droplet.color.r,droplet.color.g,droplet.color.b);
			if(macroRedFlag && droplet.color.r > 240 && droplet.color.g < 15 && droplet.color.b < 15){
				redTally++;
			}
		}
		if(rotationFlag)
		{
			fprintf(fp,", {%3.4f, %3.4f, %3.4f}",droplet.rotation.x,droplet.rotation.y,droplet.rotation.z);
		}
		if(commSAFlag)
		{
			if(droplet.commData.sendActive)
			{
				fprintf(fp,", {True}");
				if(macroSAFlag){
					SATally++;
				}
			}
			else
			{
				fprintf(fp,", {False}");
			}
		}
		fprintf(fp,"}");
	}
	fprintf(fp, "}\n}");
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
