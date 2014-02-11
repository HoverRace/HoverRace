
// Env.cpp
//
// Copyright (c) 2010, 2014 Michael Imamura.
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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../Util/Log.h"
#include "../Util/Str.h"

#include "Core.h"

#include "Env.h"

using namespace HoverRace::Util;
namespace fs = boost::filesystem;

namespace HoverRace {
namespace Script {

/**
 * Constructor.
 * @param scripting The scripting engine (may not be @c NULL).
 */
Env::Env(Core *scripting) :
	scripting(scripting), initialized(false)
{
	lua_State *state = scripting->GetState();

	// Store a placeholder at the registry location.
	lua_pushinteger(state, 1);
	envRef = luaL_ref(state, LUA_REGISTRYINDEX);
}

Env::~Env()
{
	luaL_unref(scripting->GetState(), LUA_REGISTRYINDEX, envRef);
}

/**
 * Initialize the environment in which scripts will run in.
 * Upon entry, the Lua stack will have at least one entry, the table which
 * represents the environment.  Implementing functions will fill this table
 * with the globals which will be available to the functions which are run
 * in this environment.  Upon return, this same table must be at the top of
 * the stack.
 */
void Env::InitEnv()
{
}

/**
 * Copy the global environment into the current table at the top of the stack.
 * This is meant to be called from InitEnv() as a convenience.
 */
void Env::CopyGlobals()
{
	lua_State *state = scripting->GetState();

	lua_rawgeti(state, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); // table _G
	lua_pushnil(state);  // table _G nil
	while (lua_next(state, -2)) {
		// table _G key value
		lua_pushvalue(state, -2);  // table _G key value key
		lua_insert(state, -2);  // table _G key key value
		lua_rawset(state, -5);  // table _G key
	}
	// table _G
	lua_pop(state, 1);  // table
}

void Env::SetHelpHandler(Help::HelpHandler *handler)
{
	helpHandler = handler;
}

/**
 * Execute a chunk of code in the current environment.
 * @param chunk The code to execute.
 * @param name Optional name for the chunk.  This name is used in error messages.
 *             Prefix the name with @c "=" to use the name verbatim, without
 *             decoration, in error messages.
 * @throw IncompleteExn If the code does not complete a statement; i.e.,
 *                      expecting more tokens.  Callers can catch this
 *                      to keep reading more data to finish the statement.
 * @throw ScriptExn The code either failed to compile or signaled an error
 *                  while executing.
 */
void Env::Execute(const std::string &chunk, const std::string &name)
{
	lua_State *state = scripting->GetState();

	// May throw ScriptExn or IncompleteExn, in which case stack will be unchanged.
	scripting->Compile(chunk, name);

	if (initialized) {
		lua_rawgeti(state, LUA_REGISTRYINDEX, envRef);
	}
	else {
		lua_newtable(state);
		InitEnv();
		initialized = true;
		lua_pushvalue(state, -1);
		lua_rawseti(state, LUA_REGISTRYINDEX, envRef);
	}
	lua_setupvalue(state, -2, 1);

	// May throw ScriptExn, but the function on the stack will be consumed anyway.
	scripting->CallAndPrint(0, helpHandler);
}

/**
 * Execute a script from a file.
 *
 * If there is an error executing the script, then the error message will be
 * written to the error log and the function will return @c false.
 *
 * @param filename The script filename (must be an absolute path).
 * @return @c true if the script executed successfully,
 *         @c false if there was an error.
 */
bool Env::RunScript(const OS::path_t &filename)
{
	OS::path_t scriptPath = fs::system_complete(filename);

	if (!fs::exists(scriptPath)) {
		Log::Error("Script file not found: %s (interpreted as %s)",
			(const char*)Str::PU(filename),
			(const char*)Str::PU(scriptPath));
		return false;
	}

	std::string chunkName("@");
	chunkName += (const char*)Str::PU(scriptPath);

	// Read and submit the whole script at once.
	fs::ifstream ifs(scriptPath, std::ios_base::in);
	std::string ris((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	try {
		Execute(ris, chunkName);
	}
	catch (Script::ScriptExn &ex) {
		Log::Error("%s", ex.what());
		return false;
	}

	return true;
}

}  // namespace Script
}  // namespace HoverRace
