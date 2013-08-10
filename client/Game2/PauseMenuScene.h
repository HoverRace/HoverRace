
// PauseMenuScene.h
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

#pragma once

#include "../../engine/Util/Config.h"

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class Button;
		class Display;
		class Label;
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Menu displayed when the game is paused.
 * @author Michael Imamura
 */
class PauseMenuScene : public FormScene
{
	typedef FormScene SUPER;
	public:
		PauseMenuScene(Display::Display &display, GameDirector &director);
		virtual ~PauseMenuScene();

	private:
		std::shared_ptr<Display::Button> AddButton(const std::string &text, double &y);

	private:
		void OnCancel();
		void OnQuitToMainMenu();
		void OnQuitToDesktop();

	public:
		// Scene
		virtual void AttachController(Control::InputEventController &controller);
		virtual void DetachController(Control::InputEventController &controller);
		virtual void PrepareRender();
		virtual void Render();

	private:
		Display::Display &display;
		GameDirector &director;

		std::unique_ptr<Display::ScreenFade> fader;

		boost::signals2::connection returnConn;
};

}  // namespace Client
}  // namespace HoverRace
