
// SysConsole.cpp
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

#include "../../../engine/Util/Config.h"

#include "SysConsole.h"

using HoverRace::Util::Config;

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Constructor.
 * @param scripting The underlying scripting engine.
 * @param maxLogLines The maximum number of log lines to store in the
 *                    log history.
 */
SysConsole::SysConsole(Script::Core *scripting, int maxLogLines) :
	SUPER(scripting),
	maxLogLines(maxLogLines), logLines(), baseLogIdx(0)
{
	Config *cfg = Config::GetInstance();

	// Add introductory text.

	LogInfo(PACKAGE_NAME " version " + cfg->GetVersion());

	Script::Core *env = GetScripting();
	std::string intro = env->GetVersionString();
	intro += " :: Console active.";
	LogInfo(intro);
}

SysConsole::~SysConsole()
{
}

void SysConsole::Clear()
{
	// Bump the base log index so that AddLogLine() will generate the correct
	// next log index.
	baseLogIdx += logLines.size();

	logLines.clear();

	// Notify listeners.
	logClearedSignal();
}

void SysConsole::AddLogLine(LogLevel::level_t level, const std::string &line)
{
	int sz = logLines.size();
	int idx = baseLogIdx + sz;

	// Trim excess old log lines.
	while (sz >= maxLogLines) {
		logLines.pop_front();
		sz--;
		baseLogIdx++;
	}

	logLines.emplace_back(LogLine(idx, level, line));

	// Notify listeners.
	logAddedSignal(idx);
}

void SysConsole::LogHistory(const std::string &s)
{
	AddLogLine(LogLevel::HISTORY, s);
}

void SysConsole::LogInfo(const std::string &s)
{
	AddLogLine(LogLevel::INFO, s);
}

void SysConsole::LogError(const std::string &s)
{
	AddLogLine(LogLevel::ERROR, s);
}

/**
 * Retrieve the index of the last log entry.
 * @return The index or -1 if the log has been cleared.
 */
int SysConsole::GetEndLogIndex() const
{
	return logLines.empty() ? -1 : logLines.back().idx;
}

void SysConsole::HelpClass(const Script::Help::Class &cls)
{
	//TODO
}

void SysConsole::HelpMethod(const Script::Help::Class &cls,
                            const Script::Help::Method &method)
{
	//TODO
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

