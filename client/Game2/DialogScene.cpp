
// FormScene.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../../engine/Display/ActionButton.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "DialogScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

const double DialogScene::MARGIN_WIDTH = 40;

/**
 * Constructor.
 * @param display The display.
 * @param director The current game director.
 * @param title The title to display.
 * @param name The name of the scene.  See Scene::GetName.
 */
DialogScene::DialogScene(Display::Display &display, GameDirector &director,
                         const std::string &title,
                         const std::string &name) :
	SUPER(display, name),
	director(director), title(title),
	stoppingTransitionEnabled(false)
{
	typedef Display::UiViewModel::Alignment Alignment;

	const auto &s = display.styles;

	SetPhaseTransitionDuration(200);

	fader.reset(new Display::ScreenFade(s.dialogBg, 0.0));
	fader->AttachView(display);

	auto root = GetRoot();

	contentRoot = root->AddChild(new Display::Container(
		display, Vec2(1280, 720 - 200)));
	contentRoot->SetPos(0, 100);

	statusRoot = root->AddChild(new Display::Container(
		display, Vec2(1280 - (MARGIN_WIDTH * 2), 40), false));
	statusRoot->SetPos(MARGIN_WIDTH, 720 - 80);
	statusRoot->SetVisible(false);

	auto titleLbl = root->AddChild(new Display::Label(title,
		s.headingFont, s.headingFg));
	titleLbl->SetPos(MARGIN_WIDTH, 80);
	titleLbl->SetAlignment(Alignment::SW);

	// The default action buttons (OK, Cancel).

	actionGrid = statusRoot->AddChild(new Display::FlexGrid(display));
	actionGrid->SetPos(1280 - (MARGIN_WIDTH * 2), 0);
	actionGrid->SetAlignment(Alignment::NE);

	size_t r = 0;
	size_t c = 0;

	okBtn = actionGrid->AddGridCell(r, c++,
		new Display::ActionButton(display))->GetContents();
	cancelBtn = actionGrid->AddGridCell(r, c++,
		new Display::ActionButton(display))->GetContents();
}

DialogScene::~DialogScene()
{
}

/**
 * Enable or disable content visibility while scene is stopping.
 *
 * By default, dialog scenes hide the root container immediately when the
 * scene enters the @c STOPPING phase.  Setting this to @c true keeps the
 * root container visible so that transitions can be applied.
 *
 * @param enabled true to enable, false to disable.
 */
void DialogScene::SetStoppingTransitionEnabled(bool enabled)
{
	stoppingTransitionEnabled = enabled;
}

/**
 * Set the background, replacing the currently set background.
 * @param fader The background of the scene
 *              (may be @c nullptr for no background).
 *              Ownership is passed to the DialogScene.
 */
void DialogScene::SetBackground(Display::Background *fader)
{
	this->fader.reset(fader);
	if (fader) {
		fader->AttachView(display);
	}
}

/// Called when the "OK" action is fired.
void DialogScene::OnOk()
{
	director.RequestPopScene();
}

/// Called when the "Cancel" action is fired.
void DialogScene::OnCancel()
{
	director.RequestPopScene();
}

void DialogScene::AttachController(Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	controller.AddMenuMaps();

	// If the cancel button is not enabled, then both the "OK" and "Cancel"
	// actions map to the "OnOk" handler.

	auto &menuOkAction = controller.actions.ui.menuOk;
	auto &menuCancelAction = controller.actions.ui.menuCancel;

	okConn = menuOkAction->Connect(
		std::bind(&DialogScene::OnOk, this));
	cancelConn = menuCancelAction->Connect(
		std::bind(&DialogScene::OnCancel, this));

	okBtn->AttachAction(controller, menuOkAction);
	cancelBtn->AttachAction(controller, menuCancelAction);

	statusRoot->SetVisible(true);

	// Attaching the action may have changed the button size.
	RequestLayout();
}

void DialogScene::DetachController(Control::InputEventController &controller)
{
	cancelConn.disconnect();
	okConn.disconnect();

	statusRoot->SetVisible(false);

	SUPER::DetachController(controller);
}

void DialogScene::OnPhaseTransition(double progress)
{
	if (fader) {
		fader->SetOpacity(progress);
	}

	SUPER::OnPhaseTransition(progress);
}

void DialogScene::PrepareRender()
{
	if (fader) {
		fader->PrepareRender();
	}

	SUPER::PrepareRender();
}

void DialogScene::Render()
{
	if (fader) {
		fader->Render();
	}

	if (stoppingTransitionEnabled || GetPhase() != Phase::STOPPING) {
		SUPER::Render();
	}
}

}  // namespace Client
}  // namespace HoverRace
