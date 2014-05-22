
// MetaPlayer.h
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../../../engine/Script/Peer.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class PlayerPeer;
		}
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Native base class for scripted players.
 * @author Michael Imamura
 */
class MetaPlayer
{
	protected:
		MetaPlayer(std::shared_ptr<PlayerPeer> player) :
			player(std::move(player)) { }
	public:
		virtual ~MetaPlayer() { }

	public:
		virtual void OnInit() { }

	public:
		static void Register(Script::Core *scripting);

	private:
		std::shared_ptr<PlayerPeer> player;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
