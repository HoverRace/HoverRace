
// Console.cpp
// Base class for debug consoles.
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

#include "StdAfx.h"

#include "../../engine/Script/Core.h"

#include "Console.h"

using namespace HoverRace;
using namespace HoverRace::Script;

namespace HoverRace {
namespace Client {

// Stream bits to redirect output to the console log.

class Console::LogStreamBuf : public std::stringbuf
{
	typedef std::stringbuf SUPER;
	public:
		LogStreamBuf(Console *console);
		virtual ~LogStreamBuf();

	protected:
		virtual int sync();

	private:
		Console *console;
};

class Console::LogStream : public std::ostream
{
	typedef std::ostream SUPER;
	public:
		LogStream(Console *console) :
			SUPER(new LogStreamBuf(console)) { }
		virtual ~LogStream() { delete rdbuf(); }
};

Console::Console(Script::Core *scripting) :
	SUPER(scripting), inputState(ISTATE_COMMAND)
{
	chunk.reserve(1024);
	outHandle = scripting->AddOutput(boost::make_shared<LogStream>(this));
}

Console::~Console()
{
	GetScripting()->RemoveOutput(outHandle);
}

void Console::InitEnv()
{
	lua_State *state = GetScripting()->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	lua_pushlightuserdata(state, this);  // table this
	lua_pushcclosure(state, Console::LClear, 1);  // table fn
	lua_pushstring(state, "clear");  // table fn str
	lua_insert(state, -2);  // table str fn
	lua_rawset(state, -3);  // table

	/*
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, Console::LReset, 1);
	lua_setglobal(state, "reset");

	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, Console::LSandbox, 1);
	lua_setglobal(state, "sandbox");
	*/
}

/**
 * Submit a chunk to the script engine.
 * @param s The chunk.
 */
void Console::SubmitChunk(const std::string &s)
{
	chunk += s;

	try {
		Execute(chunk);
	}
	catch (const IncompleteExn&) {
		SetInputState(ISTATE_CONTINUE);
		return;
	}
	catch (const ScriptExn &exn) {
		LogError(exn.GetMessage());
	}

	chunk.clear();
	SetInputState(ISTATE_COMMAND);
}

void Console::SetInputState(inputState_t newState)
{
	inputState = newState;
}

Console::inputState_t Console::GetInputState() const
{
	return inputState;
}

// Lua bindings.

int Console::LClear(lua_State *state)
{
	// function clear()
	// Clear the console.
	Console *self = static_cast<Console*>(lua_touserdata(state, lua_upvalueindex(1)));
	self->Clear();
	return 0;
}

/*
int Console::LReset(lua_State *state)
{
	// function reset()
	// Repopulate the global environment (standard functions, variables, etc.).
	// This is useful if global functions are accidentally overwritten or deleted.
	Console *self = static_cast<Console*>(lua_touserdata(state, lua_upvalueindex(1)));
	self->scripting->Reset();
	self->InitGlobals(self->scripting);
	self->LogInfo("Console script environment reset complete.");
	return 0;
}

int Console::LSandbox(lua_State *state)
{
	// function sandbox()
	// Activate the security sandbox.
	// In the context of the console, this is mainly just for testing purposes.
	// This protects critical tables from metatable modification and
	// deactivates functions that could break out of the sandbox.
	Console *self = static_cast<Console*>(lua_touserdata(state, lua_upvalueindex(1)));
	self->scripting->ActivateSandbox();
	self->LogInfo("Security sandbox activated.");
	self->LogInfo("Call reset() to deactivate the sandbox.");
	return 0;
}
*/

// Console::LogStreamBuf

Console::LogStreamBuf::LogStreamBuf(Console *console) :
	SUPER(), console(console)
{
}

Console::LogStreamBuf::~LogStreamBuf()
{
	sync();
}

int Console::LogStreamBuf::sync()
{
	std::string s = str();

	// Write each line to the log.
	std::string line;
	line.reserve(1024);
	for (std::string::iterator iter = s.begin();
		iter != s.end(); ++iter)
	{
		if (*iter == '\n') {
			console->LogInfo(line);
			line.clear();
		}
		else {
			line += *iter;
		}
	}
	if (!line.empty()) console->LogInfo(line);

	// Reset the string buffer.
	str(std::string());

	return 0;
}

}  // namespace Client
}  // namespace HoverRace
