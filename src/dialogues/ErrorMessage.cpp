/*
 * ErrorMessage.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include "ErrorMessage.h"
#include "interface/Button.h"
#include "interface/Label.h"

ErrorMessage::ErrorMessage(std::string title, std::string message):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 75))
{
	ui::Label * titleLabel = new ui::Label(ui::Point(2, 1), ui::Point(Size.X-4, 16), title);
	titleLabel->SetTextColour(ui::Colour(200, 100, 50));
	titleLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 18), ui::Point(Size.X-8, 60), message);
	messageLabel->SetAlignment(AlignLeft, AlignTop);
	AddComponent(messageLabel);

	class DismissAction: public ui::ButtonAction
	{
		ErrorMessage * message;
	public:
		DismissAction(ErrorMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
			//delete message; TODO: Fix component disposal
		}
	};

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "Dismiss");
	okayButton->SetAlignment(AlignRight, AlignBottom);
	okayButton->SetBorderColour(ui::Colour(200, 200, 200));
	okayButton->SetActionCallback(new DismissAction(this));
	AddComponent(okayButton);
	ui::Engine::Ref().ShowWindow(this);
}

void ErrorMessage::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

ErrorMessage::~ErrorMessage() {
}

