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

#include <Windows.h>
#include <dinput.h>

using namespace HoverRace::Client::Control;
using namespace HoverRace::MainCharacter;

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