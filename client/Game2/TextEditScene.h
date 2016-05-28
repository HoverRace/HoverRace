
// TextEditScene.h
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

#pragma once

#include "../../../engine/Control/Action.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class ActiveText;
		class Display;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Edit a line of text.
 * @author Michael Imamura
 */
class TextEditScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	TextEditScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle, const std::string &title,
		const std::string &text = {});
	virtual ~TextEditScene() { }

public:
	const std::string &GetText() const { return text; }

public:
	using confirmSignal_t = boost::signals2::signal<void(const std::string&)>;
	confirmSignal_t &GetConfirmSignal() { return confirmSignal; }

private:
	void OnTextInput(const std::string &s);
	void OnTextControl(Control::TextControl::key_t key);

public:
	// Scene
	void AttachController(Control::InputEventController &controller,
		ConnList &conns) override;
	void DetachController(Control::InputEventController &controller,
		ConnList &conns) override;

public:
	void Advance(Util::OS::timestamp_t tick) override;
	void PrepareRender() override;

private:
	std::string text;

	bool cursorOn;
	Util::OS::timestamp_t cursorTick;

	confirmSignal_t confirmSignal;

	std::shared_ptr<Display::ActiveText> inputLbl;
};

}  // namespace Client
}  // namespace HoverRace
