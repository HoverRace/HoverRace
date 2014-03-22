
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

#include "../../../engine/Control/Action.h"
#include "../UiScene.h"
#include "SysConsole.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
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
		void OnDisplayConfigChanged();

		void OnConsoleToggle();
		void OnConsoleUp();
		void OnConsoleDown();
		void OnConsoleTop();
		void OnConsoleBottom();
		void OnConsolePrevCmd();
		void OnConsoleNextCmd();
		void OnTextInput(const std::string &s);
		void OnTextControl(Control::TextControl::key_t key);

		void OnLogCleared();
		void OnLogAdded(int idx);

		void AppendLogLine(const SysConsole::LogLine &line);
		void UpdateCommandLine();
		void Layout();

	public:
		// Scene
		virtual void AttachController(Control::InputEventController &controller);
		virtual void DetachController(Control::InputEventController &controller);
	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		GameDirector &director;
		SysConsole &console;

		boost::signals2::connection displayConfigChangedConn;

		boost::signals2::connection consoleToggleConn;
		boost::signals2::connection consoleUpConn;
		boost::signals2::connection consoleDownConn;
		boost::signals2::connection consoleTopConn;
		boost::signals2::connection consoleBottomConn;
		boost::signals2::connection consolePrevCmdConn;
		boost::signals2::connection consoleNextCmdConn;
		boost::signals2::connection textInputConn;
		boost::signals2::connection textControlConn;

		boost::signals2::connection logClearedConn;
		boost::signals2::connection logAddedConn;

		class LogLines;
		LogLines *logLines;
		int lastLogIdx;
		bool logsChanged;

		bool layoutChanged;

		Display::FillBox *winShadeBox;
		Display::Label *inputLbl;

		bool cursorOn;
		Util::OS::timestamp_t cursorTick;
		Display::Label *cursorLbl;

		Display::Label *measureLbl;  ///< Used to measure the size of glyphs.
		Vec2 charSize;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
