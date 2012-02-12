/*
 * PreviewView.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include <vector>
#include "PreviewView.h"
#include "interface/Point.h"
#include "interface/Window.h"
#include "search/Thumbnail.h"

PreviewView::PreviewView():
	ui::Window(ui::Point(-1, -1), ui::Point((XRES/2)+200, (YRES/2)+150)),
	savePreview(NULL)
{
	class OpenAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		OpenAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->DoOpen();
			v->c->Exit();
		}
	};
	openButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(50, 16), "Open");
	openButton->SetAlignment(AlignLeft, AlignMiddle);
	openButton->SetIcon(IconOpen);
	openButton->SetActionCallback(new OpenAction(this));
	AddComponent(openButton);

	class BrowserOpenAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		BrowserOpenAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->OpenInBrowser();
		}
	};
	browserOpenButton = new ui::Button(ui::Point((XRES/2)-90, Size.Y-16), ui::Point(90, 16), "Open in browser");
	browserOpenButton->SetAlignment(AlignLeft, AlignMiddle);
	browserOpenButton->SetIcon(IconOpen);
	browserOpenButton->SetActionCallback(new BrowserOpenAction(this));
	AddComponent(browserOpenButton);

	saveNameLabel = new ui::Label(ui::Point(5, (YRES/2)+15), ui::Point(100, 16), "");
	saveNameLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(saveNameLabel);

	saveDescriptionTextblock = new ui::Textblock(ui::Point(5, (YRES/2)+15+14+17), ui::Point((XRES/2)-10, Size.Y-((YRES/2)+15+14+17)-21), "");
	saveDescriptionTextblock->SetAlignment(AlignLeft, AlignTop);
	saveDescriptionTextblock->SetTextColour(ui::Colour(180, 180, 180));
	AddComponent(saveDescriptionTextblock);

	authorDateLabel = new ui::Label(ui::Point(5, (YRES/2)+15+14), ui::Point(100, 16), "");
	authorDateLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(authorDateLabel);
}

void PreviewView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	//Window Background+Outline
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	//Save preview (top-left)
	if(savePreview && savePreview->Data)
	{
		g->draw_image(savePreview->Data, (Position.X+1)+(((XRES/2)-savePreview->Size.X)/2), (Position.Y+1)+(((YRES/2)-savePreview->Size.Y)/2), savePreview->Size.X, savePreview->Size.Y, 255);
	}
	g->drawrect(Position.X, Position.Y, XRES/2, YRES/2, 255, 255, 255, 100);
	g->draw_line(Position.X+XRES/2, Position.Y, Position.X+XRES/2, Position.Y+Size.Y, 255, 255, 255, XRES+BARSIZE);


	g->draw_line(Position.X+1, Position.Y+10+YRES/2, Position.X-2+XRES/2, Position.Y+10+YRES/2, 100, 100, 100, XRES+BARSIZE);
	float factor;
	if(!votesUp && !votesDown)
		return;
	else
		factor = (float)(((float)(XRES/2)-2)/((float)(votesUp+votesDown)));
	g->fillrect(1+Position.X, 1+Position.Y+YRES/2, (XRES/2)-2, 8, 200, 50, 50, 255);
	g->fillrect(1+Position.X, 1+Position.Y+YRES/2, (int)(((float)votesUp)*factor), 8, 50, 200, 50, 255);
	g->fillrect(1+Position.X, 1+Position.Y+(YRES/2), 14, 8, 0, 0, 0, 100);
	g->fillrect(Position.X+(XRES/2)-15, 1+Position.Y+(YRES/2), 14, 8, 0, 0, 0, 100);
	g->draw_icon(1+Position.X+2, Position.Y+(YRES/2)+2, IconVoteUp);
	g->draw_icon(Position.X+(XRES/2)-12, Position.Y+(YRES/2)-1, IconVoteDown);
}

void PreviewView::OnTick(float dt)
{
	c->Update();
}

void PreviewView::OnMouseDown(int x, int y, unsigned button)
{
	if(!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X)) //Clicked outside window
		c->Exit();
}

void PreviewView::NotifySaveChanged(PreviewModel * sender)
{
	Save * save = sender->GetSave();
	if(save)
	{
		votesUp = save->votesUp;
		votesDown = save->votesDown;
		saveNameLabel->SetText(save->name);
		authorDateLabel->SetText("\bgAuthor:\bw " + save->userName + " \bgDate:\bw ");
		saveDescriptionTextblock->SetText(save->Description);
	}
	else
	{
		votesUp = 0;
		votesDown = 0;
		saveNameLabel->SetText("");
		authorDateLabel->SetText("");
		saveDescriptionTextblock->SetText("");
	}
}

void PreviewView::NotifyCommentsChanged(PreviewModel * sender)
{
	for(int i = 0; i < commentComponents.size(); i++)
	{
		RemoveComponent(commentComponents[i]);
		delete commentComponents[i];
	}
	commentComponents.clear();

	int currentY = 0;
	ui::Label * tempUsername;
	ui::Textblock * tempComment;
	std::vector<Comment*> * tempComments = sender->GetComments();
	if(tempComments)
	{
		for(int i = 0; i < tempComments->size(); i++)
		{
			tempUsername = new ui::Label(ui::Point((XRES/2) + 5, currentY+5), ui::Point(Size.X-((XRES/2) + 10), 16), tempComments->at(i)->authorName);
			tempUsername->SetAlignment(AlignLeft, AlignBottom);
			currentY += 16;
			tempComment = new ui::Textblock(ui::Point((XRES/2) + 5, currentY+5), ui::Point(Size.X-((XRES/2) + 10), -1), tempComments->at(i)->comment);
			tempComment->SetAlignment(AlignLeft, AlignTop);
			tempComment->SetTextColour(ui::Colour(180, 180, 180));
			currentY += tempComment->Size.Y+4;

			if(currentY > Size.Y)
			{
				delete tempUsername;
				delete tempComment;
				break;
			}
			else
			{
				commentComponents.push_back(tempComment);
				AddComponent(tempComment);
				commentComponents.push_back(tempUsername);
				AddComponent(tempUsername);
			}
		}
	}
}

void PreviewView::NotifyPreviewChanged(PreviewModel * sender)
{
	savePreview = sender->GetPreview();
	if(savePreview && savePreview->Data && !(savePreview->Size.X == XRES/2 && savePreview->Size.Y == YRES/2))
	{
		int newSizeX, newSizeY;
		float factorX = ((float)XRES/2)/((float)savePreview->Size.X);
		float factorY = ((float)YRES/2)/((float)savePreview->Size.Y);
		float scaleFactor = factorY < factorX ? factorY : factorX;
		savePreview->Data = Graphics::resample_img(savePreview->Data, savePreview->Size.X, savePreview->Size.Y, savePreview->Size.X*scaleFactor, savePreview->Size.Y*scaleFactor);
		savePreview->Size.X *= scaleFactor;
		savePreview->Size.Y *= scaleFactor;
	}
}

PreviewView::~PreviewView() {
}

