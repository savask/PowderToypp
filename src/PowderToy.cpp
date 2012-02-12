
#include <time.h>
#include <SDL/SDL.h>
#ifdef WIN32
#include <SDL/SDL_syswm.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "Global.h"
#include "Graphics.h"
#if defined(LIN32) || defined(LIN64)
#include "icon.h"
#endif

#include "interface/Engine.h"
#include "interface/Button.h"
#include "interface/Panel.h"
#include "interface/ControlFactory.h"
#include "interface/Point.h"
#include "interface/Label.h"

#include "game/GameController.h"
#include "game/GameView.h"

#include "dialogues/ErrorMessage.h"

#include "client/HTTP.h"

using namespace std;

#ifdef WIN32
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

SDL_Surface * SDLOpen()
{
#if defined(WIN32) && defined(WINCONSOLE)
	FILE * console = fopen("CON", "w" );
#endif
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
	SDL_EnableUNICODE(1);
#if defined(WIN32) && defined(WINCONSOLE)
	//On Windows, SDL redirects stdout to stdout.txt, which can be annoying when debugging, here we redirect back to the console
	if (console)
	{

		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
		//fclose(console);
	}
#endif
#ifdef WIN32
	SDL_SysWMinfo SysInfo;
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWMInfo(&SysInfo) <= 0) {
	    printf("%s : %d\n", SDL_GetError(), SysInfo.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.window;
	HICON hIconSmall = (HICON)LoadImage(reinterpret_cast<HMODULE>(&__ImageBase), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(reinterpret_cast<HMODULE>(&__ImageBase), MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif defined(LIN32) || defined(LIN32)
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 64, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_WM_SetIcon(icon, NULL);
#endif

	SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	atexit(SDL_Quit);
	return SDL_SetVideoMode(XRES + BARSIZE, YRES + MENUSIZE, 32, SDL_SWSURFACE);
}

/*int SDLPoll(SDL_Event * event)
{
	while (SDL_PollEvent(event))
	{
		switch (event->type)
		{
			case SDL_QUIT:
				return 1;
		}
	}
	return 0;
}*/

int main(int argc, char * argv[])
{
	int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
	float fps = 0, delta = 1.0f;

	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().g->AttachSDLSurface(SDLOpen());

	ui::Engine * engine = &ui::Engine::Ref();
	engine->Begin(XRES+BARSIZE, YRES+MENUSIZE);

	GameController * gameController = new GameController();
	engine->ShowWindow(gameController->GetView());

	//new ErrorMessage("Error", "This is a test error message");

	SDL_Event event;
	while(engine->Running())
	{
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				engine->Exit();
				break;
			case SDL_KEYDOWN:
				engine->onKeyPress(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KEY_MOD_SHIFT, event.key.keysym.mod&KEY_MOD_CONTROL, event.key.keysym.mod&KEY_MOD_ALT);
				break;
			case SDL_KEYUP:
				engine->onKeyRelease(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KEY_MOD_SHIFT, event.key.keysym.mod&KEY_MOD_CONTROL, event.key.keysym.mod&KEY_MOD_ALT);
				break;
			case SDL_MOUSEMOTION:
				engine->onMouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_WHEELUP)
				{
					engine->onMouseWheel(event.motion.x, event.motion.y, 1);
				}
				else if (event.button.button == SDL_BUTTON_WHEELDOWN)
				{
					engine->onMouseWheel(event.motion.x, event.motion.y, -1);
				}
				else
				{
					engine->onMouseClick(event.motion.x, event.motion.y, event.button.button);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.button.button != SDL_BUTTON_WHEELUP && event.button.button != SDL_BUTTON_WHEELDOWN)
					engine->onMouseUnclick(event.motion.x, event.motion.y, event.button.button);
				break;
			}
			event.type = 0; //Clear last event
		}

		engine->Tick();
		engine->Draw();

		currentFrame++;
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime - lastTime;
		if(ui::Engine::Ref().FpsLimit > 2.0f && (currentFrame>2 || elapsedTime > 1000*2/ui::Engine::Ref().FpsLimit) && elapsedTime && currentFrame*1000/elapsedTime > ui::Engine::Ref().FpsLimit)
		{
			while (currentFrame*1000/elapsedTime > ui::Engine::Ref().FpsLimit)
			{
				SDL_Delay(1);
				currentTime = SDL_GetTicks();
				elapsedTime = currentTime-lastTime;
			}
		}
		if(elapsedTime>=1000)
		{
			fps = (((float)currentFrame)/((float)elapsedTime))*1000.0f;
			currentFrame = 0;
			lastTime = currentTime;
			if(ui::Engine::Ref().FpsLimit > 2.0f)
			{
				delta = ui::Engine::Ref().FpsLimit/fps;
			}
			else
			{
				delta = 1.0f;
			}
		}
		engine->SetFps(fps);
	}
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
}
