
// Env.cpp
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

#include "Env.h"

using namespace HoverRace::Script;

#define REG_LUA_LIB(st, name, fn) \
	lua_pushcfunction((st), (fn)); \
	lua_pushstring((st), (name)); \
	lua_call((st), 1, 0)

Env::Env()
{
	state = luaL_newstate();

	//TODO: Set panic handler.

	// Register a "safe" set of standard libraries.
	REG_LUA_LIB(state, "", luaopen_base);
	REG_LUA_LIB(state, LUA_MATHLIBNAME, luaopen_math);
	REG_LUA_LIB(state, LUA_STRLIBNAME, luaopen_string);
	REG_LUA_LIB(state, LUA_TABLIBNAME, luaopen_table);

	// Override the print function so we can reroute the output.
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, Env::LPrint, 1);
	lua_setglobal(state, "print");
}

Env::~Env()
{
	lua_close(state);
}

/**
 * Redirect output to a stream.
 * @param out The output stream (wrapped in a shared pointer).
 *            May be @c NULL to use the system default.
 */
void Env::setOutput(boost::shared_ptr<std::ostream> out)
{
	this->out = out;
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
void Env::Execute(const std::string &chunk)
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
std::string Env::PopError()
{
	size_t len;
	const char *s = lua_tolstring(state, -1, &len);
	std::string msg(s, len);
	lua_pop(state, 1);
	return msg;
}

int Env::LPrint(lua_State *state)
{
	Env *self = static_cast<Env*>(lua_touserdata(state, lua_upvalueindex(1)));
	std::ostream &oss = (self->out == NULL) ? std::cout : *(self->out);

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
		if (i > 1) oss << '\t';
		oss << s;
		lua_pop(state, 1);
	}
	oss << std::endl;

	return 0;
}
