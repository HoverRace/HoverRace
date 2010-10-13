// ConsoleActions.cpp
// A collection of classes implementing the ConsoleAction template class
// (just has to implement operator() and a constructor).  These are all
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

#include "ConsoleActions.h"

using namespace HoverRace::Client::Control;
using namespace HoverRace::Client::HoverScript;

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
			state[i] = static_cast<unsigned char>(GetKeyState(i));
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
			for(size_t i = 0; i < oldMaps.size(); i++)
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

void ConsoleScrollAction::operator()(int eventValue)
{
	if(eventValue > 0 && hc != NULL) {
		if(hc->IsVisible())
			hc->Scroll(scrollStep);
	}
}

void ConsoleScrollTopAction::operator()(int eventValue)
{
	if(eventValue > 0 && hc != NULL) {
		if(hc->IsVisible())
			hc->ScrollTop();
	}
}

void ConsoleScrollBottomAction::operator()(int eventValue)
{
	if(eventValue > 0 && hc != NULL) {
		if(hc->IsVisible())
			hc->ScrollBottom();
	}
}

void ConsoleHelpAction::operator()(int eventValue)
{
	if(eventValue > 0 && hc != NULL) {
		if(hc->IsVisible())
			hc->ToggleHelp();
	}
}