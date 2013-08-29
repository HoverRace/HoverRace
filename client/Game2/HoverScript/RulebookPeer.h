
// RulebookPeer.h
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

#pragma once

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "../../../engine/Script/Peer.h"
#include "../Rulebook.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
		class Rulebook;
		typedef std::shared_ptr<Rulebook> RulebookPtr;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Enables subclassing Rulebook from Lua.
 * @author Michael Imamura
 */
class RulebookPeer : public Script::Peer {
	typedef Script::Peer SUPER;
	public:
		RulebookPeer(Script::Core *scripting, GameDirector &director);
		virtual ~RulebookPeer();

	public:
		static void Register(Script::Core *scripting);

	private:
		GameDirector &director;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
