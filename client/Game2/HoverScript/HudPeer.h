
// HudPeer.h
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

namespace HoverRace {
	namespace Display {
		class Display;
		class Hud;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Scripting peer for access to the HUD for a player.
 * @author Michael Imamura
 */
class HudPeer : public Script::Peer {
	typedef Script::Peer SUPER;
	public:
		HudPeer(Script::Core *scripting, Display::Display &display,
			std::weak_ptr<Display::Hud> hud);
		virtual ~HudPeer();

	public:
		static void Register(Script::Core *scripting);

	public:
		template<class T>
		std::shared_ptr<T> LAddDecor(int align)
		{
			HudAlignment::type ha;
			try {
				ha = HudAlignment::FromInt(align);
			}
			catch (Exception &ex) {
				luaL_error(GetScripting()->GetState(), "%s", ex.what());
			}

			if (auto sp = hud.lock()) {
				return sp->AddHudChild(ha, new T(display));
			}
			else {
				return std::shared_ptr<T>();
			}
		}

		void LUseRaceDefault();

	private:
		Display::Display &display;
		std::weak_ptr<Display::Hud> hud;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
