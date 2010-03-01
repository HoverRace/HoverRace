
// SysConsole.cpp
// The global system console.
//
// Copyright (c) 2010 Michael Imamura.
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

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

#include "../../engine/Script/Env.h"

#include "SysConsole.h"

namespace fs = boost::filesystem;

using namespace HoverRace;
using namespace HoverRace::Client;
using namespace HoverRace::Script;

namespace {
	const char ON_INIT_REG_KEY = 'A';
}

SysConsole::SysConsole() :
	SUPER()
{
}

SysConsole::~SysConsole()
{
}

void SysConsole::InitGlobals(Script::Env *scripting)
{
	SUPER::InitGlobals(scripting);

	lua_State *state = scripting->GetState();

	lua_pushcclosure(state, SysConsole::LOnInit, 0);
	lua_setglobal(state, "on_init");

	lua_pushcclosure(state, SysConsole::LGetOnInit, 0);
	lua_setglobal(state, "get_on_init");

	// Initial arrays for callbacks.
	lua_pushlightuserdata(state, (void*)&ON_INIT_REG_KEY);
	lua_newtable(state);
	lua_settable(state, LUA_REGISTRYINDEX);
}

void SysConsole::LogInfo(const std::string &s)
{
#	ifdef _WIN32
		OutputDebugString(s.c_str());
		OutputDebugString("\n");
#	endif
	std::cout << s << std::endl;
}

void SysConsole::LogError(const std::string &s)
{
#	ifdef _WIN32
		OutputDebugString(s.c_str());
		OutputDebugString("\n");
#	endif
	std::cerr << s << std::endl;
}

/**
 * Execute a script from a file.
 * @param filename The script filename (must be an absolute path).
 */
void SysConsole::RunScript(const std::string &filename)
{
	fs::path scriptPath = fs::system_complete(fs::path(filename));

	if (!fs::exists(scriptPath)) {
		LogError(boost::str(
			boost::format("Init script file not found: %s (interpreted as %s)") %
			filename % scriptPath));
		return;
	}
	
	// Read and submit the whole script at once.
	fs::ifstream ifs(scriptPath, std::ios_base::in);
	std::string ris((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	SubmitChunk(ris);
}

/**
 * Executes all "on_init" callbacks.
 */
void SysConsole::OnInit()
{
	lua_State *state = GetScripting()->GetState();

	lua_pushlightuserdata(state, (void*)&ON_INIT_REG_KEY);
	lua_gettable(state, LUA_REGISTRYINDEX);  // table

	size_t sz = lua_objlen(state, -1);

	for (size_t i = 0; i < sz; ++i) {
		lua_pushinteger(state, i + 1); // table key
		lua_gettable(state, -2); // table fn
		lua_call(state, 0, 0); // table
	}
}

int SysConsole::LOnInit(lua_State *state)
{
	// function on_init(f)
	// Register a callback function for when the game starts up.

	if (!lua_isfunction(state, 1)) {
		lua_pushstring(state, "on_init requires a function.");
		return lua_error(state);
	}

	lua_pushlightuserdata(state, (void*)&ON_INIT_REG_KEY);
	lua_gettable(state, LUA_REGISTRYINDEX);  // table

	lua_pushinteger(state, lua_objlen(state, -1) + 1); // table key
	lua_pushvalue(state, 1); // table key fn
	lua_settable(state, -3); // table

	lua_pop(state, 1);

	return 0;
}

int SysConsole::LGetOnInit(lua_State *state)
{
	// function get_on_init()
	// Returns the table of on_init callbacks (for debugging purposes).

	lua_pushlightuserdata(state, (void*)&ON_INIT_REG_KEY);
	lua_gettable(state, LUA_REGISTRYINDEX);  // table

	return 1;
}
