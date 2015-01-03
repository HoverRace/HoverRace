
// MessageScene.cpp
//
// Copyright (c) 2013, 2015 Michael Imamura.
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/ActionButton.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Config.h"

#include "MessageScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param display The current display.
 * @param director The current game director.
 * @param title The title text (may be empty).
 * @param message The message text (may be empty).
 * @param hasCancel @c true for both "OK" and "Cancel" buttons,
 *                  @c false for only an "OK" button.
 */
MessageScene::MessageScene(Display::Display &display,
	GameDirector &director,
	const std::string &title,
	const std::string &message,
	bool hasCancel) :
	SUPER(display, "Message (" + title + ")"),
	director(director),
	hasCancel(hasCancel)
{
	using namespace Display;

	SetPhaseTransitionDuration(200);

	fader.reset(new ScreenFade(0xcc000000, 0.0));
	fader->AttachView(display);

	const auto &s = display.styles;

	auto root = GetRoot();

	const double textWidth = 1280 - (HORZ_PADDING * 2);

	titleLbl = root->NewChild<Label>(title, s.headingFont, s.headingFg);
	titleLbl->SetPos(HORZ_PADDING, 180);
	titleLbl->SetWrapWidth(textWidth);

	messageLbl = root->NewChild<Label>(message, s.bodyFont, s.bodyFg);
	messageLbl->SetWrapWidth(textWidth);
	// messageLbl position will be set in Layout().

	okBtn = root->NewChild<ActionButton>(display);
	okBtn->SetPos(HORZ_PADDING, 480);

	if (hasCancel) {
		cancelBtn = root->NewChild<ActionButton>(display);
		cancelBtn->SetPos(HORZ_PADDING + 200, 480);
	}
}

MessageScene::~MessageScene()
{
}

void MessageScene::OnOk()
{
	director.RequestPopScene();
	okSignal();
}

void MessageScene::OnCancel()
{
	director.RequestPopScene();
	cancelSignal();
}

void MessageScene::AttachController(Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	controller.AddMenuMaps();

	// If the cancel button is not enabled, then both the "OK" and "Cancel"
	// actions map to the "OnOk" handler.

	auto &menuOkAction = controller.actions.ui.menuOk;
	auto &menuCancelAction = controller.actions.ui.menuCancel;

	okConn = menuOkAction->Connect(std::bind(&MessageScene::OnOk, this));
	cancelConn = menuCancelAction->Connect(std::bind(
		hasCancel ? &MessageScene::OnCancel : &MessageScene::OnOk, this));

	okBtn->AttachAction(controller, menuOkAction);
	if (hasCancel) {
		cancelBtn->AttachAction(controller, menuCancelAction);
	}

	// Attaching the action may have changed the button size.
	RequestLayout();
}

void MessageScene::DetachController(Control::InputEventController &controller)
{
	cancelConn.disconnect();
	okConn.disconnect();

	SUPER::DetachController(controller);
}

void MessageScene::OnPhaseTransition(double progress)
{
	fader->SetOpacity(progress);
}

void MessageScene::Layout()
{
	// Set the message position relative to the title.
	const Vec2 titlePos = titleLbl->GetPos();
	messageLbl->SetPos(HORZ_PADDING, titlePos.y + titleLbl->Measure().y + 40);

	if (hasCancel) {
		// Set the cancel button position relative to the OK button.
		const Vec2 okPos = okBtn->GetPos();
		cancelBtn->SetPos(okPos.x + okBtn->Measure().x + 40, okPos.y);
	}
}

void MessageScene::PrepareRender()
{
	fader->PrepareRender();

	SUPER::PrepareRender();
}

void MessageScene::Render()
{
	fader->Render();

	if (GetPhase() != Phase::STOPPING) {
		SUPER::Render();
	}
}

}  // namespace Client
}  // namespace HoverRace
