
// Core.cpp
// Scripting support.
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

#include <iostream>

#include <boost/foreach.hpp>

#include "../Util/OS.h"

#include "Core.h"

using namespace HoverRace::Script;
using HoverRace::Util::OS;

#define REG_LUA_LIB(st, name, fn) \
	lua_pushcfunction((st), (fn)); \
	lua_pushstring((st), (name)); \
	lua_call((st), 1, 0)

#define UNDEF_LUA_GLOBAL(state, name) \
	lua_pushnil(state); \
	lua_setglobal((state), (name))

#define DISALLOW_LUA_GLOBAL(state, name) \
	lua_pushstring((state), (name)); \
	lua_pushcclosure((state), Core::LSandboxedFunction, 1); \
	lua_setglobal((state), (name))

Core::Core()
{
	state = luaL_newstate();

	//TODO: Set panic handler.

	Reset();
}

Core::~Core()
{
	lua_close(state);
}

/**
 * Reset changes to the global environment.
 * This is used for fixing accidental changes to globals.
 * Note that this will effectively deactivate the security sandbox.
 * Call ActivateSandbox() to reactivate if necessary.
 * The state returned by GetState() is otherwise unchanged.
 */
void Core::Reset()
{
	// Register a "safe" set of standard libraries.
	REG_LUA_LIB(state, "", luaopen_base);
	REG_LUA_LIB(state, LUA_MATHLIBNAME, luaopen_math);
	REG_LUA_LIB(state, LUA_STRLIBNAME, luaopen_string);
	REG_LUA_LIB(state, LUA_TABLIBNAME, luaopen_table);

	// Override the print function so we can reroute the output.
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, Core::LPrint, 1);
	lua_setglobal(state, "print");

	// Remove the metatable protection from the global table.
	// We don't need to do the same to the string metatable, since
	// luaopen_string handles that for us.
	lua_pushvalue(state, LUA_GLOBALSINDEX);
	lua_pushnil(state);
	lua_setmetatable(state, 1);
	lua_pop(state, 1);
}

/**
 * Activate the security sandbox.
 * This guards against methods a script may use to access the filesystem
 * or break out of the sandbox.
 */
void Core::ActivateSandbox()
{
	if (!lua_checkstack(state, 2))
		throw ScriptExn("Out of stack space.");

	// Create a reusable metatable protector.
	// This prevents modification of the metatable.
	lua_newtable(state);
	lua_pushboolean(state, 1);
	lua_setfield(state, -2, "__metatable");
	int metatableProtector = luaL_ref(state, LUA_REGISTRYINDEX);

	// Protect the metatable for the global table.
	// We do this instead of just destroying/replacing the "_G" global so that
	// users can still query the table for a list of keys, etc.
	lua_pushvalue(state, LUA_GLOBALSINDEX);
	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableProtector);
	lua_setmetatable(state, -2);
	lua_pop(state, 1);

	// Protect the shared metatable for strings.
	lua_pushstring(state, "");
	lua_getmetatable(state, -1);
	lua_pushboolean(state, 1);
	lua_setfield(state, -2, "__metatable");
	lua_pop(state, 2);

	// Clean up the registry.
	luaL_unref(state, LUA_REGISTRYINDEX, metatableProtector);

	// Mostly based on the list at: http://lua-users.org/wiki/SandBoxes
	DISALLOW_LUA_GLOBAL(state, "collectgarbage");
	DISALLOW_LUA_GLOBAL(state, "dofile");
	DISALLOW_LUA_GLOBAL(state, "getfenv");
	DISALLOW_LUA_GLOBAL(state, "load");
	DISALLOW_LUA_GLOBAL(state, "loadfile");
	DISALLOW_LUA_GLOBAL(state, "loadstring");
	DISALLOW_LUA_GLOBAL(state, "module");
	DISALLOW_LUA_GLOBAL(state, "rawequal");
	DISALLOW_LUA_GLOBAL(state, "rawget");
	DISALLOW_LUA_GLOBAL(state, "rawset");
	DISALLOW_LUA_GLOBAL(state, "require");
	DISALLOW_LUA_GLOBAL(state, "setfenv");
}

