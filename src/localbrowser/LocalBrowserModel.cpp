/*
 * StampsModel.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"
#include "client/Client.h"
#include "LocalBrowserModelException.h"

LocalBrowserModel::LocalBrowserModel():
	stamp(NULL),
	currentPage(1)
{
	// TODO Auto-generated constructor stub
	stampIDs = Client::Ref().GetStamps();
}


std::vector<SaveFile*> LocalBrowserModel::GetSavesList()
{
	return savesList;
}

void LocalBrowserModel::AddObserver(LocalBrowserView * observer)
{
	observers.push_back(observer);
	observer->NotifySavesListChanged(this);
	observer->NotifyPageChanged(this);
}

void LocalBrowserModel::notifySavesListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySavesListChanged(this);
	}
}

void LocalBrowserModel::notifyPageChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPageChanged(this);
	}
}

SaveFile * LocalBrowserModel::GetSave()
{
	return stamp;
}

void LocalBrowserModel::SetSave(SaveFile * newStamp)
{
	if(stamp)
		delete stamp;
	stamp = new SaveFile(*newStamp);
}

void LocalBrowserModel::UpdateSavesList(int pageNumber)
{
	std::vector<SaveFile*> tempSavesList = savesList;
	savesList.clear();
	currentPage = pageNumber;
	notifyPageChanged();
	notifySavesListChanged();
	/*notifyStampsListChanged();
	for(int i = 0; i < tempStampsList.size(); i++)
	{
		delete tempStampsList[i];
	}*/

	int stampsEnd = pageNumber*20;

	for(int i = stampsEnd-20; i<stampIDs.size() && i<stampsEnd; i++)
	{
		SaveFile * tempSave = Client::Ref().GetStamp(stampIDs[i]);
		if(tempSave)
		{
			savesList.push_back(tempSave);
		}
	}
	notifySavesListChanged();
}

void LocalBrowserModel::SelectSave(std::string stampID)
{
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==stampID)
		{
			return;
		}
	}
	selected.push_back(stampID);
	notifySelectedChanged();
}

void LocalBrowserModel::DeselectSave(std::string stampID)
{
	bool changed = false;
restart:
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==stampID)
		{
			selected.erase(selected.begin()+i);
			changed = true;
			goto restart; //Just ensure all cases are removed.
		}
	}
	if(changed)
		notifySelectedChanged();
}

void LocalBrowserModel::notifySelectedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		LocalBrowserView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}

LocalBrowserModel::~LocalBrowserModel() {
	if(stamp)
		delete stamp;
}

