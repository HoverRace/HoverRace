
// GamePeer.cpp
// Scripting peer for system-level control of the game.
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

#include "../../engine/Script/Core.h"

#include "GamePeer.h"

namespace HoverRace {
namespace Client {

GamePeer::GamePeer(Script::Core *scripting) :
	scripting(scripting)
{
	lua_State *L = scripting->GetState();

	// Initial table for on_init callbacks.
	lua_newtable(L);
	onInitRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

GamePeer::~GamePeer()
{
	lua_State *L = scripting->GetState();
	luaL_unref(L, LUA_REGISTRYINDEX, onInitRef);
}

/**
 * Register this peer in an environment.
 */
void GamePeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	open(L);

	module(L) [
		class_<GamePeer>("Game")
			.def("on_init", &LOnInit)
	];
}

/**
 * Executes all "on_init" callbacks.
 */
void GamePeer::OnInit()
{
	lua_State *L = scripting->GetState();

	lua_rawgeti(L, LUA_REGISTRYINDEX, onInitRef);  // table

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

void GamePeer::LOnInit(const luabind::object &fn)
{
	// function on_init(f)
	// Register a callback function for when the game starts up.
	using namespace luabind;

	//TODO: Support named callbacks.
	if (type(fn) != LUA_TFUNCTION) {
		//FIXME: Uh...
		ASSERT(FALSE);
	}

	lua_State *L = scripting->GetState();

	fn.push(L);  // fn

	lua_rawgeti(L, LUA_REGISTRYINDEX, onInitRef);  // fn table

	lua_pushinteger(L, lua_objlen(L, -1) + 1); // fn table key
	lua_pushvalue(L, -3); // fn table key fn
	lua_settable(L, -3); // fn table

	lua_pop(L, 2);
}

/*TODO
int GamePeer::LGetOnInit(lua_State *state)
{
	// function get_on_init()
	// Returns the table of on_init callbacks (for debugging purposes).
	SysConsole *self = static_cast<SysConsole*>(lua_touserdata(state, lua_upvalueindex(1)));

	lua_rawgeti(state, LUA_REGISTRYINDEX, self->onInitRef);  // table

	return 1;
}
*/

}  // namespace Client
}  // namespace HoverRace