/**
 * Redirect output to a stream.
 * @param out The output stream (wrapped in a shared pointer).
 *            May be @c NULL to use the system default.
 * @return A handle for removing the stream later.
 */
Core::OutHandle Core::AddOutput(boost::shared_ptr<std::ostream> out)
{
	outs.push_back(out);
	return --(outs.end());
}

void Core::RemoveOutput(const OutHandle &handle)
{
	outs.erase(handle);
}

/**
 * Retrieve the full scripting version string (name and version).
 * @return The string (never empty).
 */
std::string Core::GetVersionString() const
{
	return LUA_VERSION;
}

/**
 * Execute a chunk of code.
 * @param chunk The code to execute.
 * @throw IncompleteExn If the code does not complete a statement; i.e.,
 *                      expecting more tokens.  Callers can catch this
 *                      to keep reading more data to finish the statement.
 * @throw ScriptExn The code either failed to compile or signaled an error
 *                  while executing.
 */
void Core::Execute(const std::string &chunk)
{
	// Compile the chunk.
	int status = luaL_loadbuffer(state, chunk.c_str(), chunk.length(), "=lua");
	if (status != 0) {
		std::string msg = PopError();
		if (msg.find("<eof>") != std::string::npos) {
			// Incomplete chunk.
			// Callers can use this to keep reading more data.
			throw IncompleteExn(msg);
		}
		else {
			// Other compilation error.
			throw ScriptExn(msg);
		}
	}

	// Execute the chunk.
	status = lua_pcall(state, 0, LUA_MULTRET, 0);
	if (status != 0) {
		throw ScriptExn(PopError());
	}

	// If there were any return values, pass them to print().
	if (lua_gettop(state) > 0) {
		lua_getglobal(state, "print");
		lua_insert(state, 1);
		status = lua_pcall(state, lua_gettop(state) - 1, 0, 0);
		if (status != 0) {
			throw ScriptExn(PopError());
		}
	}

	// Ensure that the stack is now empty for the next run.
	lua_settop(state, 0);
}

/**
 * Pop the error message off the stack.
 * Only call this if you KNOW that there is an error on the top of the stack.
 * @return The error as a string.
 */
std::string Core::PopError()
{
	size_t len;
	const char *s = lua_tolstring(state, -1, &len);
	std::string msg(s, len);
	lua_pop(state, 1);
	return msg;
}

int Core::LPrint(lua_State *state)
{
	Core *self = static_cast<Core*>(lua_touserdata(state, lua_upvalueindex(1)));

	int numParams = lua_gettop(state);

	// Note: This aims to be pretty close to the print() function from the Lua
	//       base lib, including allowing the global tostring() to be
	//       replaced (e.g. to support additional types).

	lua_getglobal(state, "tostring");

	for (int i = 1; i <= numParams; ++i) {
		// Call the global "tostring" function 
		lua_pushvalue(state, -1);
		lua_pushvalue(state, i);
		lua_call(state, 1, 1);

		// Get the result.
		const char *s = lua_tostring(state, -1);
		if (s == NULL) {
			// tostring() returned a non-string result.
			// This can happen if the tostring function is replaced.
			ASSERT(false);
			lua_pop(state, 1);
			continue;
		}
		BOOST_FOREACH(boost::shared_ptr<std::ostream> &oss, self->outs) {
			if (i > 1) *oss << '\t';
			*oss << s;
		}
		lua_pop(state, 1);
	}
	BOOST_FOREACH(boost::shared_ptr<std::ostream> &oss, self->outs) {
		*oss << std::endl;
	}

	return 0;
}

int Core::LSandboxedFunction(lua_State *state)
{
	const char *name = lua_tostring(state, lua_upvalueindex(1));
	return luaL_error(state, "Disallowed access to protected function: %s", name);
}
