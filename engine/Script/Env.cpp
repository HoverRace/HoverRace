
// Env.cpp
//
// Copyright (c) 2010, 2014, 2016 Michael Imamura.
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
 * @param scripting The scripting engine.
 */
Env::Env(Core &scripting) :
	scripting(scripting), initialized(false), envRef(&scripting),
	helpHandler(nullptr)
{
	lua_State *state = scripting.GetState();

	// Store a placeholder at the registry location.
	lua_pushinteger(state, 1);
	envRef.SetFromStack();
}

void Env::LogScriptError(const Script::ScriptExn &ex)
{
	HR_LOG(error) << ex.what();
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
	lua_State *state = scripting.GetState();

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
 * Push the environment onto the stack.
 * InitEnv() will be called if the initial environment hasn't been set.
 */
void Env::PushEnv()
{
	lua_State *L = scripting.GetState();

	if (initialized) {
		envRef.Push();  // env
	}
	else {
		lua_newtable(L);  // env
		InitEnv();  // env
		initialized = true;
		lua_pushvalue(L, -1);  // env env
		envRef.SetFromStack();  // env
	}
}

/**
 * Apply the environment to the function on the top of the stack.
 *
 * The environment will be initialized for the first time if needed.
 *
 * The function will remain on the stack afterwards.
 */
void Env::SetupEnv()
{
	lua_State *state = scripting.GetState();

	// fn - The compiled function.
	// env - The table that will be the environment for the function.

	// Initial stack: fn

	PushEnv();  // fn env

	// Apply the environment to the function.
	// Compiled Lua functions have one upvalue, which is the environment
	// it will execute in.
	lua_setupvalue(state, -2, 1);  // fn
}

/**
 * Load a chunk from a file.
 *
 * The chunk name will be based on the filename.
 *
 * @param filename The filename.
 * @return The loaded chunk, ready for compilation.
 * @throws Script::ScriptExn The script file was not found.
 */
Core::Chunk Env::LoadChunkFromFile(const Util::OS::path_t &filename)
{
	OS::path_t scriptPath = fs::system_complete(filename);

	if (!fs::exists(scriptPath)) {
		throw Script::ScriptExn(boost::str(
			boost::format("Script file not found: %s (interpreted as %s)") %
				(const char*)Str::PU(filename) %
				(const char*)Str::PU(scriptPath)));
	}

	std::string chunkName("@");
	chunkName += (const char*)Str::PU(scriptPath);

	// Read the whole script at once.
	fs::ifstream ifs(scriptPath, std::ios_base::in);
	return Core::Chunk(std::string{
		std::istreambuf_iterator<char>{ifs},
		std::istreambuf_iterator<char>{}}, chunkName);
}

}  // namespace Script
}  // namespace HoverRace
