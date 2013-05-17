
// SysConsole.h
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

#include "Console.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class GamePeer;
			class SessionPeer;
			typedef std::shared_ptr<SessionPeer> SessionPeerPtr;
		}
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Global always-on console.
 *
 * This is the backend for the ConsoleScene, allowing it to maintain state
 * between instances.
 *
 * @author Michael Imamura
 */
class SysConsole : public Console
{
	typedef Console SUPER;
	public:
		SysConsole(Script::Core *scripting, int maxLogLines=512);
		virtual ~SysConsole();

	public:
		virtual void Advance(Util::OS::timestamp_t tick) { }
		virtual void Clear();

	public:
		struct LogLevel
		{
			enum level_t { INFO, ERROR };
		};
		struct LogLine
		{
			LogLine(int idx, LogLevel::level_t level, const std::string &line) :
				idx(idx), level(level), line(line) { }
			const int idx;
			const LogLevel::level_t level;
			const std::string line;
		};
	private:
		void AddLogLine(LogLevel::level_t level, const std::string &line);
	protected:
		virtual void LogInfo(const std::string &s);
		virtual void LogError(const std::string &s);

	public:
		/// Fired when the log is cleared.
		typedef boost::signals2::signal<void()> logClearedSignal_t;
		logClearedSignal_t &GetLogClearedSignal() { return logClearedSignal; }

		/// Fired when a log line is added.  Parameter is the log index.
		typedef boost::signals2::signal<void(int)> logAddedSignal_t;
		logAddedSignal_t &GetLogAddedSignal() { return logAddedSignal; }

	public:
		// HelpHandler.
		virtual void HelpClass(const Script::Help::Class &cls);
		virtual void HelpMethod(const Script::Help::Class &cls, const Script::Help::Method &method);

	private:
		int maxLogLines;
		std::deque<LogLine> logLines;
		int baseLogIdx;  ///< Index of the first item in logLines.

		logClearedSignal_t logClearedSignal;
		logAddedSignal_t logAddedSignal;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

