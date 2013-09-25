/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletConsole\src\main.cpp
 *
 * \brief	This file is the main file of the console project that runs the simulator without a GUI.
 *			This project compiles on windows and UNIX systems. It also provides a good example of how 
 *			to setup and run the simulator.
 *
 * 			Command line options:
 * 			-n	number of droplets
 * 			-f	output file
 * 			-t	number of steps to run the simulator
 * 			-p	print interval
 */

#include <IDroplet.h>
#include <DropletSim.h>
#include <DropletDataStructs.h>
#include <DropletSimGlobals.h>
#include <DropletSimInfo.h>

#include <DefaultPrograms/DropletRGBSense/DropletRGBSense.h>
#include <DefaultPrograms/DropletMarch/DropletMarch.h>
#include <DefaultPrograms/DropletStickPullers/DropletStickPullers.h>
#include <DefaultPrograms/DropletRandomWalk/DropletRandomWalk.h>
#include <CustomPrograms/DropletCustomTwo/DropletCustomTwo.h>

#include <btBulletCollisionCommon.h>

#include <inttypes.h>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <utility>

// Droplet Simulator specific constants. 
#define DEFAULT_ROW_TILES 4
#define DEFAULT_COL_TILES 4
#define DEFAULT_NUM_DROPLETS 1
#define DEFAULT_TILE_LENGTH 25.0f
#define DEFAULT_DROPLET_RADIUS 2.2f
#define DEFAULT_WALL_HEIGHT 15.0f // in cm
#define DEFAULT_WALL_WIDTH	1.0f
#define DEFAULT_FPS 60.0f	// 60 frames per second
#define DEFAULT_TOTAL_TIMESTEPS 5000
#define DEFAULT_PRINT_INTERVAL .5 // prints output to file for each .5 secs in sim time


/**
 * \brief	simulator instance.
 */

DropletSim sim;

/**
 * \brief	sim info instance. This class is used to gather all info on the droplets stored in DropletDataStructs
 */

DropletSimInfo simInfo;

/**
 * \property	int numRowTiles, numColTiles, numDroplets, numSteps
 * 				float tileLength, dropletRadius, wallWidth, wallHeight, fps, printInterval;
 *
 * \brief	simulator params:
 * 			numRowTiles and numColTiles refer to the dimensions of the arena
 * 			numSteps is	the number of steps to run the simulator
 * 			fps determines the steps simulated per second, not the rendering rate
 * 			printInterval determines how many steps to compute before the next print
 */
int numRowTiles, numColTiles, numDroplets, numSteps;
float tileLength, dropletRadius, wallWidth, wallHeight, fps, printInterval;

/**
 * \property	int btXWallShapeID, btYWallShapeID, btFloorShapeID, btDropletShapeID
 *
 * \brief	used to store IDs of bullet collision shapes.
 */

int btXWallShapeID, btYWallShapeID, btFloorShapeID, btDropletShapeID;
std::vector<char> prevTo, prevCo;

/**
 * \fn	float getRandomf(float min, float max)
 *
 * \brief	gets a random float within the spcified range.
 *
 * \param	min	The minimum.
 * \param	max	The maximum.
 *
 * \return	The random float value.
 */

float getRandomf(float min, float max)
{
	float range = max - min;
	return min + (range * ((float)rand() / RAND_MAX));
}

/**
 * \fn	void initSimulator(void)
 *
 * \brief	initializes the sim with default parameters and sets up the projector image and physics objects.
 */

