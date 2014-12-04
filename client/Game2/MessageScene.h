
// MessageScene.h
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

#pragma once

#include "../../engine/Util/Config.h"

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class ActionButton;
		class Display;
		class Label;
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Displays a message to the user.
 * @author Michael Imamura
 */
class MessageScene : public FormScene
{
	using SUPER = FormScene;

public:
	MessageScene(Display::Display &display, GameDirector &director,
		const std::string &title, const std::string &message,
		bool hasCancel = false);
	virtual ~MessageScene();

private:
	void OnOk();
	void OnCancel();

public:
	typedef boost::signals2::signal<void()> okSignal_t;
	okSignal_t &GetOkSignal() { return okSignal; }

	typedef boost::signals2::signal<void()> cancelSignal_t;
	cancelSignal_t &GetCancelSignal() { return cancelSignal; }

public:
	// Scene
	void AttachController(Control::InputEventController &controller) override;
	void DetachController(Control::InputEventController &controller) override;
	void OnPhaseTransition(double progress) override;
	void Layout() override;
	void PrepareRender() override;
	void Render() override;

private:
	GameDirector &director;
	bool hasCancel;

	okSignal_t okSignal;
	cancelSignal_t cancelSignal;

	std::unique_ptr<Display::ScreenFade> fader;
	std::shared_ptr<Display::Label> titleLbl;
	std::shared_ptr<Display::Label> messageLbl;
	std::shared_ptr<Display::ActionButton> okBtn;
	std::shared_ptr<Display::ActionButton> cancelBtn;

	boost::signals2::connection okConn;
	boost::signals2::connection cancelConn;

	static const int HORZ_PADDING = 40;
};

}  // namespace Client
}  // namespace HoverRace
