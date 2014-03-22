
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

#include <boost/circular_buffer.hpp>

#include "../../../engine/Util/Log.h"

#include "Console.h"

// Previously defined in WinGDI.h.
#ifdef ERROR
#	undef ERROR
#endif

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class DebugPeer;
			class GamePeer;
			class SessionPeer;
		}
		class ClientSession;
		class GameDirector;
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
		SysConsole(Script::Core *scripting, GameDirector &director,
			DebugPeer *debugPeer, GamePeer *gamePeer, int maxLogLines=512,
			int maxHistory=64);
		virtual ~SysConsole();

	protected:
		virtual void InitEnv();

	public:
		virtual void Advance(Util::OS::timestamp_t tick) { }

	private:
		void OnSessionChanged(ClientSession *session);

	public:
		virtual void Clear();
		void SubmitChunkWithHistory(const std::string &s);

	public:
		struct LogLevel
		{
			enum level_t { HISTORY, NOTE, INFO, ERROR };
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
		void OnLog(const Util::Log::Entry &entry);
		void AddLogLine(LogLevel::level_t level, const std::string &line);
	public:
		void AddIntroLines();
		void LogHistory(const std::string &s);
		void LogNote(const std::string &s);
		virtual void LogInfo(const std::string &s);
		virtual void LogError(const std::string &s);

	public:
		int GetEndLogIndex() const;

		/**
		 * Read all log entries.
		 * @param fn The callback function (will be passed a LogLine reference).
		 */
		template<class Function>
		void ReadLogs(Function fn)
		{
			if (!logLines.empty()) {
				std::for_each(logLines.cbegin(), logLines.cend(), fn);
			}
		}

		/**
		 * Read a subset of log entries, from a starting index to the end.
		 * @param start The starting log index (inclusive).
		 * @param fn The callback function (will be passed a LogLine reference).
		 */
		template<class Function>
		void ReadLogs(int start, Function fn)
		{
			if (!logLines.empty()) {
				if (start < baseLogIdx) {
					ReadLogs(fn);
				}
				else {
					ReadLogs(start, GetEndLogIndex(), fn);
				}
			}
		}

		/**
		 * Read a subset of log entries, for a specific range.
		 * @param start The starting log index (inclusive).
		 * @param end The ending log index (inclusive).
		 * @param fn The callback function (will be passed a LogLine reference).
		 */
		template<class Function>
		void ReadLogs(int start, int end, Function fn)
		{
			if (logLines.empty()) return;
			if (start > end) return;
			int endIdx = baseLogIdx + logLines.size() - 1;
			if (start > endIdx) return;
			if (start == end) fn(logLines.at(start - baseLogIdx));
			else {
				if (start < baseLogIdx) start = baseLogIdx;
				auto startIter = logLines.begin() + (start - baseLogIdx);
				auto endIter = (end > endIdx) ? logLines.end() : startIter + (end - start + 1);
				std::for_each(startIter, endIter, fn);
			}
		}

		std::string &GetCommandLine() { return commandLine; }

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
		DebugPeer *debugPeer;
		GamePeer *gamePeer;
		std::shared_ptr<SessionPeer> sessionPeer;

		boost::signals2::scoped_connection sessionChangedConn;

		bool introWritten;
		int maxLogLines;
		std::deque<LogLine> logLines;
		int baseLogIdx;  ///< Index of the first item in logLines.

		boost::circular_buffer<std::string> history;

		std::string commandLine;

		boost::signals2::scoped_connection logConn;

		logClearedSignal_t logClearedSignal;
		logAddedSignal_t logAddedSignal;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

