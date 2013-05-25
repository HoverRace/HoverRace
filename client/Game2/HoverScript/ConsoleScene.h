
// ConsoleScene.h
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

#include "../UiScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
		namespace HoverScript {
			class SysConsole;
		}
	}
	namespace Display {
		class Display;
		class FillBox;
		class Label;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * UI for the system console.
 * @author Michael Imamura
 */
class ConsoleScene : public UiScene
{
	typedef UiScene SUPER;
	public:
		ConsoleScene(Display::Display &display, GameDirector &director,
			SysConsole &console);
		virtual ~ConsoleScene();

	private:
		void OnConsoleToggle();
		void OnTextInput(const std::string &s);
		void OnTextControl(Control::TextControl::key_t key);

		void UpdateCommandLine();

	public:
		// Scene
		virtual void AttachController(Control::InputEventController &controller);
		virtual void DetachController(Control::InputEventController &controller);
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		Display::Display &display;
		GameDirector &director;
		SysConsole &console;

		boost::signals2::connection consoleToggleConn;
		boost::signals2::connection textInputConn;
		boost::signals2::connection textControlConn;

		std::string submitBuffer;
		std::string historyBuffer;
		std::string commandLine;

		Display::FillBox *winShadeBox;
		Display::Label *inputLbl;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
