#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include <deque>
#include "search/Save.h"
#include "simulation/Simulation.h"
#include "interface/Colour.h"
#include "Renderer.h"
#include "GameView.h"
#include "Brush.h"
#include "client/User.h"

#include "Tool.h"
#include "Menu.h"

using namespace std;

class GameView;
class Simulation;
class Renderer;

class ToolSelection
{
public:
	enum
	{
		ToolPrimary, ToolSecondary, ToolTertiary
	};
};

class GameModel
{
private:
	//int clipboardSize;
	//unsigned char * clipboardData;
	Save * stamp;
	Save * clipboard;
	deque<string> consoleLog;
	vector<GameView*> observers;
	vector<Tool*> toolList;
	vector<Menu*> menuList;
	Menu * activeMenu;
	int currentBrush;
	vector<Brush *> brushList;
	Save * currentSave;
	Simulation * sim;
	Renderer * ren;
	Tool * activeTools[3];
	User currentUser;
	bool colourSelector;
	ui::Colour colour;
	//bool zoomEnabled;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
	void notifyDecorationChanged();
	void notifySaveChanged();
	void notifyBrushChanged();
	void notifyMenuListChanged();
	void notifyToolListChanged();
	void notifyActiveToolsChanged();
	void notifyUserChanged();
	void notifyZoomChanged();
	void notifyClipboardChanged();
	void notifyStampChanged();
	void notifyColourSelectorColourChanged();
	void notifyColourSelectorVisibilityChanged();
	void notifyLogChanged(string entry);
public:
	GameModel();
	~GameModel();

	void SetColourSelectorVisibility(bool visibility);
	bool GetColourSelectorVisibility();

	void SetColourSelectorColour(ui::Colour colour);
	ui::Colour GetColourSelectorColour();

	void SetVote(int direction);
	Save * GetSave();
	Brush * GetBrush();
	void SetSave(Save * newSave);
	void AddObserver(GameView * observer);
	Tool * GetActiveTool(int selection);
	void SetActiveTool(int selection, Tool * tool);
	bool GetPaused();
	void SetPaused(bool pauseState);
	bool GetDecoration();
	void SetDecoration(bool decorationState);
	void ClearSimulation();
	vector<Menu*> GetMenuList();
	vector<Tool*> GetToolList();
	void SetActiveMenu(Menu * menu);
	Menu * GetActiveMenu();
	void FrameStep(int frames);
	User GetUser();
	void SetUser(User user);
	void SetBrush(int i);
	int GetBrushID();
	Simulation * GetSimulation();
	Renderer * GetRenderer();
	void SetZoomEnabled(bool enabled);
	bool GetZoomEnabled();
	void SetZoomSize(int size);
	int GetZoomSize();
	void SetZoomFactor(int factor);
	int GetZoomFactor();
	void SetZoomPosition(ui::Point position);
	ui::Point GetZoomPosition();
	void SetZoomWindowPosition(ui::Point position);
	ui::Point GetZoomWindowPosition();
	void SetStamp(Save * newStamp);
	void AddStamp(unsigned char * saveData, int saveSize);
	void SetClipboard(unsigned char * saveData, int saveSize);
	void Log(string message);
	deque<string> GetLog();
	Save * GetClipboard();
	Save * GetStamp();
};

#endif // GAMEMODEL_H
