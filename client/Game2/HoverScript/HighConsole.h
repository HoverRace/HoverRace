
// HighConsole.h
// Header for the on-screen debug console.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <boost/thread/mutex.hpp>

#include "../../engine/Util/MR_Types.h"

#include "Console.h"

class MR_VideoBuffer;
namespace HoverRace {
	namespace Client {
		namespace Control {
			class InputHandler;
			typedef boost::shared_ptr<InputHandler> InputHandlerPtr;
		}
		namespace HoverScript {
			class GamePeer;
			class SessionPeer;
			typedef boost::shared_ptr<SessionPeer> SessionPeerPtr;
		}
		class GameApp;
	}
	namespace VideoServices {
		struct Font;
		class StaticText;
	}
}
class MR_2DViewPort;

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * An on-screen debug console.
 * @author Michael Imamura
 */
class HighConsole : public Console
{
	typedef Console SUPER;
	public:
		HighConsole(Script::Core *scripting, GameApp *gameApp,
			GamePeer *gamePeer, SessionPeerPtr sessionPeer);
		virtual ~HighConsole();

	protected:
		virtual void InitEnv();

	public:
		virtual void Advance(Util::OS::timestamp_t tick);

		virtual void Clear();

	private:
		class LogLines;
		void AddLogEntry(LogLines *lines, const std::string &s, MR_UInt8 color);
	protected:
		virtual void LogInfo(const std::string &s);
		virtual void LogError(const std::string &s);

	public:
		void ToggleVisible();
		bool IsVisible() const { return visible; }

	public:
		void OnChar(char c);
		void Render(MR_VideoBuffer *dest);

	private:
		void RenderConsole();
		void RenderHelp();
		void RenderHeading(const VideoServices::StaticText *title,
			const VideoServices::StaticText *controls, int y, MR_UInt8 bgColor,
			bool reversed=false);

	public:
		// HelpHandler.
		virtual void HelpClass(const Script::Help::Class &cls);
		virtual void HelpMethod(const Script::Help::Class &cls, const Script::Help::Method &method);

	private:
		GameApp *gameApp;
		GamePeer *gamePeer;
		SessionPeerPtr sessionPeer;

		bool visible;
		bool helpVisible;
		MR_2DViewPort *vp;

		std::string submitBuffer;
		std::string historyBuffer;
		boost::mutex submitBufferMutex;
		std::string commandLine;

		VideoServices::Font *logFont;
		VideoServices::StaticText *commandPrompt, *continuePrompt;
		VideoServices::StaticText *commandLineDisplay;

		VideoServices::StaticText *consoleTitle;
		VideoServices::StaticText *consoleControls;
		VideoServices::StaticText *helpTitle;
		VideoServices::StaticText *helpControls;

		unsigned int charWidth;  ///< Guessed fixed character width, in pixels.
		unsigned int consoleWidth;  ///< Console text width, in pixels.

		bool cursorOn;
		Util::OS::timestamp_t cursorTick;
		VideoServices::StaticText *cursor;

		LogLines *logLines;
		LogLines *helpLines;

		class Input;
		boost::shared_ptr<Input> input;

		static const int PADDING_TOP = 2;
		static const int PADDING_BOTTOM = 5;
		static const int PADDING_LEFT = 5;
		static const int PADDING_RIGHT = PADDING_LEFT;

		static const int TITLE_PADDING_TOP = 2;
		static const int TITLE_PADDING_BOTTOM = 2;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
