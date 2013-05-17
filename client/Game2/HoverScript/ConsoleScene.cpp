
// ConsoleScene.cpp
//
// Copyright (c) 2013 Michael Imamura.
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

#include "StdAfx.h"

#include "../../../engine/Display/Display.h"
#include "../../../engine/Display/FillBox.h"
#include "../GameDirector.h"
#include "SysConsole.h"

#include "ConsoleScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

ConsoleScene::ConsoleScene(Display::Display &display, GameDirector &director,
                           SysConsole &console) :
	SUPER("Console"),
	display(display), director(director), console(console)
{
	winShadeBox = new Display::FillBox(1280, 720, 0xbf000000);
	winShadeBox->AttachView(display);
}

ConsoleScene::~ConsoleScene()
{
	delete(winShadeBox);
}

void ConsoleScene::OnConsoleToggle()
{
	director.RequestPopScene();
}

void ConsoleScene::AttachController(Control::InputEventController &controller)
{
	controller.AddConsoleToggleMaps();
	auto &consoleToggleAction = controller.actions.sys.consoleToggle;
	consoleToggleConn = consoleToggleAction->Connect(std::bind(&ConsoleScene::OnConsoleToggle, this));
}

void ConsoleScene::DetachController(Control::InputEventController &controller)
{
	consoleToggleConn.disconnect();
}

void ConsoleScene::Advance(Util::OS::timestamp_t tick)
{
	// Act like the starting and stopping phases don't even exist.
	switch (GetPhase()) {
		case Phase::STARTING:
			SetPhase(Phase::RUNNING);
			break;
		case Phase::STOPPING:
			SetPhase(Phase::STOPPED);
			break;
	}
}

void ConsoleScene::PrepareRender()
{
	winShadeBox->PrepareRender();
}

void ConsoleScene::Render()
{
	winShadeBox->Render();
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
