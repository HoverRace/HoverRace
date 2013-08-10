
// MessageScene.cpp
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
	display(display), director(director),
	hasCancel(hasCancel)
{
	Config *cfg = Config::GetInstance();

	SetTransitionDuration(200);

	fader.reset(new Display::ScreenFade(0xcc000000, 0.0));
	fader->AttachView(display);

	std::string fontName = cfg->GetDefaultFontName();

	Display::Container *root = GetRoot();

	const double textWidth = 1280 - (HORZ_PADDING * 2);

	titleLbl = root->AddChild(new Display::Label(title,
		Display::UiFont(fontName, 40, Display::UiFont::BOLD),
		Display::COLOR_WHITE));
	titleLbl->SetPos(HORZ_PADDING, 180);
	titleLbl->SetWrapWidth(textWidth);

	messageLbl = root->AddChild(new Display::Label(message,
		Display::UiFont(fontName, 30), 0xffbfbfbf));
	messageLbl->SetWrapWidth(textWidth);
	// messageLbl position will be set in Layout().

	okBtn = root->AddChild(new Display::Button(display, ""));
	okBtn->SetPos(HORZ_PADDING, 480);
	okBtn->GetClickedSignal().connect(std::bind(&MessageScene::OnOk, this));

	if (hasCancel) {
		cancelBtn = root->AddChild(new Display::Button(display, ""));
		cancelBtn->SetPos(HORZ_PADDING + 200, 480);
		cancelBtn->GetClickedSignal().connect(std::bind(&MessageScene::OnCancel, this));
	}
}

MessageScene::~MessageScene()
{
}

void MessageScene::OnOk()
{
	//TODO: Fire "OK" event.
	director.RequestPopScene();
}

void MessageScene::OnCancel()
{
	//TODO: Fire "Cancel" event.
	director.RequestPopScene();
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

	std::ostringstream oss;
	oss << '[' << controller.HashToString(menuOkAction->GetPrimaryTrigger()) <<
		"] " << menuOkAction->GetName();
	okBtn->SetText(oss.str());

	if (hasCancel) {
		oss.str("");
		oss << '[' << controller.HashToString(menuCancelAction->GetPrimaryTrigger()) <<
			"] " << menuCancelAction->GetName();
		cancelBtn->SetText(oss.str());
	}
}

void MessageScene::DetachController(Control::InputEventController &controller)
{
	cancelConn.disconnect();
	okConn.disconnect();

	SUPER::DetachController(controller);
}

void MessageScene::OnTransition(double progress)
{
	fader->SetOpacity(progress);
}

void MessageScene::Layout()
{
	// Set the message position relative to the title.
	const Display::Vec2 titlePos = titleLbl->GetPos();
	messageLbl->SetPos(HORZ_PADDING, titlePos.y + titleLbl->Measure().y + 40);
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
