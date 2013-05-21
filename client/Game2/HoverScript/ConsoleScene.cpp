
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
#include "../../../engine/Display/Label.h"
#include "../GameDirector.h"
#include "SysConsole.h"

#include "ConsoleScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	static const std::string COMMAND_PROMPT(">> ");
	static const std::string CONTINUE_PROMPT(":> ");
}

ConsoleScene::ConsoleScene(Display::Display &display, GameDirector &director,
                           SysConsole &console) :
	SUPER("Console"),
	display(display), director(director), console(console)
{
	typedef Display::UiViewModel::Alignment Alignment;

	Config *cfg = Config::GetInstance();

	submitBuffer.reserve(1024);
	historyBuffer.reserve(1024);
	commandLine.reserve(1024);

	winShadeBox = new Display::FillBox(1280, 720, 0xbf000000);
	winShadeBox->AttachView(display);

	Display::UiFont font(cfg->GetDefaultMonospaceFontName(), 30);

	inputLbl = new Display::Label(COMMAND_PROMPT, font, 0xffffffff);
	inputLbl->SetPos(0, 719);
	inputLbl->SetAlignment(Alignment::SW);
	inputLbl->AttachView(display);
}

ConsoleScene::~ConsoleScene()
{
	delete(inputLbl);
	delete(winShadeBox);
}

void ConsoleScene::OnConsoleToggle()
{
	director.RequestPopScene();
}

void ConsoleScene::OnTextInput(const std::string &s)
{
	commandLine += s;
	inputLbl->SetText(COMMAND_PROMPT + commandLine);
}

void ConsoleScene::AttachController(Control::InputEventController &controller)
{
	controller.AddConsoleToggleMaps();
	auto &consoleToggleAction = controller.actions.sys.consoleToggle;
	consoleToggleConn = consoleToggleAction->Connect(std::bind(&ConsoleScene::OnConsoleToggle, this));

	auto &textAction = controller.actions.ui.text;
	textInputConn = textAction->Connect(std::bind(&ConsoleScene::OnTextInput, this, std::placeholders::_1));

	SDL_StartTextInput();
}

void ConsoleScene::DetachController(Control::InputEventController &controller)
{
	SDL_StopTextInput();

	textInputConn.disconnect();
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
	inputLbl->PrepareRender();
}

void ConsoleScene::Render()
{
	winShadeBox->Render();
	inputLbl->Render();
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
