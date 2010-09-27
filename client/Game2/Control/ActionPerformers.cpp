// ActionPerformers.cpp
// A collection of classes implementing the ActionPerformer template class
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

#include "ActionPerformers.h"
#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../HoverScript/HighConsole.h"

#include <Windows.h>
#include <dinput.h>

using namespace HoverRace::Client::Control;
using namespace HoverRace::MainCharacter;
using namespace HoverRace::Client::HoverScript;
using HoverRace::Client::Observer;

// constructor does not need to do anything but save our pointer
PlayerEffectAction::PlayerEffectAction(std::string name, int listOrder, MainCharacter* pmc) : ControlAction(name, listOrder), mc(pmc) { }

void PlayerEffectAction::SetMainCharacter(MainCharacter* mc) { this->mc = mc; }

void EngineAction::operator()(int eventValue)
{	
	if(eventValue > 0)
		mc->SetEngineState(true);
	else
		mc->SetEngineState(false);
}

void TurnLeftAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetTurnLeftState(true);
	else
		mc->SetTurnLeftState(false);
}

void TurnRightAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetTurnRightState(true);
	else
		mc->SetTurnRightState(false);
}

void JumpAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetJump();
}

void PowerupAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetPowerup();
}

void ChangeItemAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetChangeItem();
}

void BrakeAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetBrakeState(true);
	else
		mc->SetBrakeState(false);
}

void LookBackAction::operator()(int eventValue)
{
	if(eventValue > 0)
		mc->SetLookBackState(true);
	else
		mc->SetLookBackState(false);
}

//// Console actions

ConsoleAction::ConsoleAction(std::string name, int listOrder, HighConsole* hc) : ControlAction(name, listOrder), hc(hc) { }

void ConsoleAction::SetHighConsole(HighConsole* hc)
{
	this->hc = hc;
}

ConsoleKeyAction::ConsoleKeyAction(std::string name, int listOrder, HighConsole* hc, OIS::KeyCode kc) : ConsoleAction(name, listOrder, hc), kc(kc) { }

void ConsoleKeyAction::operator()(int eventValue)
{
	if(eventValue > 0) {
		HKL layout = GetKeyboardLayout(0);
		unsigned char state[256];
		// Generally, in this case we would use GetKeyboardState(); however,
		// this causes bizarre issues because that method plays with the input
		// event stack and we end up with the shift key held down one character
		// too long.  So this, while potentially slower, is actually accurate.
		// Plus users don't type too incredibly quickly.
		// TODO(ryan): #ifdef blocks for platform independence; depend on OIS unless on Windows
		for(int i = 0; i < 256; i++)
			state[i] = GetKeyState(i);
		int vk = MapVirtualKeyEx(kc, 1, layout);
		int out;
		int ret = ToAsciiEx(vk, kc, state, (LPWORD) &out, 0, layout);
		if(ret == 1)
			hc->OnChar((char) out);
	}
}

void ConsoleToggleAction::operator()(int eventValue)
{
	if(eventValue > 0 && hc != NULL) {
		if(hc->IsVisible()) {
			// Delete old console action maps, re-add player action maps.
			controller->ClearActionMap();
			for(int i = 0; i < oldMaps.size(); i++)
				controller->AddActionMap(oldMaps.at(i));
		} else {
			// Save our old maps
			oldMaps = controller->GetActiveMaps();
			controller->ClearActionMap();
			controller->AddActionMap("console-keys");
			controller->AddActionMap(_("Console"));
		}
		hc->ToggleVisible();
	}
}

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