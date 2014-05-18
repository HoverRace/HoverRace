
// Peer.h
// Base class for script peers that extend Object.
//
// Copyright (c) 2009, 2010 Michael Imamura.
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

#include "RegistryRef.h"

namespace HoverRace {
namespace Script {

RegistryRef::RegistryRef(const RegistryRef &o) :
	scripting(o.scripting)
{
	o.Push();
	ref = luaL_ref(scripting->GetState(), LUA_REGISTRYINDEX);
}

RegistryRef &RegistryRef::operator=(const RegistryRef &o)
{
	scripting = o.scripting;

	o.Push();
	SetFromStack();

	return *this;
}

/**
 * Unset the value stored by this reference.
 */
void RegistryRef::Clear()
{
	// luaL_unref ignores invalid refs.
	luaL_unref(scripting->GetState(), LUA_REGISTRYINDEX, ref);
}

/**
 * Pop a value from the Lua stack and store it in the registry.
 */
void RegistryRef::SetFromStack()
{
	lua_State *L = scripting->GetState();

	if (ref == LUA_NOREF || ref == LUA_REFNIL) {
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else {
		lua_rawseti(L, LUA_REGISTRYINDEX, ref);
	}
}

/**
 * Set the value.
 * @param obj The new value.
 */
void RegistryRef::Set(const luabind::object &obj)
{
	obj.push(scripting->GetState());
	SetFromStack();
}

/**
 * Pushes the value stored by the reference onto the Lua stack.
 * If there is no value stored, then @c nil is pushed.
 */
void RegistryRef::Push() const
{
	lua_State *L = scripting->GetState();

	if (ref == LUA_NOREF || ref == LUA_REFNIL) {
		lua_pushnil(L);
	}
	else {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}
}

}  // namespace Script
}  // namespace HoverRace
