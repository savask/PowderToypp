#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <queue>
#include "GameView.h"
#include "GameModel.h"
#include "interface/Point.h"
#include "simulation/Simulation.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "login/LoginController.h"
#include "ssave/SSaveController.h"
#include "console/ConsoleController.h"
//#include "cat/TPTScriptInterface.h"
#include "cat/LuaScriptInterface.h"
#include "Menu.h"

using namespace std;

class GameModel;
class GameView;
class CommandInterface;
class ConsoleController;
class GameController
{
private:
	//Simulation * sim;
	GameView * gameView;
	GameModel * gameModel;
	SearchController * search;
	RenderController * renderOptions;
	LoginController * loginWindow;
	SSaveController * ssave;
	ConsoleController * console;
	CommandInterface * commandInterface;
public:
	class LoginCallback;
	class SearchCallback;
	class RenderCallback;
	class SSaveCallback;
	GameController();
	~GameController();
	GameView * GetView();
	void SetZoomEnabled(bool zoomEnable);
	void SetZoomPosition(ui::Point position);
	void AdjustBrushSize(int direction);
	void AdjustZoomSize(int direction);
	void DrawPoints(int toolSelection, queue<ui::Point*> & pointQueue);
	void DrawRect(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawLine(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawFill(int toolSelection, ui::Point point);
	void Update();
	void SetPaused(bool pauseState);
	void SetPaused();
	void SetActiveMenu(Menu * menu);
	void SetActiveTool(int toolSelection, Tool * tool);
	void OpenSearch();
	void OpenLogin();
	void OpenTags();
	void OpenDisplayOptions();
	void OpenRenderOptions();
	void OpenSaveWindow();
	void ClearSim();
	void ReloadSim();
	void Vote(int direction);
	void ChangeBrush();
	void ShowConsole();
	void FrameStep();
	ui::Point PointTranslate(ui::Point point);
};

#endif // GAMECONTROLLER_H
