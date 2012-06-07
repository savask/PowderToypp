#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <vector>
#include <queue>
#include <deque>
#include <string>
#include "GameController.h"
#include "GameModel.h"
#include "interface/Window.h"
#include "interface/Point.h"
#include "interface/Button.h"
#include "interface/Slider.h"
#include "ToolButton.h"
#include "Brush.h"

using namespace std;

enum DrawMode
{
	DrawPoints, DrawLine, DrawRect, DrawFill
};

enum SelectMode
{
	SelectNone, SelectStamp, SelectCopy, PlaceClipboard, PlaceStamp
};

class GameController;
class GameModel;
class GameView: public ui::Window
{
private:
	DrawMode drawMode;
	bool isMouseDown;
	bool zoomEnabled;
	bool zoomCursorFixed;
	int toolIndex;
	queue<ui::Point*> pointQueue;
	GameController * c;
	Renderer * ren;
	Brush * activeBrush;
	//UI Elements
	vector<ui::Button*> menuButtons;
	vector<ToolButton*> toolButtons;
	deque<string> logEntries;
	float lastLogEntry;
	ui::Button * searchButton;
    ui::Button * reloadButton;
    ui::Button * saveSimulationButton;
    ui::Button * downVoteButton;
    ui::Button * upVoteButton;
    ui::Button * tagSimulationButton;
    ui::Button * clearSimButton;
    ui::Button * loginButton;
    ui::Button * simulationOptionButton;
    ui::Button * displayModeButton;
	ui::Button * pauseButton;
	ui::Point currentMouse;

	ui::Slider * colourRSlider;
	ui::Slider * colourGSlider;
	ui::Slider * colourBSlider;
	ui::Slider * colourASlider;

	bool drawModeReset;
	ui::Point drawPoint1;
	ui::Point drawPoint2;

	SelectMode selectMode;
	ui::Point selectPoint1;
	ui::Point selectPoint2;

	ui::Point mousePosition;

	Thumbnail * clipboardThumb;
	Thumbnail * stampThumb;

	Particle sample;

	void changeColour();
public:
    GameView();

    //Breaks MVC, but any other way is going to be more of a mess.
    ui::Point GetMousePosition();
    void SetSample(Particle sample);

	void AttachController(GameController * _c){ c = _c; }
	void NotifyRendererChanged(GameModel * sender);
	void NotifySimulationChanged(GameModel * sender);
	void NotifyPausedChanged(GameModel * sender);
	void NotifySaveChanged(GameModel * sender);
	void NotifyBrushChanged(GameModel * sender);
	void NotifyMenuListChanged(GameModel * sender);
	void NotifyToolListChanged(GameModel * sender);
	void NotifyActiveToolsChanged(GameModel * sender);
	void NotifyUserChanged(GameModel * sender);
	void NotifyZoomChanged(GameModel * sender);
	void NotifyColourSelectorVisibilityChanged(GameModel * sender);
	void NotifyColourSelectorColourChanged(GameModel * sender);
	void NotifyClipboardChanged(GameModel * sender);
	void NotifyStampChanged(GameModel * sender);
	void NotifyLogChanged(GameModel * sender, string entry);
	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

	//Top-level handers, for Lua interface
	virtual void DoDraw();
	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned button);
	virtual void DoMouseUp(int x, int y, unsigned button);
	virtual void DoMouseWheel(int x, int y, int d);
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

	//virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) {}
	//virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) {}
	virtual void OnTick(float dt);
	virtual void OnDraw();
	class MenuAction;
	class ToolAction;
};

#endif // GAMEVIEW_H
