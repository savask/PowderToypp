/*
 * PreviewController.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWCONTROLLER_H_
#define PREVIEWCONTROLLER_H_

#include "preview/PreviewModel.h"
#include "preview/PreviewView.h"
#include "Controller.h"
#include "client/SaveInfo.h"
#include "client/ClientListener.h"

class LoginController;
class PreviewModel;
class PreviewView;
class PreviewController: public ClientListener {
	int saveId;
	PreviewModel * previewModel;
	PreviewView * previewView;
	LoginController * loginWindow;
	ControllerCallback * callback;
public:
	virtual void NotifyAuthUserChanged(Client * sender);

	bool HasExited;
	PreviewController(int saveID, ControllerCallback * callback);
	void Exit();
	void DoOpen();
	void OpenInBrowser();
	void Report(std::string message);
	void ShowLogin();
	bool GetDoOpen();
	SaveInfo * GetSave();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();
	void SubmitComment(std::string comment);

	void NextCommentPage();
	void PrevCommentPage();

	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