void initSimulator(void)
{
	// Set up required values
	// TODO : Setting up these values should go in a different function
	numRowTiles		= DEFAULT_ROW_TILES;
	numColTiles		= DEFAULT_COL_TILES;
	//	numDroplets		= DEFAULT_NUM_DROPLETS;
	tileLength		= DEFAULT_TILE_LENGTH;
	dropletRadius	= DEFAULT_DROPLET_RADIUS;
	wallHeight		= DEFAULT_WALL_HEIGHT;
	wallWidth		= DEFAULT_WALL_WIDTH; 
	fps				= DEFAULT_FPS;

	// Initialize Simulator
	SimSetupData setupData(
		numRowTiles, 
		numColTiles, 
		tileLength, 
		dropletRadius,
		fps,
		true);
	sim.Init(setupData);

	// NOTE : Set up the projector ONLY if we need it! It slows the simulation down.
	if(sim.SetUpProjector(std::string(".//"), "ScaledArena.bmp") != DS_SUCCESS)
	{
		fprintf(stderr, "Projector not set up correctly. Aborting.\n");
		exit(1);
	}

	// Set up simulator phyiscs objects
	btCollisionShape *floorShape = new btStaticPlaneShape(btVector3(0, 0, 1) , 0);
	btCollisionShape *xWallShape = new btBoxShape(btVector3(
		btScalar(tileLength * numRowTiles),  
		btScalar(wallWidth),
		btScalar(wallHeight)));
	btCollisionShape *yWallShape = new btBoxShape(btVector3(
		btScalar(wallWidth), 
		btScalar(tileLength * numColTiles),
		btScalar(wallHeight)));
	btCollisionShape *dropletShape = new btCylinderShape(btVector3(
		dropletRadius, 
		dropletRadius,
		dropletRadius * 0.825));

	sim.AddCollisionShape(floorShape, &btFloorShapeID);
	sim.AddCollisionShape(xWallShape, &btXWallShapeID);
	sim.AddCollisionShape(yWallShape, &btYWallShapeID);
	sim.AddCollisionShape(dropletShape, &btDropletShapeID);

}

/**
 * \fn	void setupSimObjects()
 *
 * \brief	creates the arena and adds droplets with physics model. Droplets are added in random locations
 * 			between the bounding rectangle of the arena. Droplets are created with a mass of 1 gram and
 * 			.8 friction.
 */

void setupSimObjects()
{
	// Create the floor and walls
	sim.CreateFloor(btFloorShapeID, btXWallShapeID, btYWallShapeID);

	float floorWidth = tileLength * numColTiles;
	float floorLength = tileLength * numRowTiles;
	float posRangeWidth = floorWidth / 2.0f;
	float posRangeLength = floorLength / 2.0f;

	for(int i = 0; i < numDroplets; i++)
	{
		// Set up the simulator/physics model
		ObjectPhysicsData *dropletPhyDat = (ObjectPhysicsData *)malloc(sizeof(ObjectPhysicsData));
		dropletPhyDat->colShapeIndex = btDropletShapeID;
		dropletPhyDat->mass = 24.2f;
		dropletPhyDat->localInertia = btVector3(0.0, 0.0, 0.0);
		dropletPhyDat->friction = 0.5f;


		//IDroplet *newDroplet = new DropletMarch(dropletPhyDat);
		//IDroplet *newDroplet = new DropletStickPullers(dropletPhyDat);
		IDroplet *newDroplet = new DropletRandomWalk(dropletPhyDat);

		sim.AddDroplet(newDroplet, std::make_pair(
			getRandomf(-posRangeWidth + dropletRadius, posRangeWidth - dropletRadius),
			getRandomf(-posRangeLength + dropletRadius, posRangeLength - dropletRadius)), 
			0.0f);

	}
}

/**
 * \fn	void PrintStatus(std::vector<unsigned char *> *colors, std::vector<GPSInfo *> *xyVals,
 * 		std::vector<DropletCommData *> *comm, FILE *file)
 *
 * \brief	prints sim info into a specified file. The params given to the function are vectors filled with
 * 			the info of that type for all the droplets.
 *
 * \param [in,out]	colors	If non-null, the colors.
 * \param [in,out]	xyVals	If non-null, the xy vals.
 * \param [in,out]	comm  	If non-null, the communications.
 * \param [in,out]	file  	If non-null, the file.
 */

