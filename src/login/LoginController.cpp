/*
 * LoginController.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#include "LoginController.h"

LoginController::LoginController() {
	// TODO Auto-generated constructor stub
	loginView = new LoginView();
	loginModel = new LoginModel();

	loginView->AttachController(this);
	loginModel->AddObserver(loginView);

}

void LoginController::Login(string username, string password)
{
	loginModel->Login(username, password);
}

void LoginController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == loginView)
	{
		ui::Engine::Ref().CloseWindow();
		loginView = NULL;
	}
}

LoginController::~LoginController() {
	if(loginView)
		delete loginView;
	delete loginModel;
}
