
// Handlers.cpp
// Tracks event handlers registered in scripts.
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

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "Core.h"

#include "Handlers.h"

namespace HoverRace {
namespace Script {

/**
 * Constructor.
 * @param scripting The scripting core (may not be @c NULL).
 */
Handlers::Handlers(Core *scripting) :
	scripting(scripting), seq(1)
{
	lua_State *L = scripting->GetState();
	lua_newtable(L);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
}

Handlers::~Handlers()
{
	lua_State *L = scripting->GetState();
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

/**
 * Call all registered event handlers.
 */
void Handlers::CallHandlers() const
{
	lua_State *L = scripting->GetState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);  // table

	int functionsToCall = 0;

	// First, gather the list of handler functions to call.
	// We do this so that the handlers can add/remove handlers.
	lua_pushnil(L);  // table nil
	while (lua_next(L, -2) != 0) {
		// (fns...) table key fn
		lua_insert(L, -3);  // (fns...fn) table key
		++functionsToCall;
	}
	// (fns...) table
	lua_pop(L, 1);  // (fns...)

	// Call the handlers, one by one.
	for (int i = 0; i < functionsToCall; ++i) {
		try {
			scripting->CallAndPrint();
		}
		catch (Script::ScriptExn &ex) {
			scripting->Print(ex.what());
		}
	}
}

/**
 * Add an unnamed event handler.
 * @param fn The function to register.
 */
void Handlers::AddHandler(const luabind::object &fn)
{
	using namespace luabind;

	lua_State *L = scripting->GetState();

	if (type(fn) != LUA_TFUNCTION) {
		luaL_error(L, "Expected: (function) or (string, function) or (string, nil)");
		return;
	}

	fn.push(L);  // fn

	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);  // fn table

	lua_pushinteger(L, seq++); // fn table key
	lua_pushvalue(L, -3); // fn table key fn
	lua_settable(L, -3); // fn table

	lua_pop(L, 2);
}

/**
 * Add a named event handler.
 * This will replace any previously-registered handler with the same name.
 * @param name The name to use.
 * @param fn The function to register.  May be @c nil to remove the handler.
 */
void Handlers::AddHandler(const std::string &name, const luabind::object &fn)
{
	using namespace luabind;

	lua_State *L = scripting->GetState();

	int paramType = type(fn);
	if (paramType != LUA_TNIL && paramType != LUA_TFUNCTION) {
		luaL_error(L, "Expected: (function) or (string, function) or (string, nil)");
		return;
	}

	fn.push(L);  // fn

	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);  // fn table

	lua_pushstring(L, name.c_str()); // fn table key
	lua_pushvalue(L, -3); // fn table key fn
	lua_settable(L, -3); // fn table

	lua_pop(L, 2);

}

}  // namespace Script
}  // namespace HoverRace