void PrintStatus(std::vector<unsigned char *> *colors, std::vector<GPSInfo *> *xyVals, 
				 std::vector<DropletCommData *> *comm, FILE *file)
{

	std::vector<GPSInfo *>::iterator it;
	it = xyVals->begin();
	// NOTE : 'i' skips over tiles & walls and starts at droplets
	int j = 0;

	for(unsigned int i = 0; i < xyVals->size(); i++)
	{
		GPSInfo *gpsInfo  = *it;
		fprintf(file,"[Droplet %05u] Color (R, G , B) = (%03u, %03u, %03u) Postion (X, Y) = (%9.5f, %9.5f)\n", 
			i + 1, 
			colors->at(i)[0],
			colors->at(i)[1],
			colors->at(i)[2],
			gpsInfo->posX,
			gpsInfo->posY);
		if (comm->at(i)->sendActive) {
			for (unsigned int j = 0; j < 6; j++) {
				fprintf(file, "\tComm Channel %u(lastMsgOut, lastMsgIn, outMsgLen, inMsgLen) = (%5u, %5u, %3u, %3u)\n",
					j,
					comm->at(i)->commChannels[j].lastMsgOutTimestamp,
					comm->at(i)->commChannels[j].lastMsgInTimestamp,
					comm->at(i)->commChannels[j].outMsgLength,
					comm->at(i)->commChannels[j].inMsgLength);
			}
		}
		it++;

	}

}

void PrintWaitStatus(
	std::vector<GPSInfo *> *gpsInfo,
	std::vector<DropletCommData *> *commData, 
	std::vector<DropletActuatorData *> *actData,
	FILE *file)
{
#ifdef _WIN32
	unsigned int size = min(commData->size(), actData->size());
#elif __linux__
	unsigned int size = std::min(commData->size(), actData->size());
#endif
	for(unsigned int i = 0; i < size; i++)
	{
		fprintf(file, "D %05u (X, Y) %f %f (R, G, B) %u %u %u MTR %g SA %u\n", 
			i, 
			gpsInfo->at(i)->posX,
			gpsInfo->at(i)->posY,
			actData->at(i)->rOut,
			actData->at(i)->gOut,
			actData->at(i)->bOut,
			actData->at(i)->moveTimeRemaining,
			commData->at(i)->sendActive);
	}
}

void PrintComputation(std::vector<DropletCompData *> *comp, FILE* fp)
{
	fprintf(fp,"====== COMPUTATION ======\n");
	for(int i = 0; i < numDroplets; i++)
	{
		fprintf(fp, "[Droplet %i] Comp (leg1Power, leg2Power, leg3Power, CapacitorPower, DropletID) = (%i,\t%i,\t%i,\t%u,%u)\n",
			i + 1,
			comp->at(i)->leg1Power,
			comp->at(i)->leg2Power,
			comp->at(i)->leg3Power,
			comp->at(i)->capacitorPower,
			comp->at(i)->dropletID);

	}
}

void PrintStateInfo(double ct, std::vector<DropletActuatorData *> *actData, FILE *fp)
{
	int wait, blind, co;
	wait = blind = co = 0;
	for(int i = 0; i < numDroplets; i++)
	{
		if(prevCo[i] && actData->at(i)->gOut < 255)
			prevCo[i] = 0;

		if(actData->at(i)->moveTimeRemaining == 0)
		{
			if(actData->at(i)->rOut < 255 &&
				actData->at(i)->gOut < 255)
				wait++;
		}
		if(actData->at(i)->rOut == 255)
			blind++;

		if(actData->at(i)->gOut == 255)
		{
			if(!prevCo[i])
			{
				prevCo[i] = 1;
				co++;
			}
			blind++;
		}

	}
	int cl = static_cast<int>(simInfo.GetNumCollisions(sim)) /*- numDroplets*/;
	fprintf(fp, "%.3f, %d, %d, %d, %d, %d\n", 
		ct, // Time
		numDroplets - wait - blind - cl, // Search
		wait, // Wait
		blind, // Blind
		cl, // Collision
		co); // Collaborate
}

/**
 * \fn	int main(int argc, char *argv[])
 *
 * \brief	main function:
 * 			1. gathers command line input arguments.  
 * 			2. initialize the simulator  
 * 			3. set up the physcis objects  
 * 			4. allocate memory for vectors containing output info  
 * 			5. steps the simulator for a given number of steps and prints info
 *			6. clean up simulator and free memory
 *
 *
 * \param	argc	Number of command-line arguments.
 * \param	argv	Array of command-line argument strings.
 *
 * \return	Exit-code for the process - 0 for success, else an error code.
 */
