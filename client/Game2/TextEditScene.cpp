
// TextEditScene.cpp
//
// Copyright (c) 2016 Michael Imamura.
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
#include "../../engine/Display/ActiveText.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/Str.h"

#include "TextEditScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param display The target display.
 * @param director The current director.
 * @param parentTitle The title of the parent dialog (may be blank).
 * @param title Dialog title.
 * @param maxLength Maximum text length, in code points.
 * @param text Optional initial text.
 */
TextEditScene::TextEditScene(Display::Display &display, GameDirector &director,
	const std::string &parentTitle, const std::string &title,
	size_t maxLength, const std::string &text) :
	SUPER(display, director, parentTitle, title,
		"Text Edit (" + title + ")"),
	maxLength(maxLength), text(),
	cursorOn(true), cursorTick(0)
{
	using namespace Display;

	this->text.reserve(maxLength * 2);
	Str::Assign(this->text, text, maxLength);

	SupportOkAction();
	SupportCancelAction();

	auto root = GetContentRoot();

	const auto &s = display.styles;

	inputLbl = root->NewChild<ActiveText>(text, s.bodyFont, s.bodyFg);
	inputLbl->SetPos(640, 250);
	inputLbl->SetAlignment(UiViewModel::Alignment::N);
	inputLbl->SetFixedScale(true);
}

/**
 * Set instructions to display to the user.
 * @param text The instruction text to accompany the input box.
 */
void TextEditScene::SetHint(const std::string &text)
{
	if (hintLbl) {
		hintLbl->SetText(text);
	}
	else {
		using namespace Display;

		auto root = GetContentRoot();
		const auto &s = display.styles;

		hintLbl = root->NewChild<Label>(text, s.bodyFont, s.bodyFg);
		hintLbl->SetPos(640, 150);
		hintLbl->SetAlignment(UiViewModel::Alignment::S);
	}
}

void TextEditScene::OnTextInput(const std::string &s)
{
	cursorTick = OS::Time();

	Str::Append(text, s, maxLength);

	inputLbl->SetText(text);
}

void TextEditScene::OnTextControl(Control::TextControl::key_t key)
{
	cursorTick = OS::Time();

	switch (key) {
		case Control::TextControl::BACKSPACE:
			if (!text.empty()) {
				text.resize(text.length() - 1);
			}
			inputLbl->SetText(text);
			break;

		case Control::TextControl::ENTER:
			//TODO: Check if multiline.
			break;

		default:
			HR_LOG(warning) <<
				"TextEditScene: Unhandled text control key: " << key;
	}
}

void TextEditScene::OnOk()
{
	confirmSignal(text);

	SUPER::OnOk();
}

void TextEditScene::AttachController(Control::InputEventController &controller,
	ConnList &conns)
{
	SUPER::AttachController(controller, conns);

	conns <<
		controller.actions.ui.control->Connect(
			std::bind(&TextEditScene::OnTextControl, this, std::placeholders::_1)) <<
		controller.actions.ui.text->Connect(
			std::bind(&TextEditScene::OnTextInput, this, std::placeholders::_1));

	SDL_StartTextInput();
}

void TextEditScene::DetachController(Control::InputEventController &controller,
	ConnList &conns)
{
	SDL_StopTextInput();

	SUPER::DetachController(controller, conns);
}

void TextEditScene::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);

	// Cursor visibility is based on the last character typed
	// (so that the cursor stays visible while typing).
	cursorOn = (OS::TimeDiff(tick, cursorTick) % 1000) < 500;
}

void TextEditScene::PrepareRender()
{
	inputLbl->SetCaretPos(inputLbl->GetText().length());
	inputLbl->SetCaretVisible(cursorOn);

	SUPER::PrepareRender();
}

}  // namespace Client
}  // namespace HoverRace
