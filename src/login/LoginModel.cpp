/*
 * LoginModel.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#include "LoginModel.h"

LoginModel::LoginModel() {
	// TODO Auto-generated constructor stub

}

void LoginModel::Login(string username, string password)
{
	statusText = "Logging in...";
	loginStatus = false;
	notifyStatusChanged();
	LoginStatus status = Client::Ref().Login(username, password);
	switch(status)
	{
	case LoginOkay:
		statusText = "Logged in";
		loginStatus = true;
		break;
	case LoginPasswordInvalid:
		statusText = "Username or Password incorrect";
		break;
	case LoginUsernameInvalid:
		statusText = "Username incorrect";
		break;
	case LoginBanned:
		statusText = "Banned: " + Client::Ref().GetLastError();
		break;
	default:
		statusText = "Error";
		break;
	}
	notifyStatusChanged();
}

void LoginModel::AddObserver(LoginView * observer)
{
	observers.push_back(observer);
}

string LoginModel::GetStatusText()
{
	return statusText;
}

bool LoginModel::GetStatus()
{
	return loginStatus;
}

void LoginModel::notifyStatusChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStatusChanged(this);
	}
}

LoginModel::~LoginModel() {
	// TODO Auto-generated destructor stub
}
