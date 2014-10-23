/**
 * \file	cu-droplet\DropletSimulator\DropletSimDemos\DropletGUI\src\RW_slots.cpp
 *
 * \brief	Implements the slots belonging to the RenderWidget class.
 */

#include "RenderWidget.h"


void RenderWidget::updateState(simState_t stateInfo)
{
	// block while resource is in use, then retain resource
	while(!_simStateLock.testAndSetOrdered(0,1)) {}

	// update shared variable
	_simState = stateInfo;

	// release resource
	_simStateLock.fetchAndStoreOrdered(0);

}

// slot that updates the arena configuration
void RenderWidget::updateArena(simSetting_t settings)
{
	_arena = settings;
	_hudInfo.simStepSize = 1000.0 / _arena.fps;
	_hudInfo.simStepSize = floor(_hudInfo.simStepSize *100.0 + 0.5) / 100.0;
	setupRenderStructs();
}

// slot that updates the paused status
void RenderWidget::updatePause(bool paused)
{
	_hudInfo.paused = paused;
}

// slot that updates time ratio information
// in theory, this could be passed along as part of updateState and all would work well
// however, this allows the timing information to change asychronously from the actual simulator state
// meaning it can be adjusted while the simulator is paused and not issuing updates
void RenderWidget::updateRate(simRate_t rates)
{
	_simRates = rates;

}
