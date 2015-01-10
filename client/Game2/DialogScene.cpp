
// DialogScene.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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
	const std::string &title, const std::string &name) :
	SUPER(display, name),
	director(director), title(title),
	stoppingTransitionEnabled(false), actionGridCol(0)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	const auto &s = display.styles;

	SetPhaseTransitionDuration(200);

	fader.reset(new ScreenFade(s.dialogBg, 0.0));
	fader->AttachView(display);

	auto root = GetRoot();

	contentRoot = root->NewChild<Container>(display, Vec2(1280, 720 - 200));
	contentRoot->SetPos(0, 100);

	statusRoot = root->NewChild<Container>(
		display, Vec2(1280 - (MARGIN_WIDTH * 2), 40), false);
	statusRoot->SetPos(MARGIN_WIDTH, 720 - 80);
	statusRoot->SetVisible(false);

	auto titleLbl = root->NewChild<Label>(title, s.headingFont, s.headingFg);
	titleLbl->SetPos(MARGIN_WIDTH, 80);
	titleLbl->SetAlignment(Alignment::SW);
	titleLbl->SetFixedScale(true);

	// The default action buttons (OK, Cancel).

	actionGrid = statusRoot->NewChild<FlexGrid>(display);
	actionGrid->SetPos(1280 - (MARGIN_WIDTH * 2), 0);
	actionGrid->SetAlignment(Alignment::NE);
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
 * Enable the "OK" action for this scene.
 * @param label The optional fixed label for the button (will use default
 *              action name if empty).
 */
void DialogScene::SupportOkAction(const std::string &label)
{
	assert(!okBtn);

	okBtn = actionGrid->At(0, actionGridCol++).
		NewChild<Display::ActionButton>(display, label)->GetContents();
}

/**
 * Enable the "Cancel" action for this scene.
 * @param label The optional fixed label for the button (will use default
 *              action name if empty).
 */
void DialogScene::SupportCancelAction(const std::string &label)
{
	assert(!cancelBtn);

	cancelBtn = actionGrid->At(0, actionGridCol++).
		NewChild<Display::ActionButton>(display, label)->GetContents();
}

/**
 * Enable the "extra" action for this scene.
 *
 * By default, scenes do not have an "extra" action, only OK and Cancel.
 * Activating this action will add a button to the status bar with the
 * specified label.
 *
 * This must only be called from the constructor, and must only be called
 * once.
 *
 * @param label The label for the button.
 */
void DialogScene::SupportExtraAction(const std::string &label)
{
	assert(!extraBtn);

	extraBtn = statusRoot->NewChild<Display::ActionButton>(display, label);
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

/// Called when the scene-specific extra action is fired.
void DialogScene::OnExtra()
{
	// Do nothing by default.
}

/**
 * Combine a parent dialog title to the child dialog title.
 * @param parent The parent title (may be empty).
 * @param title The child title.
 * @return The combined title.
 */
std::string DialogScene::JoinTitles(const std::string &parent,
	const std::string &title)
{
	return parent.empty() ? title : (parent + " // " + title);
}


void DialogScene::AttachController(Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	controller.AddMenuMaps();

	// If the cancel button is not enabled, then both the "OK" and "Cancel"
	// actions map to the "OnOk" handler.

	if (okBtn) {
		auto &menuOkAction = controller.actions.ui.menuOk;
		okConn = menuOkAction->Connect(
			std::bind(&DialogScene::OnOk, this));
		okBtn->AttachAction(controller, menuOkAction);
	}
	if (cancelBtn) {
		auto &menuCancelAction = controller.actions.ui.menuCancel;
		cancelConn = menuCancelAction->Connect(
			std::bind(&DialogScene::OnCancel, this));
		cancelBtn->AttachAction(controller, menuCancelAction);
	}
	if (extraBtn) {
		auto &menuExtraAction = controller.actions.ui.menuExtra;
		extraConn = menuExtraAction->Connect(
			std::bind(&DialogScene::OnExtra, this));
		extraBtn->AttachAction(controller, menuExtraAction);
	}

	statusRoot->SetVisible(true);

	// Attaching the action may have changed the button size.
	RequestLayout();
}

void DialogScene::DetachController(Control::InputEventController &controller)
{
	extraConn.disconnect();
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
