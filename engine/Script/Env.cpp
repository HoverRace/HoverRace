
// Env.cpp
// An environment in the scripting engine.
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

#include "Core.h"

#include "Env.h"

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

	lua_pushnil(state);  // table nil
	while (lua_next(state, LUA_GLOBALSINDEX) != 0) {
		// table key value
		lua_pushvalue(state, -2);  // table key value key
		lua_insert(state, -2);  // table key key value
		lua_rawset(state, -4);  // table key
	}
	// table
}

/**
 * Execute a chunk of code in the current environment.
 * @param chunk The code to execute.
 * @throw IncompleteExn If the code does not complete a statement; i.e.,
 *                      expecting more tokens.  Callers can catch this
 *                      to keep reading more data to finish the statement.
 * @throw ScriptExn The code either failed to compile or signaled an error
 *                  while executing.
 */
void Env::Execute(const std::string &chunk)
{
	lua_State *state = scripting->GetState();

	// May throw ScriptExn or IncompleteExn, in which case stack will be unchanged.
	scripting->Compile(chunk);

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
	lua_setfenv(state, -2);

	// May throw ScriptExn, but the function on the stack will be consumed anyway.
	scripting->CallAndPrint();
}

}  // namespace Script
}  // namespace HoverRace
