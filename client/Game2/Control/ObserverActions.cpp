// ObserverActions.cpp
// A collection of classes implementing the ObserverAction template class
// (just has to implement Perform() and a constructor).  These are all
// actions to be performed when the user presses a control key.
//
// Copyright (c) 2010, Ryan Curtin
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//

#include "StdAfx.h"

#include "ObserverActions.h"

using namespace HoverRace::Client::Control;
using HoverRace::Client::Observer;

void ObserverAction::SetObservers(Observer** observers, int nObservers)
{
	this->observers = observers;
	this->nObservers = nObservers;
}

void ObserverTiltAction::operator()(int value)
{
	if(value > 0) {
		for(int i = 0; i < nObservers; i++)
			observers[i]->Scroll(tiltIncrement);
	}
}

void ObserverZoomAction::operator()(int value)
{
	if(value > 0) {
		for(int i = 0; i < nObservers; i++)
			observers[i]->Zoom(zoomIncrement);
	}
}

void ObserverResetAction::operator()(int value)
{
	if(value > 0) {
		for(int i = 0; i < nObservers; i++)
			observers[i]->Home();
	}
}