int main(int argc, char *argv[])
{
#ifdef _WIN32
	// for some reason, time_t is not simply an alias to an unsigned integer on Win32
	srand(static_cast<unsigned int>(time(0)));
#else
	srand(time(0));
#endif

	FILE *outFile = NULL;

	/* Process program arguments to select iterations and policy */
	/* Set default iterations if not supplied */

	numDroplets = DEFAULT_NUM_DROPLETS;
	numSteps = DEFAULT_TOTAL_TIMESTEPS;
	printInterval = DEFAULT_PRINT_INTERVAL;

	int i = 1;
	while (i < argc) {
		if (!strcmp(argv[i], "-n")) {
			i++;
			if (i < argc) {
				int temp = atol(argv[i]);
				if(temp < 1){
					fprintf(stderr, "Invalid number of droplets '%s'.\n",argv[i]);
					exit(EXIT_FAILURE);
				}
				numDroplets = temp;
				i++;
			} else {
				fprintf(stderr, "Invalid droplets value\n");
				exit(EXIT_FAILURE);                
			}
		} else if (!strcmp(argv[i], "-f")) {
			i++;
			if (i < argc) {
#ifdef _WIN32
				int result = fopen_s(&outFile,argv[i], "w");
				if (result != 0) {
					perror("Error: Cannot open specified file");
					exit(EXIT_FAILURE);
				}
#else
				FILE *result = fopen(argv[i],"w");
				if (result == 0)
				{
					perror("Error: Cannot open specified file");
					exit(EXIT_FAILURE);
				}
				outFile = result;
#endif
				i++;
			} else {
				fprintf(stderr, "Invalid filename\n");
				exit(EXIT_FAILURE);                
			}
		} else if (!strcmp(argv[i], "-t")) {
			i++;
			if (i < argc) {
				int temp = atol(argv[i]);
				if(temp < 1){
					fprintf(stderr, "Invalid repetitions '%s'.\n",argv[i]);
					exit(EXIT_FAILURE);
				}
				numSteps = temp;
				i++;
			} else {
				fprintf(stderr, "Invalid repetitions\n");
				exit(EXIT_FAILURE);                
			}
		}
		else if (!strcmp(argv[i], "-p")) {
			i++;
			if (i < argc) {
#ifdef _WIN32
				// Windows returns a double for atof instead of a float
				float temp = static_cast<float>(atof(argv[i]));
#else
				float temp = atof(argv[i]);
#endif
				if (temp < 0) {
					fprintf(stderr, "Invalid print interval '%s'.\n",argv[i]);
					exit(EXIT_FAILURE);
				}
				printInterval = temp;
				i++;
			}
			else {
				fprintf(stderr, "Invalid print interval\n");
				exit(EXIT_FAILURE);                
			}
		} 
		else if (!strcmp(argv[i], "-?")) {
			fprintf(stdout,"usage: HelloDroplets -? | -n <number of droplets> -f <output filename> -t <number of steps> -p <logging interval>\n");
			exit(EXIT_SUCCESS);
		}else {
			fprintf(stdout, "Error: invalid option '%s'.\n",argv[i]);
			fprintf(stdout,"usage: HelloDroplets -? | -n <number of droplets> -f <output filename> -t <number of steps> -p <logging interval>\n");
			exit(EXIT_SUCCESS);
		}

	}

	if (outFile == NULL)
	{
#ifdef _WIN32
		int result = fopen_s(&outFile,"output.txt", "w");
		if (result != 0) {
			perror("Error: Cannot open default output file for writing");
			exit(EXIT_FAILURE);
		}
#else
		FILE *result = fopen("output.txt","w");
		if (result == 0)
		{
			perror("Error: Cannot open default output file for writing");
			exit(EXIT_FAILURE);
		}
		outFile = result;
#endif

		//fprintf(outFile, "N %d\n",numDroplets);
	}

	initSimulator();

	setupSimObjects();

	//std::vector<unsigned char *> *dropletColors = new std::vector<unsigned char *>();
	//std::vector<GPSInfo *> *dropletPos = new std::vector<GPSInfo *>;
	//std::vector<DropletCommData *> *dropletComm = new std::vector<DropletCommData *>;
	std::vector<DropletActuatorData *> *dropletAct = new std::vector<DropletActuatorData *>();
	//std::vector<DropletCompData *> *dropletComp = new std::vector<DropletCompData *>();


	for(int i = 0; i < numDroplets; i++)
	{
		//unsigned char *tmp1 = (unsigned char *)malloc(sizeof(unsigned char) * 3);
		//dropletColors->push_back(tmp1);
		//GPSInfo *tmp2 = (GPSInfo *)malloc(sizeof(GPSInfo));
		//dropletPos->push_back(tmp2);
		//DropletCommData *tmp3 = (DropletCommData *)malloc(sizeof(DropletCommData));
		//dropletComm->push_back(tmp3);
		DropletActuatorData *tmp4 = (DropletActuatorData *)malloc(sizeof(DropletActuatorData));
		dropletAct->push_back(tmp4);

		prevTo.push_back(0);
		prevCo.push_back(0);
		//DropletCompData *tmp5 = (DropletCompData *)malloc(sizeof(DropletCompData));
		//dropletComp->push_back(tmp5);

		/*DropletCompData *tmp5 = (DropletCompData *)malloc(sizeof(DropletCompData));
		dropletComp->push_back(tmp5);*/
	}
	double lastPrintTime, currentTime, realTime;
	lastPrintTime = -printInterval - 1; // makes it print at sim time 0
	sim.Step();

	for(int i = 1; i <= numSteps; i++)
	{
		sim.Step();
		currentTime = simInfo.GetTotalST(sim);
		realTime = simInfo.GetTotalRT(sim);
		if ( (currentTime - lastPrintTime) > printInterval) {
		//	fprintf(outFile, "------ %.3f:SIM STEP %i ------\n", currentTime, i + 1);
			//fprintf(outFile, "Real Time: %.3f\n", realTime);
			//simInfo.GetDropletPositions(dropletPos,sim);

			//simInfo.GetCommData(dropletComm, sim);
			simInfo.GetActuationData(dropletAct, sim);
			//simInfo.GetCompData(dropletComp, sim);
			//PrintComputation(dropletComp, outFile);
			//PrintStatus(dropletColors,dropletPos, dropletComm, outFile);
			//PrintWaitStatus(dropletPos, dropletComm, dropletAct, outFile);

			PrintStateInfo(currentTime, dropletAct, outFile);

	//		fprintf(outFile,"\n");
			lastPrintTime = currentTime;
		}
	}
	fprintf(outFile, "------ SIMULATION ENDS ------\n");
	fclose(outFile);
	sim.Cleanup();

	fprintf(stdout, "Simulation Complete\n");

	//std::vector<unsigned char *>::reverse_iterator cit;
	//for(cit = dropletColors->rbegin(); cit != dropletColors->rend(); cit++)
	//	free((unsigned char *)*cit);

	//std::vector<GPSInfo *>::reverse_iterator xyit;
	//for(xyit = dropletPos->rbegin(); xyit != dropletPos->rend(); xyit++)
	//	free((GPSInfo *)*xyit);

	//std::vector<DropletCommData *>::reverse_iterator commit;
	//for(commit = dropletComm->rbegin(); commit != dropletComm->rend(); commit++)
	//	free((DropletCommData *)*commit);

	std::vector<DropletActuatorData *>::reverse_iterator act_rit;
	for(act_rit = dropletAct->rbegin(); act_rit != dropletAct->rend(); act_rit++)
		free((DropletActuatorData *)*act_rit);

	/*std::vector<DropletCompData *>::reverse_iterator compit;
	for(compit = dropletComp->rbegin(); compit != dropletComp->rend(); compit++)
	free((DropletCompData *)*compit);*/

	//delete dropletColors;
	//delete dropletPos;
	//delete dropletComm;
	delete dropletAct;
	//delete dropletComp;
	return(EXIT_SUCCESS);
}
