
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

#include "../../../engine/Script/Help/HelpHandler.h"
#include "../../../engine/Script/Help/Class.h"
#include "../../../engine/Util/Config.h"
#include "GamePeer.h"

#include "SysConsole.h"

using HoverRace::Util::Config;

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Constructor.
 * @param scripting The underlying scripting engine.
 * @param gamePeer Game scripting peer.
 * @param maxLogLines The maximum number of log lines to store in the
 *                    log history.
 */
SysConsole::SysConsole(Script::Core *scripting, GamePeer *gamePeer,
                       int maxLogLines) :
	SUPER(scripting),
	gamePeer(gamePeer),
	maxLogLines(maxLogLines), logLines(), baseLogIdx(0)
{
	Config *cfg = Config::GetInstance();

	commandLine.reserve(1024);

	// Add introductory text.

	LogInfo(PACKAGE_NAME " version " + cfg->GetVersion());

	Script::Core *env = GetScripting();
	std::string intro = env->GetVersionString();
	intro += " :: Console active.";
	LogInfo(intro);

	LogInfo("Available global objects: game");

	std::string helpInstructions = boost::str(boost::format(
		_("For help on a class or method, call the %s method: %s")) %
		"help()" % "game:help(); game:help(\"on_init\")");
	LogInfo(helpInstructions);
}

SysConsole::~SysConsole()
{
}

void SysConsole::InitEnv()
{
	using namespace luabind;

	SUPER::InitEnv();

	lua_State *L = GetScripting()->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	object env(from_stack(L, -1));
	env["game"] = gamePeer;
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
	using Script::Help::Class;
	using Script::Help::MethodPtr;

	LogInfo(boost::str(boost::format(_("Methods for class %s:")) % cls.GetName()));

	std::string s;
	s.reserve(1024);
	BOOST_FOREACH(const Class::methods_t::value_type &ent, cls.GetMethods()) {
		MethodPtr method = ent.second;
		s.clear();
		s += "  ";
		s += method->GetName();
		
		const std::string &brief = method->GetBrief();
		if (!brief.empty()) {
			s += " - ";
			s += brief;
		}

		LogInfo(s);
	}
}

void SysConsole::HelpMethod(const Script::Help::Class &cls,
                            const Script::Help::Method &method)
{
	using Script::Help::Method;
	
	LogInfo("---");

	BOOST_FOREACH(const std::string &s, method.GetSigs()) {
		LogInfo(s);
	}

	LogInfo(method.GetBrief());
	const std::string &desc = method.GetDesc();
	if (!desc.empty()) {
		LogInfo(method.GetDesc());
	}
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

