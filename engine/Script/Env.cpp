
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

#include "Env.h"

using namespace HoverRace::Script;

Env::Env()
{
#	ifdef HAVE_LUA
		state = luaL_newstate();

		//TODO: Set panic handler.

#		ifdef _WIN32
			// Register a print function for debugging.
			lua_pushcfunction(state, &Env::LPrint);
			lua_setglobal(state, "print");
#		endif
#	endif
}

Env::~Env()
{
#	ifdef HAVE_LUA
		lua_close(state);
#	endif
}

void Env::Execute(const std::string &chunk)
{
#	ifdef HAVE_LUA
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
#	endif
}

/**
 * Pop the error message off the stack.
 * Only call this if you KNOW that there is an error on the top of the stack.
 * @return The error as a string.
 */
std::string Env::PopError()
{
#	ifdef HAVE_LUA
		size_t len;
		const char *s = lua_tolstring(state, -1, &len);
		std::string msg(s, len);
		lua_pop(state, 1);
		return msg;
#	else
		return "";
#	endif
}

#ifdef HAVE_LUA
int Env::LPrint(lua_State *state)
{
	int numParams = lua_gettop(state);

	std::string s;
	for (int i = 1; i <= numParams; ++i) {
		if (i > 1) s += '\t';
		
		const char *param = lua_tostring(state, i);
		if (param != NULL) s += param;
	}
	s += '\n';

	OutputDebugString(s.c_str());

	return 0;
}
#endif
