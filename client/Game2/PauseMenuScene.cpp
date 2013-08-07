
// PauseMenuScene.cpp
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/Util/Config.h"

#include "PauseMenuScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

PauseMenuScene::PauseMenuScene(Display::Display &display,
                               GameDirector &director) :
	SUPER(display, "Pause Menu"),
	display(display), director(director)
{

	Config *cfg = Config::GetInstance();

	fader.reset(new Display::ScreenFade(0xdd000000, 1.0));
	fader->AttachView(display);

	Display::Container *root = GetRoot();

	double y = 200;

	AddButton(_("Return to Race"), y)->GetClickedSignal().connect(std::bind(
		&PauseMenuScene::OnCancel, this));

	y += 40;

	AddButton(_("Quit to Main Menu"), y)->GetClickedSignal().connect([&](Display::ClickRegion&) {
		//TODO: Confirmation dialog.
		director.RequestMainMenu();
	});
	AddButton(_("Quit to Desktop"), y)->GetClickedSignal().connect([&](Display::ClickRegion&) {
		//TODO: Confirmation dialog.
		director.RequestShutdown();
	});
}

PauseMenuScene::~PauseMenuScene()
{
}

std::shared_ptr<Display::Button> PauseMenuScene::AddButton(
	const std::string &text, double &y)
{
	typedef Display::UiViewModel::Alignment Alignment;

	auto btn = GetRoot()->AddChild(new Display::Button(display, text));
	btn->SetPos(640, y);
	btn->SetAlignment(Alignment::N);

	y += 80;

	return btn;
}

void PauseMenuScene::OnCancel()
{
	director.RequestPopScene();
}

void PauseMenuScene::AttachController(Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	controller.AddMenuMaps();

	returnConn = controller.actions.ui.menuCancel->Connect(
		std::bind(&PauseMenuScene::OnCancel, this));
}

void PauseMenuScene::DetachController(Control::InputEventController &controller)
{
	returnConn.disconnect();

	SUPER::DetachController(controller);
}

void PauseMenuScene::PrepareRender()
{
	fader->PrepareRender();

	SUPER::PrepareRender();
}

void PauseMenuScene::Render()
{
	fader->Render();

	if (GetPhase() != Phase::STOPPING) {
		SUPER::Render();
	}
}

}  // namespace Client
}  // namespace HoverRace
