
// RulebookPeer.cpp
//
// Copyright (c) 2013 Michael Imamura.
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

#include "../../../engine/Script/Core.h"
#include "../GameDirector.h"

#include "RulebookPeer.h"

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	class RulebookWrapper : public Rulebook, public luabind::wrap_base
	{
		typedef Rulebook SUPER;
		public:
			RulebookWrapper(const std::string &trackName) :
				SUPER(trackName, 0, 0x7f) { }
			virtual ~RulebookWrapper() { }
	};
}

RulebookPeer::RulebookPeer(Script::Core *scripting, GameDirector &director) :
	SUPER(scripting, "Game"), director(director)
{
}

RulebookPeer::~RulebookPeer()
{
}

/**
 * Register this peer in an environment.
 */
void RulebookPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L) [
		class_<Rulebook, RulebookWrapper, RulebookPtr>("Rulebook")
			.def(constructor<const std::string&>())
			.property("track_name", &Rulebook::GetTrackName)
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
