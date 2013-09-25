#include "DropletTimeControl.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

//=================== Windows functions =========================
#ifdef _WIN32
void DropletTimeControl::initTimer(double sss) {

	init_time = (double)(timeGetTime())/1000.0;
	current_time = init_time;
	sim_step_size = sss;
	elapsed_ST = 0.0;
}

void DropletTimeControl::updateTimer(double sss) {
	
	sim_step_size = sss;
	last_step = current_time;
	current_time = (double)(timeGetTime())/1000.0;
	elapsed_ST += sim_step_size;
}

void DropletTimeControl::updateTimer() {

	last_step = current_time;
	current_time = (double)(timeGetTime())/1000.0;
	elapsed_ST += sim_step_size;
}

void DropletTimeControl::resetTimer() {
	init_time = (double)(timeGetTime())/1000.0;
	current_time = init_time;
	elapsed_ST = 0.0;
}

//=================== Unix functions =========================
#else

double tvToDouble(struct timeval time) {

	return time.tv_sec + (double)(time.tv_usec)/1000000.0;
}
void DropletTimeControl::initTimer(double sss) {

	gettimeofday(&time, NULL);
	init_time = tvToDouble(time);
	current_time = init_time;
	sim_step_size = sss;
	elapsed_ST = 0.0;
}

void DropletTimeControl::updateTimer(double sss) {
	
	sim_step_size = sss;
	last_step = current_time;
	gettimeofday(&time, NULL);
	current_time = tvToDouble(time);
	elapsed_ST += sim_step_size;
}

void DropletTimeControl::updateTimer() {

	last_step = current_time;
	gettimeofday(&time, NULL);
	current_time = tvToDouble(time);
	elapsed_ST += sim_step_size;
}

void DropletTimeControl::resetTimer() {
	gettimeofday(&time, NULL);
	init_time = tvToDouble(time);
	current_time = init_time;
	elapsed_ST = 0.0;
}
#endif
// =============== get functions ====================
double DropletTimeControl::getTotalRT() {

	return current_time - init_time;
}

double DropletTimeControl::getTotalST() {

	return elapsed_ST;
}

double DropletTimeControl::getStepRT() {

	return current_time - last_step;
}

double DropletTimeControl::getTotalDiff() {

	return getTotalRT() - getTotalST();
}

double DropletTimeControl::getTimeRatio() {

	return sim_step_size/getStepRT();
}

// ================= Print Functions ==========================
void DropletTimeControl::printTotalRT() {

	printf("Total Real Time Elapsed: %.3f\n", getTotalRT() );
}

void DropletTimeControl::printTotalST() {
	
	printf("Total Simulator Time Elapsed: %.3f\n", getTotalST() );
}

void DropletTimeControl::printStepRT() {
	
	printf("Time Elapsed Since the Last Step: %.3f\n", getStepRT() );
}

void DropletTimeControl::printTotalDiff() {
	
	printf("Difference Between Elapsed RT and ST: %.3f\n", getTotalDiff() );
	printf("A negative value indicates RT < ST\n");
}

void DropletTimeControl::printTimeRatio() {
	
	printf("Ratio Between ST and RT: %.3f\n", getTimeRatio() );
}

void DropletTimeControl::printAll() {
	
	printf("Total RT: %.3f, Total ST: %.3f, Step time: %.3f, Total Diff: %.3f, Ratio: %.3f\n", 
		getTotalRT(), getTotalST(), getStepRT(), getTotalDiff(), getTimeRatio() );
}

void DropletTimeControl::printVars() {

	printf("init_time: %f, current_time: %f, last_step: %f\n", 
		init_time, current_time, last_step);
	printf("sim_step_size: %f, elapsed_ST: %f\n", sim_step_size, elapsed_ST);
}