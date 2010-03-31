
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

#include <luabind/luabind.hpp>

#include "Core.h"

#include "Peer.h"

namespace HoverRace {
namespace Script {

Peer::Peer(Core *scripting, const std::string &name) :
	scripting(scripting), name(name)
{
}

Peer::~Peer()
{
}

void Peer::Register(Core *scripting)
{
	using namespace luabind;

	lua_State *L = scripting->GetState();

	module(L) [
		class_<Peer>("Object")
			.def("help", (void(Peer::*)())&Peer::LHelp)
			.def("help", (void(Peer::*)(const std::string&))&Peer::LHelp)
	];
}

void Peer::LHelp()
{
	scripting->ReqHelp(name);
}

void Peer::LHelp(const std::string &methodName)
{
	scripting->ReqHelp(name, methodName);
}

}  // namespace Script
}  // namespace HoverRace
