
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

	lua_pushnil(L);  // table nil
	while (lua_next(L, -2) != 0) {
		// table key fn
		try {
			scripting->CallAndPrint();  // table key
		}
		catch (Script::ScriptExn &ex) {
			scripting->Print(ex.what());
		}
	}
	// table
	lua_pop(L, 1);
}

/**
 * Add an unnamed event handler.
 * @param fn The function to register.
 */
void Handlers::AddHandler(const luabind::object &fn)
{
	// function on_init(f)
	// Register a callback function for when the game starts up.
	using namespace luabind;

	lua_State *L = scripting->GetState();

	//TODO: Support named callbacks.
	if (type(fn) != LUA_TFUNCTION) {
		luaL_error(L, "Expected function as first parameter.");
		return;
	}

	fn.push(L);  // fn

	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);  // fn table

	lua_pushinteger(L, seq++); // fn table key
	lua_pushvalue(L, -3); // fn table key fn
	lua_settable(L, -3); // fn table

	lua_pop(L, 2);
}

}  // namespace Script
}  // namespace HoverRace
