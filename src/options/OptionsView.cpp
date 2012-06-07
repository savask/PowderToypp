/*
 * OptionsView.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: Simon
 */

#include "OptionsView.h"
#include "Style.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/DropDown.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(300, 206)){

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "Simulation Options");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class HeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		HeatSimulationAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetHeatSimulation(sender->GetChecked()); }
	};

	heatSimulation = new ui::Checkbox(ui::Point(8, 23), ui::Point(Size.X-6, 16), "Heat simulation \bgIntroduced in version 34");
	heatSimulation->SetActionCallback(new HeatSimulationAction(this));
	AddComponent(heatSimulation);
	tempLabel = new ui::Label(ui::Point(24, heatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd behaviour with very old saves");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class AmbientHeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		AmbientHeatSimulationAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetAmbientHeatSimulation(sender->GetChecked()); }
	};

	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, 53), ui::Point(Size.X-6, 16), "Ambient heat simulation \bgIntroduced in version 50");
	ambientHeatSimulation->SetActionCallback(new AmbientHeatSimulationAction(this));
	AddComponent(ambientHeatSimulation);
	tempLabel = new ui::Label(ui::Point(24, ambientHeatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd behaviour with old saves");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class NewtonianGravityAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		NewtonianGravityAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetNewtonianGravity(sender->GetChecked()); }
	};

	newtonianGravity = new ui::Checkbox(ui::Point(8, 83), ui::Point(Size.X-6, 16), "Newtonian gravity \bgIntroduced in version 48");
	newtonianGravity->SetActionCallback(new NewtonianGravityAction(this));
	AddComponent(newtonianGravity);
	tempLabel = new ui::Label(ui::Point(24, newtonianGravity->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance on older computers");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class WaterEqualisationAction: public ui::CheckboxAction
		{
			OptionsView * v;
		public:
			WaterEqualisationAction(OptionsView * v_){	v = v_;	}
			virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetWaterEqualisation(sender->GetChecked()); }
		};

	waterEqualisation = new ui::Checkbox(ui::Point(8, 113), ui::Point(Size.X-6, 16), "Water equalisation \bgIntroduced in version 61");
	waterEqualisation->SetActionCallback(new WaterEqualisationAction(this));
	AddComponent(waterEqualisation);
	tempLabel = new ui::Label(ui::Point(24, waterEqualisation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance with a lot of water");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class AirModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		AirModeChanged(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetAirMode(option.second); }
	};
	airMode = new ui::DropDown(ui::Point(Size.X-88, 146), ui::Point(80, 16));
	AddComponent(airMode);
	airMode->AddOption(std::pair<std::string, int>("On", 0));
	airMode->AddOption(std::pair<std::string, int>("Pressure off", 1));
	airMode->AddOption(std::pair<std::string, int>("Velocity off", 2));
	airMode->AddOption(std::pair<std::string, int>("Off", 3));
	airMode->AddOption(std::pair<std::string, int>("No Update", 4));
	airMode->SetActionCallback(new AirModeChanged(this));
		
	tempLabel = new ui::Label(ui::Point(8, 146), ui::Point(Size.X-96, 16), "Air Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
		
	class GravityModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		GravityModeChanged(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetGravityMode(option.second); }
	};	
		
	gravityMode = new ui::DropDown(ui::Point(Size.X-88, 166), ui::Point(80, 16));
	AddComponent(gravityMode);
	gravityMode->AddOption(std::pair<std::string, int>("Vertical", 0));
	gravityMode->AddOption(std::pair<std::string, int>("Off", 1));
	gravityMode->AddOption(std::pair<std::string, int>("Radial", 2));
	gravityMode->SetActionCallback(new GravityModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 166), ui::Point(Size.X-96, 16), "Gravity Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);


	class CloseAction: public ui::ButtonAction
	{
	public:
		OptionsView * v;
		CloseAction(OptionsView * v_) { v = v_; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->Exit();
		}
	};

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	tempButton->SetActionCallback(new CloseAction(this));
	AddComponent(tempButton);
}

void OptionsView::NotifySettingsChanged(OptionsModel * sender)
{
	heatSimulation->SetChecked(sender->GetHeatSimulation());
	ambientHeatSimulation->SetChecked(sender->GetAmbientHeatSimulation());
	newtonianGravity->SetChecked(sender->GetNewtonianGravity());
	waterEqualisation->SetChecked(sender->GetWaterEqualisation());
	airMode->SetOption(sender->GetAirMode());
	gravityMode->SetOption(sender->GetGravityMode());
}

void OptionsView::AttachController(OptionsController * c_)
{
	c = c_;
}

void OptionsView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}


OptionsView::~OptionsView() {
	// TODO Auto-generated destructor stub
}

