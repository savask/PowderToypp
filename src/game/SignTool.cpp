#include <iostream>
#include "Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/DropDown.h"

class SignWindow: public ui::Window
{
public:
	ui::DropDown * justification;
	ui::Textbox * textField;
	SignTool * tool;
	Simulation * sim;
	int signID;
	ui::Point signPosition;
	SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_);
	virtual void OnDraw();
	virtual ~SignWindow() {}
	class OkayAction: public ui::ButtonAction
	{
	public:
		SignWindow * prompt;
		OkayAction(SignWindow * prompt_) { prompt = prompt_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();		
			if(prompt->signID==-1 && prompt->textField->GetText().length())
			{
				prompt->sim->signs.push_back(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, (sign::Justification)prompt->justification->GetOption().second));
			}
			else if(prompt->textField->GetText().length())
			{
				prompt->sim->signs[prompt->signID] = sign(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, (sign::Justification)prompt->justification->GetOption().second));
			}
			prompt->SelfDestruct();
		}
	};
};

SignWindow::SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
	tool(tool_),
	signID(signID_),
	sim(sim_),
	signPosition(position_)
{
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "New sign");
	messageLabel->SetTextColour(style::Colour::InformationTitle);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);
	
	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	okayButton->Appearance.BorderInactive = (ui::Colour(200, 200, 200));
	okayButton->SetActionCallback(new OkayAction(this));
	AddComponent(okayButton);
	
	justification = new ui::DropDown(ui::Point(8, 46), ui::Point(50, 16));
	AddComponent(justification);
	justification->AddOption(std::pair<std::string, int>("Left", (int)sign::Left));
	justification->AddOption(std::pair<std::string, int>("Centre", (int)sign::Centre));
	justification->AddOption(std::pair<std::string, int>("Right", (int)sign::Right));
	justification->SetOption(0);
	
	textField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 16), "");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	textField->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	AddComponent(textField);
	
	ui::Engine::Ref().ShowWindow(this);
}
void SignWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void SignTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	new SignWindow(this, sim, -1, position);
}