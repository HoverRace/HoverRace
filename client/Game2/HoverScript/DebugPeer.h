
// DebugPeer.h
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

#include "../../../engine/Script/Handlers.h"
#include "../../../engine/Script/Peer.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Provides access to debugging tools.
 * @author Michael Imamura
 */
class DebugPeer : public Script::Peer {
	typedef Script::Peer SUPER;
	public:
		DebugPeer(Script::Core *scripting, GameDirector &gameDirector);
		virtual ~DebugPeer();

	public:
		static void Register(Script::Core *scripting);

	public:
		void LShowPalette();
		void LStartTestLab();

	private:
		GameDirector &gameDirector;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
