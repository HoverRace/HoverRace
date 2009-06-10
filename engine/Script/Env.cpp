
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

Env::Env()
{
	state = luaL_newstate();

	//TODO: Set panic handler.

	//TODO: Load base library.

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
			throw IncompleteExn();
		}
		else {
			// Other compilation error.
			throw ScriptExn(msg);
		}
	}

	// Execute the chunk.
	status = lua_pcall(state, 0, /*LUA_MULTRET*/0, 0);
	if (status != 0) {
		throw ScriptExn(PopError());
	}
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
	for (int i = 1; i <= numParams; ++i) {
		if (i > 1) oss << '\t';
		
		const char *param = lua_tostring(state, i);
		if (param != NULL) oss << param;
	}
	oss << std::endl;

	return 0;
}
