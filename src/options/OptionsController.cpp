/*
 * OptionsController.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#include "OptionsController.h"

OptionsController::OptionsController(Simulation * sim, ControllerCallback * callback_):
	callback(callback_),
	HasExited(false)
{
	view = new OptionsView();
	model = new OptionsModel(sim);
	model->AddObserver(view);

	view->AttachController(this);

}

void OptionsController::SetHeatSimulation(bool state)
{
	model->SetHeatSimulation(state);
}

void OptionsController::SetAmbientHeatSimulation(bool state)
{
	model->SetAmbientHeatSimulation(state);
}

void OptionsController::SetNewtonianGravity(bool state)
{
	model->SetNewtonianGravity(state);
}

void OptionsController::SetWaterEqualisation(bool state)
{
	model->SetWaterEqualisation(state);
}
void OptionsController::SetGravityMode(int gravityMode)
{
	model->SetGravityMode(gravityMode);
}
void OptionsController::SetAirMode(int airMode)
{
	model->SetAirMode(airMode);
}

OptionsView * OptionsController::GetView()
{
	return view;
}

void OptionsController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == view)
	{
		ui::Engine::Ref().CloseWindow();
	}
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}


OptionsController::~OptionsController() {
	if(ui::Engine::Ref().GetWindow() == view)
	{
		ui::Engine::Ref().CloseWindow();
	}
	delete model;
}

